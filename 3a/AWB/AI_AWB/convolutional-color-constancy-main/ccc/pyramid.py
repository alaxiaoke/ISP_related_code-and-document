import numpy as np
from pathlib import Path
import tqdm
import cv2 as cv2
from scipy.signal import convolve2d
import matplotlib.pyplot as plt

from ccc.minimizer import fit_filter
from ccc.dataset import Dataset, AugList, LogChromHist, Resize, CubePlusPlus
from ccc.log_chrominance import illum_uvy2illum_rgb
from pytorch_model import PytorchModel

class GaussianPyramid:
    def __init__(self, num_levels, filter_size=5):
        self.num_levels = num_levels
        self.filter_size = filter_size
        self.filters = None
    
    def fit(self, dataset: Dataset, lambda_regularization=0.1):
        dataset = dataset.copy()
        if isinstance(dataset.augmentations, AugList):
            if not isinstance(dataset.augmentations.augmentations_list[-1], LogChromHist):
                raise TypeError("Last transform in dataset should be LogChromHist")
            u_ticks, v_ticks = dataset.augmentations.augmentations_list[-1].histogram.u_coords, dataset.augmentations.augmentations_list[-1].histogram.v_coords
        elif not isinstance(dataset.augmentations, LogChromHist):
            raise TypeError("Last transform in dataset should be LogChromHist")
        else:
            u_ticks, v_ticks = dataset.augmentations.histogram.u_coords, dataset.augmentations.histogram.v_coords

        orig_transforms = dataset.augmentations
        orig_img_size = dataset[0][0].shape
        # print("orig_img_size", orig_img_size)
        self.filters = []

        pytorch_model = PytorchModel(self.num_levels, orig_img_size[0])
        for i in tqdm.tqdm(range(self.num_levels)):
            dataset.augmentations = AugList([orig_transforms, Resize((orig_img_size[0] // 2 ** i, orig_img_size[1] // 2 ** i))])
            train_imgs, data_gt = dataset.get_data()        #这里数据的处理就结束了，可以直接用网络来学了
            pytorch_model.train_new(train_imgs, data_gt, i)
            print("train_imgs.shape", train_imgs.shape)
            # for j in range(train_imgs.shape[0]):
            #     cv2.imwrite('D:\\code\\data\\ccc_generated_uv\\' + str(j) + '.jpg', train_imgs[j]*(255*255))
            # print("train_imgs[0]", train_imgs[0])
            # break
            # self.filters.append(fit_filter(train_imgs, data_gt, lambda_regularization, self.filter_size, u_ticks, v_ticks).reshape((self.filter_size, self.filter_size)))
        # pytorch_model
        dataset.augmentations = AugList([orig_transforms, Resize((orig_img_size[0], orig_img_size[1]))])
        train_imgs, data_gt = dataset.get_data()
        pytorch_model.pred(train_imgs, data_gt)

        print("Pyramid fitted")

    def apply(self, img, stored_filters=False):
        if self.filters is None and stored_filters is False:
            raise RuntimeError("Filters haven't been fitted yet")
        if stored_filters:
            filters = []
            for i in range(self.num_levels):
                filters.append(np.load('D:\code\convolutional-color-constancy-main/filters/filter_' + str(i) + '.npy'))
                # print("filters[i].shape", filters[i].shape)
        else:
            filters = self.filters
        pyramid = []
        for i in range(self.num_levels):
            pyramid.append(cv2.resize(img, (img.shape[0] // 2 ** i, img.shape[1] // 2 ** i), interpolation=cv2.INTER_LINEAR))
            # print(">>", pyramid[-1].shape)
        result = 0
        for i in range(self.num_levels - 1, 0, -1):
            result += convolve2d(pyramid[i], filters[i], mode='same', boundary='fill', fillvalue=0)
            # print("1 ", result.shape)
            result = cv2.resize(result, (result.shape[0] * 2, result.shape[1] * 2), interpolation=cv2.INTER_LINEAR)
            # print("2 ", result.shape)
            result = cv2.GaussianBlur(result,(3,3),0)
        #     print(self.filters[i])
        #     plt.imshow(self.filters[i])
        #     plt.show()
        # plt.imshow(self.filters[0])
        # plt.show()
        result += convolve2d(pyramid[0], filters[0], mode='same', boundary='fill', fillvalue=0)
        return result 
    
    def save_filters(self, file):
        if self.filters is None:
            raise RuntimeError("Filters haven't been fitted yet")
        file = Path(file)
        for i, f in enumerate(self.filters):
            np.save(file / f'filter_{i}.npy', f)


def test():
    epsilon = 0.0125
    lg = LogChromHist(epsilon, (-64 * epsilon, 64 * epsilon), (-64 * epsilon, 64 * epsilon))
    dataset = CubePlusPlus('/media/kvsoshin/Transcend/Work/cube++/SimpleCube++', 'train', 300, lg)
    gp = GaussianPyramid(6, filter_size=5)
    gp.fit(dataset)
    
    dataset.return_gt_rgb = True
    img, gt = dataset[0]
    # plt.imshow(img)
    # plt.show()
    print(gt, gt / np.linalg.norm(gt))
    print("!", img.shape)
    filtered = gp.apply(img)
    print(filtered.shape)
    plt.imshow(filtered / np.amax(filtered))
    plt.show()
    l_uv = np.array(np.unravel_index(np.argmax(filtered), filtered.shape)) * epsilon
    print("Illum_estim: ", l_uv)
    l_rgb = illum_uvy2illum_rgb([l_uv])
    print(l_rgb)
    gp.save_filters('./.')

if __name__ == '__main__':
    test()
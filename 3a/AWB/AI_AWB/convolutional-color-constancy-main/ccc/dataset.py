import numpy as np
from pathlib import Path
import matplotlib.pyplot as plt
import tqdm
import csv
import cv2 as cv2
from log_chrominance import Histogram, rgb2uvy

class Augmentation:
    def __init__(self):
        pass
    def __call__(self, image):
        raise NotImplementedError

class LogChromHist(Augmentation):
    def __init__(self, epsilon, u_range, v_range):
        super().__init__()
        self.histogram = Histogram(epsilon, u_range, v_range)
    def __call__(self, img, mode='hist'):
        img = img.reshape((-1, 3))
        img = img[~np.all(img == 0, axis=1)]
        img_uvy = rgb2uvy(img)
        if mode == 'hist':
            return self.histogram.get_hist(img_uvy)
        if mode == 'color_hist':
            return self.histogram.get_colorhist(img_uvy, img.reshape((-1, 3)))
        else:
            raise ValueError("Wrong mode")


class Resize(Augmentation):
    def __init__(self, dsize):
        self.dsize = dsize
    def __call__(self, img):
        return cv2.resize(img, self.dsize, interpolation=cv2.INTER_LINEAR)

class AugList(Augmentation):
    def __init__(self, augmentations_list):
        super().__init__()
        self.augmentations_list = augmentations_list
    def __call__(self, img):
        img = img.copy()
        for aug in self.augmentations_list:
            img = aug(img)
        return img
    def append(self, new_transform):
        self.augmentations_list.append(new_transform)



class Dataset:
    def __init__(self, dataset_path, split, size, augmentations, img_size=512, return_gt_rgb=False):
        self.dataset_path = Path(dataset_path)
        self.size= size
        self.augmentations = augmentations
        self.split = split
        self.return_gt_rgb = int(return_gt_rgb)

        if self.size is None:
            self.data_samples = self._data_samples()
        else:
            self.data_samples = self._data_samples()[:self.size]#[img_name for img_name in (self.dataset_path / self.split).glob('*.png')]
        self.gt_csv = self._gt_csv()
        self.image_path = self._image_path()
        self.img_size = img_size

    def _gt_csv(self):
        raise NotImplementedError
    def _image_path(self):
        raise NotImplementedError
    def _data_samples(self):
        # print('data_sample',[img_name for img_name in sorted((self.dataset_path / self.split).glob('*.jpg'))])
        return [img_name for img_name in sorted((self.dataset_path / self.split).glob('*.jpg'))]

    def __getitem__(self, idx):
        # if idx > 0:
        #     idx = 35
        img_name = self.data_samples[idx]
        img = cv2.imread(str(img_name))
        img = cv2.resize(img, (1024, 1024))
        img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB).astype(np.float64)
        # img = img/255
        # print("__getitem__ img.shape", img.shape)

        print("img_name,img_name.stem", img_name, img_name.stem)
        if self.return_gt_rgb:
            if self.augmentations is None:
                return img, np.array(self.gt_csv[img_name.stem])
            else:
                return self.augmentations(img), np.array(self.gt_csv[img_name.stem])
        if self.augmentations is None:
            return img, rgb2uvy([self.gt_csv[img_name.stem]])[0, :2]
        else:
            return self.augmentations(img), rgb2uvy([self.gt_csv[img_name.stem]])[0, :2]

    def __len__(self):
        return len(self.data_samples)

    def get_data(self):
        data = []
        gt_data = []
        for idx in tqdm.tqdm(range(self.__len__())):
            img, gt = self.__getitem__(idx)
            data.append(img)
            gt_data.append(gt)
        return np.array(data), np.array(gt_data)


class CubePlusPlus(Dataset):
    def __init__(self, dataset_path, split, size, augmentations, img_size=512, return_gt_rgb=False):
        super().__init__(dataset_path, split, size, augmentations,img_size, return_gt_rgb)
    def copy(self):
        return CubePlusPlus(self.dataset_path, self.split, self.size, self.augmentations, self.return_gt_rgb)
    def _gt_csv(self):
        mydict = {}
        with open(self.dataset_path / self.split / 'gt.txt', mode='r') as infile:
            all_lines = infile.readlines()
            for i in range(len(all_lines)):
                line1 = all_lines[i].split(" ")
                Ggain = 1/float(line1[1])
                Bgain = (float(line1[2])*Ggain)
                Rgain = (float(line1[0])*Ggain)
                pic_name = str(i+1)
                mydict[pic_name] = [Rgain, 1.0, Bgain]
                # print(mydict[pic_name])
            # mydict = {rows[0]:[float(rows[1]), float(rows[2]), float(rows[3])] for rows in reader if rows[0] != 'image'}
        return mydict

    def _data_samples(self):
        # print('data_sample', [img_name for img_name in sorted((self.dataset_path / self.split).glob('*.jpg'))])
        return [img_name for img_name in sorted((self.dataset_path / self.split).glob('*.jpg'))]
    def _image_path(self):
        return self.dataset_path / self.split / 'PNG'

def test():
    # hist = Histogram(0.0125, (-256 * 0.025, 256 * 0.025), (-256 * 0.025, 256 * 0.025))
    lg = LogChromHist(0.0125, (-64 * 0.025, 64 * 0.025), (-64 * 0.025, 64 * 0.025))
    d = CubePlusPlus('/media/kvsoshin/Transcend/Work/cube++/SimpleCube++', 'train', 20, lg)
    print(d[0][0])
    plt.imshow(d[0][0])
    plt.show()
    X, y = d.get_data()
    print(X.shape, y.shape)
if __name__ == '__main__':
    test()

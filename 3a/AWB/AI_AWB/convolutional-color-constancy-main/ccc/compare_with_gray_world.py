import numpy as np
import tqdm
from ccc.dataset import Dataset, AugList, LogChromHist, Resize, CubePlusPlus
from ccc.log_chrominance import illum_uvy2illum_rgb
from ccc.pyramid import GaussianPyramid
import cv2 as cv2

def cmp_ccc_and_gray_world(gp, test_dataset, test_dataset_raw_photos):
    assert len(test_dataset) == len(test_dataset_raw_photos)
    err_gw = []
    err_barron = []
    for i in tqdm.tqdm(range(len(test_dataset))):
        hist, gt_uv = test_dataset[i]
        img, gt_rgb_raw = test_dataset_raw_photos[i]
        # assert np.allclose(gt_uv, gt_rgb_raw)

        filtered = gp.apply(hist, stored_filters=False)
        tint_index = np.unravel_index(np.argmax(filtered), filtered.shape)
        print("np.argmax(filtered)", np.argmax(filtered))
        print("tint_index", tint_index)

        l_uv = np.array(np.unravel_index(np.argmax(filtered), filtered.shape)) * test_dataset.augmentations.histogram.epsilon
        print(i, "predict uv:", l_uv, "ground-truth uv:", np.round(gt_uv, 3))
        l_rgb = illum_uvy2illum_rgb([l_uv])
        l_rgb_norm = l_rgb / np.sum(l_rgb)
        l_rgb_gt = illum_uvy2illum_rgb([gt_uv])
        l_rgb_norm_gt = l_rgb_gt / np.sum(l_rgb_gt)
        l_rgb_gray_world = np.mean(img, axis=(0, 1))
        l_rgb_gray_world = l_rgb_gray_world / np.sum(l_rgb_gray_world)
        print(f"Gray world: {np.round(l_rgb_gray_world, 3)}; predict rgb: {np.round(l_rgb_norm, 3)}; ground-truth rgb: {np.round(l_rgb_norm_gt, 3)}")
        imgTmp = test_dataset.data_samples[i]
        img = cv2.imread(str(imgTmp))
        l_rgb_norm = l_rgb_norm[0]
        imgNew = np.zeros(img.shape, dtype=np.uint16)
        Bgain = float(l_rgb_norm[2]) * (1 / float(l_rgb_norm[1]))
        Rgain = float(l_rgb_norm[0]) * (1 / float(l_rgb_norm[1]))
        imgNew[:, :, 0] = img[:, :, 0] / Bgain
        imgNew[:, :, 1] = img[:, :, 1]
        imgNew[:, :, 2] = img[:, :, 2] / Rgain
        imgNew = np.clip(imgNew, 0, 255)
        imgNew = imgNew.astype('uint8')

        cv2.imwrite('D:\\code\\data\\ccc_generated_JPG\\' + str(imgTmp.stem) + '.jpg', imgNew)

        l_rgb_norm_gt = l_rgb_norm_gt[0]
        imgNew = np.zeros(img.shape, dtype=np.uint16)
        Bgain = float(l_rgb_norm_gt[2]) * (1 / float(l_rgb_norm_gt[1]))
        Rgain = float(l_rgb_norm_gt[0]) * (1 / float(l_rgb_norm_gt[1]))
        imgNew[:, :, 0] = img[:, :, 0] / Bgain
        imgNew[:, :, 1] = img[:, :, 1]
        imgNew[:, :, 2] = img[:, :, 2] / Rgain
        imgNew = np.clip(imgNew, 0, 255)
        imgNew = imgNew.astype('uint8')

        cv2.imwrite('D:\\code\\data\\ccc_generated_JPG\\' + str(imgTmp.stem) + '_gt.jpg', imgNew)
    #     err_gw.append(np.linalg.norm(l_rgb_gray_world - gt_uv))
    #     err_barron.append(np.linalg.norm(l_rgb_as_in_gt - gt_uv))
    # return np.array(err_gw), np.array(err_barron)


def main():
    epsilon = 1/64
    # lg = LogChromHist(epsilon, (-32 * epsilon, 32 * epsilon), (-32 * epsilon, 32 * epsilon))
    lg = LogChromHist(epsilon, (0,  64 * epsilon), (0, 64 * epsilon))
    train_dataset = CubePlusPlus('D:\code\data', 'deAWB_data', 40, lg, 1024)
    gp = GaussianPyramid(4, filter_size=6)
    gp.fit(train_dataset)
    # gp.save_filters('D:\code\convolutional-color-constancy-main/filters')

    test_dataset = CubePlusPlus('D:\code\data', 'deAWB_data', 40, lg, return_gt_rgb=False)
    test_dataset_raw_photos = CubePlusPlus('D:\code\data', 'deAWB_data', 40, None, return_gt_rgb=True)

    cmp_ccc_and_gray_world(gp, test_dataset, test_dataset_raw_photos)
    #
    # print("Gray-world mean error: ", err_gw.mean())
    # print("Barron algorithm mean error: ", err_barron.mean())


if __name__ == '__main__':
    main()
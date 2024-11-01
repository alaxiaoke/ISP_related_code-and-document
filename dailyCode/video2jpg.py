import numpy as np
import cv2
import os
import sys


def cut(video_file, target_dir):
    cap = cv2.VideoCapture(video_file)  # 获取到一个视频
    isOpened = cap.isOpened  # 判断是否打开
    # 为单张视频，以视频名称所谓文件名，创建文件夹
    temp = os.path.split(video_file)[-1]
    dir_name = temp.split('.')[0]

    single_pic_store_dir = os.path.join(target_dir, dir_name)
    if not os.path.exists(single_pic_store_dir):
        os.makedirs(single_pic_store_dir)


    i = 0
    while isOpened:

        i += 1
        # if  i>10: break
        (flag, frame) = cap.read()  # 读取一张图像

        fileName = 'image' + str(i) + ".jpg"
        if (flag == True):
            # 以下三行 进行 旋转
            #frame = np.rot90(frame, -1)
            #print(fileName)
            # 设置保存路径
            save_path = os.path.join(single_pic_store_dir, fileName)
            #print(save_path)
            res = cv2.imwrite(save_path, frame, [cv2.IMWRITE_JPEG_QUALITY, 90])
            #print(res)
        else:
            break

    return single_pic_store_dir

if __name__ == '__main__':
    video_file = r'C:\Users\xlluo\Desktop\AEDEMO\wdr\window_sunset - Trim - Trim.mp4'
    cut(video_file, r'D:\work\AE\data\demo\wdr')

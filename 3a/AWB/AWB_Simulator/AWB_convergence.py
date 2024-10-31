import cv2
import numpy as np
from moviepy.editor import *


class AWBProcess( ):
    def __init__(self):
        self.algo_num_left = 0
        self.algo_params_left = {}
        self.algo_num_right = 0
        self.algo_params_right = {}
        self.input_path = {}
        self.AWB_sequence_left = {}
        self.AWB_sequence_right = {}
        self.img_width = 512
        self.img_height = 512
        self.num_frames = 120
        self.fps = 60
        self.current_index = 0
        self.current_cam = "left"
        self.communication_way = 0

    def awbGainSequence_Algo1(self, gt_path, left_or_right, algo_params, img_path):
        AWB_sequence = {}
        with open(gt_path, mode='r') as infile:
            lines = infile.readlines()
            self.num_frames = len(lines)
            print("self.num_frames, len(lines)", self.num_frames, len(lines))
            for i in range(self.num_frames):
                print(i)
                line1 = lines[i].split(" ")
                # print(line1)
                Bgain = float(line1[2])
                Rgain = float(line1[1])
                IMG = float(line1[0])
                pic_name = str(i + 1)
                AWB_sequence[pic_name] = [Rgain, Bgain]

        frame_num = len(AWB_sequence)
        # first = AWB_sequence[str(1)]

        frame_speed = float(algo_params[0]) * 10
        print("frame_num, frame_speed,left_or_right::", frame_num, frame_speed, left_or_right)

        for i in range(1, self.num_frames):
            if left_or_right == "left":
                self.AWB_sequence_left[i] = AWB_sequence[str(i)]

            else:
                self.AWB_sequence_right[i] = AWB_sequence[str(i)]


        if left_or_right == "left":
            return self.AWB_sequence_left
        else:
            return self.AWB_sequence_right

    def awbGainSequence_Algo2(self, gt_path, left_or_right, algo_params, img_path):
        AWB_sequence = {}

        with open(gt_path, mode='r') as infile:
            all_lines = infile.readlines()
            for i in range(len(all_lines)):
                line1 = all_lines[i].split(" ")
                # print(line1)
                G_tmp = float(line1[1])
                Bgain = G_tmp/float(line1[2])
                Ggain = 1
                Rgain = G_tmp/float(line1[0])
                pic_name = str(i + 1)
                AWB_sequence[pic_name] = [Rgain, Ggain, Bgain]

        frame_num = len(AWB_sequence)
        # first = AWB_sequence[str(1)]
        first = [0.8, 1.0, 1.2]
        second = [first[0]*1.4, first[1], first[2]*0.7]

        frame_speed = float(algo_params[0]) * 10
        print("frame_num,first, second, frame_speed,left_or_right::", frame_num, first, second, frame_speed, left_or_right)
        delta_R = (second[0] - first[0]) / frame_speed
        delta_B = (second[2] - first[2]) / frame_speed
        for i in range(1, self.num_frames):
            if i < 70:
                self.AWB_sequence_left[i] = first
                self.AWB_sequence_right[i] = first
                continue
            if left_or_right == "left":
                i = i - 70
                if i <= frame_speed:
                    self.AWB_sequence_left[i+70] = [first[0]+i*delta_R, first[1], first[2]+i*delta_B]
                    # print("self.AWB_sequence_left[i]", self.AWB_sequence_left[i])
                else:
                    self.AWB_sequence_left[i+70] = second
            # for i in range(1, frame_num):
            #     print(i, self.AWB_sequence_left[i])
            else:
                i = i - 70
                if i <= frame_speed:
                    self.AWB_sequence_right[i+70] = [first[0] + i * delta_R, first[1], first[2] + i * delta_B]
                else:
                    self.AWB_sequence_right[i+70] = second
                # for i in range(1, frame_num):
                # print(i, self.AWB_sequence_right[i])

        if left_or_right == "left":
            return self.AWB_sequence_left
        else:
            return self.AWB_sequence_right

    def awbGainSequence(self, gt_path, left_or_right, algo_num, algo_params, img_path):
        if algo_num == 0:
            return self.awbGainSequence_Algo1(gt_path, left_or_right, algo_params, img_path)
        if algo_num == 1:
            return self.awbGainSequence_Algo2(gt_path, left_or_right, algo_params, img_path)
        if algo_num == 2:
            return self.awbGainSequence_Algo2(gt_path, left_or_right, algo_params, img_path)

    def adjust_gamma(self, image, gamma=2.2):
        invGamma = 1.0 / gamma
        table = []
        for i in range(256):
            table.append(((i / 255.0) ** invGamma) * 255)
        table = np.array(table).astype("uint8")
        return cv2.LUT(image, table)

    def images_to_video(self, left_or_right):
        img_array = []
        if left_or_right == "left":
            gt_path = self.input_path[1] + "/gt.txt"
            img_path = self.input_path[0]
            AWB_sequence = self.AWB_sequence_left
            algo_params = self.algo_params_left
            algo_num = self.algo_num_left
        else:
            gt_path = self.input_path[3] + "/gt.txt"
            img_path = self.input_path[2]
            AWB_sequence = self.AWB_sequence_right
            algo_params = self.algo_params_right
            algo_num = self.algo_num_right

        if self.communication_way == 0:
            gt_path = self.input_path[1] + "/gt.txt"        #主从模式 都用左边的AWB结果

        print("计算参数：", gt_path, img_path, AWB_sequence, algo_params, algo_num)
        AWB_sequence_smooth = self.awbGainSequence(gt_path, left_or_right, algo_num, algo_params, img_path)

        imgListA = os.listdir(img_path)
        imgList = [int(i.split(".")[0]) for i in imgListA]
        imgList.sort()

        self.current_cam = left_or_right
        for i in range(1, len(imgList)):
            filename = img_path + "/" + str(imgList[i]) + ".jpg"
            img = cv2.imread(filename)
            img = cv2.resize(img, (self.img_width, self.img_height))
            imgNew = np.zeros((self.img_width, self.img_height, 3), dtype=np.uint16)
            imgNew[:, :, 0] = img[:, :, 0] * AWB_sequence_smooth[i][1]
            imgNew[:, :, 1] = img[:, :, 1]
            imgNew[:, :, 2] = img[:, :, 2] * AWB_sequence_smooth[i][0]
            imgNew = np.clip(imgNew, 0, 255)
            imgNew = imgNew.astype('uint8')
            imgNew = self.adjust_gamma(imgNew, 2)
            if img is None:
                print(filename + " is non-existent!")
                continue
            # self.current_index = i
            print("生成中：", i, AWB_sequence_smooth[i][0], AWB_sequence_smooth[i][1])
            img_array.append(imgNew)

        mp4_name = "demo_" + str(left_or_right) + ".mp4"
        out = cv2.VideoWriter(mp4_name, cv2.VideoWriter_fourcc(*'mp4v'), self.fps, (self.img_width, self.img_height))
        for i in range(len(img_array)):
            out.write(img_array[i])
        out.release()

        return AWB_sequence_smooth


    def AWB_Calculate(self, img):
        Rmean = np.mean(img[:, :, 2])
        Gmean = np.mean(img[:, :, 1])
        Bmean = np.mean(img[:, :, 0])
        Rgain = Gmean / Rmean
        Bgain = Gmean / Bmean
        print('rgain, bgain', Rgain, Bgain)
        return Rgain, Bgain

   # def combine(self):
    #     clips = [VideoFileClip(r"D:\code\dailyCode\demo_5.mp4"),
    #              VideoFileClip(r"D:\code\dailyCode\demo_10.mp4")]
    #     video = clips_array([clips])
    #     video.write_videofile(r'D:\code\dailyCode\result.mp4')

    def view(self):
        ##选择需要同时播放的两个视频路径
        videoLeftUp = cv2.VideoCapture(r".\demo_left.mp4")
        videoRightUp = cv2.VideoCapture(r".\demo_right.mp4")

        width = 512
        height = 512

        while(videoLeftUp.isOpened()):
            retLeftUp, frameLeftUp = videoLeftUp.read()
            retRightUp, frameRightUp = videoRightUp.read()

            # 将两个视频窗口左右进行拼接
            frameUp = np.hstack((frameLeftUp, frameRightUp))

            cv2.imshow('frame', frameUp)
            # 设置等待时间(ms),在每一个等待时间中处理当前帧
            key = cv2.waitKey(3)
            if int(key) == 113:
                break

        videoLeftUp.release()
        videoRightUp.release()


def main():
    SDS = AWBProcess()
    SDS.images_to_video("right")

if __name__ == "__main__":
    main()

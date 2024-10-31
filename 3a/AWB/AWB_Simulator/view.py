import time
import tkinter as tk
from tkinter import *
from tkinter.messagebox import *
import tkinter.messagebox
import tkinter.filedialog
from tkinter.scrolledtext import ScrolledText
import cv2
import numpy as np
from AWB_convergence import AWBProcess
from threading import Thread
from time import sleep



class InputFrame(object):  # 继承Frame类
    def __init__(self, processor=AWBProcess(), master=None, page=None):
        # Frame.__init__(self, master)
        self.root = master  # 定义内部变量root
        self.page = page
        self.processor = processor

        # Label(self.page, text='左侧图像目录: ').grid(row=0,  pady=10, stick=W)
        # Entry(self.page, textvariable=self.importPrice).grid(row=2, column=1, stick=E)
        self.button1 = Button(self.page, text="选择左侧图像目录", command=self.button1_click).grid(row=0, column=0, stick=W)

        # Label(self.page, text='左侧相机单帧AWB结果: ').grid(row=1, pady=10, stick=W)
        self.button2 = Button(self.page, text="选择左侧相机单帧AWB结果", command=self.button2_click).grid(row=1, column=0, stick=W)

        # Label(self.page, text='右侧图像目录: ').grid(row=2, pady=10, stick=W)
        self.button3 = Button(self.page, text="选择右侧图像目录", command=self.button3_click).grid(row=2, column=0, stick=W)

        # Label(self.page, text='右侧相机单帧AWB结果: ').grid(row=3, pady=10, stick=W)
        self.button4 = Button(self.page, text="选择右侧相机单帧AWB结果", command=self.button4_click).grid(row=3, column=0, stick=W)

        Button(self.page, text='确认', command=self.printData).grid(row=4, column=0, stick=W, pady=10)
        Button(self.page, text='重选', command=self.reload).grid(row=5, column=0, stick=W, pady=10)

        self.text = ScrolledText(self.page)
        self.text.configure(state=tkinter.DISABLED)
        self.text.grid(row=6, column=0, columnspan=1, stick=W, pady=0)

    # 追加日志
    def append_log(self, str):
        self.text.configure(state=tkinter.NORMAL)
        final_str = str + "\n"
        self.text.insert(tk.END, final_str)
        self.text.see(tk.END)
        self.text.configure(state=tkinter.DISABLED)

    def clean_log(self):
        self.text.configure(state = tkinter.NORMAL)
        self.text.delete(0.0, tk.END)
        self.text.configure(state = tkinter.DISABLED)

    def button1_click(self):
        dirname = tk.filedialog.askdirectory()  # 返回文件路径
        if dirname:
            self.append_log("左侧图像目录：" + dirname)
            self.processor.input_path[0] = dirname


    def button2_click(self):
        dirname = tk.filedialog.askdirectory()  # 返回文件名
        if dirname:
            self.append_log("左侧相机单帧AWB结果目录：" + dirname)
            self.processor.input_path[1] = dirname

    def button3_click(self):
        dirname = tk.filedialog.askdirectory()  # 返回文件名
        if dirname:
            self.append_log("右侧图像目录：" + dirname)
            self.processor.input_path[2] = dirname

    def button4_click(self):
        dirname = tk.filedialog.askdirectory()  # 返回文件名
        if dirname:
            self.append_log("右侧相机单帧AWB结果目录：" + dirname)
            self.processor.input_path[3] = dirname

    def printData(self):
        b = tk.messagebox.askokcancel("提示", "请确认数据无误")
        if b:
            self.append_log("确认数据无误")

    def reload(self):
        self.clean_log()

class OutfitFrame(object):  # 继承Frame类
    def __init__(self, master=None,page=None):
        # Frame.__init__(self, master)
        self.root = master  # 定义内部变量root
        self.page = page

        Label(self.page, text='1.进行收敛算法选择').grid(row=1, stick=W, pady=10)
        Label(self.page, text='2.调整算法的相关参数').grid(row=2, stick=W, pady=10)
        Label(self.page, text='3.点击“输入帧数据”输入图像相关数据').grid(row=3, stick=W, pady=10)
        Label(self.page, text='4.点击“对比双目结果 ').grid(row=4, stick=W, pady=10)



class Algo1FrameL(object):  # 继承Frame类
    def __init__(self, processor=AWBProcess(), master=None, page=None):
        self.root = master  # 定义内部变量root
        self.page = page
        self.processor = processor
        self.createPage()

    def createPage(self):

        Label(self.page, text='左侧相机 算法1参数').grid(padx=20,pady=10)
        Label(self.page).grid(row=0, stick=W, pady=10)
        Label(self.page, text='收敛速度: ').grid(row=1, stick=W, pady=10)
        Label(self.page, text='平滑程度: ').grid(row=2, stick=W, pady=10)
        Label(self.page, text='触发阈值: ').grid(row=3, stick=W, pady=10)
        Label(self.page, text='降速收敛参数: ').grid(row=4, stick=W, pady=10)
        # self.input1 = Entry(self.page, textvariable=self.itemName)
        self.input1 = Entry(self.page)
        self.input1.grid(row=1, column=1, stick=E)
        self.input2 = Entry(self.page)
        self.input2.grid(row=2, column=1, stick=E)
        self.input3 = Entry(self.page)
        self.input3.grid(row=3, column=1, stick=E)
        self.input4 = Entry(self.page)
        self.input4.grid(row=4, column=1, stick=E)
        Button(self.page, text='确认', command=self.getData).grid(row=6, column=1, stick=E, pady=10)

    def getData(self):
        self.processor.algo_params_left[0] = self.input1.get()
        self.processor.algo_num_left = 0
        print("input_path", self.processor.algo_params_left[0])


class Algo2FrameL(object):  # 继承Frame类
    def __init__(self, processor=AWBProcess(), master=None, page=None):
        self.root = master  # 定义内部变量root
        self.page = page
        self.processor = processor
        self.createPage()

    def createPage(self):
        Label(self.page, text='左侧相机 算法2参数').grid(padx=20, pady=10)
        Label(self.page).grid(row=0, stick=W, pady=10)
        Label(self.page, text='收敛速度: ').grid(row=2, stick=W, pady=10)
        Label(self.page, text='延迟收敛阈值: ').grid(row=3, stick=W, pady=10)
        # self.input1 = Entry(self.page, textvariable=self.itemName)
        self.input1 = Entry(self.page)
        self.input1.grid(row=2, column=1, stick=E)
        self.input2 = Entry(self.page)
        self.input2.grid(row=3, column=1, stick=E)

        Button(self.page, text='确认', command=self.getData).grid(row=5, column=1, stick=E, pady=10)

    def getData(self):
        self.processor.algo_params_left[0] = self.input1
        self.processor.algo_params_left[1] = self.input2
        self.processor.algo_num_left = 1
        print("input_path", self.processor.algo_params_left[1])


class Algo3FrameL(object):  # 继承Frame类
    def __init__(self, processor=AWBProcess(), master=None, page=None):
        self.root = master  # 定义内部变量root
        self.page = page
        self.processor = processor
        self.createPage()

    def createPage(self):

        Label(self.page, text='左侧相机 算法3参数').grid(padx=20,pady=10)
        Label(self.page).grid(row=0, stick=W, pady=10)
        Label(self.page, text='xxxxxxxx: ').grid(row=1, stick=W, pady=10)
        Label(self.page, text='xxxxxxxx: ').grid(row=2, stick=W, pady=10)
        # self.input1 = Entry(self.page, textvariable=self.itemName)
        self.input1 = Entry(self.page)
        self.input1.grid(row=1, column=1, stick=E)
        self.input2 = Entry(self.page)
        self.input2.grid(row=2, column=1, stick=E)
        Button(self.page, text='确认', command=self.getData).grid(row=6, column=1, stick=E, pady=10)

    def getData(self):
        self.processor.algo_params_left[0] = self.input1.get()
        self.processor.algo_params_left[1] = self.input2.get()
        self.processor.algo_num_left = 2
        print("input_path", self.processor.algo_params_left[1])


class Algo1FrameR(object):  # 继承Frame类
    def __init__(self, processor=AWBProcess(), master=None, page=None):
        self.root = master  # 定义内部变量root
        self.page = page
        self.processor = processor
        self.createPage()

    def createPage(self):

        Label(self.page, text='右侧相机 算法1参数').grid(padx=20,pady=10)
        Label(self.page).grid(row=0, stick=W, pady=10)
        Label(self.page, text='收敛速度: ').grid(row=1, stick=W, pady=10)
        Label(self.page, text='平滑程度: ').grid(row=2, stick=W, pady=10)
        Label(self.page, text='触发阈值: ').grid(row=3, stick=W, pady=10)
        Label(self.page, text='降速收敛参数: ').grid(row=4, stick=W, pady=10)
        # self.input1 = Entry(self.page, textvariable=self.itemName)
        self.input1 = Entry(self.page)
        self.input1.grid(row=1, column=1, stick=E)
        self.input2 = Entry(self.page)
        self.input2.grid(row=2, column=1, stick=E)
        self.input3 = Entry(self.page)
        self.input3.grid(row=3, column=1, stick=E)
        self.input4 = Entry(self.page)
        self.input4.grid(row=4, column=1, stick=E)
        Button(self.page, text='确认', command=self.getData).grid(row=6, column=1, stick=E, pady=10)

    def getData(self):
        self.processor.algo_params_right[0] = self.input1.get()
        self.processor.algo_num_left = 0
        print("input_path", self.processor.algo_params_right[0])


class Algo2FrameR(object):  # 继承Frame类
    def __init__(self, processor=AWBProcess(), master=None, page=None):
        self.root = master  # 定义内部变量root
        self.page = page
        self.processor = processor
        self.createPage()

    def createPage(self):
        Label(self.page, text='右侧相机 算法2参数').grid(padx=20, pady=10)
        Label(self.page).grid(row=0, stick=W, pady=10)
        Label(self.page, text='收敛速度: ').grid(row=2, stick=W, pady=10)
        Label(self.page, text='延迟收敛阈值: ').grid(row=3, stick=W, pady=10)
        # self.input1 = Entry(self.page, textvariable=self.itemName)
        self.input1 = Entry(self.page)
        self.input1.grid(row=2, column=1, stick=E)
        self.input2 = Entry(self.page)
        self.input2.grid(row=3, column=1, stick=E)
        Button(self.page, text='确认', command=self.getData).grid(row=5, column=1, stick=E, pady=10)

    def getData(self):
        self.processor.algo_params_right[0] = self.input1.get()
        self.processor.algo_params_right[1] = self.input2.get()
        self.processor.algo_num_left = 1
        print("input_path", self.processor.algo_params_right[1])

class Algo3FrameR(object):  # 继承Frame类
    def __init__(self, processor=AWBProcess(), master=None, page=None):
        self.root = master  # 定义内部变量root
        self.page = page
        self.processor = processor
        self.createPage()

    def createPage(self):

        Label(self.page, text='右侧相机 算法3参数').grid(padx=20,pady=10)
        Label(self.page).grid(row=0, stick=W, pady=10)
        Label(self.page, text='xxxxxxxx: ').grid(row=1, stick=W, pady=10)
        Label(self.page, text='xxxxxxxx: ').grid(row=2, stick=W, pady=10)
        # self.input1 = Entry(self.page, textvariable=self.itemName)
        self.input1 = Entry(self.page)
        self.input1.grid(row=1, column=1, stick=E)
        self.input2 = Entry(self.page)
        self.input2.grid(row=2, column=1, stick=E)
        Button(self.page, text='确认', command=self.getData).grid(row=6, column=1, stick=E, pady=10)

    def getData(self):
        self.processor.algo_params_right[0] = self.input1.get()
        self.processor.algo_params_right[1] = self.input2.get()
        self.processor.algo_num_left = 2
        print("input_path", self.processor.algo_params_right[1])


class CountFrame(object):  # 继承Frame类
    def __init__(self, processor=AWBProcess(), master=None,page=None):
        # Frame.__init__(self, master)
        self.root = master  # 定义内部变量root
        self.page = page
        self.processor = processor

        OptionList = [
            "左master 右slave",
            "左右独立计算",
            "左右融合后计算"
        ]
        variable = tk.StringVar(self.page)
        variable.set("选择双目通信方式")
        self.opt = tk.OptionMenu(self.page, variable, *OptionList, command=self.communication_way)
        self.opt.config(width=20)
        self.opt.grid(row=0, column=1, pady=10)

        self.button1 = Button(self.page, text="计算收敛过程并合成对比视频", command=self.button1_click).grid(row=1, column=1, pady=10)
        self.button2 = Button(self.page, text="展示对比视频", command=self.button2_click).grid(row=2, column=1, pady=10)

        self.text = ScrolledText(self.page)
        self.text.configure(state=tkinter.DISABLED)
        self.text.grid(row=8, column=1, columnspan=1, stick=W, pady=0)



    # 追加日志
    def append_log(self, str):
        self.text.configure(state=tkinter.NORMAL)
        final_str = str + "\n"
        self.text.insert(tk.END, final_str)
        self.text.see(tk.END)
        self.text.configure(state=tkinter.DISABLED)

    def clean_log(self):
        self.text.configure(state = tkinter.NORMAL)
        self.text.delete(0.0, tk.END)
        self.text.configure(state = tkinter.DISABLED)

    def communication_way(self, value):
        print(value)
        if value == "左master 右slave":
            self.processor.communication_way = 0
        if value == "左右独立计算":
            self.processor.communication_way = 1
        if value == "左右融合后计算":
            self.processor.communication_way = 2



    def button1_click(self):
        self.clean_log()
        tk.messagebox.askokcancel("提示", "计算需要较长时间，请耐心等待一下")
        self.processor.images_to_video("left")
        self.processor.images_to_video("right")
        self.clean_log()
        for i in range(self.processor.num_frames-1):
            i = i + 1
            self.append_log("左侧相机：第" + str(i) + "帧 (" + str(round(self.processor.AWB_sequence_left[i][0], 2)) + "," + str(round(self.processor.AWB_sequence_left[i][1], 2)) + "), "
                            + "右侧相机：第" + str(i) + "帧 (" + str(round(self.processor.AWB_sequence_right[i][0], 2)) + "," + str(round(self.processor.AWB_sequence_right[i][1], 2)) + ")")

    def button2_click(self):
        ##选择需要同时播放的两个视频路径
        videoLeftUp = cv2.VideoCapture(r".\demo_left.mp4")
        videoRightUp = cv2.VideoCapture(r".\demo_right.mp4")

        while (videoLeftUp.isOpened()):
            retLeftUp, frameLeftUp = videoLeftUp.read()
            retRightUp, frameRightUp = videoRightUp.read()

            # frameLeftUp = cv2.resize(frameLeftUp, (int(width), int(height)), interpolation=cv2.INTER_CUBIC)
            # frameRightUp = cv2.resize(frameRightUp, (int(width), int(height)), interpolation=cv2.INTER_CUBIC)

            # 将两个视频窗口左右进行拼接
            frameUp = np.hstack((frameLeftUp, frameRightUp))

            # 将两个视频窗口上下进行拼接
            # frameUp = np.hstack((frameLeftUp, frameRightUp))
            # frameDown = np.hstack((frameLeftDown, frameRightDown))
            # frame = np.vstack((frameUp, frameDown))

            cv2.imshow('frame', frameUp)
            # 设置等待时间(ms),在每一个等待时间中处理当前帧
            key = cv2.waitKey(3)
            if key == 32:  # 如果按下空格键
                cv2.waitKey(0)  # 不刷新图像，实现暂停效果
                continue
            if int(key) == 113:
                break

        videoLeftUp.release()
        videoRightUp.release()

    def reload(self):
        self.clean_log()


class AboutFrame(object):  # 继承Frame类
    def __init__(self, master=None,page=None):
        self.root = master  # 定义内部变量root
        self.page = page
        self.createPage()

    def createPage(self):
        Label(self.page, text='AWB simulator').grid(padx=20, pady=10)
        Label(self.page).grid(row=0, stick=W, pady=10)
        Label(self.page, text='主要用于仿真双目的颜色一致性和AWB收敛策略 ').grid(row=1, stick=W, pady=10)


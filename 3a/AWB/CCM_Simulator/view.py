import time
import tkinter as tk
from tkinter import *
from tkinter.messagebox import *
import tkinter.messagebox
import tkinter.filedialog
from tkinter.scrolledtext import ScrolledText
import cv2
import numpy as np
from CCMcalculator import CCM_IMGProcess
from threading import Thread
import os
from PIL import Image, ImageTk


class InputFrame(object):  # 继承Frame类
    def __init__(self, processor, master=None, page=None):
        # Frame.__init__(self, master)
        self.root = master  # 定义内部变量root
        self.page = page
        self.processor = processor

        # Entry(self.page, textvariable=self.importPrice).grid(row=2, column=1, stick=E)
        self.button1 = Button(self.page, text="选择左侧图像目录", command=self.button1_click).grid(row=0, column=0, stick=W)

        self.button2 = Button(self.page, text="加载\更新ccm", command=self.button2_click).grid(row=7, column=3, stick=W)

        # Label(self.page, text='右侧图像目录: ').grid(row=2, pady=10, stick=W)
        self.button3 = Button(self.page, text="选择右侧图像目录", command=self.button3_click).grid(row=2, column=0, stick=W)

        self.text1 = Text(self.page, height=2, width=5)
        self.text1.grid(row=3, column=0, stick=E, pady=0)

        self.text2 = Text(self.page, height=2, width=5)
        self.text2.grid(row=3, column=1, stick=E, pady=0)

        self.text3 = Text(self.page, height=2, width=5)
        self.text3.grid(row=3, column=2, stick=E, pady=0)

        self.text4 = Text(self.page, height=2, width=5)
        self.text4.grid(row=4, column=0, stick=E, pady=0)

        self.text5 = Text(self.page, height=2, width=5)
        self.text5.grid(row=4, column=1, stick=E, pady=0)

        self.text6 = Text(self.page, height=2, width=5)
        self.text6.grid(row=4, column=2, stick=E, pady=0)

        self.text7 = Text(self.page, height=2, width=5)
        self.text7.grid(row=5, column=0, stick=E, pady=0)

        self.text8 = Text(self.page, height=2, width=5)
        self.text8.grid(row=5, column=1, stick=E, pady=0)

        self.text9 = Text(self.page, height=2, width=5)
        self.text9.grid(row=5, column=2, stick=E, pady=0)

        Button(self.page, text='确认', command=self.printData).grid(row=1, column=3, stick=W)


    def button1_click(self):
        dirname = tk.filedialog.askdirectory()  # 返回文件路径
        if dirname:
            self.processor.input_path[0] = dirname

    def button2_click(self):

        self.text1.delete(0.0, END)
        self.text2.delete(0.0, END)
        self.text3.delete(0.0, END)
        self.text4.delete(0.0, END)
        self.text5.delete(0.0, END)
        self.text6.delete(0.0, END)
        self.text7.delete(0.0, END)
        self.text8.delete(0.0, END)
        self.text9.delete(0.0, END)

        self.text1.insert(INSERT, round(self.processor.ccm[0][0], 2))
        self.text2.insert(INSERT, round(self.processor.ccm[0][1], 2))
        self.text3.insert(INSERT, round(self.processor.ccm[0][2], 2))
        self.text4.insert(INSERT, round(self.processor.ccm[1][0], 2))
        self.text5.insert(INSERT, round(self.processor.ccm[1][1], 2))
        self.text6.insert(INSERT, round(self.processor.ccm[1][2], 2))
        self.text7.insert(INSERT, round(self.processor.ccm[2][0], 2))
        self.text8.insert(INSERT, round(self.processor.ccm[2][1], 2))
        self.text9.insert(INSERT, round(self.processor.ccm[2][2], 2))
        # print("self.processor.ccm", self.processor.ccm)

    def button3_click(self):
        dirname = tk.filedialog.askdirectory()  # 返回文件名
        if dirname:
            self.processor.input_path[1] = dirname

    def printData(self):

        content1 = self.text1.get("1.0", "end")
        content2 = self.text2.get("1.0", "end")
        content3 = self.text3.get("1.0", "end")
        content4 = self.text4.get("1.0", "end")
        content5 = self.text5.get("1.0", "end")
        content6 = self.text6.get("1.0", "end")
        content7 = self.text7.get("1.0", "end")
        content8 = self.text8.get("1.0", "end")
        content9 = self.text9.get("1.0", "end")

        self.processor.ccm[0][0] = float(content1)
        self.processor.ccm[0][1] = float(content2)
        self.processor.ccm[0][2] = float(content3)
        self.processor.ccm[1][0] = float(content4)
        self.processor.ccm[1][1] = float(content5)
        self.processor.ccm[1][2] = float(content6)
        self.processor.ccm[2][0] = float(content7)
        self.processor.ccm[2][1] = float(content8)
        self.processor.ccm[2][2] = float(content9)

        for dirname in self.processor.input_path.values():
            file_names = [f for f in os.listdir(dirname) if os.path.isfile(os.path.join(dirname, f))]
            for i in file_names:
                print(i)
                if i.split(".")[-1] in ["jpg", "png", "jpeg", "gif"]:
                    imgTmp = cv2.imread(dirname + "/" + i)
                    imgCCMTmp = np.dot(imgTmp, self.processor.ccm)
                    cv2.imwrite(dirname + "/" + i + "_CCM.jpg", imgCCMTmp)

        tk.messagebox.askokcancel("提示", "批量ccm作用完成！")


class OutfitFrame(object):  # 继承Frame类
    def __init__(self, master=None,page=None):
        # Frame.__init__(self, master)
        self.root = master  # 定义内部变量root
        self.page = page

        Label(self.page, text='1.进行CCM拟合，对比拟合得到颜色与target的色差').grid(row=1, stick=W, pady=10)
        Label(self.page, text='2.单帧作用ccm，展示前后对比').grid(row=2, stick=W, pady=10)
        Label(self.page, text='3.双目批量作用ccm').grid(row=3, stick=W, pady=10)
        Label(self.page, text='*******').grid(row=4, stick=W, pady=10)


class Algo1FrameL(object):  # 继承Frame类
    def __init__(self, processor, master=None, page=None):
        self.root = master  # 定义内部变量root
        self.page = page
        self.processor = processor
        self.createPage()

    def createPage(self):

        Label(self.page, text='CCM 计算').grid(padx=20,pady=10)
        Label(self.page).grid(row=0, stick=W, pady=10)
        self.text1 = Text(self.page, height=2, width=5)
        self.text1.grid(row=2, column=0, stick=E, pady=0)

        self.text2 = Text(self.page, height=2, width=5)
        self.text2.grid(row=2, column=1, stick=E, pady=0)

        self.text3 = Text(self.page, height=2, width=5)
        self.text3.grid(row=2, column=2, stick=E, pady=0)

        self.text4 = Text(self.page, height=2, width=5)
        self.text4.grid(row=3, column=0, stick=E, pady=0)

        self.text5 = Text(self.page, height=2, width=5)
        self.text5.grid(row=3, column=1, stick=E, pady=0)

        self.text6 = Text(self.page, height=2, width=5)
        self.text6.grid(row=3, column=2, stick=E, pady=0)

        self.text7 = Text(self.page, height=2, width=5)
        self.text7.grid(row=4, column=0, stick=E, pady=0)

        self.text8 = Text(self.page, height=2, width=5)
        self.text8.grid(row=4, column=1, stick=E, pady=0)

        self.text9 = Text(self.page, height=2, width=5)
        self.text9.grid(row=4, column=2, stick=E, pady=0)

        self.button1 = Button(self.page, text="选择标定用色卡", command=self.button1_click).grid(row=0, column=0, stick=W)
        self.button2 = Button(self.page, text="展示与target颜色对比", command=self.button2_click).grid(row=1, column=0, stick=W)

        Button(self.page, text='加载\更新ccm', command=self.getData).grid(row=7, column=3, stick=E, pady=10)

    def button1_click(self):
        dirname = tk.filedialog.askopenfilenames()  # 返回文件路径
        img = cv2.imread(dirname[0])
        self.processor.get_pts(img)

        self.text1.delete(0.0, END)
        self.text2.delete(0.0, END)
        self.text3.delete(0.0, END)
        self.text4.delete(0.0, END)
        self.text5.delete(0.0, END)
        self.text6.delete(0.0, END)
        self.text7.delete(0.0, END)
        self.text8.delete(0.0, END)
        self.text9.delete(0.0, END)

        self.text1.insert(INSERT, round(self.processor.ccm[0][0], 2))
        self.text2.insert(INSERT, round(self.processor.ccm[0][1], 2))
        self.text3.insert(INSERT, round(self.processor.ccm[0][2], 2))
        self.text4.insert(INSERT, round(self.processor.ccm[1][0], 2))
        self.text5.insert(INSERT, round(self.processor.ccm[1][1], 2))
        self.text6.insert(INSERT, round(self.processor.ccm[1][2], 2))
        self.text7.insert(INSERT, round(self.processor.ccm[2][0], 2))
        self.text8.insert(INSERT, round(self.processor.ccm[2][1], 2))
        self.text9.insert(INSERT, round(self.processor.ccm[2][2], 2))
        # print("self.processor.ccm", self.processor.ccm)

    def button2_click(self):
        img = self.Jpg2gif("./output.png")
        self.img_show = Label(self.page, image=img).grid(row=8)
        self.img_show.pack()

    def getData(self):
        content1 = self.text1.get("1.0", "end")
        content2 = self.text2.get("1.0", "end")
        content3 = self.text3.get("1.0", "end")
        content4 = self.text4.get("1.0", "end")
        content5 = self.text5.get("1.0", "end")
        content6 = self.text6.get("1.0", "end")
        content7 = self.text7.get("1.0", "end")
        content8 = self.text8.get("1.0", "end")
        content9 = self.text9.get("1.0", "end")


        self.processor.ccm[0][0] = float(content1)
        self.processor.ccm[0][1] = float(content2)
        self.processor.ccm[0][2] = float(content3)
        self.processor.ccm[1][0] = float(content4)
        self.processor.ccm[1][1] = float(content5)
        self.processor.ccm[1][2] = float(content6)
        self.processor.ccm[2][0] = float(content7)
        self.processor.ccm[2][1] = float(content8)
        self.processor.ccm[2][2] = float(content9)

        self.processor.re_apply_ccm(self.processor.ccm)

    def Jpg2gif(self, image_path):
        image = Image.open(image_path)
        image = ImageTk.PhotoImage(image.resize((400, 600)))
        print(image, type(image))  # pyimage1 <class 'PIL.ImageTk.PhotoImage'>
        return image


class Algo1FrameR(object):  # 继承Frame类
    def __init__(self, processor, master=None, page=None):
        self.root = master  # 定义内部变量root
        self.page = page
        self.processor = processor
        self.createPage()

    def createPage(self):
        # Label(self.page, text='CCM 作用到图上').grid(padx=20,pady=10)
        # Label(self.page).grid(row=0, stick=W, pady=10)
        self.text1 = Text(self.page, height=2, width=5)
        self.text1.grid(row=3, column=0, stick=E, pady=0)

        self.text2 = Text(self.page, height=2, width=5)
        self.text2.grid(row=3, column=1, stick=E, pady=0)

        self.text3 = Text(self.page, height=2, width=5)
        self.text3.grid(row=3, column=2, stick=E, pady=0)

        self.text4 = Text(self.page, height=2, width=5)
        self.text4.grid(row=4, column=0, stick=E, pady=0)

        self.text5 = Text(self.page, height=2, width=5)
        self.text5.grid(row=4, column=1, stick=E, pady=0)

        self.text6 = Text(self.page, height=2, width=5)
        self.text6.grid(row=4, column=2, stick=E, pady=0)

        self.text7 = Text(self.page, height=2, width=5)
        self.text7.grid(row=5, column=0, stick=E, pady=0)

        self.text8 = Text(self.page, height=2, width=5)
        self.text8.grid(row=5, column=1, stick=E, pady=0)

        self.text9 = Text(self.page, height=2, width=5)
        self.text9.grid(row=5, column=2, stick=E, pady=0)

        self.button1 = Button(self.page, text="选择单张图像", command=self.button1_click).grid(row=0, column=0, stick=W)

        self.button2 = Button(self.page, text="选择批量作用的图像目录", command=self.button2_click).grid(row=1, column=0, stick=W)

        Button(self.page, text='加载\更新ccm', command=self.getData).grid(row=7, column=3, stick=E, pady=10)

    def button1_click(self):
        dirname = tk.filedialog.askopenfilenames()  # 返回文件路径
        img = self.Jpg2gifDouble(dirname[0])
        imgTmp = cv2.imread(dirname[0])
        imgCCMTmp = np.dot(imgTmp, self.processor.ccm)
        cv2.imwrite("./imgCCM.jpg", imgCCMTmp)
        imgCCM = self.Jpg2gifDouble("./imgCCM.jpg")

        self.img_show = Label(self.page, image=img).grid(row=8, column=0)
        self.imgCCM_show = Label(self.page, image=imgCCM).grid(row=8, column=1)

        self.text1.delete(0.0, END)
        self.text2.delete(0.0, END)
        self.text3.delete(0.0, END)
        self.text4.delete(0.0, END)
        self.text5.delete(0.0, END)
        self.text6.delete(0.0, END)
        self.text7.delete(0.0, END)
        self.text8.delete(0.0, END)
        self.text9.delete(0.0, END)

        self.text1.insert(INSERT, round(self.processor.ccm[0][0], 2))
        self.text2.insert(INSERT, round(self.processor.ccm[0][1], 2))
        self.text3.insert(INSERT, round(self.processor.ccm[0][2], 2))
        self.text4.insert(INSERT, round(self.processor.ccm[1][0], 2))
        self.text5.insert(INSERT, round(self.processor.ccm[1][1], 2))
        self.text6.insert(INSERT, round(self.processor.ccm[1][2], 2))
        self.text7.insert(INSERT, round(self.processor.ccm[2][0], 2))
        self.text8.insert(INSERT, round(self.processor.ccm[2][1], 2))
        self.text9.insert(INSERT, round(self.processor.ccm[2][2], 2))
        # print("self.processor.ccm", self.processor.ccm)
        self.img_show.pack()
        self.imgCCM_show.pack()

    def button2_click(self):
        dirname = tk.filedialog.askdirectory()  # 返回文件路径
        file_names = [f for f in os.listdir(dirname) if os.path.isfile(os.path.join(dirname, f))]
        for i in file_names:
            print(i)
            if i.split(".")[-1] in ["jpg", "png", "jpeg", "gif"]:
                imgTmp = cv2.imread(dirname + "/" + i)
                imgCCMTmp = np.dot(imgTmp, self.processor.ccm)
                cv2.imwrite(dirname + "/" + i + "_CCM.jpg", imgCCMTmp)

        tk.messagebox.askokcancel("提示", "批量ccm作用完成！")


    def getData(self):
        content1 = self.text1.get("1.0", "end")
        content2 = self.text2.get("1.0", "end")
        content3 = self.text3.get("1.0", "end")
        content4 = self.text4.get("1.0", "end")
        content5 = self.text5.get("1.0", "end")
        content6 = self.text6.get("1.0", "end")
        content7 = self.text7.get("1.0", "end")
        content8 = self.text8.get("1.0", "end")
        content9 = self.text9.get("1.0", "end")

        self.processor.ccm[0][0] = float(content1)
        self.processor.ccm[0][1] = float(content2)
        self.processor.ccm[0][2] = float(content3)
        self.processor.ccm[1][0] = float(content4)
        self.processor.ccm[1][1] = float(content5)
        self.processor.ccm[1][2] = float(content6)
        self.processor.ccm[2][0] = float(content7)
        self.processor.ccm[2][1] = float(content8)
        self.processor.ccm[2][2] = float(content9)



    def Jpg2gifDouble(self, image_path):
        image = Image.open(image_path)
        image = ImageTk.PhotoImage(image.resize((300, 200)))
        print(image, type(image))  # pyimage1 <class 'PIL.ImageTk.PhotoImage'>
        return image


class CountFrame(object):  # 继承Frame类
    def __init__(self, processor, master=None,page=None):
        # Frame.__init__(self, master)
        self.root = master  # 定义内部变量root
        self.page = page
        self.processor = processor

        self.button1 = Button(self.page, text="计算收敛过程并合成对比视频", command=self.button1_click).grid(row=0, column=1, pady=10)
        self.button2 = Button(self.page, text="展示对比视频", command=self.button2_click).grid(row=1, column=1, pady=10)

        self.text = ScrolledText(self.page)
        self.text.configure(state=tkinter.DISABLED)
        self.text.grid(row=6, column=1, columnspan=1, stick=W, pady=0)


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
        self.clean_log()
        tk.messagebox.askokcancel("提示", "计算需要较长时间，请耐心等待一下")
        self.clean_log()

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
        Label(self.page, text='CCM simulator').grid(padx=20, pady=10)
        Label(self.page).grid(row=0, stick=W, pady=10)
        Label(self.page, text='主要用于仿真CCM').grid(row=1, stick=W, pady=10)


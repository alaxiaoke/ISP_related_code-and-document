import tkinter as tk
from tkinter import *
from tkinter.messagebox import *
import tkinter.messagebox
import tkinter.filedialog
import subprocess
from tkinter.scrolledtext import ScrolledText
import cv2
import numpy as np
from threading import Thread
import os
from PIL import Image, ImageTk
from statsAWB import statsAWB


class Algo1FrameL(object):  # 继承Frame类
    def __init__(self, master=None, page=None):
        self.root = master  # 定义内部变量root
        self.page = page
        self.createPage()
        self.AWB_Algo = statsAWB()

    def createPage(self):
        Label(self.page, text='AWB 自研简陋调试工具')
        self.button3 = Button(self.page, text="输入raw height:")
        self.button3.pack()
        self.text1 = Text(self.page, height=2, width=21)
        self.text1.pack()
        self.button4 = Button(self.page, text="输入raw weight:")
        self.button4.pack()
        self.text2 = Text(self.page, height=2, width=21)
        self.text2.pack()
        self.button5 = Button(self.page, text="输入raw Bayer pattern:")
        self.button5.pack()
        self.text3 = Text(self.page, height=2, width=21)
        self.text3.pack()
        self.button6 = Button(self.page, text='加载\更新', command=self.getData)
        self.button6.pack()
        self.button1 = Button(self.page, text="选择raw图并展示统计落点", command=self.button1_click)
        self.button1.pack()
        self.button2 = Button(self.page, text="调整AWB参数", command=self.button2_click)
        self.button2.pack()

        # Label(self.page, text='AWB 自研简陋调试工具').grid(padx=20,pady=10)
        # Label(self.page).grid(row=0, stick=W, pady=10)
        # self.text1 = Text(self.page, height=2, width=21)
        # self.text1.grid(row=2, column=0, stick=E, pady=0)
        # self.text2 = Text(self.page, height=2, width=21)
        # self.text2.grid(row=3, column=0, stick=E, pady=0)
        # self.text3 = Text(self.page, height=2, width=21)
        # self.text3.grid(row=4, column=0, stick=E, pady=0)
        #
        # self.button1 = Button(self.page, text="选择raw图并展示统计落点", command=self.button1_click).grid(row=0, column=0, stick=W)
        # self.button2 = Button(self.page, text="调整AWB参数", command=self.button2_click).grid(row=1, column=0, stick=W)
        #
        # self.button3 = Button(self.page, text="输入raw height").grid(row=2, column=1, stick=W)
        # self.button3 = Button(self.page, text="输入raw weight").grid(row=3, column=1, stick=W)
        # self.button3 = Button(self.page, text="输入raw Bayer pattern").grid(row=4, column=1, stick=W)
        # Button(self.page, text='加载\更新', command=self.getData).grid(row=7, column=3, stick=E, pady=10)

    def button1_click(self):
        singe_image_name = tk.filedialog.askopenfilenames()[0]  # 返回文件路径
        print(singe_image_name)

        path = r".\\"
        statsAWB = self.AWB_Algo
        m = 33
        n = 33
        img_int8, img_10bit = statsAWB.readRaw(singe_image_name)
        # cv.imwrite(savePath + './before_wb.jpg', img_int8)
        # imgForCpp = cv.imread('./img_int8.jpg')
        blocks_stats = statsAWB.AWBstats(img_int8, m, n, False)

        img_to_save = statsAWB.applyAWB(img_int8)
        img_to_save = statsAWB.gamma(img_to_save)

        statsAWB.readFromTxt()
        statsAWB.draw(path)

        img = self.Jpg2gif("./img_draw.jpg")

        self.img_show = Label(self.page, image=img)
        self.img_show.pack()

        self.page.mainloop()

    def button2_click(self):

        exe = r"./AWBTuningTool.exe"
        subprocess.run(exe)

    def getData(self):
        height = self.text1.get("1.0", "end")
        width = self.text2.get("1.0", "end")
        cfa = self.text3.get("1.0", "end")
        print(cfa)
        self.AWB_Algo.height = int(height)
        self.AWB_Algo.width = int(width)
        self.AWB_Algo.cfa = cfa.strip()

    def Jpg2gif(self, image_path):
        image = Image.open(image_path)
        image = ImageTk.PhotoImage(image.resize((500, 500)))
        # print(image, type(image))  # pyimage1 <class 'PIL.ImageTk.PhotoImage'>
        return image



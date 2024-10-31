from tkinter import *
from view import *  # 菜单栏对应的各个子页面


class MainPage(object):
    def __init__(self, master=None):
        self.root = master  # 定义内部变量root
        self.root.geometry('%dx%d' % (600, 600))  # 设置窗口大小
        self.createPage()

    def createPage(self):
        menubar = Menu(self.root)

        # 将所创建的子菜单关联到主菜单上面
        menubar.add_cascade(label=u'AWB 自研简陋调试工具', command=self.algo1DataL)
        # menubar.add_cascade(label=u'', command=self.algo1DataL)

        # menubar.add_command(label=u'算法3', command=self.countData)
        self.root['menu'] = menubar  # 设置菜单栏
        self.algo1PageL_show = Frame(self.root)

        self.algo1PageL = Algo1FrameL( self.root, self.algo1PageL_show)


    def algo1DataL(self):
        self.algo1PageL_show.pack()




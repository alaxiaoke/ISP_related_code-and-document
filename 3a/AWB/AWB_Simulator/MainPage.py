from tkinter import *
from view import *  # 菜单栏对应的各个子页面


class MainPage(object):
    def __init__(self, master=None):
        self.root = master  # 定义内部变量root
        self.root.geometry('%dx%d' % (600, 600))  # 设置窗口大小
        self.AWBProcessor = AWBProcess()
        self.createPage()

    def createPage(self):
        menubar = Menu(self.root)
        menubar2 = Menu(self.root)
        # 创建一个子菜单
        filemenu = Menu(menubar, tearoff=0)
        filemenu.add_command(label=u'算法1',command=self.algo1DataR)
        filemenu.add_command(label=u'算法2',command=self.algo2DataR)
        filemenu.add_command(label=u'算法3',command=self.algo3DataR)

        filemenu2 = Menu(menubar2, tearoff=0)
        filemenu2.add_command(label=u'算法1',command=self.algo1DataL)
        filemenu2.add_command(label=u'算法2',command=self.algo2DataL)
        filemenu2.add_command(label=u'算法3',command=self.algo3DataL)

        # 将所创建的子菜单关联到主菜单上面
        menubar.add_command(label=u"home", command=self.outfitData)
        menubar.add_cascade(label=u'左侧相机收敛算法选择', menu=filemenu2)
        menubar.add_cascade(label=u'右侧相机收敛算法选择', menu=filemenu)
        menubar.add_command(label=u'输入帧数据', command=self.imgData)
        menubar.add_command(label=u'对比双目结果', command=self.showData)
        # menubar.add_command(label=u'算法3', command=self.countData)
        self.root['menu'] = menubar  # 设置菜单栏

        self.inputPage_show = Frame(self.root, width=400, height=400)   # 定义相关视图并且传到对应方法
        self.queryPage_show = Frame(self.root)
        self.countPage_show = Frame(self.root)
        self.outfitPage_show = Frame(self.root)
        self.algo1PageL_show = Frame(self.root)
        self.algo2PageL_show = Frame(self.root)
        self.algo3PageL_show = Frame(self.root)
        self.algo1PageR_show = Frame(self.root)
        self.algo2PageR_show = Frame(self.root)
        self.algo3PageR_show = Frame(self.root)

        self.inputPage = InputFrame(self.AWBProcessor, self.root, self.inputPage_show)  # 创建不同Frame 将view里定义好的视图传过来 点击菜单对应的方法来显示页面或者隐藏页面
        self.countPage = CountFrame(self.AWBProcessor, self.root, self.countPage_show)
        self.outfitPage = OutfitFrame(self.root, self.outfitPage_show)
        self.algo1PageL = Algo1FrameL(self.AWBProcessor, self.root, self.algo1PageL_show)
        self.algo2PageL = Algo2FrameL(self.AWBProcessor, self.root, self.algo2PageL_show)
        self.algo3PageL = Algo3FrameL(self.AWBProcessor, self.root, self.algo3PageL_show)
        self.algo1PageR = Algo1FrameR(self.AWBProcessor, self.root, self.algo1PageR_show)
        self.algo2PageR = Algo2FrameR(self.AWBProcessor, self.root, self.algo2PageR_show)
        self.algo3PageR = Algo3FrameR(self.AWBProcessor, self.root, self.algo3PageR_show)

        self.outfitPage_show.pack()  # 默认显示数据录入界面

    def algo1DataL(self):
        self.algo1PageL_show.pack()
        self.algo2PageL_show.pack_forget()
        self.algo3PageL_show.pack_forget()
        self.algo1PageR_show.pack_forget()
        self.algo2PageR_show.pack_forget()
        self.algo3PageR_show.pack_forget()
        self.inputPage_show.pack_forget()
        self.countPage_show.pack_forget()
        self.outfitPage_show.pack_forget()

    def algo2DataL(self):
        self.algo1PageL_show.pack_forget()
        self.algo2PageL_show.pack()
        self.algo3PageL_show.pack_forget()
        self.algo1PageR_show.pack_forget()
        self.algo2PageR_show.pack_forget()
        self.algo3PageR_show.pack_forget()
        self.inputPage_show.pack_forget()
        self.countPage_show.pack_forget()
        self.outfitPage_show.pack_forget()

    def algo3DataL(self):
        self.algo1PageL_show.pack_forget()
        self.algo2PageL_show.pack_forget()
        self.algo3PageL_show.pack()
        self.algo1PageR_show.pack_forget()
        self.algo2PageR_show.pack_forget()
        self.algo3PageR_show.pack_forget()
        self.inputPage_show.pack_forget()
        self.countPage_show.pack_forget()
        self.outfitPage_show.pack_forget()

    def algo1DataR(self):
        self.algo1PageL_show.pack_forget()
        self.algo2PageL_show.pack_forget()
        self.algo3PageL_show.pack_forget()
        self.algo1PageR_show.pack()
        self.algo2PageR_show.pack_forget()
        self.algo3PageR_show.pack_forget()
        self.inputPage_show.pack_forget()
        self.countPage_show.pack_forget()
        self.outfitPage_show.pack_forget()

    def algo2DataR(self):
        self.algo1PageL_show.pack_forget()
        self.algo2PageL_show.pack_forget()
        self.algo3PageL_show.pack_forget()
        self.algo1PageR_show.pack_forget()
        self.algo2PageR_show.pack()
        self.algo3PageR_show.pack_forget()
        self.inputPage_show.pack_forget()
        self.countPage_show.pack_forget()
        self.outfitPage_show.pack_forget()

    def algo3DataR(self):
        self.algo1PageL_show.pack_forget()
        self.algo2PageL_show.pack_forget()
        self.algo3PageL_show.pack_forget()
        self.algo1PageR_show.pack_forget()
        self.algo2PageR_show.pack_forget()
        self.algo3PageR_show.pack()
        self.inputPage_show.pack_forget()
        self.countPage_show.pack_forget()
        self.outfitPage_show.pack_forget()

    def imgData(self):
        self.inputPage_show.pack()
        self.algo1PageL_show.pack_forget()
        self.algo2PageL_show.pack_forget()
        self.algo3PageL_show.pack_forget()
        self.algo1PageR_show.pack_forget()
        self.algo2PageR_show.pack_forget()
        self.algo3PageR_show.pack_forget()
        self.countPage_show.pack_forget()
        self.outfitPage_show.pack_forget()

    def showData(self):
        self.inputPage_show.pack_forget()
        self.algo1PageL_show.pack_forget()
        self.algo2PageL_show.pack_forget()
        self.algo3PageL_show.pack_forget()
        self.algo1PageR_show.pack_forget()
        self.algo2PageR_show.pack_forget()
        self.algo3PageR_show.pack_forget()
        self.countPage_show.pack()
        self.outfitPage_show.pack_forget()

    def outfitData(self):
        self.inputPage_show.pack_forget()
        self.algo1PageL_show.pack_forget()
        self.algo2PageL_show.pack_forget()
        self.algo3PageL_show.pack_forget()
        self.algo1PageR_show.pack_forget()
        self.algo2PageR_show.pack_forget()
        self.algo3PageR_show.pack_forget()
        self.countPage_show.pack_forget()
        self.outfitPage_show.pack()



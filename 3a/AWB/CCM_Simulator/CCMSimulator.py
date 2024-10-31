from MainPage import *

root = Tk()
root.title('CCM simulator')
MainPage(root)
root.mainloop()


# coding=utf-8
#
# from tkinter import *
# from PIL import Image, ImageTk
#
#
# class Interface():
#
#     def __init__(self):
#         self.win = Tk()
#         self.win.title('界面')  # 标题
#         image_jpg = self.Jpg2gif('./colorcard.jpg')  # 先调用函数，再将image_jpg传入label中image参数
#         label = Label(self.win,
#                       image=image_jpg)
#         label.pack()
#         self.win.mainloop()

#     def Jpg2gif(self, image_path):
#         image = Image.open(image_path)
#         image = ImageTk.PhotoImage(image)
#         print(image, type(image))  # pyimage1 <class 'PIL.ImageTk.PhotoImage'>
#         return image
#
#
# if __name__ == '__main__':
#     Interface()

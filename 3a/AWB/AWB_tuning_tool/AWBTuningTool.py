from MainPage import *

root = Tk()
root.title('AWB tuning tool')
MainPage(root)
root.mainloop()

'''
D65: ccm [[1.1869170822538415, -0.20731760581847006, 0.020400523564628542], [-0.20376900538340983, 1.3077349541928638, -0.10396594880945398], [-0.02251246815914349, -0.3999969128618772, 1.4225093810210208]]
A:ccm [[1.22, -0.2, -0.02], [-0.37, 1.37, 0.0], [0.2, -0.96, 1.76]]
H:ccm [[0.7472341414379751, 0.2147191572341987, 0.03804670132782623], [-0.7092181813950956, 1.611066731684407, 0.09815144971068879], [-0.16223765355624187, -1.2825970038460832, 2.4448346574023248]]
cwf:ccm [[1.5718010419314687, -0.5366712193432646, -0.035129822588204185], [-0.40811328437894545, 1.4960372929534678, -0.08792400857452232], [-0.045494718606737705, -0.45154666221788453, 1.4970413808246223]]
tl84:ccm ccm [[1.3868775839993468, -0.27627750622163705, -0.11060007777770975], [-0.31218764304616997, 1.5010195780346776, -0.1888319349885077], [-0.013556943842843672, -0.5790535463680002, 1.592610490210844]]
'''
# coding=utf-8
#
# from tkinter import *
# from PIL import Image, ImageTk
#当前账号：18401692950
# 组织代码：x0nzayde
# 最新账号：zlzou
# 最新密码：与当前账号密码一致
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

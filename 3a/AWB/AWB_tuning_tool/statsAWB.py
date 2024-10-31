import cv2 as cv
import numpy as np
import cv2
import matplotlib.pyplot as plt
import os
from os import close, read
import matplotlib.patches as mpatches
#这里的close是关闭文件，read是读取文件

class statsAWB():
    def __init__(self):
        self.lightList = []
        self.ROIList = []
        self.seriesList = []
        self.weightList = []
        self.blocks_stats = []
        self.rg = 0.5
        self.bg = 0.5
        self.cfa = 'bggr'#rggb/grbg/bggr/gbrg
        self.bpp = 10
        self.width = int(2328)
        self.height = int(1744)
        self.img_name = "img.jpg"
        self.lightSourceTable = ""
        self.cctWeightTable = ""
        self.lvWeight = ""
        self.greyRegions = ""
        self.colorRegions = ""
        self.misColorRegions = ""
        self.colorRegionsParams = ""

    def readRaw(self, path):
        self.img_name = path
        raw = np.fromfile(path, dtype=np.uint16)
        raw = raw.reshape((self.height,self.width))
        ob = np.ones(raw.shape, dtype = np.uint16) * 64*2**(self.bpp - 10)
        img = cv2.subtract(raw, ob)
        img = img/959*1023
        img = np.array(img, dtype=np.uint16)
        print(img.shape)
        if self.cfa == 'rggb':
            img = cv.cvtColor(img, cv.COLOR_BAYER_RG2RGB)
        elif self.cfa == 'grbg':
            img = cv.cvtColor(img, cv.COLOR_BAYER_GR2RGB)
        elif self.cfa == 'bggr':
            print("enter bggr process")
            img = cv.cvtColor(img, cv.COLOR_BAYER_BG2RGB)
        elif self.cfa == 'gbrg':
            img = cv.cvtColor(img, cv.COLOR_BAYER_GB2RGB)
        img = np.array(img, dtype='float64')
        img1 = img.copy()
        print(img.shape)
        img1[:,:,0] = img[:,:,0]/2**(self.bpp - 8)
        img1[:,:,1] = img[:,:,1]/2**(self.bpp - 8)
        img1[:,:,2] = img[:,:,2]/2**(self.bpp - 8)
        # cv.imwrite('./demosaic.jpg', img1)
        return img1, img


    def AWBstats(self, img, m, n, saveForCpp = False):
        h, w = img.shape[0], img.shape[1]
        print("ssss", np.max(img))

        grid_h = int(h * 1.0 / (m - 1) + 0.5)  # 每个网格的高
        grid_w = int(w * 1.0 / (n - 1) + 0.5)  # 每个网格的宽

        # 满足整除关系时的高、宽
        h = grid_h * (m - 1)
        w = grid_w * (n - 1)

        gx, gy = np.linspace(0, w, n), np.linspace(0, h, m)

        # print("gx:", gx)
        # print("gy:", gy)

        divide_image = []
        self.blocks_stats.clear()
        for i in range(m - 1):
            for j in range(n - 1):
                tmp_img = img[int(gy[i]):int(gy[i + 1]), int(gx[j]):int(gx[j + 1]), :]
                tmp_img = tmp_img.reshape(tmp_img.shape[0]*tmp_img.shape[1], 3)
                # deleteList = []
                # for w in range(tmp_img.shape[0]):
                #     if(sum(tmp_img[w]) >= 720 or sum(tmp_img[w]) <= 35):
                #         # print(tmp_img[w])
                #         deleteList.append(w)
                # tmp_img = np.delete(tmp_img, deleteList, axis=0)
                if tmp_img.shape[0] == 0:
                    continue
                Rmean = np.mean(tmp_img[:, 2])
                Gmean = np.mean(tmp_img[:, 1])
                Bmean = np.mean(tmp_img[:, 0])

                Rgain = Rmean / (Gmean + 0.01)
                Bgain = Bmean / (Gmean + 0.01)
                self.blocks_stats.append([Rgain, Bgain])

        return self.blocks_stats


    def applyAWB(self, img):
        imgNew = np.zeros((img.shape[0], img.shape[1], 3), dtype='float64')
        imgNew[:,:,0] = img[:,:,0]/self.bg
        imgNew[:,:,1] = img[:,:,1]
        imgNew[:,:,2] = img[:,:,2]/self.rg
        imgNew = np.clip(imgNew, 0, 255)
        imgNew = imgNew.astype('uint8')
        # imgNew = gamma(imgNew)
        return imgNew


    def gamma(self, image, inverse = False, gamma=2.2):
        if inverse:
            gamma = 1.0 / gamma
        table = []
        for i in range(256):
            table.append(((i / 255.0) ** gamma) * 255)
        table = np.array(table).astype("uint8")
        return cv2.LUT(image, table)


    def readFromTxt(self):
        f = open("./AWBParam.bin", "rb")
        data = f.readlines()[0]
        params_list = []
        # print(type(data))
        # print(len(data))
        for i in range(0, len(data) - 4, 4):
            bt = data[i:i + 4][::-1]
            # print(bt)
            i2 = int.from_bytes(bt, byteorder='big', signed=True)
            # print(i2)
            params_list.append(i2)

        print(params_list)

        maxMeteringParamINTCount = 9
        colorRegionsCount = 8
        misColorRegionsCount = 8
        CCTLevel = 7
        LightLevel = 5
        lightSourceNum = 7

        self.lightSourceTable = np.array(params_list[:lightSourceNum * 2]).reshape(lightSourceNum, 2)
        print("lightSourceTable:", self.lightSourceTable)
        index = lightSourceNum * 4  #包括了rg bg的shift没有读出来用
        self.cctWeightTable = np.array(
            params_list[index :index + LightLevel * CCTLevel]).reshape(LightLevel, CCTLevel)
        print("cctWeightTable:", self.cctWeightTable)
        self.lvWeight = np.array(
            params_list[index + LightLevel * CCTLevel:index + LightLevel * CCTLevel + 5])
        print("lvWeight:", self.lvWeight)
        index = index + LightLevel * CCTLevel + 5
        self.greyRegions = np.array(params_list[index:index + lightSourceNum * 8]).reshape(lightSourceNum, 8)
        print("greyRegions:", self.greyRegions)
        index = index + lightSourceNum * 8
        self.colorRegions = np.array(params_list[index:index + colorRegionsCount * 8]).reshape(colorRegionsCount, 8)
        print("colorRegions:", self.colorRegions)
        index = index + colorRegionsCount * 8
        self.misColorRegions = np.array(params_list[index:index + misColorRegionsCount * 8]).reshape(misColorRegionsCount,8)
        print("misColorRegions:", self.misColorRegions)
        index = index + misColorRegionsCount * 8
        self.colorRegionsParams = np.array(params_list[index:index + 15]).reshape(5, 3)
        print("colorRegionsParams:", self.colorRegionsParams)


    def draw(self, save_path='./data/', img_name="img"):

        AWB_GREY_REGION = self.greyRegions[0]
        AWB_GREY_REGION2 = self.greyRegions[1]
        AWB_GREY_REGION3 = self.greyRegions[2]
        AWB_GREY_REGION4 = self.greyRegions[3]
        AWB_GREY_REGION5 = self.greyRegions[4]
        AWB_GREY_REGION6 = self.greyRegions[5]
        AWB_GREY_REGION7 = self.greyRegions[6]
        x = []
        y = []
        for wp in self.lightList:
            x.append(wp[0])
            y.append(wp[1])
        rg_ = []
        bg_ = []
        for wp in self.blocks_stats:
            rg_.append(wp[0])
            bg_.append(wp[1])

        color = ['c', 'b', 'g', 'r', 'm', 'y', 'k', 'w']
        # plt.xticks(range(0.0, 1.7, 0.2), range(0.0, 1.7, 0.2))
        plt.clf()
        plt.xlim(0.05, 1.5)
        plt.ylim(0.05, 1.2)
        plt.scatter(rg_, bg_, c=color[2], s=2)
        plt.scatter(x, y, c=color[3], edgecolors='y')
        plt.scatter(self.rg, self.bg, c=color[3], s=100,  marker="*")

        # weight = self.ROIList[2][0] - self.ROIList[1][0]
        # height = self.ROIList[2][1] - self.ROIList[1][1]
        # rect = mpatches.Rectangle(self.ROIList[1], weight, height, linestyle='dotted', edgecolor='r', facecolor='none')
        # plt.gca().add_patch(rect)
        #可以用 16进制颜色代码 给颜色区边框用edgecolor 绘制各种颜色 如#FFEC8B

        rect = mpatches.Polygon([[AWB_GREY_REGION[0]/100,AWB_GREY_REGION[1]/100], [AWB_GREY_REGION[2]/100,AWB_GREY_REGION[3]/100], [AWB_GREY_REGION[4]/100,AWB_GREY_REGION[5]/100], [AWB_GREY_REGION[6]/100,AWB_GREY_REGION[7]/100]], edgecolor = 'grey', alpha = 0.8, facecolor = 'none', linestyle = 'dotted')
        plt.gca().add_patch(rect)
        rect = mpatches.Polygon([[AWB_GREY_REGION2[0]/100,AWB_GREY_REGION2[1]/100], [AWB_GREY_REGION2[2]/100,AWB_GREY_REGION2[3]/100],[AWB_GREY_REGION2[4]/100,AWB_GREY_REGION2[5]/100],[AWB_GREY_REGION2[6]/100,AWB_GREY_REGION2[7]/100]], edgecolor = 'r', alpha = 0.8, facecolor = 'none', linestyle = 'dotted')
        plt.gca().add_patch(rect)
        rect = mpatches.Polygon([[AWB_GREY_REGION3[0]/100,AWB_GREY_REGION3[1]/100], [AWB_GREY_REGION3[2]/100,AWB_GREY_REGION3[3]/100],[AWB_GREY_REGION3[4]/100,AWB_GREY_REGION3[5]/100],[AWB_GREY_REGION3[6]/100,AWB_GREY_REGION3[7]/100]], edgecolor = 'g', alpha = 0.8, facecolor = 'none', linestyle = 'dotted')
        plt.gca().add_patch(rect)
        rect = mpatches.Polygon([[AWB_GREY_REGION4[0]/100,AWB_GREY_REGION4[1]/100], [AWB_GREY_REGION4[2]/100,AWB_GREY_REGION4[3]/100],[AWB_GREY_REGION4[4]/100,AWB_GREY_REGION4[5]/100],[AWB_GREY_REGION4[6]/100,AWB_GREY_REGION4[7]/100]], edgecolor = 'b', alpha = 0.8, facecolor = 'none', linestyle = 'dotted')
        plt.gca().add_patch(rect)
        rect = mpatches.Polygon([[AWB_GREY_REGION5[0]/100,AWB_GREY_REGION5[1]/100], [AWB_GREY_REGION5[2]/100,AWB_GREY_REGION5[3]/100],[AWB_GREY_REGION5[4]/100,AWB_GREY_REGION5[5]/100],[AWB_GREY_REGION5[6]/100,AWB_GREY_REGION5[7]/100]], edgecolor = 'c', alpha = 0.8, facecolor = 'none', linestyle = 'dotted')
        plt.gca().add_patch(rect)
        rect = mpatches.Polygon([[AWB_GREY_REGION6[0]/100,AWB_GREY_REGION6[1]/100], [AWB_GREY_REGION6[2]/100,AWB_GREY_REGION6[3]/100],[AWB_GREY_REGION6[4]/100,AWB_GREY_REGION6[5]/100],[AWB_GREY_REGION6[6]/100,AWB_GREY_REGION6[7]/100]], edgecolor = 'black', alpha = 0.8, facecolor = 'none', linestyle = 'dotted')
        plt.gca().add_patch(rect)
        rect = mpatches.Polygon([[AWB_GREY_REGION7[0]/100,AWB_GREY_REGION7[1]/100], [AWB_GREY_REGION7[2]/100,AWB_GREY_REGION7[3]/100],[AWB_GREY_REGION7[4]/100,AWB_GREY_REGION7[5]/100],[AWB_GREY_REGION7[6]/100,AWB_GREY_REGION7[7]/100]], edgecolor = 'black', alpha = 0.8, facecolor = 'none', linestyle = 'dotted')
        plt.gca().add_patch(rect)

        rect = mpatches.Polygon(
            [[self.colorRegions[0][0] / 100, self.colorRegions[0][1] / 100], [self.colorRegions[0][2] / 100, self.colorRegions[0][3] / 100],
             [self.colorRegions[0][4] / 100, self.colorRegions[0][5] / 100],
             [self.colorRegions[0][6] / 100, self.colorRegions[0][7] / 100]], edgecolor='r', alpha=0.8, facecolor='none',
            linestyle='dotted')
        plt.gca().add_patch(rect)
        rect = mpatches.Polygon(
            [[self.colorRegions[1][0] / 100, self.colorRegions[1][1] / 100], [self.colorRegions[1][2] / 100, self.colorRegions[1][3] / 100],
             [self.colorRegions[1][4] / 100, self.colorRegions[1][5] / 100],
             [self.colorRegions[1][6] / 100, self.colorRegions[1][7] / 100]], edgecolor='r', alpha=0.8, facecolor='none',
            linestyle='dotted')
        plt.gca().add_patch(rect)
        rect = mpatches.Polygon(
            [[self.colorRegions[2][0] / 100, self.colorRegions[2][1] / 100], [self.colorRegions[2][2] / 100, self.colorRegions[2][3] / 100],
             [self.colorRegions[2][4] / 100, self.colorRegions[2][5] / 100],
             [self.colorRegions[2][6] / 100, self.colorRegions[2][7] / 100]], edgecolor='r', alpha=0.8, facecolor='none',
            linestyle='dotted')
        plt.gca().add_patch(rect)
        rect = mpatches.Polygon(
            [[self.colorRegions[3][0] / 100, self.colorRegions[3][1] / 100], [self.colorRegions[3][2] / 100, self.colorRegions[3][3] / 100],
             [self.colorRegions[3][4] / 100, self.colorRegions[3][5] / 100],
             [self.colorRegions[3][6] / 100, self.colorRegions[3][7] / 100]], edgecolor='r', alpha=0.8, facecolor='none',
            linestyle='dotted')
        plt.gca().add_patch(rect)
        rect = mpatches.Polygon(
            [[self.colorRegions[4][0] / 100, self.colorRegions[4][1] / 100], [self.colorRegions[4][2] / 100, self.colorRegions[4][3] / 100],
             [self.colorRegions[4][4] / 100, self.colorRegions[4][5] / 100],
             [self.colorRegions[4][6] / 100, self.colorRegions[4][7] / 100]], edgecolor='r', alpha=0.8, facecolor='none',
            linestyle='dotted')
        plt.gca().add_patch(rect)

        rect = mpatches.Polygon(
            [[self.misColorRegions[0][0] / 100, self.misColorRegions[0][1] / 100], [self.misColorRegions[0][2] / 100, self.misColorRegions[0][3] / 100],
             [self.misColorRegions[0][4] / 100, self.misColorRegions[0][5] / 100],
             [self.misColorRegions[0][6] / 100, self.misColorRegions[0][7] / 100]], edgecolor='r', alpha=0.8, facecolor='none',
            linestyle='dotted')
        plt.gca().add_patch(rect)
        rect = mpatches.Polygon(
            [[self.misColorRegions[1][0] / 100, self.misColorRegions[1][1] / 100], [self.misColorRegions[1][2] / 100, self.misColorRegions[1][3] / 100],
             [self.misColorRegions[1][4] / 100, self.misColorRegions[1][5] / 100],
             [self.misColorRegions[1][6] / 100, self.misColorRegions[1][7] / 100]], edgecolor='r', alpha=0.8, facecolor='none',
            linestyle='dotted')
        plt.gca().add_patch(rect)
        rect = mpatches.Polygon(
            [[self.misColorRegions[2][0] / 100, self.misColorRegions[2][1] / 100], [self.misColorRegions[2][2] / 100, self.misColorRegions[2][3] / 100],
             [self.misColorRegions[2][4] / 100, self.misColorRegions[2][5] / 100],
             [self.misColorRegions[2][6] / 100, self.misColorRegions[2][7] / 100]], edgecolor='r', alpha=0.8, facecolor='none',
            linestyle='dotted')
        plt.gca().add_patch(rect)
        rect = mpatches.Polygon(
            [[self.misColorRegions[3][0] / 100, self.misColorRegions[3][1] / 100], [self.misColorRegions[3][2] / 100, self.misColorRegions[3][3] / 100],
             [self.misColorRegions[3][4] / 100, self.misColorRegions[3][5] / 100],
             [self.misColorRegions[3][6] / 100, self.misColorRegions[3][7] / 100]], edgecolor='r', alpha=0.8, facecolor='none',
            linestyle='dotted')
        plt.gca().add_patch(rect)
        rect = mpatches.Polygon(
            [[self.misColorRegions[4][0] / 100, self.misColorRegions[4][1] / 100], [self.misColorRegions[4][2] / 100, self.misColorRegions[4][3] / 100],
             [self.misColorRegions[4][4] / 100, self.misColorRegions[4][5] / 100],
             [self.misColorRegions[4][6] / 100, self.misColorRegions[4][7] / 100]], edgecolor='r', alpha=0.8, facecolor='none',
            linestyle='dotted')
        plt.gca().add_patch(rect)


        plt.savefig(save_path + img_name  + '_draw.jpg', dpi=300)
        # plt.show()










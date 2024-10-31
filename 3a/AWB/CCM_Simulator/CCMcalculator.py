import imutils
import numpy as np
import joblib
import cv2 as cv
from joblib import load
from scipy.optimize import leastsq
from plotChart import Compare

# pts = []  # 用于存放点
class CCM_IMGProcess():
    def __init__(self):
        self.pts = []
        self.source_colors = []
        self.ccm = np.zeros([3, 3])
        self.target_colors = np.array([[115, 194, 98,  87,  133, 103, 214, 80,  193, 94,  157, 224, 56,  70,  175, 231, 187, 8, 5, 5, 5, 5, 5, 5],
                  [82,  150, 122, 108, 128, 189, 126, 91,  90,  60,  188, 163, 61,  148, 54,  199, 86,  133, 5, 5, 5, 5, 5, 5],
                  [68,  130, 157, 67,  177, 170, 44,  166, 99,  108, 64,  46,  150, 73,  60,  31,  149, 161, 5, 5, 5, 5, 5, 5]])
        self.plotCompare = Compare()
        self.input_path = {}


    # 统一的：mouse callback function
    def draw_roi(self, event, x, y, flags, param):
        img = param
        img2 = img.copy()

        if event == cv.EVENT_LBUTTONDOWN:  # 左键点击，选择点
            self.pts.append((x, y))

        if event == cv.EVENT_RBUTTONDOWN:  # 右键点击，取消最近一次选择的点
            self.pts.pop()

        if event == cv.EVENT_MBUTTONDOWN:  # 中键绘制轮廓
            mask = np.zeros(img.shape, np.uint8)
            points = np.array(self.pts, np.int32)
            points = points.reshape((-1, 1, 2))
            # 画多边形
            mask = cv.polylines(mask, [points], True, (255, 255, 255), 2)
            mask2 = cv.fillPoly(mask.copy(), [points], (255, 255, 255))  # 用于求 ROI
            mask3 = cv.fillPoly(mask.copy(), [points], (0, 255, 0))  # 用于 显示在桌面的图像

            show_image = cv.addWeighted(src1=img, alpha=0.8, src2=mask3, beta=0.2, gamma=0)

            cv.imshow("mask", mask2)
            cv.imshow("show_img", show_image)

            ROI = cv.bitwise_and(mask2, img)

            # cv2.imshow("ROI", ROI)
            # cv2.waitKey(0)

        if len(self.pts) > 0:
            # 将pts中的最后一点画出来
            cv.circle(img2, self.pts[-1], 3, (0, 0, 255), -1)

        if len(self.pts) > 1:
            # 画线
            for i in range(len(self.pts) - 1):
                cv.circle(img2, self.pts[i], 5, (0, 0, 255), -1)  # x ,y 为鼠标点击地方的坐标
                cv.line(img=img2, pt1=self.pts[i], pt2=self.pts[i + 1], color=(255, 0, 0), thickness=2)

        cv.imshow('image', img2)


    def get_pts(self, img):
        # 创建图像与窗口并将窗口与回调函数绑定
        img = imutils.resize(img, width=500)
        cv.namedWindow('image')
        cv.setMouseCallback('image', self.draw_roi, img)
        print("[INFO] 单击左键：选择点，单击右键：删除上一次选择的点，单击中键：确定ROI区域")
        print("[INFO] 按‘S’确定选择区域并保存")
        print("[INFO] 按 ESC 退出")

        while True:
            key = cv.waitKey(1) & 0xFF
            # print("key", key)
            if key == 13:
                break
            if key == ord("s"):
                # saved_data = {"ROI": self.pts}
                # joblib.dump(value=saved_data, filename="config.pkl")
                # print("[INFO] ROI坐标已保存到本地.")
                break
        cv.destroyAllWindows()
        self.get_24_colors(img)
        self.ccmCalculation()
        self.plotCompare.plotCompare(self.source_colors, self.target_colors, self.ccm)

    def re_apply_ccm(self, ccm):

        self.plotCompare.plotCompare(self.source_colors, self.target_colors, ccm)


    def get_24_colors(self, img):
        color_matrix = []
        if len(self.pts) >= 4:
            pts = self.pts[:4]
            pts = self.sort_point(pts)
            print("ROI:", pts)
            pts = np.float32(pts)
            # dst_w = int(pts[1][0] - pts[0][0])
            # dst_h = int(pts[0][0] - pts[1][0])
            # pos2 = np.array([[0, 0], [dst_w, 0],  [0, dst_h], [dst_w, dst_h]], dtype="float32")
            pos2 = np.float32([[0, 0], [300, 0], [0, 200],  [300, 200]])
            M = cv.getPerspectiveTransform(pts, pos2)
            # print("M:", M, dst_h, dst_w)
            img_persp = cv.warpPerspective(img, M, (300, 200))

            # print(img_persp.shape)
            pos_vert = [20, 70, 120, 170]
            pos_hori = [20, 70, 120, 170, 220, 270]
            for i in pos_vert:
                for j in pos_hori:
                    cv.rectangle(img_persp, (j - 8, i - 8), (j + 15, i + 15), (0, 0, 255), 2)

            # img_persp = np.rot90(img_persp, 1)
            for i in pos_vert:
                for j in pos_hori:
                    data = img_persp[i - 5:i + 5, j - 6:j + 6]
                    # print(i,j, data.shape)
                    b, g, r = cv.split(data)
                    color_matrix.append([int(np.mean(r)), int(np.mean(g)), int(np.mean(b))])

            self.source_colors = np.array(color_matrix)
            cv.imshow("img_show", img_persp)
            cv.waitKey(0)
            cv.destroyAllWindows()
        else:
            print("未找到色卡")
        print("24 COLORS:", color_matrix)
        return self.source_colors


    def sort_point(self, points):
        """
        对四个定位点进行排序，排序后顺序分别为左上、右上、左下、右下
        :param points: 待排序的点集
        :return: 排序完成的点集
        """
        sp = sorted(points, key=lambda x: (int(x[1])+int(x[0]), int(x[0])))
        # print("sp", sp)
        if sp[1][0] < sp[2][0]:
            sp[1], sp[2] = sp[2], sp[1]
        # if sp[0][0] > sp[1][0]:
        #     sp[0], sp[1] = sp[1], sp[0]
        #
        # if sp[2][0] > sp[3][0]:
        #     sp[2], sp[3] = sp[3], sp[2]

        return sp


    def func(self, x1, x2, p):
        a, b = p
        # print(a,b)
        return x1 * a + x2 * b

    def residuals(self, p, y, x1, x2):
        return y - self.func(x1, x2, p)

    def degamma(self, img):
        img = pow((img / 255), 2.2)
        return img * 255

    def ccmCalculation(self):
        standard_sRGB = self.degamma(self.target_colors)

        raw_RGB = np.array(self.source_colors)[:18, :]
        raw_RGB = self.degamma(raw_RGB[:, :])
        # print("raw_RGB", raw_RGB)
        light_adjust = self.lightnessAlign(standard_sRGB, raw_RGB)
        print("亮度较准系数", light_adjust)
        raw_RGB = raw_RGB*light_adjust

        standard_sRGB = standard_sRGB[:, :18]

        y1 = standard_sRGB[0]
        y2 = standard_sRGB[1]
        y3 = standard_sRGB[2]

        x1 = []
        x2 = []
        for i in range(len(y1)):
            y1[i] = y1[i] - raw_RGB[i][2]
            y2[i] = y2[i] - raw_RGB[i][2]
            y3[i] = y3[i] - raw_RGB[i][2]
            x1.append(raw_RGB[i][0] - raw_RGB[i][2])
            x2.append(raw_RGB[i][1] - raw_RGB[i][2])

        x1 = np.array(x1)
        x2 = np.array(x2)

        p0 = [0.5, 0.5]  # 第一次猜测的函数拟合参数

        plsq1 = leastsq(self.residuals, p0, args=(y1, x1, x2))
        plsq2 = leastsq(self.residuals, p0, args=(y2, x1, x2))
        plsq3 = leastsq(self.residuals, p0, args=(y3, x1, x2))

        self.ccm = [[plsq1[0][0], plsq1[0][1], 1 - plsq1[0][0] - plsq1[0][1]],
               [plsq2[0][0], plsq2[0][1], 1 - plsq2[0][0] - plsq2[0][1]],
               [plsq3[0][0], plsq3[0][1], 1 - plsq3[0][0] - plsq3[0][1]]]
        print("ccm", self.ccm)

    def lightnessAlign(self, standard_sRGB, raw_RGB):
        Rmean = np.mean(raw_RGB[:, 0])
        Gmean = np.mean(raw_RGB[:, 1])
        Bmean = np.mean(raw_RGB[:, 2])
        Lightness_raw = 0.3 * Rmean + 0.6 * Gmean + 0.1 * Bmean
        Rmean = np.mean(standard_sRGB[0])
        Gmean = np.mean(standard_sRGB[1])
        Bmean = np.mean(standard_sRGB[2])
        Lightness_std = 0.3 * Rmean + 0.6 * Gmean + 0.1 * Bmean
        return Lightness_std/Lightness_raw



# img = cv.imread("./colorcard.jpg")
# SDS = CCM_IMGProcess(img)
# SDS.get_pts()

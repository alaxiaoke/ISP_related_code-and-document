import copy

import cv2 as cv
import numpy as np
from joblib import load

def findGrayLightness():
    img = cv.imread(r'D:\work\CCM\lightAligned.jpg', 1)
    print(img.shape)
    m = cv.selectROI(windowName="roi", img=img, showCrosshair=False, fromCenter=False)
    x, y, w, h = m
    print('m=', m)
    roi_r = img[int(m[1]):int(m[1] + m[3]), int(m[0]):int(m[0] + m[2])]
    roi_g = img[int(m[1]):int(m[1] + m[3]), int(m[0]):int(m[0] + m[2])]
    roi_b = img[int(m[1]):int(m[1] + m[3]), int(m[0]):int(m[0] + m[2])]
    Rmean = np.mean(roi_r[:, :, 2])
    Gmean = np.mean(roi_g[:, :, 1])
    Bmean = np.mean(roi_b[:, :, 0])
    print("Rmean,Gmean,Bmean","[" , int(Rmean),",",int(Gmean),",",int(Bmean),"]",",")
    Lightness = 0.3 * Rmean + 0.6 * Gmean + 0.1 * Bmean
    print("Lightness", Lightness)

# findGrayLightness()

def sort_point(points):
    """
    对四个定位点进行排序，排序后顺序分别为左上、右上、左下、右下
    :param points: 待排序的点集
    :return: 排序完成的点集
    """
    sp = sorted(points, key=lambda x: (int(x[1]), int(x[0])))
    if sp[0][0] > sp[1][0]:
        sp[0], sp[1] = sp[1], sp[0]

    if sp[2][0] > sp[3][0]:
        sp[2], sp[3] = sp[3], sp[2]

    return sp

def get_24_colors():
    data = load("./config.pkl")
    # print(data)
    pts = data["ROI"]
    img = cv.imread("./colorcard.jpg")
    color_matrix = []
    if len(pts) >= 4:
        pts = pts[:4]
        pts = sort_point(pts)
        print("ROI:", pts)
        pts = np.float32(pts)
        pos2 = np.float32([[200, 0], [200, 300], [0, 0],  [0, 300]])
        M = cv.getPerspectiveTransform(pts, pos2)
        img_persp = cv.warpPerspective(img, M, (200, 300))

        # print(img_persp.shape)
        pos_vert = [20, 70, 120, 170]
        pos_hori = [20, 70, 120, 170, 220, 270]
        for i in pos_hori:
            for j in pos_vert:
                cv.rectangle(img_persp, (j - 8, i - 8), (j + 15, i + 15), (0, 0, 255), 2)

        img_persp = np.rot90(img_persp, 1)
        for i in pos_vert:
            for j in pos_hori:
                data = img_persp[i - 5:i + 5, j - 6:j + 6]
                # print(i,j, data.shape)
                b, g, r = cv.split(data)
                color_matrix.append([int(np.mean(b)), int(np.mean(g)), int(np.mean(r))])

        color_matrix = np.array(color_matrix)
        cv.imshow("img_show", img_persp)
        cv.waitKey(0)
        cv.destroyAllWindows()
    else:
        print("未找到色卡")
    print("24 COLORS:", color_matrix)
    return color_matrix


def lightnessAdjust():
    img1 = cv.imread(r'D:\work\CCM\wb.jpg',1)
    img1[:,:,0] = img1[:,:,0] * 5.80
    img1[:,:,1] = img1[:,:,1] * 5.80
    img1[:,:,2] = img1[:,:,2] * 5.80
    cv.imwrite(r'D:\work\CCM\lightAligned.jpg', img1)

# cv.imshow("light",img1)
# cv.waitKey(0)
# cv.destroyAllWindows()
# findGrayLightness()

def degamma(img):
    img = pow((img/255),2.2)
    return img*255

from scipy.optimize import leastsq

def func(x1,x2, p):
    a, b = p
    # print(a,b)
    return x1*a + x2*b

def residuals(p, y, x1, x2):
    return y - func(x1, x2, p)

def ccmCalculation():
    standard_sRGB = np.array([[115, 194, 98,  87,  133, 103, 214, 80,  193, 94,  157, 224, 56,  70,  175, 231, 187, 8],
                  [82,  150, 122, 108, 128, 189, 126, 91,  90,  60,  188, 163, 61,  148, 54,  199, 86,  133],
                  [68,  130, 157, 67,  177, 170, 44,  166, 99,  108, 64,  46,  150, 73,  60,  31,  149, 161]])

    raw_RGB = np.array([[49, 31, 19],
                        [ 164 , 115 , 82 ] ,
                        [ 57 , 71 , 100 ] ,
                        [ 44 , 49 , 26 ] ,
                        [ 95 , 95 , 126 ] ,
                        [ 92 , 160 , 146 ] ,
                        [ 163 , 72 , 23 ] ,
                        [ 31 , 44 , 96 ] ,
                        [ 145 , 52 , 35 ] ,
                         [ 33 , 22 , 38 ] ,
                         [ 124 , 145 , 58 ] ,
                         [ 195 , 120 , 33 ] ,
                          [ 13 , 18 , 51 ] ,
                          [ 35 , 72 , 32 ] ,
                          [ 106 , 29 , 20 ] ,
                          [ 219 , 169 , 47 ] ,
                          [ 122 , 51 , 72 ] ,
                          [ 24 , 65 , 98 ]])

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

    # standard_sRGB = degamma(standard_sRGB)
    # print(standard_sRGB)

    x1 = np.array(x1)
    x2 = np.array(x2)

    p0 = [0.5, 0.5]                           # 第一次猜测的函数拟合参数

    plsq1 = leastsq(residuals, p0, args=(y1, x1, x2))
    plsq2 = leastsq(residuals, p0, args=(y2, x1, x2))
    plsq3 = leastsq(residuals, p0, args=(y3, x1, x2))

    ccm = [[plsq1[0][0],plsq1[0][1],1-plsq1[0][0]-plsq1[0][1]],
           [plsq2[0][0],plsq2[0][1],1-plsq2[0][0]-plsq2[0][1]],
           [plsq3[0][0],plsq3[0][1],1-plsq3[0][0]-plsq3[0][1]]]
    # print(ccm)
    return np.array(ccm)

def applyCCM(ccm):
    print(ccm)
    ccm = ccm.T
    img1 = cv.imread(r'D:\work\CCM\lightAligned.jpg', 1)
    wh = img1.shape
    img1 = img1.reshape(wh[0]*wh[1],3)
    # img_ccm = np.zeros(img1.shape)
    img_ccm = np.matmul(img1, ccm)
    img_ccm = img_ccm.reshape(wh[0], wh[1], 3)
    # img_ccm[:,:,0] = img1 * ccm[0]
    print(img_ccm.shape)
    # print(img1[:,:,:])
    # img_ccm[:,:,1] = img1[:,:,1] * 5.80
    # img_ccm[:,:,2] = img1[:,:,2] * 5.80
    cv.imwrite(r'D:\work\CCM\img_ccm.jpg', img_ccm)

get_24_colors()
# ccm = ccmCalculation()
# print(ccm)
# applyCCM(ccm)
# findGrayLightness()



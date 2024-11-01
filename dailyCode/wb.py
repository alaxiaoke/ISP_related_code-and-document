import copy
import cv2 as cv
import numpy as np
import cv2
import os
from os import close, read
#这里的close是关闭文件，read是读取文件

def WB_ROI():
    path = r"D:\work\IQ\gamma"
    rawList = os.listdir(path)
    for i in rawList:
        if i.split(".")[-1] == "raw":
            rawName = path + "//" + i
            raw = np.fromfile(rawName, dtype=np.uint16)
            cfa = 'bggr'#rggb/grbg/bggr/gbrg
            bpp = 10
            width = int(2328)
            # height = 1748
            height = int(1744)
            raw = raw.reshape((height,width))
            ob = np.ones(raw.shape, dtype = np.uint16) * 64*2**(bpp - 10)
            # raw = np.ones(raw.shape, dtype = np.uint16) * 1023
            img = cv2.subtract(raw, ob)
            img = np.array(img, dtype=float)
            img = img / 959 * 1023
            print("1", img[0][0], img[0][1], img[1][0], img[1][1])
            img = np.array(img, dtype=np.uint16)
            print("2", img[0][0], img[0][1], img[1][0], img[1][1])

            if cfa == 'rggb':
                img = cv.cvtColor(img, cv.COLOR_BAYER_RG2RGB)
            elif cfa == 'grbg':
                img = cv.cvtColor(img, cv.COLOR_BAYER_GR2RGB)
            elif cfa == 'bggr':
                img = cv.cvtColor(img, cv.COLOR_BAYER_BG2RGB)
            elif cfa == 'gbrg':
                img = cv.cvtColor(img, cv.COLOR_BAYER_GB2RGB)

            img = np.array(img, dtype=float)
            print("3", img[0][0], img[0][1], img[1][0], img[1][1])
            img1 = img
            img1[:,:,0] = img[:,:,0]/2**(bpp - 8)
            img1[:,:,1] = img[:,:,1]/2**(bpp - 8)
            img1[:,:,2] = img[:,:,2]/2**(bpp - 8)
            print("4", img1[0][0], img1[0][1], img1[1][0], img1[1][1])

            imgGamma = img1.astype('uint8')
            gamma = 2.0
            imgGamma = adjust_gamma(imgGamma, gamma)
            rawName = rawName + "_gamma_" + str(gamma)
            cv.imwrite(rawName + '_demosaic.jpg', imgGamma)
            # img2 = cv.imread(path + '_demosaic.jpg')

            Rmean = np.mean(imgGamma[:, :, 2])
            Gmean = np.mean(imgGamma[:, :, 1])
            Bmean = np.mean(imgGamma[:, :, 0])
            Rgain = Gmean / Rmean
            Bgain = Gmean / Bmean

            # Rgain = 1/Rgain
            # Bgain = 1/Bgain
            print(img1.shape)
            for i in range(img1.shape[0]):
                for j in range(img1.shape[1]):
                    img1[i ,j ,0] = img1[i ,j ,0]*Rgain if (img1[i ,j ,0]*Rgain) <=255 else 255
                    # img1[i ,j ,1] = img1[i ,j ,1]
                    img1[i ,j ,2] = img1[i ,j ,2]*Bgain if (img1[i ,j ,2]*Bgain) <=255 else 255

            # img1= np.clip(img1,a_min=0,a_max=255)
            img1 = img1.astype('uint8')
            img1 = adjust_gamma(img1, 2.2)
            cv.imwrite(rawName + "_" + str(Rgain) + "_" + str(Bgain) + '_wb.jpg', img1)
    # ccm_rgb = [1.254,-0.431, 0.188,           #d65
    # -0.110,1.415,-0.305,
    # -0.0369,-0.374,1.421]
    # ccm_rgb = [1.22, -0.34, 0.12,             #tl84
    # -0.15, 1.34, -0.19,
    # 0.05, -0.45, 1.40]

    # ccm_rgb = [1.32, -0.24, 0.08,               #A
    # -0.19, 1.32, -0.13,
    # 0.18, -0.58, 1.40]
    #
    # ccm = [ccm_rgb[8], ccm_rgb[7],ccm_rgb[6],
    #        ccm_rgb[5],ccm_rgb[4],ccm_rgb[3],
    #        ccm_rgb[2],ccm_rgb[1],ccm_rgb[0]]
    # 471数据：
    # H:ccm[[1.1008228225891568, 0.21691123653005545, -0.3177340591192122], [-0.508193997116603, 1.6280791081381727,-0.11988511102156973], [-0.03357709974590373,-1.2854012937118895,2.318978393457793]]
    # TL84: [1.534680557046921, -0.3070843203826519, -0.2275962366642692, -0.3279509272194943, 1.6383419564055717,-0.3103910291860774, 0.0032412061435029504,-0.650337930395597, 1.647096724252094]
    # A: [1.127789838809493, -0.025815736431265446, -0.10197410237822746, -0.3828881570185042, 1.4416998328640358, -0.058811675845531486, 0.032553756067260714, -0.6634654260132914, 1.6249116699460307]
    # cwf:ccm[[1.7170729445017774, -0.5980194779970078, -0.11905346650476956], [-0.4162258401649105, 1.5839260400515685, -0.16770019988665807], [-0.03066494867052829,-0.5109164840214209,1.5415814326919492]]
    # d65:ccm [1.59115323566714, -0.32410956848828454, -0.2670436671788555, -0.236470593770047, 1.6580412865285343, -0.42157069275848724, -0.0025168985503146724, -0.5868075113422635, 1.589324409892578]
    # d65:ccm  [1.254, -0.431, 0.188, -0.110, 1.401, -0.295, -0.0369, -0.374, 1.421]

    # JD1数据：
    # a:[1.1833032768606697, -0.2298722224869505, 0.04656894562628078, -0.4511475239699278, 1.4206948662296173, 0.030452657740310585, -0.2529969860642032, -1.0537433634151185, 2.3067403494793215]
    # d50:[1.6811486111553104, -0.6303972637389265, -0.050751347416383985, -0.29936386042510343, 1.5430938657001794, -0.24373000527507593, -0.10706800480773401, -0.6474497760793338, 1.7545177808870678]
    # cwf:[1.9285819852297068, -0.8656796305103039, -0.06290235471940286, -0.4438215678324109, 1.4011426766386077, 0.04267889119380319, -0.125621940886404, -0.675850166316683, 1.8014721072030873]

    # ccm = np.array(ccm).reshape(3, 3).T
    # # ccm = np.array(ccm).reshape(3, 3)
    # print(ccm)
    # img1 = np.dot(img1, ccm)
    # cv.imwrite(path + '_CCM_d65.jpg', img1)
    #
    # img3 = cv.imread(path + '_wb.jpg')
    # print('ASRrgain=',Rgain)
    # print('ASRbgain=',Bgain)
    # cv.namedWindow('WB', 0)
    # w = int(width/4)
    # h = int(height/4)
    # cv.resizeWindow('WB', w, h)
    # cv.imshow("WB", img3)
    # cv.waitKey(0)
    # cv.destroyAllWindows()


def adjust_gamma(image, gamma=2.2):
    invGamma = 1.0 / gamma
    table = []
    for i in range(256):
        table.append(((i / 255.0) ** invGamma) * 255)
    table = np.array(table).astype("uint8")
    return cv2.LUT(image, table)


def AWB_Cmodel():

    # import ctypes
    # dll = ctypes.cdll.LoadLibrary('D:\code\ColorConstancy\FaugerasAlgorithm\AWB.dll')
    # print(dll.hello())

    import subprocess
    import os
    main = "D:\code\ColorConstancy_dll_generate\FaugerasAlgorithm\main.exe"
    args = ["3", "11111_22222_33333"]
    if os.path.exists(main):
        process = subprocess.Popen([main] + args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

        output, err = process.communicate()
        print("output", output)
        print('*' * 10)
        print("err", err)

#鼠标选区域做白平衡的函数在这
def WB_single_JPG():
    path = r'D:\code\libfacedetection-master\AR_FaceDetection\images\\'
    img = cv.imread(path + '\\' + '24.jpg')

    m = cv.selectROI(windowName="roi", img=img, showCrosshair=False, fromCenter=False)
    x, y, w, h = m
    print('m=',m)
    # cv.imwrite(path + '\\' + 'wb.jpg', m)

    roi_r = img[int(m[1]):int(m[1]+m[3]),int(m[0]):int(m[0]+m[2])]
    roi_g = img[int(m[1]):int(m[1]+m[3]),int(m[0]):int(m[0]+m[2])]
    roi_b = img[int(m[1]):int(m[1]+m[3]),int(m[0]):int(m[0]+m[2])]

    Rmean = np.mean(roi_r[:,:,2])
    Gmean = np.mean(roi_g[:,:,1])
    Bmean = np.mean(roi_b[:,:,0])
    Rgain = Gmean/Rmean
    Bgain = Gmean/Bmean
    print('rgain=',1/Rgain, ',bgain=',1/Bgain)

    imgNew = np.zeros((img.shape[0], img.shape[1], 3), dtype=np.uint16)
    imgNew[:,:,0] = img[:,:,0]*Bgain
    imgNew[:,:,1] = img[:,:,1]
    imgNew[:,:,2] = img[:,:,2]*Rgain
    imgNew = np.clip(imgNew, 0, 255)
    imgNew = imgNew.astype('uint8')
    imgNew = adjust_gamma(imgNew, 2.2)

    cv.imwrite(path + '\\' + 'wb.jpg', imgNew)
    img3 = cv.imread(path + '\\' + 'wb.jpg')
    print('ASRrgain=',Rgain*128)
    print('ASRbgain=',Bgain*128)
    cv.namedWindow('WB',0)
    cv.imshow("WB",img3)
    cv.waitKey(0)
    cv.destroyAllWindows()


def demasic():

    path = r'D:\data\shanghai_platform_captured\\'
    num = [2,3,4,5,6,7]
    lr = ["\\L", "\\R"]
    for n in num:
        for l in lr:

            save_path = r'D:\data\shanghai_platform_captured\\' + str(n) + l + '_demasic'
            path_n = r'D:\data\shanghai_platform_captured\\' + str(n) + l
            rawList = os.listdir(path_n)
            cfa = 'rggb'#rggb/grbg/bggr/gbrg
            bpp = 10
            width = 2328
            height = 1744

            for i in rawList:
                print('raw name =', i)
                raw = np.fromfile(path_n + "\\" + i, dtype=np.uint16)

                raw = raw.reshape((height,width))
                ob = np.ones(raw.shape, dtype = np.uint16) * 64*2**(bpp - 10)
                img = cv2.subtract(raw, ob)

                if cfa == 'rggb':
                    img = cv.cvtColor(img, cv.COLOR_BAYER_RG2RGB)
                elif cfa == 'grbg':
                    img = cv.cvtColor(img, cv.COLOR_BAYER_GR2RGB)
                elif cfa == 'bggr':
                    img = cv.cvtColor(img, cv.COLOR_BAYER_BG2RGB)
                elif cfa == 'gbrg':
                    img = cv.cvtColor(img, cv.COLOR_BAYER_GB2RGB)

                img1 = img
                img1[:,:,0] = img[:,:,0]/2**(bpp - 8)
                img1[:,:,1] = img[:,:,1]/2**(bpp - 8)
                img1[:,:,2] = img[:,:,2]/2**(bpp - 8)

                img_name = i.split(".")[0]
                print(save_path + '\\' + img_name +'.jpg')
                cv.imwrite(save_path + '\\' + img_name +'.jpg', img1)


def demasic2():

    path = r"D:\\data\\demo-face-cc\\"

    save_path = r"D:\\data\\out\\"
    rawList = os.listdir(path)
    cfa = 'rggb'#rggb/grbg/bggr/gbrg
    bpp = 10
    width = 2328
    height = 1744
    # width = 2920
    # height = 3000

    for i in rawList:
        if i.split(".")[-1] == "raw":
            print('raw name =', i)
            raw = np.fromfile(path + i, dtype=np.uint16)

            raw = raw.reshape((height,width))
            ob = np.ones(raw.shape, dtype = np.uint16) * 64*2**(bpp - 10)
            img = cv2.subtract(raw, ob)

            if cfa == 'rggb':
                img = cv.cvtColor(img, cv.COLOR_BAYER_RG2RGB)
            elif cfa == 'grbg':
                img = cv.cvtColor(img, cv.COLOR_BAYER_GR2RGB)
            elif cfa == 'bggr':
                img = cv.cvtColor(img, cv.COLOR_BAYER_BG2RGB)
            elif cfa == 'gbrg':
                img = cv.cvtColor(img, cv.COLOR_BAYER_GB2RGB)

            img1 = img
            img1[:,:,0] = img[:,:,0]/2**(bpp - 8)
            img1[:,:,1] = img[:,:,1]/2**(bpp - 8)
            img1[:,:,2] = img[:,:,2]/2**(bpp - 8)

            img_name = i
            print(save_path + '\\' + img_name +'.jpg')
            cv.imwrite(save_path + '\\' + img_name +'.jpg', img1)



def align():
    pathA = r'D:\code\data\4-oneside_code_light_oneside_warm_light\25_demasic\\'
    rawListA = os.listdir(pathA)
    pathB = r'D:\code\data\4-oneside_code_light_oneside_warm_light\56_demasic\\'
    rawListB = os.listdir(pathB)
    rawList = [i for i in rawListA if i in rawListB]

    for img in rawListA:
        if img not in rawList:
            os.remove(pathA + img)
    for img in rawListB:
        if img not in rawList:
            os.remove(pathB + img)


def saveGain():
    path = r'D:\code\data\4-oneside_code_light_oneside_warm_light\25_demasic\\'
    rawListA = os.listdir(path)
    txt_path = r'D:\code\data\4-oneside_code_light_oneside_warm_light\25_gt\gt.txt'
    txt = open(txt_path, 'w')
    rawList = [int(i.split(".")[0]) for i in rawListA]
    rawList.sort()

    for i in rawList:
        img1 = cv.imread(path + '\\' + str(i) + ".jpg")
        Rmean = np.mean(img1[:, :, 2])
        Gmean = np.mean(img1[:, :, 1])
        Bmean = np.mean(img1[:, :, 0])
        Rgain = Gmean / Rmean
        Bgain = Gmean / Bmean
        # print('rgain=', Rgain)
        # print('bgain=', Bgain)
        txt.write(str(i) + " " + str(Rgain) + " " + str(Bgain) + '\n')
        txt.flush()

# WB_ROI()

def number_gamma(gamma=1.):
    invGamma = 1.0 / gamma
    table = [15, 80, 160]
    for i in table:
        i_gamma = ((i / 255.0) ** invGamma) * 255
        print(i_gamma)




WB_single_JPG()

# demasic2()
# align()
# saveGain()
# AWB_Cmodel()
# WB_single_JPG()


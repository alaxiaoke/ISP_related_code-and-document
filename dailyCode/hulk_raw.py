import os
import numpy as np
import argparse
import time
import cv2

global bayer_order_maps

bayer_order_maps = {
    "bayer_bg": cv2.COLOR_BayerBG2RGB,
    "bayer_gb": cv2.COLOR_BayerGB2RGB,
    "bayer_rg": cv2.COLOR_BayerRG2RGB,
    "bayer_gr": cv2.COLOR_BayerGR2RGB,
    "gray": 0,
}

def unpack_mipi_raw10(byte_buf):
    data = np.frombuffer(byte_buf, dtype=np.uint8)
    b1, b2, b3, b4, b5 = np.reshape(
        data, (data.shape[0]//5, 5)).astype(np.uint16).T
    p1 = (b1) + ((b2& 0x3)<<8)
    p2 = ((b2 & 0xfc) >> 2) + ((b3 & 0xf) << 6)
    p3 = ((b3 & 0xf0) >> 4) + ((b4 & 0x3f) << 4)
    p4 = ((b4 & 0xc0) >> 6) + (b5 << 2)
    unpacked = np.reshape(np.concatenate(
        (p1[:, None], p2[:, None], p3[:, None], p4[:, None]), axis=1),  4*p1.shape[0])
    return unpacked

def convertMipi2Raw(mipiFile, imgWidth, imgHeight, bitDeepth, footer,bayer_order=cv2.COLOR_BayerBG2BGR):
    mipiData = np.fromfile(mipiFile, dtype='uint8')
    if footer > 0:
        mipiData = mipiData[:-footer]
        mipiData = mipiData.reshape(1736,2256)
        mipiData = mipiData[:,:-6]
    else:
        mipiData = mipiData.reshape(1736,2256)
        mipiData = mipiData[:,:-6]
    print("mipiraw file size:", mipiData.size)

    if bitDeepth == 8 or bitDeepth == 16:
        print("raw8 and raw16 no need to unpack")
        return
    elif bitDeepth == 10:
        # raw10
        mipiData = mipiData.reshape(int(imgWidth * 10 /8) * imgHeight)
        bayerData = unpack_mipi_raw10(mipiData)
        img = bayerData >> 2
        print("bayer bitDeepth:", bitDeepth)

    bayerData.tofile(mipiFile[:-4]+'_unpack.raw')

    img = img.astype(np.uint8).reshape(imgHeight, imgWidth, 1)
    print(bayer_order)
    rgbimg = cv2.cvtColor(img, bayer_order)
    # cv2.imwrite(mipiFile[:-4]+'_unpack.jpg', rgbimg)


def ProcSingleFile(rawFile, img_width, img_height, rawDepth, footer,bayer_order):
    print("process ", rawFile, "...")
    convertMipi2Raw(rawFile, img_width, img_height, rawDepth,footer, bayer_order)


def ProcPath(path, img_width, img_height, rawDepth, footer,bayer_order):
    print('procesing ', path)
    file_list = os.listdir(path)
    for f in file_list:
        f_lower = f.lower()
        if f_lower.endswith('.raw'):
            raw_name = '%s\%s' % (path, f)
            ProcSingleFile(raw_name, img_width, img_height,
                           rawDepth, footer,bayer_order)


if "__main__" == __name__:
    parser = argparse.ArgumentParser()

    # parser.add_argument(
    #     "--path", help="input raw path", required=False, type=str)
    # parser.add_argument(
    #     "--file", help="input raw file name", required=False, type=str)

    # args = parser.parse_args()

    rawPath = r'D:\data\hulkIMG\\'
    files = os.listdir(rawPath)
    for file in files:
        if os.path.splitext(file)[1] == ".raw":
            rawFile = np.fromfile(rawPath+file, dtype=np.uint16)
    img_width = 1800
    img_height = 1736
    rawDepth = 10
    footer = 0#512
    bayer = 'bayer_bg'

    bayer_order = bayer_order_maps[bayer.lower()]

    if rawPath is not None:
        ProcPath(rawPath, img_width, img_height, rawDepth,footer, bayer_order)
    elif rawFile is not None:
        ProcSingleFile(rawFile, img_width, img_height, rawDepth,footer, bayer_order)
    else:
        print("parameters wrong!!! no path or file")

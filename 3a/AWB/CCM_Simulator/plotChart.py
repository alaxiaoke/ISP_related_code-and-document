#!/usr/bin/env python3

from numpy.random import *
import numpy as np
import cv2 as cv
from PIL import Image, ImageDraw, ImageFont
import matplotlib.pyplot as plt

class Compare():
    def __init__(self):
        print("plot compare")

    def csvfile2nparray(self, f):
        str_data = f.read()
        lines = str_data.replace(' ', '').split('\n')
        del lines[len(lines) - 1]

        data = list()
        cells = list()

        for i in range(len(lines)):
            cells.append(lines[i].split(','))

        start_row = 0
        if not cells[0][0].replace(".","",1).isdigit():
            del cells[0]
            start_row = 1

        i = 0
        for line in cells:
            data.append(list())
            for j in range(start_row, len(line)):
                data[i].append(float(line[j]))
            i += 1
        # print(data)

        return np.asarray(data, dtype=np.float32)

    def loadCCM(self, ccmCsvFile) :
        csvData = ccmCsvFile.read()
        lines = csvData.replace(' ', '').split('\n')
        del lines[len(lines) - 1]

        data = list()
        cells = list()

        for i in range(len(lines)):
            cells.append(lines[i].split(','))

        i = 0
        for line in cells:
            data.append(list())
            for j in range(len(line)):
                data[i].append(float(line[j]))
            i += 1

        return np.asarray(data)

    def drawChartComparison(self, reference, corrected, matchRatio):
        offset = 15
        patchSize = 100
        patchHalfsize = patchSize / 2
        width = offset + (patchSize + offset) * 6
        height = offset + (patchSize + offset) * 4
        im = Image.new("RGB", (width, height), (255, 255, 255))
        draw = ImageDraw.Draw(im)

        for i in range(len(reference)):
            ix = i % 6
            iy = int(i / 6)
            rx = offset + (patchSize + offset) * ix
            ry = offset + (patchSize + offset) * iy
            draw.rectangle((rx, ry, rx + patchSize, ry + patchHalfsize),
                           fill=(int(reference[i][0] * 255),
                                 int(reference[i][1] * 255),
                                 int(reference[i][2] * 255)))
            draw.rectangle((rx, ry + patchHalfsize, rx + patchSize, ry + patchSize),
                           fill=(int(corrected[i][0] * 255),
                                 int(corrected[i][1] * 255),
                                 int(corrected[i][2] * 255)))
            draw.multiline_text((rx + patchHalfsize - 10, ry + 2 + patchSize),
                                '{0:3.1f}%'.format(matchRatio[i]),
                                fill=(0, 0, 0))
        return im

    def saveResultImg(self, chart, graph, filename):
        offset = 0
        dst = Image.new('RGB', (max(chart.width, graph.width) + offset,
                                chart.height + graph.height + offset),
                        (255, 255, 255))
        dst.paste(chart, (0, 0))
        dst.paste(graph, (0, chart.height + offset))
        dst.save('{}.png'.format(filename))

    def sRGB2XYZ(self, rgbList):
        # D 50
        # M = np.array([[0.4360747  0.3850649  0.1430804]
        #                  [0.2225045  0.7168786  0.0606169]
        #                  [0.0139322  0.0971045  0.7141733]])
        # D 65
        M = np.array([[0.412391, 0.357584, 0.180481],
                      [0.212639, 0.715169, 0.072192],
                      [0.019331, 0.119195, 0.950532]])
        xyzList = []
        for rgb in rgbList:
            # (r, g, b)
            xyz = np.dot(M, rgb.transpose())
            xyzList.append(xyz.transpose())
        return np.asarray(xyzList)

    def XYZ2sRGB(self, rgbList):
        # D 50
        # M = np.array([[3.1338561 -1.6168667 -0.4906146]
        #                  [-0.9787684  1.9161415  0.0334540]
        #                  [0.0719453 -0.2289914  1.4052427]])
        # D 65
        M = np.array([[3.240970, -1.537383, -0.498611],
                      [-0.969244, 1.875968, 0.041555],
                      [0.055630, -0.203977, 1.056972]])
        xyzList = []
        for rgb in rgbList:
            # (r, g, b)
            xyz = np.dot(M, rgb.transpose())
            xyzList.append(xyz.transpose())
        return np.asarray(xyzList)

    def correctChartXYZ(self, source, ccm):
        sourceXYZ = self.sRGB2XYZ(source)
        correctedSource = []
        sourceXYZ = np.append(sourceXYZ, np.ones((24, 1)), axis=1)
        correctedSource = np.dot(sourceXYZ, ccm)
        return self.XYZ2sRGB(correctedSource)

    def correctChart(self, source, ccm):
        correctedSource = np.dot(source, ccm)
        return correctedSource

    def plotCompare(self, source, reference, ccm):
        print("source:", source)
        correctedSource = self.correctChart(source, ccm)
        print("correctedSource:", correctedSource)

        # reference = np.array([[96, 106, 166], [142, 173, 224], [191, 118, 39], [72, 145, 83], [199, 112, 112], [168, 180, 50], [57, 133, 218], [173, 68, 29], [97, 85, 221], [137, 62, 99], [0, 194, 172], [5, 175, 224], [165, 58, 1], [66, 151, 0], [73, 47, 215], [0, 210, 222], [137, 32, 214], [212, 138, 1], [211, 207, 200], [191, 182, 169], [167, 155, 150], [130, 116, 110], [101, 82, 75], [42, 35, 32]])
        ref = []
        for i in range(24):
            ref.append([reference[0][i], reference[1][i], reference[2][i]])
        ref = np.array(ref)/255

        # reference = correctedSource*0.92/255
        correctedSource = correctedSource/255
        # print("reference", reference.shape)

        matchRatio = np.multiply(np.add(np.divide(np.subtract(correctedSource, ref).sum(axis=1),
                                                  3),
                                        0),
                                 100)
        print(matchRatio.shape)
        print(matchRatio)
        diffIm = self.drawChartComparison(ref, correctedSource, matchRatio)

        plt.ylim([-15, 15])
        plt.axes().yaxis.grid(True)
        plt.xlim([-1, 24])
        plt.hlines([0], -1, 25, "red")
        plt.bar(np.arange(len(matchRatio)), matchRatio, align="center", width=0.7)
        plt.vlines([5.5, 11.5, 17.5, 23.5], -20, 20, "red")
        plt.xlabel("Patch")
        plt.ylabel("Match %")
        plt.savefig('graph.png')

        graphIm = Image.open('./graph.png', 'r')
        self.saveResultImg(diffIm, graphIm, "output")
        # image = cv.imread("./output.png")
        # cv.imshow("img_show", image)
        # cv.waitKey(0)
        # cv.destroyAllWindows()



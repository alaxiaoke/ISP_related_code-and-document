
import cv2 as cv
import numpy
import numpy as np
import cv2
import matplotlib.pyplot as plt
import os
from os import close, read
import matplotlib.patches as mpatches

f = open("./AWBParam.txt","rb")
data = f.readlines()[0]
params_list = []
# print(type(data))
# print(len(data))
for i in range(0, len(data)-4, 4):
    bt = data[i:i+4][::-1]
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

lightSourceTable = numpy.array(params_list[:lightSourceNum*2]).reshape(lightSourceNum,2)
print("lightSourceTable:", lightSourceTable)
cctWeightTable = numpy.array(params_list[lightSourceNum*2 :lightSourceNum*2 + LightLevel* CCTLevel]).reshape(LightLevel,CCTLevel)
print("cctWeightTable:", cctWeightTable)
lvWeight = numpy.array(params_list[lightSourceNum*2 + LightLevel* CCTLevel :lightSourceNum*2 + LightLevel* CCTLevel +5])
print("lvWeight:", lvWeight)
index = lightSourceNum*2 + LightLevel* CCTLevel +5
greyRegions = numpy.array(params_list[index:index + lightSourceNum * 8]).reshape(lightSourceNum,8)
print("greyRegions:", greyRegions)
index = index + lightSourceNum * 8
colorRegions = numpy.array(params_list[index:index + colorRegionsCount * 8]).reshape(colorRegionsCount,8)
print("colorRegions:", colorRegions)
index = index + colorRegionsCount * 8
misColorRegions = numpy.array(params_list[index:index + misColorRegionsCount * 8]).reshape(misColorRegionsCount,8)
print("misColorRegions:", misColorRegions)
index = index + misColorRegionsCount * 8
colorRegionsParams = numpy.array(params_list[index:index + 15]).reshape(5,3)
print("colorRegionsParams:", colorRegionsParams)

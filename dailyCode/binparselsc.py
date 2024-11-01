from os.path import join
from os.path import exists
from os import mkdir
from os import walk
from os import listdir
import numpy as np

def readShunyuBin():
    colortable = ['lsc table r', 'lsc table gr', 'lsc table gb', 'lsc table b']

    rows = 13
    cols = 17
    trows = rows
    tcols = cols
    total = rows * cols

    rtable = [0 for i in range(total)]
    grtable = [0 for i in range(total)]
    gbtable = [0 for i in range(total)]
    btable = [0 for i in range(total)]
    awbTable = [0 for i in range(16)]

    binpath = r'D:\work\otp\OV64B_otp_data'
    allbin = [x for x in listdir(binpath) if x.endswith('.bin')]
    for each in allbin:
        print("bin name:",each)
        data = np.fromfile(binpath + '/' + each, dtype=np.uint8)
        sensorID= data[0]
        print("sensorID", sensorID)

        no = data[12] + (data[13] << 8) + (data[14] << 16) + (data[15] << 24)
        print('\n')
        print("unique identifier",no)

        no = data[16]
        print('\n')
        print('Flag of awb')
        print(no)

        year = data[6]
        month = data[7]
        day = data[8]
        print("date:",year,"-", month, "-", day)

        print('awb')
        index = 17
        # for i in range(15):
        #     print('index：',index)
        #     print(data[index])
        #     print(data[index + 1])
        #     awbTable[i] = data[index] + (data[index + 1] << 8)
        #     print((data[index + 1] << 8))
        #     print(awbTable[i])
        #     index += 2
        # print(awbTable)

        # index = 33
        # for i in range(total):
        #     rtable[i] = (data[index] << 8) + data[index + 1]
        #     grtable[i] = (data[index + 2] << 8) + data[index + 3]
        #     gbtable[i] = (data[index + 4] << 8) + data[index + 5]
        #     btable[i] = (data[index + 6] << 8) + data[index + 7]
        #     index += 8
        #
        # print('r')
        # for h in range(rows):
        #     line = ''
        #     for w in range(cols):
        #         line = line + '{: >4}, '.format(rtable[h * cols + w])
        #     print(line)
        # print('\n')
        #
        # print('gr')
        # for h in range(rows):
        #     line = ''
        #     for w in range(cols):
        #         line = line + '{: >4}, '.format(grtable[h * cols + w])
        #     print(line)
        # print('\n')
        #
        # print('gb')
        # for h in range(rows):
        #     line = ''
        #     for w in range(cols):
        #         line = line + '{: >4}, '.format(gbtable[h * cols + w])
        #     print(line)
        # print('\n')
        #
        # print('b')
        # for h in range(rows):
        #     line = ''
        #     for w in range(cols):
        #         line = line + '{: >4}, '.format(btable[h * cols + w])
        #     print(line)
        # print('\n')



# def readAWBstats():
#     rows = 32
#     cols = 32
#     total = rows * cols
#
#     rtable = [0 for i in range(total)]
#     grtable = [0 for i in range(total)]
#     gbtable = [0 for i in range(total)]
#     btable = [0 for i in range(total)]
#     awbTable = [0 for i in range(16)]
#
#     binpath = r"D:\code\AWB_tuning_tool\stats+lsc\AWB_algo_based_on_Altek_frame\Rg_Bg_data.bin"
#
#     import struct
#
#     # 打开二进制文件
#     with open(binpath, 'rb') as bin_file:
#         # 读取二进制数据
#         data = bin_file.read()
#     # integer = struct.unpack('i', data)[0]
#     print('integer:', len(data))
#     print(data)
#     i = 0
#     int_data = []
#     for x in range(0,len(data),20):
#         # print(x)
#         # integer_data = struct.unpack('i', data[x:x+4])[0]
#         # print(integer_data)
#         # int_data.append(integer_data)
#
#
#         rtable[i] = struct.unpack('i', data[x:x+4])[0]
#         grtable[i] = struct.unpack('i', data[x+4:x+8])[0]
#         gbtable[i] = struct.unpack('i', data[x+8:x+12])[0]
#         btable[i] = struct.unpack('i', data[x+12:x+16])[0]
#
#         print(rtable[i], grtable[i], gbtable[i],  btable[i])
#         i+=1
#
#     print(i, rtable)


# def runEXE():
#     import subprocess
#     # 定义exe文件的路径
#     exe_path = r"D:\code\AWB_tuning_tool\stats+lsc\AWB_algo_based_on_Altek_frame\x64\Release\AWBDll.exe"
#     # 定义参数列表
#     arguments = [r"D:\code\AWB_tuning_tool\stats+lsc\AWB_algo_based_on_Altek_frame\\Frame_2_RAW[BIT10]_2024-07-17 14-38-31.raw"]
#     # 调用外部程序，并传入参数
#     process = subprocess.run([exe_path] + arguments)
#     # return_code = process.returncode
#     stdout = process.stdout
#     print(f"stdout code: {stdout}")

# readAWBstats()
readShunyuBin()

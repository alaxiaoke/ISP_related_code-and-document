

def read_write_gt_txt():
    AWB_sequence_l = {}
    AWB_sequence_r = {}
    left_gt_path = r"D:\data\shanghai_platform_captured\20230829\Raw_2\1_gt\gt.txt"
    right_gt_path = r"D:\data\shanghai_platform_captured\20230829\Raw_2\2_gt\gt.txt"
    with open(left_gt_path, mode='r') as infile:
        lines = infile.readlines()
        for i in range(len(lines)):
            print(i)
            line1 = lines[i].split(" ")
            # print(line1)
            Bgain = float(line1[2])
            Rgain = float(line1[1])
            AWB_sequence_l[i] = [Rgain, Bgain]

    with open(right_gt_path, mode='r') as infile:
        lines = infile.readlines()
        for i in range(len(lines)):
            print(i)
            line1 = lines[i].split(" ")
            # print(line1)
            Bgain = float(line1[2])
            Rgain = float(line1[1])
            AWB_sequence_r[i] = [Rgain, Bgain]

    txt_path_l = r'D:\data\shanghai_platform_captured\20230829\Raw_2\1_gt_new\gt.txt'
    txt_path_r = r'D:\data\shanghai_platform_captured\20230829\Raw_2\2_gt_new\gt.txt'
    txt_l = open(txt_path_l, 'w')
    txt_r = open(txt_path_r, 'w')
    eyes_consistency = EyesConsistensy()
    for frame in range(AWB_sequence_l.__len__()):
        left_rg, left_bg, right_rg, right_bg = eyes_consistency.oneFrameCombine(AWB_sequence_l[frame][0], AWB_sequence_l[frame][1], AWB_sequence_r[frame][0], AWB_sequence_r[frame][1])
        print("left_rg, left_bg, right_rg, right_bg:", left_rg, left_bg, right_rg, right_bg)
        txt_l.write(str(frame) + " " + str(left_rg) + " " + str(left_bg) + '\n')
        txt_l.flush()
        txt_r.write(str(frame) + " " + str(right_rg) + " " + str(right_bg) + '\n')
        txt_r.flush()


class EyesConsistensy(object):  # 继承Frame类
    def __init__(self):
        self.history_gain_l_rg = []
        self.history_gain_l_bg = []
        self.history_gain_r_rg = []
        self.history_gain_r_bg = []

    # 追加日志
    def oneFrameCombine(self, left_rg, left_bg, right_rg, right_bg):
        if len(self.history_gain_r_bg) < 30:
            self.history_gain_l_rg.append(left_rg)
            self.history_gain_l_bg.append(left_bg)
            self.history_gain_r_rg.append(right_rg)
            self.history_gain_r_bg.append(right_bg)
            return left_rg, left_bg, right_rg, right_bg
        else:
            del self.history_gain_l_rg[0]
            del self.history_gain_l_bg[0]
            del self.history_gain_r_rg[0]
            del self.history_gain_r_bg[0]
            delta = abs(left_rg - right_rg) + abs(left_bg - right_bg)
            if delta < 0.02:
                self.history_gain_l_rg.append(left_rg)
                self.history_gain_l_bg.append(left_bg)
                self.history_gain_r_rg.append(right_rg)
                self.history_gain_r_bg.append(right_bg)
                return left_rg, left_bg, right_rg, right_bg
            else:
                avg_left_rg = sum(self.history_gain_l_rg) / 30
                avg_left_bg = sum(self.history_gain_l_bg) / 30
                avg_right_rg = sum(self.history_gain_r_rg) / 30
                avg_right_bg = sum(self.history_gain_r_bg) / 30
                left_delta = abs(left_rg - avg_left_rg) + abs(left_bg - avg_left_bg)
                right_delta = abs(right_rg - avg_right_rg) + abs(right_bg - avg_right_bg)
                if left_delta < right_delta:
                    self.history_gain_l_rg.append(left_rg)
                    self.history_gain_l_bg.append(left_bg)
                    self.history_gain_r_rg.append(left_rg)
                    self.history_gain_r_bg.append(left_bg)
                    return left_rg, left_bg, left_rg, left_bg
                else:
                    self.history_gain_l_rg.append(right_rg)
                    self.history_gain_l_bg.append(right_bg)
                    self.history_gain_r_rg.append(right_rg)
                    self.history_gain_r_bg.append(right_bg)
                    return right_rg, right_bg, right_rg, right_bg


read_write_gt_txt()
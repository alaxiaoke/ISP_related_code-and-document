import numpy as np
import matplotlib.pyplot as plt
import cv2 as cv2
from numba import jit, prange


def rgb2uvy(rgb) -> np.ndarray:
    rgb_array = np.array(rgb)
    assert rgb_array.shape[1] == 3, "Wrong input array shape"

    I_u = np.log(rgb_array[:, 1] / rgb_array[:, 0])
    I_v = np.log(rgb_array[:, 1] / rgb_array[:, 2])
    I_y = np.linalg.norm(rgb_array, axis=1)
    I_uv = np.array([I_u, I_v, I_y]).T
    # print("rgb,uv:", rgb, I_uv)
    return I_uv


#这个函数并不是通常理解的逆转换，而是一种非线性规范化后的rgb
def illum_uvy2illum_rgb(illum_uv):
    illum_uv = np.array(illum_uv)
    z = np.sqrt(np.exp(-illum_uv[:, 0]) ** 2 + np.exp(-illum_uv[:, 1]) ** 2 + 1)
    r = np.exp(-illum_uv[:, 0]) / z
    g = 1 / z
    b = np.exp(-illum_uv[:, 1]) / z
    return np.array([r, g, b]).T

class Histogram:
    def __init__(self, epsilon, u_range=None, v_range=None):
        assert epsilon > 0
        self.epsilon = epsilon
        self.u_range = u_range
        self.v_range = v_range
        self.num_ticks_u = int(round((self.u_range[1] - self.u_range[0]) / self.epsilon))
        self.num_ticks_v = int(round((self.v_range[1] - self.v_range[0]) / self.epsilon))

        self.u_coords = np.linspace(*self.u_range, self.num_ticks_u)
        self.v_coords = np.linspace(*self.v_range, self.num_ticks_v)

    
    def get_hist(self, uvy_array, is_normalised=True):
        uvy_array = np.array(uvy_array)
        assert uvy_array.shape[1] == 3, "Wrong input array shape"
        uvy_array = uvy_array / self.epsilon
        uvy_array[:,:2] = np.round(uvy_array[:,:2])
        
        u_shift = int(round(self.u_range[0] / self.epsilon))
        v_shift = int(round(self.v_range[0] / self.epsilon))

        # hist = np.zeros((num_ticks_u, num_ticks_v))

        # for point in uvy_array:
        #     # print(point[0], u_shift, point[0] - u_shift)
        #     hist[int(point[0]) - u_shift, int(point[1]) - v_shift] += point[2]

        hist = _count_hist(uvy_array, self.num_ticks_u, self.num_ticks_v, u_shift, v_shift)

        if is_normalised:
            hist = np.sqrt(hist / hist.sum())
        return hist
    
    def get_hist_numpy(self, uvy_array, is_normalised=True):
        uvy_array = np.array(uvy_array)
        assert uvy_array.shape[1] == 3, "Wrong input array shape"
        h, u_coords, v_coords = np.histogram2d(
            uvy_array[:, 0], 
            uvy_array[:, 1], 
            range=(self.u_range, self.v_range), 
            bins=(int((self.u_range[1] - self.u_range[0]) / self.epsilon), int((self.u_range[1] - self.u_range[0]) / self.epsilon)),
            density=False)
        h = h.astype(np.float64)

        if is_normalised:
            h = np.sqrt(h / np.sum(h))

        return h, u_coords, v_coords
    # def get_colorhist_numpy(self, uvy_array, rgb_array, is_normalised=True):


    def get_colorhist(self, uvy_array, rgb_array, is_normalised=True):
        uvy_array = np.array(uvy_array)
        assert uvy_array.shape[1] == 3, "Wrong input array shape"
        uvy_array = uvy_array / self.epsilon
        uvy_array[:,:2] = np.round(uvy_array[:,:2])
        # num_ticks_u = int(round((self.u_range[1] - self.u_range[0]) / self.epsilon)) + 1
        # num_ticks_v = int(round((self.v_range[1] - self.v_range[0]) / self.epsilon)) + 1

        # u_coords = np.linspace(*self.u_range, num_ticks_u)
        # v_coords = np.linspace(*self.v_range, num_ticks_v)

        u_shift = int(round(self.u_range[0] / self.epsilon))
        v_shift = int(round(self.v_range[0] / self.epsilon))

        # hist = np.zeros((num_ticks_u, num_ticks_v))
        # colorhist = np.zeros((num_ticks_u, num_ticks_v, 3))
        # numbers = np.zeros((num_ticks_u, num_ticks_v))
        
        # for point, rgb in zip(uvy_array, rgb_array):
        #     # print(point[0], u_shift, point[0] - u_shift)
        #     hist[int(point[0]) - u_shift, int(point[1]) - v_shift] += point[2]
        #     colorhist[int(point[0]) - u_shift, int(point[1]) - v_shift] += rgb.astype(np.float64)
        #     numbers[int(point[0]) - u_shift, int(point[1]) - v_shift] += 1
        
        hist, colorhist, numbers = _count_color_hist(uvy_array, rgb_array, self.num_ticks_u, self.num_ticks_v, u_shift, v_shift)
        numbers[numbers == 0] = 1
        colorhist = colorhist / numbers[:, :, None]
        colorhist /= 255
        
        if is_normalised:
            hist = np.sqrt(hist / hist.sum())
            
        colorhist *= hist[:, :, None]
        colorhist /= np.amax(colorhist)
        
        return colorhist

@jit(parallel=True, nopython=True, fastmath=True, cache=True)
def _count_hist(uvy_array, num_ticks_u, num_ticks_v, u_shift, v_shift):
    hist = np.zeros((num_ticks_u, num_ticks_v))

    for i in prange(len(uvy_array)):
        # print(point[0], u_shift, point[0] - u_shift)
        if 0 <= int(uvy_array[i][0]) + u_shift < num_ticks_u and 0 <= int(uvy_array[i][1]) + v_shift < num_ticks_v:
            hist[int(uvy_array[i][0]) + u_shift, int(uvy_array[i][1]) + v_shift] += uvy_array[i][2]
    return hist

@jit(parallel=True, nopython=True, fastmath=True, cache=True)
def _count_color_hist(uvy_array, rgb_array, num_ticks_u, num_ticks_v, u_shift, v_shift):
    hist = np.zeros((num_ticks_u, num_ticks_v))
    colorhist = np.zeros((num_ticks_u, num_ticks_v, 3))
    numbers = np.zeros((num_ticks_u, num_ticks_v))
    
    for i in prange(len(uvy_array)):
        # print(point[0], u_shift, point[0] - u_shift)
        hist[int(uvy_array[i][0]) - u_shift, int(uvy_array[i][1]) - v_shift] += uvy_array[i][2]
        colorhist[int(uvy_array[i][0]) - u_shift, int(uvy_array[i][1]) - v_shift] += rgb_array[i].astype(np.float64)
        numbers[int(uvy_array[i][0]) - u_shift, int(uvy_array[i][1]) - v_shift] += 1

    # numbers[numbers == 0] = 1
    # for i in prange(len(numbers)):
    #     for j in prange(len(numbers[i])):
    #         if numbers[i][j] != 0:
    #             for k in prange(len())
    #             colorhist[i][j] = colorhist[i][j] / np.array([numbers[i], numbers[i], numbers[i]])
    # numbers = np.expand_dims(numbers, -1)
    # print(colorhist.shape, numbers.shape)
    # colorhist = colorhist / numbers#[:, :, None]
    
    # colorhist /= 255
    return hist, colorhist, numbers


def test3():
    img1 = cv2.imread('datasets/Color-checker (by Shi) (2010)/png_canon1d/cs/chroma/data/canon_dataset/568_dataset/png/8D5U5524.png')
    img1 = cv2.cvtColor(img1, cv2.COLOR_BGR2RGB).astype(np.float64)
    img1 /= np.amax(img1)
    plt.imshow(img1)
    plt.show()
    print(img1)
    # img1 *= 255
    img1[img1 == 0] = 1
    img1_uvy = rgb2uvy(img1)
    hist = Histogram(0.0125, (-256 * 0.025, 256 * 0.025), (-256 * 0.025, 256 * 0.025))
    h, _, _ = hist.get_hist(img1_uvy)
    plt.figure(figsize=(15, 15))
    plt.imshow(h)
    plt.show()

def test4():
    img1 = cv2.imread('/media/kvsoshin/Transcend/Work/cube++/SimpleCube++/train/PNG/00_0044.png')
    img1 = cv2.cvtColor(img1, cv2.COLOR_BGR2RGB)
    plt.imshow(img1)
    plt.show()
    illum = np.array([0.20305266848652131,0.4707563192426342, 0.3261910122708445])
    img1_illum = img1.astype(np.float64) / illum
    img1_illum = np.uint8(img1_illum / np.amax(img1_illum) * 255)
    plt.imshow(img1_illum)
    plt.show()
    # exit()
    # print(img1.max())
    # exit()
    img1 = img1.reshape((-1, 3))
    img2 = []
    # for c in img1:
    #     if not 0 in c:
    #         img2.append(c)
    img2 = img1[~np.all(img1 == 0, axis=1)]
    img2 = np.array(img2)
    img1_uvy = rgb2uvy(img2)
    print(img1_uvy.shape)
    
    
    img1_illum1 = img1_illum.reshape((-1, 3))
    img1_illum2 = img1_illum1[~np.all(img1_illum1 == 0, axis=1)]
    img1_illum2 = np.array(img1_illum2)
    img1_illum1_uvy = rgb2uvy(img1_illum2)
    
    
    hist = Histogram(0.0125, (-64 * 0.025, 64 * 0.025), (-64 * 0.025, 64 * 0.025))
    h= hist.get_hist(img1_uvy)
    plt.figure(figsize=(15, 15))
    plt.imshow(h)
    plt.show()
    
    
    h= hist.get_hist(img1_uvy)
    plt.figure(figsize=(15, 15))
    plt.imshow(h)
    plt.show()
    # exit()


    chist = hist.get_colorhist(img1_uvy, img2)
    chist2 = hist.get_colorhist(img1_illum1_uvy, img1_illum1)
    plt.figure(figsize=(15, 7))
    plt.subplot(121)
    plt.imshow(chist)
    plt.subplot(122)
    plt.imshow(chist2)
    plt.show()

def test2():
    x = np.random.randn(300)
    y = np.random.randn(300)
    e = np.ones((300))
    points = np.array([x, y, e]).T
    print(points.shape)
    # plt.scatter(x, y)
    # plt.show()

    hist = Histogram(0.2, (-5, 5), (-5, 5))
    h, u_range, v_range = hist.get_hist(points, is_normalised=False)
    plt.imshow(h)
    plt.show()
    h= np.histogram2d(points[:, 0], points[:, 1], range=((-5, 5), (-5, 5)), bins=int(10 / 0.2))
    plt.imshow(h)
    plt.show()

def test():
    # rgb, uv: [[0.32947977, 0.46885035, 0.20166988]][[0.35276868 0.84365153 0.60749347]]
    img = cv2.imread(r"D:\code\data\deAWB_data\1.jpg")
    img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB).astype(np.float64)
    img = img / 255
    print(img.shape)
    # for i in range(len(img.shape[0])):
    #     print(img[][])

    # print(rgb2uvy([[0.18823529, 0.36470588, 0.23529412]]))
    # print(rgb2uvy([[0.32780083, 0.46995543, 0.20224374]]))



if __name__ == '__main__':
    test2()





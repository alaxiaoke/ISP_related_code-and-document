import numpy as np

# 假设有一组数据点
# x = np.array([100, 75, 57,49, 52, 44, 42])
# y = np.array([25, 32, 41, 38,52, 63, 66])
# cct = np.array([2300, 2880, 4000, 5000, 6500, 7500])

x = np.array([123, 99, 68, 72, 64, 58, 52])
y = np.array([32, 37, 45, 46, 55, 61, 66])
cct = np.array([2300, 2880,3850, 4000, 5000, 6500, 7500])

# x_cwf = np.array([100, 75, 57,49, 52, 44, 42])
# y_cwf = np.array([25, 32, 41, 38,52, 63, 66])

# 使用2阶多项式拟合，其中n是拟合的阶数，p是拟合参数
n = 3
p = np.polyfit(x, cct, n)

# 创建多项式对象
poly = np.poly1d(p)

x_new = [i for i in range(50,125)]
print("x_new：", len(x_new))
# 用多项式对象来计算新的y值
cct_fit = poly(x_new)

# 打印拟合参数
cct_int = [int(i) for i in cct_fit]
print("拟合参数：", p)
print("cct_fit：", cct_fit)

#############################################################
n = 3
p1 = np.polyfit(x, y, 3)

# 创建多项式对象
poly1 = np.poly1d(p1)

x_new = [i for i in range(50,125)]
# 用多项式对象来计算新的y值
y_fit = poly1(x_new)

# 打印拟合参数
y_int = [int(i) for i in y_fit]
print("拟合参数：", p)
print("y_fit：", y_fit)



# 绘制原始数据点和拟合曲线
import matplotlib.pyplot as plt

plt.plot(x, cct, 'o', label='原始数据')
plt.plot(x_new, cct_fit,  'o', label='拟合曲线')
plt.legend()
plt.show()

# delta = (7500-2300)/(len(x_new)-1)
# for i in range(len(x_new)):
#     print("{", x_new[i], ",", y_int[i], ",", int(7500 - i*delta), ",", i,"}", ",","\\")

for i in range(len(x_new)):
    print("{", x_new[i], ",", y_int[i], ",", cct_int[i], ",", i,"}", ",","\\")
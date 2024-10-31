# -*- coding=utf-8 -*-
# Kalman filter example demo in Python

# A Python implementation of the example given in pages 11-15 of "An
# Introduction to the Kalman Filter" by Greg Welch and Gary Bishop,
# University of North Carolina at Chapel Hill, Department of Computer
# Science, TR 95-041,
# http://www.cs.unc.edu/~welch/kalman/kalmanIntro.html

# by Andrew D. Straw
# coding:utf-8
import pylab
import numpy as np
import pandas as pd
import random


def gauss_noisy(x, y):
    """
    对输入数据加入高斯噪声
    :param x: x轴数据
    :param y: y轴数据
    :return:
    """
    mu = 0
    sigma = 0.1
    for i in range(len(x)):
        x[i] += random.gauss(mu, sigma)
        y[i] += random.gauss(mu, sigma)
    return y

# 卡尔曼滤波  这里是假设A=1，H=1的情况
def kalman():
    xl = np.linspace(0, 40, 500, endpoint=True)
    yl = np.sin(xl)

    # 加入高斯噪声
    yl = gauss_noisy(xl, yl)
    print(yl)
    # 参数初始化
    n_iter = 500
    sz = (n_iter,)  # size of array
    x = -0.37727  # 真实值
    z = np.random.normal(x, 0.1, size=sz)  #观测值 ,观测时存在噪声
    z = gauss_noisy(xl, yl)
    Q = 1e-5  # process variance

    # 分配数组空间
    xhat = np.zeros(sz)  # x 滤波估计值
    P = np.zeros(sz)  # 滤波估计协方差矩阵
    xhatminus = np.zeros(sz)  # x 估计值
    Pminus = np.zeros(sz)  # 估计协方差矩阵
    K = np.zeros(sz)  # 卡尔曼增益

    R = 0.1 ** 2  # estimate of measurement variance, change to see effect  这个参数对平滑程度作用非常大

    # intial guesses
    xhat[0] = 0.0
    P[0] = 1.0  #参数用处不太大

    for k in range(1, n_iter):
        # 预测
        xhatminus[k] = xhat[k - 1]  # X(k|k-1) = AX(k-1|k-1) + BU(k) + W(k),A=1,BU(k) = 0
        Pminus[k] = P[k - 1] + Q  # P(k|k-1) = AP(k-1|k-1)A' + Q(k) ,A=1

        # 更新
        K[k] = Pminus[k] / (Pminus[k] + R)  # Kg(k)=P(k|k-1)H'/[HP(k|k-1)H' + R],H=1
        xhat[k] = xhatminus[k] + K[k] * (z[k] - xhatminus[k])  # X(k|k) = X(k|k-1) + Kg(k)[Z(k) - HX(k|k-1)], H=1
        P[k] = (1 - K[k]) * Pminus[k]  # P(k|k) = (1 - Kg(k)H)P(k|k-1), H=1

    print(z)
    pylab.figure()
    pylab.plot(z, 'k+', label='noisy measurements')  # 观测值
    pylab.plot(xhat, 'b-', label='a posteri estimate')  # 滤波估计值
    pylab.axhline(x, color='g', label='truth value')  # 真实值
    pylab.legend()
    pylab.xlabel('Iteration')
    pylab.ylabel('Voltage')

    # pylab.figure()
    # valid_iter = range(1, n_iter)  # Pminus not valid at step 0
    # pylab.plot(valid_iter, Pminus[valid_iter], label='a priori error estimate')
    # pylab.xlabel('Iteration')
    # pylab.ylabel('$(Voltage)^2$')
    # pylab.setp(pylab.gca(), 'ylim', [0, .01])
    pylab.show()

#二阶指数平滑
def double_exponential_smoothing(series, alpha, beta):
    """
        series - dataset with timeseries
        alpha - float [0.0, 1.0], smoothing parameter for level
        beta - float [0.0, 1.0], smoothing parameter for trend
    """
    # first value is same as series
    result = [series[0]]
    for n in range(1, len(series)+1):
        if n == 1:
            level, trend = series[0], series[1] - series[0]
        if n >= len(series): # forecasting
            value = result[-1]
        else:
            value = series[n]
        last_level, level = level, alpha*value + (1-alpha)*(level+trend)
        trend = beta*(level-last_level) + (1-beta)*trend
        result.append(level+trend)
    return result


#一阶指数平滑
def exponential_smoothing(alpha = 0.5 , s = []):

    xl = np.linspace(0, 5, 50, endpoint=True)
    yl = np.sin(xl)

    # 加入高斯噪声
    yl = gauss_noisy(xl, yl)
    '''
    一次指数平滑
    :param alpha:  平滑系数
    :param s:      数据序列， list
    :return:       返回一次指数平滑模型参数， list
    '''
    # n_iter = 50
    # sz = (n_iter,)  # size of array
    # x = -0.37727  # 真实值
    # s = np.random.normal(x, 0.1, size=sz)  # 观测值 ,观测时存在噪声

    s_temp = []
    s_temp.append(yl[0])
    print(s_temp)
    for i in range(1, len(yl), 1):
        s_temp.append(alpha * yl[i-1] + (1 - alpha) * s_temp[i-1])

    pylab.figure()
    pylab.plot(yl, 'k+', label='noisy measurements')  # 观测值
    pylab.plot(s_temp, 'b-', label='a posteri estimate')  # 滤波估计值
    pylab.legend()
    pylab.show()

    return s_temp



#加权平均
def moving_avg():
    df = pd.DataFrame()
    df["data"] = np.random.rand(20)
    # 数据也可以是series格式

    # 简单移动平均
    simp_moving_avg = df["data"].rolling(window=window, min_periods=1).mean()
    # 加权移动平均
    weighted_moving_avg = df["data"].rolling(window=window, min_periods=1, win_type="cosine").mean()
    # 指数加权移动平均
    ewma = df["data"].ewm(alpha=alpha, min_periods=1).mean()


kalman()
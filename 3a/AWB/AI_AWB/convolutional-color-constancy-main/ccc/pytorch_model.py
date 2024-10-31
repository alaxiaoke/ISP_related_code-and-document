# -*- coding: utf-8 -*-
"""
Created on Wed Apr 25 14:27:20 2018

@author: phamh
"""
import copy

import h5py
import numpy as np
import cv2;
import tensorflow as tf;

from tensorflow.python.keras import layers, optimizers
from tensorflow.python.keras.layers import Input, Dense, Activation, ZeroPadding2D, BatchNormalization, Flatten, Conv2D
from tensorflow.python.keras.layers import AveragePooling2D, MaxPooling2D, Dropout, GlobalMaxPooling2D, \
    GlobalAveragePooling2D
from tensorflow.python.keras.models import Model
from tensorflow.python.keras.preprocessing import image
from tensorflow.python.keras.initializers import glorot_uniform
from tensorflow.python.keras import backend as K

class PytorchModel:
    def __init__(self, stages=4, shape=64):
        self.stages = stages
        self.shape = shape
        self.models = []
        for i in range(self.stages):
            self.models.append(self.ColorNet(input_shape=(self.shape// 2 ** i,self.shape// 2 ** i,1)))


    def ColorNet(self, input_shape):
        # Define the input as a tensor with shape input_shape
        X_input = Input(input_shape)

        # Stage 1
        X = Conv2D(12, (5, 5), strides=(1, 1), name='conv1', kernel_initializer=glorot_uniform(seed=0))(X_input)
        X = BatchNormalization(name='bn_conv1')(X)

        # output layer
        X = Flatten()(X)
        X = Dense(40, activation='relu', name='fc' + str(40))(X);
        X = Dropout(rate=0.5)(X);

        X = Dense(2, activation=None, name='fc' + str(2))(X);

        # Create model
        color_model = Model(inputs=X_input, outputs=X, name='ColorNet');

        return color_model;

    def train_new(self, X_data, Y_data, stage = 0):
        X_data = np.expand_dims(X_data, axis=3)
        print("X_data.shape", X_data.shape)
        # Y_train = np.expand_dims(Y_train, axis=3)
        train_size = int(0.9 * len(X_data))
        X_train = X_data[:train_size]
        Y_train = Y_data[:train_size]
        Y_test = Y_data[train_size:]
        X_test = X_data[train_size:]

        rmsprop = optimizers.rmsprop_v2.RMSprop(lr=0.00001, rho=0.9, epsilon=None, decay=0.0);

        # for i in range(stage):

        cc_model = self.models[stage]

        print("X_train.shape[1:4]", X_train.shape[1:4])

        cc_model.compile(optimizer=rmsprop, loss=tf.keras.losses.mean_squared_error);


        estimate = cc_model.fit(X_train, Y_train);

        # cc_model.evaluate(X_test, Y_test, return_dict=True);
        # 对新的样本进行预测
        # preds_train = cc_model.predict(X_train)
        # preds_test = cc_model.predict(X_test)
        #
        # #print("preds", preds)
        # for i in range(100):
        #     print(preds_train[i])
        #     print(Y_train[i])
        #     print("-----------------preds_train------------------")
        #
        # for i in range(preds_test.shape[0]):
        #     # print(preds[i])
        #     # t = preds[i][1]
        #     # print(t)
        #     print(preds_test[i])
        #     print(Y_test[i])
        #     print("------------------preds_test-----------------")

    def pred(self, imgs, data_gt):
        pred_list = []
        size = imgs.shape[1]
        img_copy = copy.copy(imgs)
        for i in range(len(self.models)):
            model_i = self.models[i]
            imgInput = np.zeros((img_copy.shape[0], int(size/(2**i)), int(size/(2**i))))
            for j in range(img_copy.shape[0]):
                imgInput[j] = cv2.resize(img_copy[j], (int(size/(2**i)), int(size/(2**i))), interpolation=cv2.INTER_LINEAR)

            imgInput = np.expand_dims(imgInput, axis=3)
            preddd = model_i.predict(imgInput)
            print("pred shape:", preddd.shape)
            pred_list.append(preddd)
        for k in range(img_copy.shape[0]):
            print("1,2,3,4:", pred_list[0, k, :], pred_list[1, k, :], pred_list[2, k, :], pred_list[3, k, :])





# print("preds", preds)

# print();
# print ("Loss = " + str(preds[0]));
# print ("Test Accuracy = " + str(preds[1]));
#
# # serialize model to JSON
# model_json = cc_model.to_json()
# with open("cc_model.json", "w") as json_file:
#     json_file.write(model_json)
# # serialize weights to HDF5
# cc_model.save_weights("cc_model.h5")
# print("Saved model to disk")


# =============================================================================
# img1 = cv2.imread('0044_0002.png');
# img1 = np.expand_dims(img1, axis=0);
# a = cc_model.predict(img1);
#
# img2 = cv2.imread('0015_0015.png');
# img2 = np.expand_dims(img2, axis=0);
# b = cc_model.predict(img2);
# =============================================================================

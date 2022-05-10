#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Sat Dec 12 09:00:37 2020

@author: jxzhang
"""

import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import copy
import pandas as pd
from pandas import DataFrame
import seaborn as sns 


def plt_box_1(slot,data):
    plt.figure(figsize=(10,5))
    plt.title('Boxplot of transport time from'+slot[0]+' to '+slot[1],fontsize=20)
    # labels = ['x1','x2']
    plt.boxplot(data, labels = ['256', '512', '1K', '2K', '4K', '8K', '16K', '32K', '64K', '128K', '256K', '512K', '1M', '2M', '4M'],whis=1.5)
    # plt.savefig('test data raw'+ num + '.jpg')
    plt.grid()
    plt.show()#显示图像
    
    
def plt_box_2(data):
    plt.figure(figsize=(10,5))
    plt.title('Boxplot of pingpong time ',fontsize=20)
    # labels = ['x1','x2']
    plt.boxplot(data, labels = ['256', '512', '1K', '2K', '4K', '8K', '16K', '32K', '64K', '128K', '256K', '512K', '1M', '2M', '4M'],whis=2,showfliers=False)
    # plt.savefig('test data raw'+ num + '.jpg')
    plt.grid()
    plt.show()#显示图像

# def dataprocess(old_data):
#     new_data = copy.deepcopy(old_data)
#     for k in range(len(new_data)):   
#         for i in range(len(new_data[k])):
#             if new_data[k][i] > 2*(np.mean(old_data[k])):
#                 new_data[k][i] = np.median(new_data[k])
#     return new_data


if __name__=="__main__":
    
    
    
    num = '0';
    names = ['256', '512', '1K', '2K', '4K', '8K', '16K', '32K', '64K', '128K', '256K', '512K', '1M', '2M', '4M']
    rawdata = []
    
    
    
    for i in range(len(names)):
        filename = "./evaluation/pingpong_time/pingpong_time_" + names[i] + "byte.txt"
        a = np.loadtxt(filename)
        a=a[50:-1]
        rawdata.append(a)
    plt_box_2(rawdata)

    num = '0';
    msg_size = ['256', '512', '1K', '2K', '4K', '8K', '16K', '32K', '64K', '128K', '256K', '512K', '1M', '2M', '4M']
    nodes=['A','B','C','D','E']
    final_data=[]
    
    
    # Store all the data from the transport time
    # with non-real propotities and no extra load on CPU
    for node in range(len(nodes)-1):
        temp_data = []
        for size in range(len(msg_size)):
            filename = "./evaluation/transport_"+nodes[node]+"2"+nodes[node+1]+"/transport_time_" + msg_size[size] + "byte.txt"
            # print(filename)
            a = np.loadtxt(filename)
            a=a[50:-1]
            temp_data.append(a)
        plt_box_1([nodes[node],nodes[node+1]],temp_data)
            
        final_data.append(temp_data)
        
    
    
    print(len(temp_data))
    print(temp_data[0][0])
    # print(rawdata)
    
    temp_data=[]
    # A2E
    for size in range(len(msg_size)):
        filename = "./evaluation/transport_A2E/transport_time_" + msg_size[size] + "byte.txt"
        a = np.loadtxt(filename)
        a=a[50:-1]
        temp_data.append(a)
    
    plt_box_1(['A','E'],temp_data)
        
    final_data.append(temp_data)
    
    
    # num = '0';
    # msg_size = ['256', '512', '1K', '2K', '4K', '8K', '16K', '32K', '64K', '128K', '256K', '512K', '1M', '2M', '4M']
    # nodes=['A','B','C','D','E']
    # rawdata=[]
    
    
    # for node in range(len(nodes)-1):
    #     temp_data = []
    #     for size in range(len(msg_size)):
    #         filename = "./evaluation/transport_"+nodes[node]+"2"+nodes[node+1]+"/transport_time_" + msg_size[size] + "byte.txt"
    #         # print(filename)
    #         a = np.loadtxt(filename)
    #         a=a[50:-1]
    #         temp_data.append(a)
    #     plt_box_1([nodes[node],nodes[node+1]],temp_data)
            
    #     rawdata.append(temp_data)
        
    
    
    # print(len(temp_data))
    # print(temp_data[0][0])
    # # print(rawdata)
    
    # temp_data=[]
    # # A2E
    # for size in range(len(msg_size)):
    #     filename = "./evaluation/transport_A2E/transport_time_" + msg_size[size] + "byte.txt"
    #     a = np.loadtxt(filename)
    #     temp_data.append(a)
    
    # plt_box_1(['A','E'],temp_data)
        
    # rawdata.append(temp_data)
    




    

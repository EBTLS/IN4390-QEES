# -*- coding: utf-8 -*-
"""
Created on Mon Dec 14 11:15:25 2020

@author: xuan
"""

import numpy as np
import matplotlib.pyplot as plt 
from scipy import stats 

def calculate_confidence_Interval(data,level):
    mean, std =data.mean(), data.std(ddof=1)
    print(mean, std)
    conf_intveral = stats.norm.interval(level, loc=mean, scale=std/np.sqrt(len(data)))
    return [conf_intveral[0],conf_intveral[1]]


def plt_box_1(workload,data):
    plt.title('Boxplot of transport time (to 512K) of '+workload,fontsize=20)
    # labels = ['x1','x2']
    plt.boxplot(data, labels = ['256', '512', '1K', '2K', '4K', '8K', '16K', '32K', '64K', '128K', '256K', '512K'],whis=1.5)
    # plt.savefig('test data raw'+ num + '.jpg')
    plt.grid()
    plt.show()#显示图像

def plt_box_2(workload,data):
    plt.title('Boxplot of transport time (to 256K) of '+workload,fontsize=20)
    # labels = ['x1','x2']
    plt.boxplot(data, labels = ['256', '512', '1K', '2K', '4K', '8K', '16K', '32K', '64K', '128K', '256K'],whis=1.5)
    # plt.savefig('test data raw'+ num + '.jpg')
    plt.grid()
    plt.show()#显示图像


workload_list = ["without","normal","heavy" ]
msg_size = ['256', '512', '1K', '2K', '4K', '8K', '16K', '32K', '64K', '128K', '256K', '512K']
color=["g","b","r"]
marker_list=["o","+","*"]


# draw boxplot
data=[]
mean=[]
confidence_interval=[]

for workload in workload_list:
    temp_data=[]
    temp_mean=[]
    temp_confidence_interval=[]
    for size in msg_size:
        filename="./lab_data/evaluation_"+workload+"/transport_time/transport_time_" + size + "byte.txt"
        a=np.loadtxt(filename)
        a=a[20:-1]
        a_mean=np.mean(a)
        temp_mean.append(a_mean)
        temp_data.append(a)
    
    
    plt_box_1(workload,temp_data)

# draw boxplot
data=[]
mean=[]
confidence_interval=[]

for workload in workload_list:
    temp_data=[]
    temp_mean=[]
    temp_confidence_interval=[]
    for size in msg_size[0:-1]:
        filename="./lab_data/evaluation_"+workload+"/transport_time/transport_time_" + size + "byte.txt"
        a=np.loadtxt(filename)
        a=a[20:-1]
        a_mean=np.mean(a)
        temp_mean.append(a_mean)
        temp_data.append(a)
    
    
    plt_box_2(workload,temp_data)


data=[]
mean=[]
confidence_interval=[]


# draw curve
for workload in workload_list:
    temp_data=[]
    temp_mean=[]
    temp_confidence_interval=[]
    for size in msg_size:
        filename="./lab_data/evaluation_"+workload+"/transport_time/transport_time_" + size + "byte.txt"
        a=np.loadtxt(filename)
        a=a[20:-1]
        a_mean=np.mean(a)
        temp_mean.append(a_mean)
        temp_data.append(a)
        
        a_conf_interval=calculate_confidence_Interval(a, 0.8)
        # print(type(a_conf_interval))
        temp_confidence_interval.append(a_conf_interval)
    
    confidence_interval.append(temp_confidence_interval)
    
    
    data.append(temp_data)
    mean.append(temp_mean)
    # print(temp_data)
    # print(temp_mean)
    # print(temp_confidence_interval)
    plt.plot(msg_size,temp_mean,
             color=color[workload_list.index(workload)]   
             ,linestyle = "--"
             ,linewidth = 1   
             ,marker = marker_list[workload_list.index(workload)]
             ,markeredgecolor = color[workload_list.index(workload)]
             ,markeredgewidth = 0.5    #设置数据点边框的宽度，输入浮点数
             ,markerfacecolor = color[workload_list.index(workload)]    #设置数据点颜色，”w“表示白色
             ,markersize = 5
             ,label=workload+"_extra_load"
             )
    plt.legend()

plt.title("transport_time (to 512K)")
plt.grid()
plt.show()

for workload in workload_list:
    temp_data=[]
    temp_mean=[]
    temp_confidence_interval=[]
    for size in msg_size[0:-1]:
        filename="./lab_data/evaluation_"+workload+"/transport_time/transport_time_" + size + "byte.txt"
        a=np.loadtxt(filename)
        a=a[10:-1]
        a_mean=np.mean(a)
        temp_mean.append(a_mean)
        temp_data.append(a)
        
        a_conf_interval=calculate_confidence_Interval(a, 0.8)
        # print(type(a_conf_interval))
        temp_confidence_interval.append(a_conf_interval)
    
    confidence_interval.append(temp_confidence_interval)
    
    
    data.append(temp_data)
    mean.append(temp_mean)
    # print(temp_data)
    # print(temp_mean)
    # print(temp_confidence_interval)
    plt.plot(msg_size[0:-1],temp_mean,
             color=color[workload_list.index(workload)]   
             ,linestyle = "--"
             ,linewidth = 1   
             ,marker = marker_list[workload_list.index(workload)]
             ,markeredgecolor = color[workload_list.index(workload)]
             ,markeredgewidth = 0.5    #设置数据点边框的宽度，输入浮点数
             ,markerfacecolor = color[workload_list.index(workload)]    #设置数据点颜色，”w“表示白色
             ,markersize = 5
             ,label=workload+"_extra_load"
             )
    plt.legend()

plt.title("transport_time (to 256K)")
plt.grid()
plt.show()


# draw CI


for i in range(0,len(workload_list)):
    for j in range (0,2,1):
        bound=[]
        for k in range(0,len(msg_size)):
            bound.append(confidence_interval[i][k][j])
        
        plt.plot(msg_size,bound
             ,color=color[i]   
             ,linestyle = "--"
             ,linewidth = 1   
             ,marker = marker_list[i]
             ,markeredgecolor = color[i]
             ,markeredgewidth = 0.5    #设置数据点边框的宽度，输入浮点数
             ,markerfacecolor = color[i]    #设置数据点颜色，”w“表示白色
             ,markersize = 5
             ,label=workload_list[i]+"_extra_load")

plt.title("Confidence Interval (to 512K)")
plt.legend()
plt.grid()
plt.show()

for i in range(0,len(workload_list)):
    for j in range (0,2,1):
        bound=[]
        for k in range(0,len(msg_size)-1):
            bound.append(confidence_interval[i][k][j])
        
        plt.plot(msg_size[0:-1],bound
             ,color=color[i]   
             ,linestyle = "--"
             ,linewidth = 1   
             ,marker = marker_list[i]
             ,markeredgecolor = color[i]
             ,markeredgewidth = 0.5    #设置数据点边框的宽度，输入浮点数
             ,markerfacecolor = color[i]    #设置数据点颜色，”w“表示白色
             ,markersize = 5
             ,label=workload_list[i]+"_extra_load")

plt.title("Confidence Interval (to 256K)")
plt.legend()
plt.grid()
plt.show()
            
            
            








    
    
        

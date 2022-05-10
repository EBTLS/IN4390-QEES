#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Sat Nov 28 18:34:18 2020

@author: jxzhang
"""

import pandas as pd
import researchpy as rp


from scipy import stats
import numpy as np
from math import sqrt
import matplotlib.pyplot as plt

# file Q_2_File=open("question2 transport time.txt","r")

Q_2_File=open("question2_transport_time.txt","r")

# Q_2_Data=Q_2_File.read()


Q_2_Data=[]

content=Q_2_File.readline()
while (content!=''):
    print(content,end="")
    try:
        Q_2_Data.append(float(content))
    except ValueError as e:
        print("number over:",e)
    content=Q_2_File.readline()
    
# print(Q_2_Data)
Q_2_d={"Transport_Time":Q_2_Data}
Q_2_df=pd.DataFrame(Q_2_d)
# print(Q_2_df)

Q_2_result_0_80=rp.summary_cont(Q_2_df["Transport_Time"],conf=0.8,decimals=10)
print(Q_2_result_0_80)

Q_2_result_0_98=rp.summary_cont(Q_2_df["Transport_Time"],conf=0.98,decimals=10)
print(Q_2_result_0_98)

# mean=np.array(Q_2_Data).mean()
# std=np.array(Q_2_Data).std(ddof=1)
# print(mean)
# print(std)

# conf_intveral = stats.norm.interval(0.8, loc=mean, scale=std/sqrt(len(Q_2_Data)))
# print(conf_intveral)

x = np.linspace(-5,5,100000)
y = stats.t.pdf(x,len(Q_2_Data)-1)
plt.plot(x,y,c="orange")
plt.title('t distribution with df len(Q2_DATA)-10')
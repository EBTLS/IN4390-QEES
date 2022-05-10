#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Sun Dec  6 18:52:48 2020

@author: jxzhang
"""

import pandas as pd
import researchpy as rp

import seaborn as sns
import numpy as np
from matplotlib import pyplot as plt 
import statsmodels.api as sm
from statsmodels.formula.api import ols
import statsmodels.stats.multicomp


DDS_methods=["fastrtps", "connext", "opensplice"]
message_load=["256byte","128Kbyte"]
Sample_number=300
Experiment_number=7
Observation=[]


# raw data
for i in DDS_methods:
    for k in message_load:
        temp_Observation=[]
        for j in range(0,2*Experiment_number,2):
            file_name="./Q_4_Data/"+i+"/transport_time_"+k+"_"+str(j)+".txt"
            Q_4_File=open(file_name,"r")
            content=Q_4_File.readline()
            while (content!=''):
                # print(content,end="")
                try:
                    Observation.append(100*float(content))
                    temp_Observation.append(float(content))
                    
                except ValueError as e:
                    print("number over:",e)
                content=Q_4_File.readline()
            
        plt.hist(np.array(temp_Observation),50,facecolor="blue", edgecolor="black", alpha=0.7)
        plt.title("transport_time"+" "+ i+" "+ k+" "+"raw")
        plt.show()
            

# print(len(Observation))
# Observation=Observation*1000


Q_4_Data={"Transport_Time": Observation,
          "DDS":    np.repeat(DDS_methods,
                                len(message_load)
                                *len(Observation)/len(message_load)/len(DDS_methods)/Experiment_number
                                *Experiment_number),
          "Message":
                    np.tile(
                        np.repeat(message_load,
                                  len(Observation)/len(message_load)/len(DDS_methods)/Experiment_number
                                  *Experiment_number),
                    len(DDS_methods)),
                  }
# porint(Q_4_DataD)
Q_4_df=pd.DataFrame(Q_4_Data)
print(Q_4_df)


# modified data
Observation=[]
for i in DDS_methods:
    for k in message_load:
        Observation_history=[]
        for j in range(0,2*Experiment_number,2):
            temp_Observation=[]
            file_name="./Q_4_Data/"+i+"/transport_time_"+k+"_"+str(j)+".txt"
            Q_4_File=open(file_name,"r")
            content=Q_4_File.readline()
            while (content!=''):
                # print(content,end="")
                try:
                    # Observation.append(100*float(content))
                    temp_Observation.append(float(content))
                    # print(temp_Observation.index(max(temp_Observation)))
                    
                except ValueError as e:
                    print("number over:",e)
                content=Q_4_File.readline()
            temp_Observation=temp_Observation[30:len(temp_Observation)-1]
            # print(len(temp_Observation))
            Observation_history=Observation_history+temp_Observation
            # print(temp_Observation.index(max(temp_Observation)))
            Observation=Observation+temp_Observation
        print(len((Observation_history)))
        plt.hist(np.array(Observation_history),100,facecolor="blue", edgecolor="black", alpha=0.7)
        plt.title("transport_time"+" "+ i+" "+ k+" "+"modified")
        plt.show()
        
        
            

# print(len(Observation))
temp_Observation=Observation
Observation=[]

for i in range(0,len(temp_Observation)):
    
    Observation.append(100*temp_Observation[i])

Q_4_Data_modified={"Transport_Time": Observation,
          "DDS":    np.repeat(DDS_methods,
                                len(message_load)
                                *len(Observation)/len(message_load)/len(DDS_methods)/Experiment_number
                                *Experiment_number),
          "Message":
                    np.tile(
                        np.repeat(message_load,
                                  len(Observation)/len(message_load)/len(DDS_methods)/Experiment_number
                                  *Experiment_number),
                    len(DDS_methods)),
                  }
    

Q_4_df_modified=pd.DataFrame(Q_4_Data_modified)
print(Q_4_df_modified)


# ANOVA for Q_4

Q_4_model=ols('Transport_Time~DDS*Message',Q_4_df_modified).fit()

Q_4_model.summary()

Q_4_res=sm.stats.anova_lm(Q_4_model,typ=2)

print(Q_4_res)


# CI for Q_4
# Q_4_CI_experiment=rp.summary_cont(Q_4_df['Transport_Time'].groupby(Q_4_df['Experiement']))
# print(Q_4_CI_experiment)

# CI for Q_4
Q_4_CI_DDS=rp.summary_cont(Q_4_df_modified['Transport_Time'].groupby(Q_4_df_modified['DDS']))

print(Q_4_CI_DDS)

Q_4_CI_Message=rp.summary_cont(Q_4_df_modified['Transport_Time'].groupby(Q_4_df_modified['Message']))
        
print(Q_4_CI_Message)

Q_4_CI_Interaction=rp.summary_cont(Q_4_df_modified.groupby(['DDS', 'Message']))['Transport_Time']
        
print(Q_4_CI_Interaction)

# # ANOVA for Q_4 2

# Q_4_model=ols('Transport_Time~DDS+Message',Q_4_df_modified).fit()

# Q_4_model.summary()

# Q_4_res=sm.stats.anova_lm(Q_4_model,typ=2)

# print(Q_4_res)


# # CI for Q_4
# # Q_4_CI_experiment=rp.summary_cont(Q_4_df_modified['Transport_Time'].groupby(Q_4_df_modified['Experiement']))
# # print(Q_4_CI_experiment)

# # CI for Q_4
# Q_4_CI_DDS=rp.summary_cont(Q_4_df_modified['Transport_Time'].groupby(Q_4_df_modified['DDS']))

# print(Q_4_CI_DDS)

# Q_4_CI_Message=rp.summary_cont(Q_4_df_modified['Transport_Time'].groupby(Q_4_df_modified['Message']))
        
# print(Q_4_CI_Message)
        
        
    
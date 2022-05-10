# -*- coding: utf-8 -*-
"""
Created on Tue Feb  2 22:41:59 2021

@author: xuan
"""

import random

number=[0,0]

for i in range(0,100,1):
    node=random.choices([1,2],[0.5,-0.5])[0]
    
    if node==1:
        number[0]=number[0]+1

    if node==2:
        number[1]=number[1]+1
        

print(number)
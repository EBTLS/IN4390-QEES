from pyDOE2 import *
import pandas as pd
import researchpy as rp
import seaborn as sns
import numpy as np

import statsmodels.api as sm
from statsmodels.formula.api import ols
import statsmodels.stats.multicomp


# first column: memory_size
# second column: cache_size
# third column: number of processors
base_number=3
base=ff2n(base_number)
# print(base)

# product_2=[]
sign_table=base
for i in range(0,base_number):
    for j in range (i+1,base_number):
        temp_product=np.array(base[:,i]*base[:,j]).reshape(-1,1)
        sign_table=np.hstack((sign_table,temp_product))

for i in range(0,base_number):
    for j in range (i+1,base_number):
        for k in range(j+1,base_number):
            temp_product = np.array(base[:, i] * base[:, j] * base[:,k]).reshape(-1, 1)
            sign_table=np.hstack((sign_table,temp_product))


temp_intercept=np.ones((np.size(sign_table,0),1))
sign_table=np.hstack((temp_intercept,sign_table))
print(sign_table)

performance=[14,22,10,34,46,58,50,86];

performance=[14,22,10,34,46,58,50,86];

data=pd.DataFrame({"performance":performance,
      "A": sign_table[:,1],
      "B": sign_table[:,2],
      "C": sign_table[:,3],
      "AB":sign_table[:,4],
      "AC":sign_table[:,5],
      "BC":sign_table[:,6]});

formula='performance ~ A*B*C-A:B:C'

model=ols(formula,data).fit()

# formula='performance ~ intercept+A*B*C-A:B:C'

# formula='performance ~ intercept+A*B*C'
model=ols(formula,data).fit()

model.summary()

replicated_performance=[15,18,12,
                        22,18,20,
                        11,15,19,
                        34,30,35,
                        46,42,44,
                        58,62,60,
                        50,55,54,
                        86,80,74]

data_2=pd.DataFrame({"performance":replicated_performance,
      "A": np.repeat(sign_table[:,1],3),
      "B": np.repeat(sign_table[:,2],3),
      "C": np.repeat(sign_table[:,3],3),
      "AB":np.repeat(sign_table[:,4],3),
      "AC":np.repeat(sign_table[:,5],3),
      "BC":np.repeat(sign_table[:,6],3)});

print(data_2)

formula_1='performance ~ A*B*C'
formula_2='performance ~ A*B*C-A:B:C'

model=ols(formula_1,data_2).fit()

model.summary()

print(model.params)
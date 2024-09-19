#!/usr/bin/env python3
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
from scipy.stats import norm
import sys
import pymbar
import csv

import os

#####################################################################################
num_sims = int(sys.argv[1]) #3
num_cols_per_sim = int(sys.argv[2]) #30
num_data = int(sys.argv[3]) #2001

#####################################################################################
fontsize = 8
outputwidth = 7
outputheight = 3.9375

#####################################################################################
fig = plt.figure()
ax = fig.add_subplot(111)

#####################################################################################

df = pd.read_csv("ws.txt")
colors = ['#00239C','#E10600']

########################################################################################
last_col = num_sims * num_cols_per_sim + 1

steps = df.iloc[:num_data,0]
forward_works = df.iloc[:num_data,1:last_col]
backward_works = df.iloc[num_data:,1:last_col]

########################################################################################


forward_works = forward_works - forward_works.iloc[0,:]
backward_works = backward_works.iloc[-1,:] - backward_works
backward_works = backward_works.iloc[::-1] 

dists = []
dGs = []
dGEs = []

for index, row in forward_works.iterrows():
    fw = forward_works.iloc[index,:].values
    bw = backward_works.iloc[index,:].values
    
    fw = [x/0.592 for x in fw]
    bw = [x/0.592 for x in bw]
    
    fw = np.array(fw)
    bw = np.array(bw)
    

    
    
    results = pymbar.other_estimators.bar(fw,bw)
    #results = pymbar.other_estimators.exp(fw)
    
    dG = results['Delta_f']
    dGE = results['dDelta_f']

    dists.append(index*0.01)
    dGs.append(dG*0.592)
    dGEs.append(dGE*0.592)
    
########################################################################################
forward_works = df.iloc[num_data:,1:last_col]
backward_works = df.iloc[:num_data,1:last_col]



forward_works = forward_works.iloc[-1,:] - forward_works
backward_works = backward_works - backward_works.iloc[0,:]
forward_works = forward_works[::-1]


for index, row in forward_works.iterrows():
    fw = forward_works.iloc[index-num_data,:].values
    bw = backward_works.iloc[index-num_data,:].values
    
    fw = [-x/0.592 for x in fw]
    bw = [-x/0.592 for x in bw]
    
    
    fw = np.array(fw)
    bw = np.array(bw)
    
    results = pymbar.other_estimators.bar(fw,bw)
    #results = pymbar.other_estimators.exp(fw)
    
    dG = results['Delta_f']
    dGE = results['dDelta_f']
    
    dists.append((index-(num_data-1))*0.01)
    dGs.append(dG*0.592)
    dGEs.append(dGE*0.592)
    
#################################################################################################

with open("BAR.dat","w") as csvfile:
   writer = csv.writer(csvfile,delimiter=' ')
   writer.writerow(["Distance(A)","dG(kcal/mol)","dG-Error"])
   for row in zip(dists,dGs,dGEs):
       writer.writerow(row)

#################################################################################################

dGmin = [x - y for x, y in zip(dGs,dGEs)]
dGmax = [x + y for x, y in zip(dGs,dGEs)]

plt.plot(dists, dGs,  linewidth=1, color=colors[0])
ax.fill_between(dists, dGmin, dGmax, color=colors[0],alpha=0.1)


plt.xlabel("Distance (A)")
plt.ylabel("Free energy (kcal/mol)")
plt.legend()

################################################################################################

fig.tight_layout()
plt.subplots_adjust(top=0.95,bottom=0.15,left=0.1,right=0.95,hspace=0.1,wspace=0.15)


#################################################################################################
fileName = "BAR"
fig.set_size_inches(outputwidth,outputheight)
fig.savefig(fileName+".svg",dpi=1200)




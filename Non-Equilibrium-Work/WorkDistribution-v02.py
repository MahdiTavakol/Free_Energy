#!/usr/bin/env python3
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
from scipy.stats import norm
import sys

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

backward_works = -df.iloc[-1,1:last_col].values
forward_works = df.iloc[num_data,1:last_col].values - df.iloc[0,1:last_col]

forward_works = forward_works[forward_works != 0]
backward_works = backward_works[backward_works != 0]


num_bins = 20

plt.hist(forward_works,bins=num_bins,alpha=0.5,label="forward works",color=colors[0],density=True)
plt.hist(backward_works,bins=num_bins,alpha=0.5,label="backward works",color=colors[1],density=True)


mu1, std1 = norm.fit(forward_works)
mu2, std2 = norm.fit(backward_works)
x = np.linspace(min([backward_works.min(),forward_works.min()]),max([backward_works.max(),forward_works.max()]),1000)
p1 = norm.pdf(x,mu1,std1)
p2 = norm.pdf(x,mu2,std2)

plt.plot(x,p1,'--',linewidth=2,color=colors[0])
plt.plot(x,p2,'--',linewidth=2,color=colors[1])

plt.xlabel("Works")
plt.ylabel("Frequency")
plt.title("Work Distribution")
plt.legend()

################################################################################################

fig.tight_layout()
plt.subplots_adjust(top=0.95,bottom=0.15,left=0.1,right=0.95,hspace=0.1,wspace=0.15)


###########################################################################################################
fileName = "Work-Dist"
fig.set_size_inches(outputwidth,outputheight)
fig.savefig(fileName+".svg",dpi=1200)



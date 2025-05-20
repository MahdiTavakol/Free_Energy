#!/usr/bin/env python3
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
from scipy.stats import norm
import sys
import csv

import os

#####################################################################################
fontsize = 8
outputwidth = 7
outputheight = 3.9375

#####################################################################################
def average_forward_reverse_gExp(gExp):
	n  = len(gExp)
	n2 = int(len(gExp)/2)
	
	gExpFW = np.zeros(n)
	
	for i in range(n2):
		elem = (gExp[i] + gExp[n-i-1])/2.0 
		gExpFW[i] = elem
		gExpFW[n-i-1] = elem
		
		
	return gExpFW
	
#####################################################################################
fig = plt.figure()
ax = fig.add_subplot(111)

#####################################################################################

colors = ['#DA291C','#56A8CB','#53A567','#FFC857']

#####################################################################################

df = pd.read_csv("FreeEnergy.dat")

zs = df.z1.to_numpy()
gExp = df.gExp.to_numpy()
gBar = df.gBar.to_numpy()
wAvg = df.wAvg.to_numpy()
gExpFW = average_forward_reverse_gExp(gExp)


#####################################################################################

ax.plot(zs,gExp,   color=colors[0],label='exp')
ax.plot(zs,gBar,   color=colors[1],label='bar')
ax.plot(zs,wAvg,   color=colors[2],label='avg')
ax.plot(zs,gExpFW, color=colors[3],label='exp-FW')

#####################################################################################

plt.xlabel("Distance (A)")
plt.ylabel("Free energy (kcal/mol)")
plt.legend()

#####################################################################################

fig.tight_layout()
plt.subplots_adjust(top=0.95,bottom=0.15,left=0.1,right=0.95,hspace=0.1,wspace=0.15)


#####################################################################################
fileName = "FreeEnergies-v02"
fig.set_size_inches(outputwidth,outputheight)
fig.savefig(fileName+".svg",dpi=1200)



#!/usr/bin/env python3
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
from scipy.stats import norm
import sys
import csv

import os

#####################################################################################
stepmid = (int(sys.argv[1]))
#####################################################################################
xperstep = 1000000
num_bins = 20

#####################################################################################
fontsize = 8
outputwidth = 7
outputheight = 3.9375

#####################################################################################
fig = plt.figure()
ax1 = fig.add_subplot(221)

#####################################################################################

colors = ['#DA291C','#56A8CB','#53A567']

########################################################################################

df = pd.read_csv("ws.txt")

# getting rid of columns with all zeros
df = df.drop(columns=df.columns[(df==0).all()])

#######################################################################################

Wf = df.iloc[:stepmid,1:-4]
Wr = df.iloc[stepmid:,1:-4] - df.iloc[-1,1:-4]

Xf = df.iloc[:stepmid,0] / xperstep
Xr = df.iloc[-1,0] / xperstep - df.iloc[stepmid:,0] / xperstep 

######################################################################################

Wfm = Wf.mean(axis=1).to_numpy()
Wfs = Wf.std(axis=1).to_numpy()

Wrm = Wr.mean(axis=1).to_numpy()
Wrs = Wr.std(axis=1).to_numpy()


######################################################################################
	
Xf = np.array(Xf)
Xr = np.array(Xr)
Wfm = np.array(Wfm)
Wfs = np.array(Wfs) 
Wrm = np.array(Wrm)
Wrs = np.array(Wrs)  

#################################################################################################

plt.sca(ax1)
ax1.plot(Xf,Wfm,color=colors[1])
ax1.plot(Xr,Wrm,color=colors[0])
plt.fill_between(Xf,Wfm-Wfs,Wfm+Wfs,color=colors[1],alpha=0.3,label="Wf")
plt.fill_between(Xr,Wrm-Wrs,Wrm+Wrs,color=colors[0],alpha=0.3,label="-Wr")

#################################################################################################

plt.xlabel("Distance (A)")
plt.ylabel("Waverage (kcal/mol)")
plt.legend()

################################################################################################
ax2 = fig.add_subplot(222)
plt.sca(ax2)

f = df.iloc[stepmid, 1:-4].to_numpy() - df.iloc[0, 1:-4].to_numpy()
r = -(df.iloc[-1, 1:-4].to_numpy() - df.iloc[stepmid, 1:-4].to_numpy())

###############################################################################################

plt.hist(f,bins=num_bins,alpha=0.5,label="Wf",color=colors[1],density=True,orientation='horizontal')
plt.hist(r,bins=num_bins,alpha=0.5,label="-Wb",color=colors[0],density=True,orientation='horizontal')

################################################################################################

mu1, std1 = norm.fit(f)
mu2, std2 = norm.fit(r)
x = np.linspace(min([r.min(),f.min()]),max([r.max(),f.max()]),1000)
p1 = norm.pdf(x,mu1,std1)
p2 = norm.pdf(x,mu2,std2)

plt.plot(p1,x,'--',linewidth=2,color=colors[1])
plt.plot(p2,x,'--',linewidth=2,color=colors[0])

################################################################################################
ymin1, ymax1 = ax1.get_ylim()
ymin2, ymax2 = ax2.get_ylim()
ax1.set_xlim(0,15)
ax1.set_ylim(min([ymin1,ymin2]),max([ymax1,ymax2]))
ax2.set_ylim(min([ymin1,ymin2]),max([ymax1,ymax2]))
ax2.set_yticklabels([])
plt.sca(ax2)
plt.xlabel("Frequency")

################################################################################################
ax3 = fig.add_subplot(212)
plt.sca(ax3)

Xs = df.iloc[:,0].to_numpy()
Ws = df.iloc[:,1:-4].to_numpy()

cmap = plt.cm.rainbow
colors = cmap(np.linspace(0,1,Ws.shape[1]))

for i in range(Ws.shape[1]):
	plt.plot(Xs,Ws[:,i],color=colors[i])
	
ax3.set_xlabel("Step")
ax3.set_ylabel("Ws (kcal/mol)")
ax3.set_xlim(0,2*stepmid*xperstep/100)


################################################################################################

fig.tight_layout()
plt.subplots_adjust(top=0.95,bottom=0.15,left=0.1,right=0.95,hspace=0.4,wspace=0.15)


#################################################################################################
fileName = "Wdist-04"
fig.set_size_inches(outputwidth,outputheight)
fig.savefig(fileName+".svg",dpi=1200)



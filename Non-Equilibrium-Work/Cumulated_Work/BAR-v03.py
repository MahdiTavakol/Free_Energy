#!/usr/bin/env python3
import pandas as pd
import numpy as np
import sys
import pymbar

# ----------------------------- Argument Parsing -----------------------------
if (len(sys.argv) < 4):
	sys.stderr.write(f"Usage: {sys.argv[0]} <num_data> <num_cols> <col_indices...>\n")
	sys.exit(1)
	
num_data = int(sys.argv[1])
num_cols = int(sys.argv[2])

col_indexes = [0]

for i in range(num_cols):
	try:
		col_indexes.append(int(sys.argv[i+3])+1) # first column is for steps.
	except IndexError:
		sys.stderr.write("Not enough input arguments!")
		sys.exit(1)
	

# ----------------------------- Data Reading -----------------------------

df = pd.read_csv("ws.txt",usecols=col_indexes)


# Drop all-zero columns (after steps column)
df = df.loc[:, ~((df == 0).all())]


# ----------------------------- Work Extraction -----------------------------

steps = df.iloc[:num_data,0]
forward_works = df.iloc[:num_data,1:]
backward_works = df.iloc[num_data:,1:] - df.iloc[-1,1:]
backward_works = backward_works[::-1]



fw = forward_works.iloc[-1,:].values / 0.592
bw = -backward_works.iloc[-1,:].values / 0.592

# ----------------------------- BAR Computation -----------------------------
results = pymbar.other_estimators.bar(fw,bw)
G = -0.592*results['Delta_f']
print(G)

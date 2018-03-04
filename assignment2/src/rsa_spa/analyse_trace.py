#!/usr/bin/env python3

import sys
import numpy as np
import matplotlib as mp
import matplotlib.pyplot as plt

# Read trace input file
c = np.fromfile(sys.argv[1], np.int8)

# Calculate mean
y_mean = [np.mean(c)]*len(c)

# Trim everything before the first mean crossing
c_trim = c[np.argmax(c > y_mean):]

arr=[]
for i, (a, b) in enumerate(zip(c_trim[:-1],c_trim[1:])):
    if a <= y_mean[0] <= b:
        arr.append(i)

# TODO: Calculate 1 or 0 threshold?
# remove shit before trigger
# otherwise just ask the user
# calculate rising and falling triggers
# calculate last (and first?) points using triggers
# automatically perform (extra) filtering

thres = 1500

arr = np.array(arr)
cross = arr[1:] - arr[:-1]
ints = ((arr[1:] - arr[:-1]) > thres) * 1

# raw crossing points
print(cross)
# binary representation
print("".join(np.char.mod('%i', ints)))

# Plot graph
fig,graph = plt.subplots()
#graph.scatter(c_trim[arr], ([y_mean[0]] * len(arr)))
data_line = graph.plot(c_trim, label='Trace Data')
mean_line = graph.plot(y_mean, label='Mean', linestyle='--')
plt.show()

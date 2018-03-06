#!/usr/bin/env python3

import sys

import matplotlib.pyplot as plt
import numpy as np

# Read trace input file
c = np.fromfile(sys.argv[1], np.int8)

# Calculate mean
y_mean = [np.mean(c)] * len(c)

# Trim everything before the first mean crossing
c_trim = c[np.argmax(c > y_mean):]

start_thresh = y_mean[0] - 1

start_index = 0
for i, (a, b) in enumerate(zip(c_trim[:-1], c_trim[1:])):
    if a <= start_thresh < b:
        start_index = i
        break

c_trim = c_trim[start_index:]

stop_thresh = y_mean[0] - 2

stop_index = 0
reversed_arr = c_trim[::-1]
for i, (a, b) in enumerate(zip(reversed_arr[:-1], reversed_arr[1:])):
    if a < stop_thresh <= b:
        stop_index = len(c_trim) - i
        break

c_trim = np.append(c_trim[:stop_index], y_mean[0])

arr = []
for i, (a, b) in enumerate(zip(c_trim[:-1], c_trim[1:])):
    if a <= y_mean[0] <= b:
        arr.append(i)

# TODO: Calculate 1 or 0 threshold?
# otherwise just ask the user
# calculate rising and falling triggers
# calculate last (and first?) points using triggers
# automatically perform (extra) filtering

exp_thresh = 1500

arr = np.array(arr)
cross = arr[1:] - arr[:-1]
ints = ((arr[1:] - arr[:-1]) > exp_thresh) * 1

# binary representation
print("Result (len {}):".format(len(ints)))
print("".join(np.char.mod('%i', ints)))

# Plot graph
fig, graph = plt.subplots()
data_line = graph.plot(c_trim, label='Trace Data')
mean_line = graph.plot(y_mean, label='Mean', linestyle='--')
plt.show()

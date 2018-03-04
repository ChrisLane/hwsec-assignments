#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
Created on Wed Feb 24 14:41:23 2016

@author: david
"""

import sys

if len(sys.argv) < 2:
    print("usage: " + sys.argv[0] + " <infile> <outfile>")
    sys.exit(1)

import matplotlib as mp
import matplotlib.pyplot as plt
import numpy as np
from scipy import signal

f_s = 2.5e6

nyq = 0.5 * f_s
normal_cutoff = 1 / nyq
b2 = signal.firwin(199, [1000/nyq, 1e6/nyq])
b = signal.firwin(1700, normal_cutoff)

c = np.fromfile(sys.argv[1], np.int8)
c = c.astype(float)


c = signal.lfilter(b2, 1.0, c)

c = np.absolute(c)

y = signal.lfilter(b, 1.0, c)
 
ys = signal.decimate(y, 10);

ys = ys.astype(np.int8)
ys = ys[0:520000]

ys.tofile(sys.argv[2])

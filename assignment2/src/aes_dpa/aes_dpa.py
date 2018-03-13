#!/usr/bin/env python3

import sys
import numpy as np
import matplotlib.pyplot as plt

# For path sanitisation in find_key_candidates
from os.path import basename,splitext

from aes_sbox import *

# Takes an array of traces, in np.array 
# form and plots them on a single graph
def plot_traces(traces):
    fig, graph = plt.subplots()
    lines = []
    for num,key,trace in traces:
        line, = graph.plot(trace, label='Trace #{}'.format(num))
        lines.append(line)

    # Plot graph
    plt.legend(handles=lines)
    plt.show()

def parse_trace_ids(filelist):
    # Remove path and file extension
    files = map(splitext, map(basename, filelist))
    # Take first tuple element
    files = [x[0] for x in files]
    # Take only digits from filename
    return list(map(lambda n: int(''.join(filter(str.isdigit, n))), files))

def find_inputs(keyfile, ids):
    file = open(keyfile)
    lines = file.readlines()
    file.close()
    # Find lines specified by ids and strip whitespace
    return list(map(str.strip, [lines[i] for i in ids]))

if __name__ == "__main__":

    if len(sys.argv) < 3:
        print('usage: ' + basename(sys.argv[0]) + ' keyfile trace#0 [..trace#n]')
        sys.exit(1)

    # Load trace files and parse them into arrays
    infile = sys.argv[1]
    files = sys.argv[2:]
    ids = parse_trace_ids(files)
    inputs = find_inputs(infile, ids)
    traces = map(lambda t: np.fromfile(t, np.int8), files)

    # Combine id, input and trace into a tuple
    fulltraces = zip(ids, inputs, traces)

    # Plot all traces on a single graph
    plot_traces(fulltraces)

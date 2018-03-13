#!/usr/bin/env python3

import sys
import numpy as np
import matplotlib.pyplot as plt


# Takes an array of traces, in np.array 
# form and plots them on a single graph
def plot_traces(traces):
    fig, graph = plt.subplots()
    for trace in traces:
        graph.plot(trace, label=trace)
    # Plot graph
    plt.show()


if __name__ == "__main__":
    # Load trace files and parse them into arrays
    files = sys.argv[1:]
    traces = map(lambda t: np.fromfile(t, np.int8), files)

    # Plot all traces on a single graph
    plot_traces(traces)

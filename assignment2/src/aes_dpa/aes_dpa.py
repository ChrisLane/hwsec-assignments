#!/usr/bin/env python3

import operator
import sys
# For path sanitisation in find_key_candidates
from os.path import basename, splitext

import matplotlib.pyplot as plt
import numpy as np

from aes_sbox import *


# Takes an array of traces, in np.array
# form and plots them on a single graph
def plot_traces(traces):
    fig, graph = plt.subplots()
    lines = []
    for num, key, trace in traces:
        line, = graph.plot(trace, label='Trace #{}'.format(num))
        lines.append(line)

    # Plot graph
    plt.legend(handles=lines)
    plt.show()


# Takes an array of traces, in np.array
# form and plots them on a single graph
def plot_means(means):
    fig, graph = plt.subplots()
    key, traces = means
    [graph.plot(trace) for trace in traces]
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


# bit is 0..255, inclusive
def group_traces(k, bit, traces):
    nthbyte = int(bit / 8)

    onebin = []
    zerobin = []
    for t in traces:
        id, input, trace = t
        inbyte = int(input.split(' ')[nthbyte], 16)
        outbyte = sbox[inbyte ^ k]

        if ((outbyte >> (bit % 8)) & 1) == 1:
            onebin.append(t)
        else:
            zerobin.append(t)

    return zerobin, onebin


def guess_key(bit, traces):
    grouped = {}
    for keycnd in range(0x00, 0xFF + 1):
        _, ones = group_traces(keycnd, bit, traces)
        grouped[keycnd] = ones
        # print('byte 0x{:02x}: '.format(keycnd), len(zero), 'zeroes,', len(one), 'ones')

    return grouped


def mean(traces):
    """
    Returns the mean trace given a list of traces
    """

    def avg(i):
        return np.mean(list(map(operator.itemgetter(i), traces)))

    return [avg(i) for i in range(len(min(traces, key=len)))]


def find_bit(bit, fulltraces):
    keycnds = guess_key(bit, fulltraces)
    diffs = {}
    maxpeak = (0, 0)
    for kc, ones in keycnds.items():
        traces = list(map(operator.itemgetter(2), ones))
        # Calculate difference of means
        means = mean(traces)

        diffs[kc] = kc, []
        for tr in traces:
            diffofmean = np.subtract(means, tr)
            diffs[kc][1].append(diffofmean)

            peak = np.amax(diffofmean)
            if peak > maxpeak[1]:
                maxpeak = kc, peak

    return (maxpeak[0] >> (bit % 8)) & 1


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
    fulltraces = list(zip(ids, inputs, traces))

    keybyte = 0
    for i in range(1, 9):
        keybyte += (find_bit(i, fulltraces) << i - 1)

    print('First key byte guess is: byte 0x{:02x} ({})'.format(keybyte, keybyte))
    # Plot a particular key-candidate `i`
    # plot_means(diffs[maxpeak[0]])

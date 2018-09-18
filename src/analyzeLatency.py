#!/usr/bin/env python

import sys, os
import argparse
import numpy as np
import matplotlib.pyplot as plt
import glob
import attr
from collections import namedtuple

from pathlib import Path

Configuration = namedtuple("Configuration", "datatype numelements parallel")

@attr.s
class LatencyResult(object):
    mean = attr.ib(default=0)
    std = attr.ib(default=0)
    histX = attr.ib(default=[])
    histY = attr.ib(default=[])

    datatype = attr.ib(default="")
    numelements = attr.ib(default=0)
    parallel = attr.ib(default=0)

    def config(self):
        return Configuration(datatype=self.datatype,
                             numelements=self.numelements,
                             parallel = self.parallel)

def analyzeData(fn, data):
    result = LatencyResult()
    result.mean = np.mean(data)
    result.std = np.std(data)
    histY, histX = np.histogram(data, bins=50)
    result.histY = histY
    result.histX = 0.5*(histX[:-1]+histX[1:])

    dt, ne, pl = os.path.splitext(fn)[0].split("_")
    result.datatype = dt
    result.numelements = int(ne.replace("el", ""))
    result.parallel = int(pl.replace("par", ""))

    return result

def createReport(reportDir, pictureNames, metadata):
    txt = "# Report\n\n\n"
    txt = txt + metadata + "\n\n"
    for pic in pictureNames:
        txt = txt + "![%s](%s)\n\n" % (pic, pic)
    with open(os.path.join(reportDir, "report.md"), "w") as f:
              print (txt, file=f)

def main():
    parser = argparse.ArgumentParser(description="Analyze Latency measurement")
    parser.add_argument("-d", "--directory", required=True)

    args = parser.parse_args()

    files = glob.glob(os.path.join(args.directory, "*.txt"))
    data = {}
    metadata = "unknown device"
    for fn in files:
        if "_metadata.txt" in fn:
            metadata = open(fn).read()
            continue
        latencies = np.loadtxt(fn) / 1000  # transform ns to us
        fn = os.path.basename(fn)
        result = analyzeData(fn, latencies)
        config = result.config()
        data[config] = result

        # print (fn, result.mean)

    reportDir = Path(args.directory).parents[0] / "report"
    if not os.path.isdir(reportDir):
        os.mkdir(reportDir)


    pictureFiles = []

    num = 0
    for k, v in data.items():
        plt.figure();
        plt.bar(v.histX, v.histY,width=v.histX[1]-v.histX[0]); plt.title(k);
        outName = "%s-el%d-par%d.png" % (v.datatype, v.numelements, v.parallel)
        plt.xlabel('latency us'); plt.ylabel('frequency'); plt.grid(True)
        plt.savefig(os.path.join(reportDir, outName), dpi=150)
        plt.close()
        pictureFiles.append(outName)
        num = num + 1
        print ("%d / %d" % (num, len(data)))

    pars = np.unique(np.array([v.parallel for v in data.values()]))
    num_elements = np.unique(np.array([v.numelements for v in data.values()]))
    datatypes = set([v.datatype for v in data.values()])
    data_sorted = {}
    for dt in datatypes:
        for par in pars:
            data_sorted[(dt, par)] = sorted((v for v in data.values() if v.datatype==dt and v.parallel==par), key=lambda x: x.numelements)

    means = {kv[0]: np.array([x.mean for x in kv[1]]) for kv in data_sorted.items()}
    stds = {kv[0]: np.array([x.std for x in kv[1]]) for kv in data_sorted.items()}


    for dt in datatypes:
        plt.figure()
        for config, data in data_sorted.items():
            if config[0] != dt:
                continue
            try:
                plt.semilogy(np.log2(num_elements), means[config], '-x', lw=1, label='par=%d' % config[1])
            except ValueError as E:
                print ("ERROR: ", E, config, means[config])
                # plt.loglog(num_elements, means+stds, 'b--')
            # plt.loglog(num_elements, means-stds, 'b--')
        plt.grid(True)
        plt.xlabel('log2(elements)')
        plt.ylabel('latency us')
        plt.legend()
        plt.title(dt)
        fn = "overall_%s.png" % dt
        plt.savefig(os.path.join(reportDir, fn), dpi=150)
        pictureFiles.insert(0, fn)


    createReport(reportDir, pictureFiles, metadata)


if __name__ == '__main__':
    main()

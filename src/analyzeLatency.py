#!/usr/bin/env python

import sys, os
import argparse
import numpy as np
import matplotlib.pyplot as plt
import glob
import attr

from pathlib import Path

@attr.s
class LatencyResult(object):
    mean = attr.ib(default=0)
    std = attr.ib(default=0)
    histX = attr.ib(default=[])
    histY = attr.ib(default=[])

    datatype = attr.ib(default="")
    numelements = attr.ib(default=0)

def analyzeData(fn, data):
    result = LatencyResult()
    result.mean = np.mean(data)
    result.std = np.std(data)
    histY, histX = np.histogram(data, bins=50)
    result.histY = histY
    result.histX = 0.5*(histX[:-1]+histX[1:])

    dt, ne = os.path.splitext(fn)[0].split("_")
    result.datatype = dt
    result.numelements = int(ne)

    return result

def createReport(reportDir, pictureNames):
    txt = "# Report\n\n\n"
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
    for fn in files:
        latencies = np.loadtxt(fn)
        fn = os.path.basename(fn)
        result = analyzeData(fn, latencies)
        data[fn] = result
        
        print (fn, result.mean)

    reportDir = Path(args.directory).parents[0] / "report"
    if not os.path.isdir(reportDir):
        os.mkdir(reportDir)


    pictureFiles = []

    for k, v in data.items():
        plt.figure();
        plt.bar(v.histX, v.histY,width=v.histX[1]-v.histX[0]); plt.title(k);
        outName = "%s-%d.png" % (v.datatype, v.numelements)
        plt.savefig(os.path.join(reportDir, outName), dpi=150)
        plt.close()
        pictureFiles.append(outName)

    num_elements = np.array([v.numelements for v in data.values()])
    means = np.array([v.mean for v in data.values()])
    stds = np.array([v.std for v in data.values()])    

    index = np.argsort(num_elements)
    num_elements = num_elements[index]
    means = means[index]
    stds = stds[index]    
    plt.figure()

    plt.loglog(num_elements, means, 'b-x', lw=3)
    plt.loglog(num_elements, means+stds, 'b--')
    plt.loglog(num_elements, means-stds, 'b--')
    plt.savefig(os.path.join(reportDir, "overall.png"), dpi=150)
    pictureFiles.insert(0, "overall.png")
    

    createReport(reportDir, pictureFiles)
    

if __name__ == '__main__':
    main()


import csv
import numpy as np
import itertools as itr
import matplotlib.pyplot as plt
import sys, argparse,os
import glob
import re
import plotly.plotly as py
import plotly.graph_objs as go
import pandas

def pre_set_scale(voltList):
    """

    :param voltList:  must be a 2-d array, tuple, or list, but 1st dimension is tw_stime, second is the membrane potential.
    :return: new voltList
    """
    rounded = {}
    for entry in voltList:
        ts = int(round(entry[0]))
        volts = 0
        if entry[1] > 0:
            volts = entry[1]

        rounded[ts] = volts
    return (list(rounded.keys()),list(rounded.values()))
def clampsAxons(axelist):
    pass

def plotVoltages(voltList,fn="Neuron Membrane Potential"):
    """

    :param voltList: must be a 2-d array, 1st dimension is tw_stime, second is the membrane potential.
    :return: nada
    """
    #Plot non-scaled and then scaled
    t = []
    v = []
    for entry in voltList:
        t.append(entry[0])
        v.append(entry[1])

    print("Unscaled graphing")
    plt.plot(t,v)
    plt.title("unscaled " + fn)
    plt.show()


    times,values = pre_set_scale(voltList)
    print("Graphing...")
    plt.plot(times,values)
    plt.title(fn)
    plt.show()


def plotVoltsAxes(voltList, axonList):



    #set up the axon IDs:
    axonData = []
    axds = ['bo','r*','yh']
    fig, ax = plt.subplots()
    for temp in range(3):
        dat = axonList[axonList['axonLocal'] == temp]
        times = dat['time']
        ax.plot(times,dat['v'],axds[temp],label="Axon #{0}".format(temp))


    for extrm in axonData:
        ax.plot(extrm[0],extrm[1],extrm[3],label = "Axon #{0}".format(extrm[1]))



    ##Plot membrane potential
    times,volts = pre_set_scale(voltList)
    ax2 = ax.twinx()
    times.append(0)
    volts.append(0)

    ax2.plot(times,volts)
    ax2.set_ylabel('Membrane Potential',color='b')

    plt.show()









def loadVolts(fileName):
    return np.loadtxt(fileName,delimiter=',',skiprows=1,usecols=(0,3))
def loadAxes(fileName):
    return pandas.read_csv(fileName)


def main():
    #CLI
    parser = argparse.ArgumentParser(description="automatic validation graph tool")
    parser.add_argument('--loadAll', help="load all *_*-voltage-record.csv files",type=bool, default=False)
    parser.add_argument('--path', help="set working path", default="./", type=str)
    parser.add_argument('--logfiles', help="specify neuron membrane potential log file(s) to load",
                        default=["./0_0-voltage-record.csv"],nargs="*")
    parser.add_argument('--axChk', help="plot axon events", default=True, type=bool)


    args = parser.parse_args()
    print("Starting parser")
    print("Directory set to " + args.path)
    pth = args.path
    if not pth.endswith("/"):
        pth = pth + "/"
    #load the csv files
    print("DF is " + pth)




    if args.loadAll:
        files = glob.glob(pth + '*_*-voltage-record.csv')
        print("loadall is " + str(args.loadAll) + ", got the following list of files " + str(files))
    else:
        files = [pth + "0_0-voltage-record.csv"]

    if not args.axChk:
        for fn in files:
            print("loading up " + fn)
            plotVoltages(loadVolts(fn),fn)
    else:
        for fn in files:
            print("ld - " + fn)
            plotVoltsAxes(loadVolts(fn),loadAxes(pth + 'axon_evts.csv'))


if __name__ == '__main__':
    main()
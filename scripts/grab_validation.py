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
        #if entry[1] > 0:
        volts = entry[1]

        rounded[ts] = volts
    return (list(rounded.keys()),list(rounded.values()))
def clampsAxons(axelist):
    pass

def no_ps_scale(voltlist):
    times = []
    volts = []
    for entry in voltlist:
        ts = int(round(entry[0]))
        times.append(ts)
        volts.append(entry[1])
    return times, volts

def plotVoltages(voltList,fn="Neuron Membrane Potential",norm=False):
    """

    :param voltList: must be a 2-d array, 1st dimension is tw_stime, second is the membrane potential.
    :return: nada
    """
    #Plot non-scaled and then scaled
    plt.cla()
    t = []
    v = []
    for entry in voltList:
        t.append(entry[0])
        v.append(entry[1])


    print("Unscaled graphing")
    fig,ax = plt.subplots()
    plt.plot(t,v)
    #plt.title("phasic spiking")

    ax.set_ylabel('Membrane Potential')
    ax.set_xlabel("Simulation Ticks")
    ax.yaxis.set_major_formatter(plt.NullFormatter())
    ax.axes.get_yaxis().set_visible(False)
    #plt.show()
    sv = np.random.randint(0,1000)
    print("Saving to dir  as " + str( os.path.curdir) + "/validation_chart_" + str(sv) + ".png")
    plt.savefig("validation_chart_" + str(sv) + ".png",dpi=800,format="png")

    # times,values = pre_set_scale(voltList)
    # print("Graphing...")
    # plt.plot(times,values)
    # plt.title(fn)
    # plt.show()

def clampVolts(voltList):
    assert isinstance(voltList, list)
    vl = np.array(voltList)
    min = vl.min()
    vl = np.array(list(map(lambda xi: xi + np.abs(min), vl )))
    print(vl)
    max = vl.max()

    normlist = list(map(lambda xi: (xi - vl.min()) / vl.max()-vl.min(),vl))
    #print("Normalized is " + str(normlist))
   # zi=xi−min(x)max(x)−min
    return normlist

def plotVoltsAxes(voltList, axonList,norm=False,clamp=False,log=False):



    #set up the axon IDs:
    axonData = []
    axds = ['bo','r*','yh']
    fig, ax = plt.subplots()
    # for temp in range(3):
    #     dat = axonList[axonList['axonLocal'] == temp]
    #     times = dat['time']
    #     ax.plot(times,dat['v'],axds[temp],label="Axon #{0}".format(temp))


    for extrm in axonData:
        ax.plot(extrm[0],extrm[1],extrm[3],label = "Axon #{0}".format(extrm[1]))

    times,volts = pre_set_scale(voltList)

    ##Plot membrane potential
    if norm:
        times.insert(0,0)
        volts.insert(0,0)
    elif clamp:
        volts = clampVolts(volts)
        times.insert(0,0)
        volts.insert(0,0)

    else:
        times.insert(0,0)
        volts.insert(0,0)


    ax2 = ax.twinx()

    ax2.plot(times,volts)
    ax2.set_ylabel('Membrane Potential')
    ax2.yaxis.set_major_formatter(plt.NullFormatter())
    ax2.axes.get_yaxis().set_visible(False)
    ax.yaxis.set_major_formatter(plt.NullFormatter())
    ax.axes.get_yaxis().set_visible(False)
    if log:
        ax2.set_yscale('log')

    sv = np.random.randint(0,1000)

    print("Saving to dir  as " + str( os.path.curdir) + "/validation_chart_" + str(sv) + ".png")
    plt.savefig("validation_chart_" + str(sv) + ".png",dpi=800,format="png")
    #plt.show()
    plotVoltages(voltList)
    #plt.show()









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
    parser.add_argument('--norm', help="normalize values [0,1]", default = False, type=bool)
    parser.add_argument('--clamp', help="clamp values", default = False, type=bool)
    parser.add_argument('-l', help="log scale", default=True, type=bool)


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
            plotVoltsAxes(loadVolts(fn),loadAxes(pth + 'axon_evts.csv'),args.norm,args.clamp, args.l)


if __name__ == '__main__':
    main()

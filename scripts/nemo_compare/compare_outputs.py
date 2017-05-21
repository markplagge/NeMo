import numpy as np
import csv
import json
import sys
import matplotlib.pyplot as plt


def importNeMoOutput(inputFile):
    nemo_out = []
    print("Loading NeMo Output",end='')
    sys.stdout.flush()

    #Form of (spikeTime, destCore, destAxon)
    with open(inputFile,'r') as csvf:
        theReader = csv.reader(csvf, delimiter=',')
        dummyList = list(theReader)
        num_records = len(dummyList)
        for i,row in enumerate(dummyList):
            new_row = []
            new_row.extend([int(float(row[0])),int(row[1]),int(row[2])])
            nemo_out.append(new_row)
            if(i % int(num_records/9) == 0):
                print('.',end='')
                sys.stdout.flush()

    print()
    print("Imported NeMo Output! Loaded %i spikes"%len(nemo_out))

    return(nemo_out)


def importCompassOutput(inputFile):
    compass_out =[]
    print("Loading Compass Output", end='')
    sys.stdout.flush()

    with open(inputFile) as jsonf:
        dummyList = list(jsonf)
        num_records = len(dummyList)
        for i,jsonRow in enumerate(dummyList):
            if(i > 1): #ignore the first two rows
                dict_of_row = json.loads(jsonRow)['spike']
                new_row = []
                new_row.extend([dict_of_row['srcTime'],dict_of_row['destCore'],dict_of_row['destAxon']])
                compass_out.append(new_row)
                if(i % int(num_records/9) == 0):
                    print('.',end='')
                    sys.stdout.flush()

    print()
    print("Imported Compass Output! Loaded %i spikes"%len(compass_out))

    return(compass_out)



def create_plots(nemo_out, compass_out):
    print("Generating Spike Plot of NeMo")
    nemo_spikes = []
    for i in range(len(nemo_out)):
        nemo_spikes.append([nemo_out[i][0],nemo_out[i][2]])

    nemo_spikes = np.array(nemo_spikes)


    fig = plt.figure()
    plt.scatter(nemo_spikes[:,0],nemo_spikes[:,1],marker='.',s=.1)
    plt.title("NeMo Output Spikes")
    plt.xlabel("Simulation Time")
    plt.ylabel("DestAxon")

    print("Generating Spike Plot of Compass")
    compass_spikes = []
    for i in range(len(compass_out)):
            compass_spikes.append([compass_out[i][0],compass_out[i][2]])

    compass_spikes = np.array(compass_spikes)


    fig2 = plt.figure()
    plt.scatter(compass_spikes[:,0],compass_spikes[:,1],marker='.',s=.1)
    plt.title("Compass Output Spikes")
    plt.xlabel("Simulation Time")
    plt.ylabel("DestAxon")

    return(fig,fig2)



if __name__ == '__main__':
    nemoInput = 'nemo_neil2_output_spikes.csv'
    compassInput = 'compass_neil2_output_spikes.sfci'

    nemo_out = importNeMoOutput(nemoInput)
    compass_out = importCompassOutput(compassInput)

    (fig,fig2) = create_plots(nemo_out,compass_out)
    fig.show()
    fig2.show()


    plt.show()

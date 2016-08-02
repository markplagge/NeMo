import urwid
import npyscreen
from ctypes import *
from struct import *
import pandas as pd
import numpy as np



## Utility to read in binary spikes from various files ##
## Also can stitch together multiple csv files ##

##struct that is the C binary data

class neuronSpikeStruct(Structure) :
    _fields_ = [('timeStamp', c_double),
                ('sourceCore', c_ushort),
                ('sourceLocal', c_ushort),
                ('destinationGID', c_ulonglong)
    ]


def getBinResults(filename):
    with open (filename, 'rb') as file:
        pass
    
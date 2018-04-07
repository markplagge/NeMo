import sqlite3
import tempfile
import concurrent.futures
import copy
import json
import multiprocessing as mp
from collections import OrderedDict
from concurrent.futures import as_completed

import jsoncomment.package.comments as comments

import numpy as np
from tqdm import tqdm
from joblib import Parallel, delayed, Memory
from tn_nf2_api import TN, Spike, ConfigFile
import pandas
try:

    from numba import jit

    print("Numba JIT enabled")
except:
    print('no numba;')


    def jit(func):
        def _decorator(*args):
            return func(*args)

        return _decorator

""" Various support functions, including JSON fixing tools and Hex Array conversion tools"""


class Model():
    neuronClass = "NeuronGeneral"


def toByteArr(string):
    ba = np.array([ord(x) for x in string])
    return ba


def toStringByte(byteArr):
    assert (isinstance(byteArr, np.array))
    st = "".join(chr(x) for x in byteArr)
    return st


def hex_byte_to_bits(hex_byte):
    binary_byte = bin(int(hex_byte, base=16))
    # Use zfill to pad the string with zeroes as we want all 8 digits of the byte.
    bits_string = binary_byte[2:].zfill(8)
    return [int(bit) for bit in bits_string]


def make_unique(key, dct):
    """
	Makes each element of a JSON file unique - for parsing the TN JSON files since they have multiple identical keys/
	:param key: 
	:param dct: 
	:return: 
	"""
    counter = 0
    unique_key = key

    while unique_key in dct:
        counter += 1
        unique_key = '{}_{}'.format(key, counter)
    return unique_key


def parse_object_pairs(pairs):
    dct = OrderedDict()
    for key, value in pairs:
        if key in dct:
            key = make_unique(key, dct)
        dct[key] = value
    return dct


"""TN defines some generic neuron types. These functions return values based on those parameters"""


def NeuronGeneral():
    sigmas = [1, 1, 1, 1]
    s = [0, 0, 0, 0]
    b = False
    sigma_lambda = -1
    lambd = 0
    c_lambda = False
    epsilon = False
    alpha = 256
    beta = 0
    TM = 0
    gamma = 0
    kappa = False
    sigmaVR = 1
    VR = 1
    vvals = [sigmas, s, b, sigma_lambda, lambd, c_lambda, epsilon, alpha,
             beta, TM, gamma, kappa, sigmaVR, VR]

    return vvals


"""@todo: need more of these defs. """


def DefaultNeuronClasses(className="NeuronGeneral"):
    """
	Given a classname, we get the default object.
	:param className: 
	:return: 
	"""
    sigmas, s, b, sigma_lambda, lambd, c_lambda, epsilon, \
    alpha, beta, TM, gamma, kappa, sigmaVR, VR = eval(className + "()")

    vvals = [sigmas, s, b, sigma_lambda, lambd, c_lambda, epsilon, alpha,
             beta, TM, gamma, kappa, sigmaVR, VR]

    mnames = ["sigmas", "s", "b", "sigma_lambda", "lambd", "c_lambda", "epsilon", "alpha",
              "beta", "TM", "gamma", "kappa", "sigmaVR", "VR"]
    return dict((vname, vval) for vname, vval in zip(mnames, vvals))


"""Functions that parse components of the JSON file"""


def parseCrossbar(crossbarDef, n=256):
    """
	Given a crossbar of size n, return a hashtable representing the crossbars for use in parsing.
	:param crossbarDef: 
	:param n: 
	:return: 
	"""
    getType = lambda typeStr: int(typeStr[1])
    crossbars = {}
    for c in crossbarDef:
        name = c['name']
        cb = c['crossbar']
        rowID = 0
        synapseTypes = [0] * n
        connectivityGrid = [[]] * n

        for row in cb['rows']:
            synapseTypes[rowID] = getType(row['type'])
            superNintendoChalmers = row['synapses'].split(' ')
            superNintendoChalmers = [val for sl in [hex_byte_to_bits(byte) for byte in superNintendoChalmers]
                                     for val in sl]
            connectivityGrid[rowID] = superNintendoChalmers  # [bool(bit) for bit in superNintendoChalmers]
            rowID += 1
        crossbars[name] = [synapseTypes, connectivityGrid]

    return crossbars


def getConnectivityForNeuron(crossbarDat, crossbarName, neuronID):
    cb = crossbarDat[crossbarName]
    connectivity = cb[1]
    neuronConnectivity = []

    for row in connectivity:
        neuronConnectivity.append(row[neuronID])
    return neuronConnectivity


def getSynapseTypesForNeuron(crossbarDat, crossbarName):
    cb = crossbarDat[crossbarName]
    types = cb[0]
    typeMap = []
    for row in types:
        typeMap.append(row)


def createNeuronfromNeuronTemplate(neuronObject, coreID, localID, synapticConnectivity, typeList):
    """
	Very slow function that takes a base Neuron object (template) and changes its parameters to match
	the passed values. Sets up the neuron connectivity and types.
	
	:param neuronObject: The neuron template object to clone 
	:param coreID: The new neuron's core
	:param localID: The new neuron's local ID
	:param synapticConnectivity: A synaptic connectivity array of elements (1,0) for this neuron
	:param typeList: The array containing types 
	:return: A new neuron object
	"""
    n = copy.deepcopy(neuronObject)
    n.coreID = coreID
    n.localID = localID
    n.synapticConnectivity = synapticConnectivity
    n.g_i = typeList

    return n


def createNeuronTemplateFromEntry(line, nSynapses=256, weights=4):
    """
	Function that creates a new neuron from a dictionary. The dictionary is parsed from a line in the JSON file.
	:param line: The JSON line in a dictionary format
	:param nSynapses: number of synapses in the core
	:param weights: number of weights in the core
	:return: A new neuron shiny and chrome 
	"""
    neuron = TN(nSynapses, weights)
    sigmas = []
    s = []
    b = []
    for i in range(0, weights):
        sigmas.append(line[f'sigma{i}'])
        s.append(line[f's{i}'])
        bs = line[f'b{i}']
        bs = 0 if bs == False else 1

        b.append(bs)

    neuron.S = s
    neuron.sigmaG = sigmas
    neuron.epsilon = line['epsilon']
    neuron.sigma_lmbda = line['sigma_lambda']
    neuron.lmbda = line['lambda']
    neuron.c = line['c_lambda']
    neuron.epsilon = line['epsilon']
    neuron.alpha = line['alpha']
    neuron.beta = line['beta']
    neuron.TM = line['TM']
    neuron.gamma = line['gamma']
    neuron.kappa = line['kappa']
    neuron.sigmaVR = line['sigma_VR']
    neuron.VR = line['VR']
    neuron.membrane_potential = line["V"]

    return neuron


def convertNeuronName(neuronName):
    """
	Converts the neuron type name to an integer. The input is in hex. The actual naming doesn't particularly matter in
	this case so this function makes any changes consistent
	:param neuronName: neuron type name of format: hex string
	:return: integer representing neuron type
	"""
    return int(neuronName, 16)


def getNeuronModels(neuronTypes, nsynapses=256, nweights=4):
    """
	Generates a dictionary of predefined neuron types (per the neuron templates desc. in the TN documentation)
	:param neuronTypes: 
	:param nsynapses: 
	:param nweights: 
	:return: 
	"""
    neuronTemplates = {}
    for name, vals in zip(neuronTypes.keys(), neuronTypes.values()):
        name = name.replace("N", "")
        neuronTemplates[name] = createNeuronTemplateFromEntry(vals, nsynapses, nweights)
        neuronTemplates[name].nnid = convertNeuronName(name)
    return neuronTemplates


""" File read functions  """


def readTN(filename):
    """
	Initial pre-process of JSON file. 
	:param filename: 
	:return: A tuple of the model info, neuron types defined in the model, a list of crossbars, and a list of the cores
	"""
    print("Loadfing templates from JSON file...")
    data = open(filename, 'r').readlines()
    data = comments.json_preprocess(data)
    tnData = json.loads(data, object_pairs_hook=parse_object_pairs)
    mdl = tnData['model']
    crossbarDat = tnData['crossbarTypes']
    coreDat = []
    for k in tnData.keys():
        if "core" in k:
            coreDat.append(tnData[k])

    cbid = 0

    for core in coreDat:
        if 'name' not in core['crossbar'].keys():
            core['crossbar']['name'] = f"cb{cbid}"

            crossbarDat.append({'crossbar': core['crossbar'], 'name': f"cb{cbid}"})
            cbid += 1

    model = Model()
    model.params = mdl
    model.neuronClass = mdl['neuronclass']
    neuronTypes = {}

    for nt in tnData['neuronTypes']:
        neuronTypes[nt['name']] = DefaultNeuronClasses(nt['class'])
        # filtered = dict((k,val) for k,val in zip(nt.keys(), nt.values()) if k!= 'name')
        # for k,v in filtered:
        # 	neuronTypes[nt['name']][k] = v
        neuronTypes[nt['name']] = dict((k, v) for k, v in zip(nt.keys(), nt.values()) if k != 'name')

        defaults = DefaultNeuronClasses(nt['class'])
        for k, v in zip(defaults.keys(), defaults.values()):

            if k not in neuronTypes[nt['name']]:
                neuronTypes[nt['name']][k] = v

    for nk in neuronTypes.keys():
        n_num = nk.replace('N', '')
        n_num = convertNeuronName(n_num)

        neuronTypes[nk]['nid'] = n_num

    crossbarDat = parseCrossbar(crossbarDat, mdl['crossbarSize'])
    return (mdl, neuronTypes, crossbarDat, coreDat)


"""JSON Multiplicity Parsing Functions - Handles the section of  JSON files that are of the format
"0:100", "1","2", "5x30".. etc..."""


def getArrayFromMulti(jItem):
    increment = 1
    if jItem.count(':') >= 1:
        vals = jItem.split(":")

        if jItem.count(':') == 1:
            start = int(vals[0])
            end = int(vals[1]) + 1
        else:
            start = int(vals[0])
            end = int(vals[2]) + 1
            increment = int(vals[1])

        rng = np.arange(start, end, increment, dtype=np.int32)
        return rng
    elif jItem.count('x') == 1:
        sp = jItem.split('x')
        num = int(sp[1])
        val = int(sp[0])
        its = [val] * (num)
        return np.array(its, dtype=np.int32)
    else:
        return [int(jItem)]


def populateArray(inputArr, lines):
    pos = 0
    for typeItem in lines:

        itms = getArrayFromMulti(str(typeItem))
        for itm in itms:
            inputArr[pos] = itm
            if pos > 255:
                print("Err - line item {} pushed past the end of the array from {}".format(itm, lines))
                # raise IndexError("Error OOB from lineitem")

                return inputArr
            pos += 1

    return inputArr


"""Main Model JSON reader"""

gmodel = []
gneuronTypes = []
gcrossbars = []
gcores = []
gtemplates =[]

def createCFGProdCon(cores,crossbars,nc,neuronTemplates,cfgFile,compWork, coreWork):
    def rundmc():
        while not compWork.empty() or not coreWork.empty():
            v = compWork.get()
            for c in v:
                cfgFile.addNeuron(c)

    for ch in cores:
        coreWork.put([ch])

    with concurrent.futures.ProcessPoolExecutor() as e:
        workers = []
        parsers = []
        for i in range(0,mp.cpu_count()):
            workers.append(e.submit(neuronCSVWorker,ch,crossbars,nc,neuronTemplates))
            parsers.append(e.submit(rundmc,))
            for p in tqdm(as_completed(workers), desc="JSON Parsing", position=0):
                pass
            for q in tqdm(as_completed(parsers), desc="assembling"):
                pass
    return cfgFile

def rundmc(ch,crossbars,nc,neuronTemplates,dq):
        datum = neuronCSVFut(ch,crossbars,nc,neuronTemplates)
        for v in datum:
            dq.push(v)

def createNeMoCFGFromJson(filename, modelFN='nemo_model.nfg1'):
    model, neuronTypes, crossbars, cores = readTN(filename)
    print("Configuring templates from JSON file")
    neuronTemplates = getNeuronModels(neuronTypes)
    nullNeuron = TN(256, 4)
    nullNeuron.S = [0, 0, 0, 0]
    ntmp = {}
    for nt in neuronTemplates.values():
        ntmp[nt.nnid] = nt

    neuronTemplates = ntmp
    nc = 0
    gmodel = model
    gneuronTypes = neuronTypes
    gcrossbars = crossbars
    gtemplates = neuronTemplates
    cfgFile = ConfigFile(modelFN, nc=len(cores))

    print("Generating CSV...")
    data = []

    compWork = mp.Queue()
    coreWork = mp.Queue()


    print()

    print("Priming JSON conversion...")
    nw = int(mp.cpu_count() / 2)

    manager = mp.Manager()
    compWork = manager.Queue()
    coreWork = manager.Queue()
    corect = 0
    for ch in cores:
        coreWork.put([ch])
        corect += 1
    wrkrs = []
    # for i in range(nw):
    #     p = mp.Process(target=neuronCSVWorker,args=(crossbars,nc,neuronTemplates,coreWork,compWork))
    #     p.start()
    #
    #     wrkrs.append(p)
    #
    # for
    with concurrent.futures.ProcessPoolExecutor(max_workers=nw) as e:
        f = []
        tw = []
        for i in range(nw):

            f.append(e.submit(neuronCSVWorker,crossbars,nc,neuronTemplates,coreWork,compWork))
        #for _ in tqdm(as_completed(f), position=0,total=nw):
        print("Converting JSON Neurons...")

        with(tqdm(total=corect,position=0,desc="Parsing core data...")) as tq:
            cc=0

            while(not compWork.empty() or not coreWork.empty()):
                    cc += 1
                    for d in tqdm(compWork.get(), position=1,desc="Converting core"):
                        cfgFile.addNeuron(d)
                    tq.update(cc)
                    cc = 0
            tq.update(cc)
    #emg cleanup:
    print("JSON convert Complete... cleaning up and continuing processing.")
    while(not compWork.empty()):
        for d in compWork.get():
            cfgFile.addNeuron(d)
    return cfgFile

    # with concurrent.futures.ProcessPoolExecutor(max_workers=mp.cpu_count()) as e:
    #     f = []
    #     for ch in cores:
    #         ch = [ch]
    #         f.append(e.submit(rundmc, ch, crossbars, nc, neuronTemplates,cfgFile.neuronQueue))
    #     print("Running JSON--> NeMo neuron conversion...")
    #     for dti in tqdm(as_completed(f), desc="Parsing core data and assembling text",position=0):
    #         while(not cfgFile.neuronQueue.empty()):
    #             cfgFile.calcMPNeuron(cfgFile.neuronQueue.get())
    #
    #     cfgFile.finishMP()

    #return cfgFile


    data = Parallel(n_jobs=mp.cpu_count(), verbose=10)(
        delayed(neuronCSVFut)([cores[i]], crossbars,nc,neuronTemplates) for i in range(len(cores)))
    for d in tqdm(data,position=0):
        for v in tqdm(d, position=1):
            cfgFile.addNeuron(v)
    return cfgFile
    with concurrent.futures.ProcessPoolExecutor(max_workers=mp.cpu_count(),batch_size=32) as e:
        f = []
        thd = []

        for ch in cores:
            ch = [ch]
            f.append(e.submit(neuronCSVFut, ch, crossbars, nc, neuronTemplates))

        kwargs = {
            'total': len(f),
            'unit': 'nap',
            'unit_scale': True,
            'leave': True
        }
        print("Running JSON neuron conversion...")

        for dti in tqdm(as_completed(f), desc="Parsing TN Core Data", total=len(f), position=0):
            #data.append(dti.result())
            data = dti.result()
           # thd.append(e.submit(rundmc, data))
            for v in tqdm(data, position=1, desc="Generating config data from core"):
                cfgFile.addNeuron(v)

        print("assembling text...")
        for proc in tqdm(as_completed(thd)):
            pass

        # for n in tqdm(data):
        # 	for v in n:
        # 		cfgFile.addNeuron(v)

        # [[cfgFile.addNeuron(v) for v in n] for n in data]

    # cfgFile.neuron_text = cfgFile.neuron_text + data
    return cfgFile


neuronQueue = mp.Queue()

def compileNeuronTxt(neurons):
    pass

def neuronCSVWorker(crossbars, nc, neuronTemplates,coreWork,compWork):
    while not coreWork.empty():
        compWork.put(neuronCSVFut(coreWork.get(),crossbars,nc,neuronTemplates))



def neuronCSVFut(cores, crossbars=0, nc=0, neuronTemplates=0):
    """
	Multiprocessing parser for TrueNorth
	:param cores: 
	:param crossbars: 
	:param nc: 
	:param neuronTemplates: 
	:return: 
	"""
    d = ""
    coreNeuronTypes = np.zeros(256, dtype=np.int32)
    destCores = np.zeros(256, dtype=np.int32)
    destAxons = np.zeros(256, dtype=np.int32)
    destDelays = np.zeros(256, dtype=np.int32)

    for core in cores:
        crossbar = crossbars[core['crossbar']['name']]

        tp = ""

        populateArray(coreNeuronTypes, (core['neurons']['types']))

        populateArray(destCores, (core['neurons']['destCores']))
        populateArray(destAxons, (core['neurons']['destAxons']))
        populateArray(destDelays, (core['neurons']['destDelays']))
        coreID = core['id']

        # synapse types:
        tl = crossbar[0]
        nc += 1
        nrs = []
        # core data configured, create neurons for this core:
        # genNeuronBlock(coreID, coreNeuronTypes, crossbar, destAxons, destCores, destDelays, neuronTemplates, neurons, tl)
        for i in range(0, 256):
            # get synaptic connectivity col for this neuron:
            connectivity = np.array(crossbar[1])[:, i]
            if coreNeuronTypes[i] in neuronTemplates.keys():
                neuron = createNeuronfromNeuronTemplate(neuronTemplates[coreNeuronTypes[i]], coreID, i,
                                                        connectivity,
                                                        tl)
                neuron.destCore = destCores[i]
                neuron.destLocal = destAxons[i]
                neuron.signalDelay = destDelays[i]
                # d = d + neuron.to_csv()
                if neuron.destCore < 0 or neuron.destLocal < 0:
                    neuron.selfFiring = 1
                nrs.append(neuron)
            #			else:
            #				neuron = TN(256, 4)

        # for i in nrs:
        # ic = i.to_csv()
        # d = "{}{}".format(d, ic)  # d + i.to_csv()
        # cfgFile.add_neuron(neuron)
        # coreNeuronTypes.fill(0)
        # destCores.fill(0)
        # destAxons.fill(0)
        # destDelays.fill(0)
        return nrs


"""Spike file reader"""


def readSpikeJSON(filename):
    data = open(filename, 'r').readlines()

    spikes = []
    for line in data:
        spike = json.loads(line)
        if 'spike' in spike.keys():
            if "srcTime" in spike['spike'].keys():
                spike = spike['spike']
                spk = Spike(spike['srcTime'], spike['destCore'], spike['destAxon'])
                spikes.append(spk)

    return spikes


def readSpikeFile(filename, type='json'):
    return readSpikeJSON(filename)

def line_prepender(filename, line):
    with open(filename, 'r+') as f:
        content = f.read()
        f.seek(0, 0)
        f.write(line.rstrip('\r\n') + '\n' + content)


def readAndSaveSpikeFile(filename, type="json", saveFile="spikes.csv"):
    spikes = readSpikeFile(filename, type)

    out = [s.toCSV() for s in spikes]
    fp = tempfile.SpooledTemporaryFile(max_size=8192,mode='w+t')
    fp.writelines(out)
    with open(saveFile, 'w') as f:
        f.writelines(out)



#    fp = tempfile.TemporaryFile(mode='w+t')

#    fp.write("time,core,axon\n")
#    with open(saveFile, 'r') as f:
#        fp.writelines(f.read())
    fp.seek(0)
    df = pandas.read_csv(fp, header=None,names=["time","core","axon"])

    fp.close()
    print("sqlite generation - post csv parsing...")
    #SQLITE DB FOR SPIKES:
    fn = saveFile.split('.')[0] + ".sqlite"
    #    con.execute(tblcreate)
    
        
    
    con = sqlite3.connect(fn)
    df.to_sql('input_spikes',con,if_exists='replace',index=False)

    #c = conn.cursor()
    tblcreate = '''CREATE TABLE "input_spikes"(
	"time" Integer,
	"core" Integer,
	"axon" Integer );'''
    indxcreate = '''CREATE INDEX "idx1" ON "input_spikes"( "core" DESC, "axon" DESC ); '''
    con.execute(indxcreate)
#    rawdat = [s.toDB() for s in spikes] 
#    con.executemany("INSERT INTO input_spikes(time,core,axon) values (?,?,?)", rawdat)
    con.close()


if __name__ == '__main__':
    # print("Testing read TN Json")
    # print("Example 1 JSON file loading")
    # ex1_model = createNeMoCFGFromJson('./test/mnist.json','neil_mnist_nemo_model.nfg1')
    # ex1_model.closeFile()
    # print("Example 1 JSON Spike load")
    # readAndSaveSpikeFile('./test/ex1_spikes.sfti', saveFile='nemo_spike.csv')

    print("Neil JSON load")
    n_mod = createNeMoCFGFromJson('/Users/mplagge/development/NeMo/scripts/test/NM/mnist_neil.json',
                                  '/Users/mplagge/development/NeMo/scripts/nemo_model.nfg1')
    n_mod.closeFile()

    readAndSaveSpikeFile('/Users/mplagge/development/NeMo/scripts/test/NM/mnist_neil_spikes.sfci',
                         '/Users/mplagge/development/NeMo/scripts/nemo_spike.cfg')

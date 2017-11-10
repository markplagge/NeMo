import io
import numpy as np
import multiprocessing as mp

class TN(object):
    type = "TN"
    coreID = 0
    localID = 0
    synapticConnectivity = []
    g_i = []  # type of ith axon
    sigmaG = []  # sign of the ith axon
    S = []  # weights
    b = []  # deterministic / stochastic mode select (integration, per weight)
    epsilon = 0  # monotonic/divergent leak selection
    sigma_lmbda = -1  # leak sign bit
    lmbda = 0  # Leak value
    c = 0  # selects between stochastic leak and det. leak mode.
    alpha = 0  # positive threshold
    beta = 0  # negative threshold
    TM = 0  # encoded pseudo-random number mask: (2^TM - 1)
    VR = 0  # encoded reset potential: sigma^VR (2^VR -1)
    sigmaVR = 0  # reset voltage sign bit
    gamma = 0  # Vj(t) reset mode
    kappa = 0  # negative reset mode, sets negative threshold mode to reset or saturate
    signalDelay = 0  # TN Signal Delay option.
    destCore = 0  # dest simulation core
    destLocal = 0  # dest simulation neuron
    outputNeuron = 0  # is this a neuron that belongs to an output layer?
    selfFiring = 0  # is this neuron capable of spontaneous firing?

    def __init__(self, numSynapsesPerCore, weightsPerNeuron):
        """Creates a connected neuron with random weights, no leak, and a threshold of 10."""
        self.synapticConnectivity = [0] * numSynapsesPerCore
        self.g_i = [0] * numSynapsesPerCore  # [random.randint(0,4)],size=256)
        self.sigmaG = [1] * weightsPerNeuron
        self.S = [1] * weightsPerNeuron
        self.synaptic_weights = [0] * weightsPerNeuron  # random.randint(0,5,size=4)
        self.b = [0] * 4
        self._numSyns = numSynapsesPerCore

    def sanity_check(self):
        assert (len(self.g_i) == len(self.synapticConnectivity))
        assert (len(self.g_i) == self._numSyns)

    def setOut(self):
        if self.destCore < 0:
            self.outputNeuron = "1"
        else:
            self.outputNeuron = "0"

    def nf2GS(self, pname):
        def toI(vv):
            if vv == True:
                return "1"
            if vv == False:
                return "0"
            return vv

        assert (isinstance(pname, str))
        val = self.__getattribute__(pname)
        template = f"{pname} = "
        if isinstance(val, (list, np.ndarray)):
            val = [toI(v) for v in val]
            template = template + "{ "
            outstr = template + ",".join([f"{x}" for x in val])
            outstr = f"{outstr} {'}'}"
        else:
            val = toI(val)
            if pname is 'type':
                val = f'"{val}"'

            outstr = f"{template} {val}"
        return f"{outstr} "

    def _toI(val):
        if val == True:
            return "1"
        if val == False:
            return "0"
        return val

    def toNeMoStr(self):
        self.setOut()
        self.sanity_check()

        hdr = ["type", "coreID", "localID"]
        arrs = ["synapticConnectivity", "g_i", "sigmaG", "S", "b"]
        singles = ["epsilon",
                   "sigma_lmbda",
                   "lmbda",
                   "c",
                   "alpha",
                   "beta",
                   "TM",
                   "VR",
                   "sigmaVR",
                   "gamma",
                   "kappa",
                   "signalDelay",
                   "destCore",
                   "destLocal",
                   "outputNeuron",
                   "selfFiring"
                   ]
        name = f"{self.type}_{self.coreID}_{self.localID}"
        full = hdr + arrs + singles

        fn = ",".join([self.nf2GS(pn) for pn in full])

        return f"{name} = {'{'} {fn} {'}'} "


class ConfigFile(object):
    nsCores = 1024
    neuronsPerCore = 256
    it = 0
    destination = 'nemo_model.nfg1'
    fileDat = []
    neuronQueue = mp.Queue()
    datQueue = mp.Queue()

    def __init__(self, destFileName, nc=1024, npc=256, nw=4):
        if ".nfg1" not in destFileName:
            destFileName = f"{destFileName}.nfg1"
        self.ns_cores = nc
        self.neurons_per_core = npc
        self.neuron_weight_count = nw
        self.destination = destFileName
        self.openFile()
        self.header = f"cores = {nc} \n neuronsPerCore = {npc} \n" \
                      f"neuron_weights = {nw} \n " \
                      "neurons = { "
        self.fhandle.write(self.header)


    def openFile(self):
        self.fhandle = open(self.destination, mode='w')

    def closeFile(self):
        self.fhandle.write(",\n".join(self.fileDat))
        self.fhandle.write(" }")
        self.fhandle.close()

    def addNeuron(self, neuron):
        self.fileDat.append(f"{neuron.toNeMoStr()}")

    def addMPNeuron(self, neuron):
        self.neuronQueue.put(neuron)

    def calcMPNeuron(self,neuron):
        self.datQueue.put( f"{neuron.toNeMoStr()}" )

    def finishMP(self):
        while not self.datQueue.empty():
            d = self.datQueue.get()
            self.fileDat.append(d)







def mpAddNeuron(self, neurons):
    pass




class Spike(object):
    time = 0.0
    destCore = 0
    destAxon = 0

    def __init__(self, time, destCore, destAxon):
        self.time = time
        self.destcore = destCore
        self.destAxon = destAxon

    def toCSV(self):
        return f"{self.time},{self.destCore},{self.destAxon}\n"


if __name__ == '__main__':
    n = TN(numSynapsesPerCore=256, weightsPerNeuron=4)
    c = ConfigFile("testConfigFile.nfg1")
    c.addNeuron(n)
    c.closeFile()

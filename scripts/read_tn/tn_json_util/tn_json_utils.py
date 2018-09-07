### TN object represents a true north object

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

    def __init__(self, num_synapses_per_core=256, weights_per_neuron=4):
        """Creates a connected neuron with random weights, no leak, and a threshold of 10."""
        self.synapticConnectivity = [0] * numSynapsesPerCore
        self.g_i = [0] * numSynapsesPerCore  # [random.randint(0,4)],size=256)
        self.sigmaG = [1] * weightsPerNeuron
        self.S = [1] * weightsPerNeuron
        self.synaptic_weights = [0] * weightsPerNeuron  # random.randint(0,5,size=4)
        self.b = [0] * 4
        self._numSyns = numSynapsesPerCore

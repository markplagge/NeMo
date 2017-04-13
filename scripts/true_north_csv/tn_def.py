import itertools
from functools import reduce
import numpy.random as random
import numpy as np

class TN:
	type="TN"
	coreID = 0

	localID = 0
	synapticConnectivity = []

	g_i = []  # type of ith neuron
	sigmaG = []  # sign of ith neuron
	S = []  # synaptic weight table
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




	def to_csv(self):
		self.sanity_check()
		os = lambda xx, yy: str(xx) + "," + str(yy)
		ocsv = "{},{},{},".format(self.type, self.coreID, self.localID)
		for elms in [self.synapticConnectivity, self.g_i, self.sigmaG, self.S, self.b]:
			ocsv += reduce(os, elms) + ","
		for var in [self.epsilon,
					self.sigma_lmbda,
					self.lmbda,
					self.c,
					self.alpha,
					self.beta,
					self.TM,
					self.VR,
					self.sigmaVR,
					self.gamma,
					self.kappa,
					self.signalDelay,
					self.destCore,
					self.destLocal,
					self.outputNeuron]:
			ocsv += f"{var}"

		ocsv += f"{self.selfFiring} \n" # Final line of csv

		return ocsv



	def sanity_check(self):
		assert (len(self.g_i) == len(self.synapticConnectivity))
		assert (len(self.g_i) == self._numSyns)




class ConfigFile:
	neuron_list = []
	ns_cores = 1024
	neurons_per_core = 256
	neuron_weight_count = 4

	def __init__(self,neuronList,ns_cores=1024,neurons_per_core=256,neuron_weight_count=4):
		#Set up the list of neurons:

		for i in neuronList:
			self.neuron_list.append(i)

	def add_neuron(self, neuron):
		self.neuron_list.append(neuron)

	def to_csv(self):
		out = ""
		out += str(self.ns_cores) + "\n"
		out += str(self.neurons_per_core) + "\n"
		out += str(self.neuron_weight_count) + "\n"
		for n in self.neuron_list:
			out += n.to_csv()
		return out

	def save_csv(self, filename):
		with open(filename, 'w') as f:
			f.write(self.to_csv())





def testNeuronFile():
	#generates two cores - one is random, the other is a synaptic id matrix benchmark, connected to the first core.


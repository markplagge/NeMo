from functools import reduce
from numba import jit
import numpy as np


def subCSV(xx, yy):
	return str(xx) + "," + str(yy)


import io


class TN:
	type = "TN"
	coreID = 0
	localID = 0
	synapticConnectivity = []
	g_i = []  # type of ith neuron
	# smaller arrays
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

	def sanity_check(self):
		assert (len(self.g_i) == len(self.synapticConnectivity))
		assert (len(self.g_i) == self._numSyns)

	def os(self, xx, yy):
		# return f"{xx},{yy}"
		return f"{xx},{yy}"
		return "%s,%s" % (xx, yy)
		return str(xx) + "," + str(yy)

	def to_csv(self):
		if self.destCore > 0:
			self.outputNeuron = 1
		else:
			self.outputNeuron = 0

		self.sanity_check()
		# os = lambda xx, yy: str(xx) + "," + str(yy)
		sio = io.StringIO()
		sio.write(f"{self.type},{self.coreID},{self.localID},")
		# p1 = "{},{},{},".format(self.type, self.coreID, self.localID)
		svs = []
		for elms in [self.synapticConnectivity, self.g_i, self.sigmaG, self.S, self.b]:
			#p2 = reduce(self.os, elms) + ","
			sio.write(f"{reduce(self.os,elms)},")

		sio.write = f"{self.epsilon},{self.sigma_lmbda},{self.lmbda},{self.c},{self.alpha},{self.beta},{self.TM},{self.VR}," \
					f"{self.sigmaVR},{self.gamma},{self.kappa},{self.signalDelay},{self.destCore},{self.destLocal},{self.outputNeuron}," \
					f"{self.selfFiring}"
		# for var in [self.epsilon,
		# 			self.sigma_lmbda,
		# 			self.lmbda,
		# 			self.c,
		# 			self.alpha,
		# 			self.beta,
		# 			self.TM,
		# 			self.VR,
		# 			self.sigmaVR,
		# 			self.gamma,
		# 			self.kappa,
		# 			self.signalDelay,
		# 			self.destCore,
		# 			self.destLocal,
		# 			self.outputNeuron]:
		# 	p3 += "{},".format(var)

		# p4 += str(self.selfFiring) + "\n"  # final

		# oscv = f"{p1}{p2}{p3}"

		return sio.getvalue() + "\n"


class ConfigFile:
	neuron_list = []

	ns_cores = 1024
	neurons_per_core = 256
	neuron_weight_count = 4
	it = 0
	neuron_text = ''

	def __init__(self, nc=1024, npc=256, nw=4):
		self.ns_cores = nc
		self.neurons_per_core = npc
		self.neuron_weight_count = nw
		self.neuron_text = self.getHeader()

	def add_neuron(self, neuron):
		self.neuron_list.append(neuron)
		self.it += 1
		if self.it > 512:
			for i in self.neuron_list:
				self.neuron_text += i.to_csv()
			self.neuron_list = []
			self.it = 0

	def getHeader(self):
		out = ""
		out += str(self.ns_cores) + "\n"
		out += str(self.neurons_per_core) + "\n"
		out += str(self.neuron_weight_count) + "\n"
		return out

	def to_csv(self):
		out = self.neuron_text
		for n in self.neuron_list:
			out += n.to_csv()
		return out

	def save_csv(self, filename):
		with open(filename, 'w') as f:
			f.write(self.to_csv())


class Spike:
	time = 0.0
	destCore = 0
	destAxon = 0

	def __init__(self, time, destCore, destAxon):
		self.time = time
		self.destcore = destCore
		self.destAxon = destAxon

	def toCSV(self):
		return f"{self.time},{self.destCore},{self.destAxon}\n"


# Vnames sets the variable names and order.
vnames = ["name", "cls" "sigmas", "s", "b", "sigma_lambda", "lambd", "c_lambda",
		  "epsilon", "alpha", "beta", "TM", "gamma", "kappa", "sigmaVR", "VR", "V"]
vnamedefs = ['type']


def TNFunct(**kwargs):
	t = TN(256, 4)
	for key, value in kwargs:
		t.key = value
	return t.to_csv()


if __name__ == '__main__':
	n = TN(numSynapsesPerCore=256, weightsPerNeuron=4)
	n.to_csv()

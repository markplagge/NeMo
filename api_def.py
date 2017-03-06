from functools import reduce


def subCSV(xx, yy):
	return str(xx) + "," + str(yy)
class TN:
	type = "TN"
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



	def to_csv(self):
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
			ocsv += "{},".format(var)

		ocsv += str(self.selfFiring) + "\n"  # final

		return ocsv

		return out


if __name__ == '__main__':
	print("Generate a tonic spiking neuron demo file (demo.csv) from the API.")
	n0 = TN(256,4)
	n0.synapticConnectivity = [1] + ([0] * 255)
	print("SC Size is: " + str(len(n0.synapticConnectivity)))
	n0.S = [3,0,0,0]
	n0.epsilon = 0
	n0.lmbda = 0
	n0.c = 0
	n0.alpha = 32
	n0.beta = 0
	n0.TM = 0
	n0.sigmaVR = 1
	n0.VR = 0
	n0.kappa = 1
	n0.gamma = 0
	n0.outputNeuron = 1
	n0.selfFiring = 1
	n0.destCore = 0
	n0.destLocal = 0
	n0.sigmaG = [1,1,1,1]

	n1 = TN(256,4)
	n1.synapticConnectivity = [1] * 256
	n1.S = [1,1,1,1]
	n1.alpha = 1
	n1.destCore=0
	n1.destLocal=0
	n1.coreID = 1
	n1.localID = 0
	
	n2 = n1
	n2.destCore=1
	n2.destLocal=0
	n2.coreID = 1
	n2.localID = 1
	
	n0CSV = n0.to_csv()
	
	with open("demo_ts.csv", 'w') as f:
		f.write(n0CSV)
		f.write(n1.to_csv())
		f.write(n2.to_csv())

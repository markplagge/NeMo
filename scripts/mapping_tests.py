
#test cases:
class test():

	def __init__(self, lid, mycore, destcore, destneuron, mychip=0, destchip=0):

		myLid = lid

		myCore = mycore

		destCore = destcore
		destNeuron = destneuron

		myChip = mychip
		destChip = destchip



	def testDest(self, destCore, destNeuron, destChip):
		return self.destCore == destCore and self.destNeuron == destNeuron and self.destCHip == destChip


num_cores = 4096
cores_per_chip = 64


def createNeurons(num_cores, cpc, num_layers, cores_in_layer):
	for i in range(0, num_cores):
		for j in range(0, )


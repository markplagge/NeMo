import copy
import typing
from typing import List
import typing

import json
from jsoncomment import JsonComment
from api_def import TN

""" JSON REMOVE COMMMENTS"""
import re

""" MAIN APPLICATION """

vnames = ["name", "cls" "sigmas", "s", "b", "sigma_lambda", "lambd", "c_lambda",
		  "epsilon", "alpha", "beta", "TM", "gamma", "kappa", "sigmaVR", "VR", "V"]


class Model():
	neuronClass = "NeuronGeneral"


class NeuronType():
	def __init__(self, model, name, sigmas, s, b, sigma_lambda, lambd, c_lambda, epsilon, alpha,
				 beta, TM, gamma, kappa, sigmaVR, VR, V, cls=""):
		vvals = [name, sigmas, s, b, sigma_lambda, lambd, c_lambda, epsilon, alpha,
				 beta, TM, gamma, kappa, sigmaVR, VR, V, cls]

		self.model = model
		if cls == "":
			cls = model.neuronClass
		self.params = dict((vname, vval) for vname, vval in zip(vnames, vvals))


class Core():
	def __init__(self, id, rngSeed, neurons, crossbar):
		params = {}
		self.id = id
		self.rngSeed = rngSeed
		self.neurons = neurons
		self.crossbar = crossbar


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


def DefaultNeuronClasses(className="NeuronGeneral"):
	sigmas, s, b, sigma_lambda, lambd, c_lambda, epsilon, \
	alpha, beta, TM, gamma, kappa, sigmaVR, VR = eval(className + "()")

	vvals = [sigmas, s, b, sigma_lambda, lambd, c_lambda, epsilon, alpha,
			 beta, TM, gamma, kappa, sigmaVR, VR]

	mnames = ["sigmas", "s", "b", "sigma_lambda", "lambd", "c_lambda", "epsilon", "alpha",
			  "beta", "TM", "gamma", "kappa", "sigmaVR", "VR"]
	return dict((vname, vval) for vname, vval in zip(mnames, vvals))


def hex_byte_to_bits(hex_byte):
	binary_byte = bin(int(hex_byte, base=16))
	# Use zfill to pad the string with zeroes as we want all 8 digits of the byte.
	bits_string = binary_byte[2:].zfill(8)
	return [int(bit) for bit in bits_string]


def parseCrossbar(crossbarDef, n=256):
	synapseTypes = [0] * n
	connectivityGrid = [[]] * n
	getType = lambda typeStr: int(typeStr[1])
	crossbars = {}
	for c in crossbarDef:
		name = c['name']
		cb = c['crossbar']
		rowID = 0
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


# def createNeuronfromNeuronTemplate(neuronObject:TN, coreID:int, localID:int, synapticConnectivity:List[int])->TN:


def createNeuronfromNeuronTemplate(neuronObject, coreID, localID, synapticConnectivity, typeList):
	n = copy.deepcopy(neuronObject)
	n.coreID = coreID
	n.localID = localID
	n.synapticConnectivity = synapticConnectivity
	n.g_i = typeList

	return n


def createNeuronTemplateFromEntry(line, nSynapses=256, weights=4):
	neuron = TN(nSynapses, weights)
	sigmas = []
	s = []
	b = []
	for i in range(0, weights):
		sigmas.append(line[f'sigma{i}'])
		s.append(line[f's{i}'])
		bs = line[f'b{i}']
		bs = 0 if bs == False else True

		b.append(bs)

	neuron.S = s
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


def getNeuronModels(neuronTypes, nsynapses=256, nweights=4):
	neuronTemplates = {}
	for name, vals in zip(neuronTypes.keys(), neuronTypes.values()):
		neuronTemplates[name] = createNeuronTemplateFromEntry(vals, nsynapses, nweights)
	return neuronTemplates


import re
import jsoncomment.package.comments as comments


def uniqueify(jsData):
	coreCount = 0

	jsLines = []  # jsData.split()

	for l in jsData.split():
		if "core" in l:
			line = str.replace(l, '"core"', f'"core{coreCount}"')
			coreCount += 1
		else:
			line = l
		jsLines.append(line)

	jsData = ""
	for l in jsLines:
		jsData += l
	return jsData


def tnJSONHandler(di):
	if 'coreCount' in di.keys():
		model = Model()
		model.params = di
		return model

	if 'crossbar' in di.keys():
		return {'crossbarTypes': di}

	return di


from collections import OrderedDict


def make_unique(key, dct):
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


def readTN(filename):
	# with open(filename, 'r') as f:
	#	data = f.readLines()

	data = open(filename, 'r').readlines()

	data = comments.json_preprocess(data)

	data = uniqueify(data)

	# tnData = json.loads(data)
	#tnData = json.load(open(filename, 'r'), object_pairs_hook=tnJSONHandler)
	tnData = json.load(open(filename,'r'), 	object_pairs_hook=parse_object_pairs)

	mdl = tnData['model']
	crossbarDat = tnData["crossbarTypes"]
	coreDat = []
	for k in tnData.keys():
		if "core" in k:
			coreDat.append(tnData[k])

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
	crossbarDat = parseCrossbar(crossbarDat, mdl['crossbarSize'])
	return (mdl, neuronTypes, crossbarDat, coreDat)


"""Per Neuron CSV for NeMo.
 CSV format is:
 type, coreID, localID, connectivityGrid, synapseTypes
 sigma, S, b,
 epsilon, sigma_l, lambda, c, alpha, beta, TN, VR, sigmaVR, resetVoltage,encodedResetVotage,
 resteMode, kappa, signaldelay, destCore, destLocal
 isOutputNeuron, isSelfFiring, isActive
"""


def createTNNeMoCSV(filename):
	model, neuronTypes, crossbars, cores = readTN(filename)
	crossbarSize = model['crossbarSize']

	neuronTemplates = getNeuronModels(neuronTypes)

	neurons = []  # create neurons and store them in this array for the def file

	# given the crossbar def
	# each crossbar contains neuron connection info and names.


	return "d"


if __name__ == '__main__':
	mdl = Model()

	nt = NeuronType(model=mdl, name="Tester", sigmas=[1, 2, 3, 4], s=[1, 2, 3, 4], b=[False, False, False, False],
					sigma_lambda=-1,
					lambd=0, c_lambda=0, epsilon=False, alpha=10, beta=0, TM=18, gamma=2, kappa=False, sigmaVR=1, VR=4,
					V=0, cls="TestNeuron")

	gen = readTN('sobel.json')

	print('-' * 50)
	print(gen[3])
	print('-' * 100)

	ns = createTNNeMoCSV('bistable_spike.json')
	print(ns)

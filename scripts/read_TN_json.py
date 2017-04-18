
import copy
import typing
from typing import List
import re
import jsoncomment.package.comments as comments
from collections import OrderedDict
import numpy as np
import json
from numba import jit
import itertools
import progressbar
from jsoncomment import JsonComment
try:
	import  api_def
	from api_def import TN,ConfigFile, Spike
except:
	import scripts.api_def
	from scripts.api_def import TN,ConfigFile, Spike

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


def getNeuronModels(neuronTypes, nsynapses=256, nweights=4):
	neuronTemplates = {}
	for name, vals in zip(neuronTypes.keys(), neuronTypes.values()):
		name = name.replace("N","")
		neuronTemplates[name] = createNeuronTemplateFromEntry(vals, nsynapses, nweights)
		neuronTemplates[name].nnid = int(name)
	return neuronTemplates



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

	# tnData = json.loads(data)
	#tnData = json.load(open(filename, 'r'), object_pairs_hook=tnJSONHandler)
	tnData = json.loads(data, 	object_pairs_hook=parse_object_pairs)



	mdl = tnData['model']
	crossbarDat = tnData["crossbarTypes"]
	coreDat = []
	for k in tnData.keys():
		if "core" in k:
			coreDat.append(tnData[k])

	cbid = 0
	#go through cores and find any non-prototye crossbars and put them in the
	for core in coreDat:
		if 'name' not in core['crossbar'].keys():
			v = {}
			core['crossbar']['name'] = f"cb{cbid}"

			crossbarDat.append({'crossbar':core['crossbar'], 'name':f"cb{cbid}"})
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

	neuronTypesTrim = {}

	for nk in neuronTypes.keys():
		n_num = nk.replace('N','')
		neuronTypes[nk]['nid'] = int(n_num)

	crossbarDat = parseCrossbar(crossbarDat, mdl['crossbarSize'])
	return (mdl, neuronTypes, crossbarDat, coreDat)


def getRange(jItem,start=0):
	return getRangeAndValue(jItem, start)[0]

def getRangeAndValue(jItem, start=0):
	stm = start


	inc = 1
	if jItem.count(':') == 1:
		start = int( jItem.split(":")[0])
		end = int(jItem.split(":")[1])
		value = 0
		stm += 1
		return [np.arange(start,end), value]
	elif jItem.count(":") == 2:
		sp = jItem.split(":")
		value = int(sp[0])
		end = int(sp[2])
		inc = int(sp[1])
	elif jItem.count('x') == 1:
		sp = jItem.split("x")
		value = int(sp[0])
		end = int(sp[1])
	else:
		print("jItem")
		end = int(jItem)
		value = -1

	end = end + stm

	return [np.arange(start,end,inc),value]

def getNeuronTypeList(typeList):
	types = [-1] * 256 #create NULL neuron list (-1 is null)
	pos = 0
	for typeDef in typeList:
		if "x" in typeDef:
			tp = int(typeDef.split('x')[0])
			for i in getRange(typeDef,pos):
				types[i] = tp
				pos += 1
		elif ":" in typeDef:
			pass

	return types

def getNeuronDendrites(dendList):
	dendrites = [-1] * 256
	pos = 0
	for denditm in dendList:
		rng,value = getRangeAndValue(denditm, pos)
		for i in rng:
			dendrites[pos] = value
			pos += 1
	return dendrites


def getNeuronDestCores(destList):
	return getNeuronDendrites(destList)
	destCores = [-1] * 256
	pos = 0
	for dCore in destList:
		rng, value = getRangeAndValue(dCore, pos)
		for _ in rng:
			destCores[pos] = value
			pos += 1

	return destCores

def getNeuronDestAxons(ll):
	destAxons = [0] * 256
	axonloc  = 0
	for axon in ll:
		rng, value = getRangeAndValue(axon, 0)
		for v in rng:
			destAxons[axonloc] = v
			axonloc += 1
	return destAxons

def getNeuronDelays(ll):
	return getNeuronTypeList(ll)

def getNeuronFromID(id, neuronList):

	for k,v in zip(neuronList.keys(), neuronList.values()):
		pass



"""Per Neuron CSV for NeMo.
 CSV format is:
 type, coreID, localID, connectivityGrid, synapseTypes
 sigma, S, b,
 epsilon, sigma_l, lambda, c, alpha, beta, TN, VR, sigmaVR, resetVoltage,encodedResetVotage,
 resteMode, kappa, signaldelay, destCore, destLocal
 isOutputNeuron, isSelfFiring, isActive
"""
@jit
def genCore(core):
	pass

import multiprocessing as mp
import concurrent.futures



def chunks(l, n):
	"""Yield successive n-sized chunks from l."""
	for i in range(0, len(l), n):
		yield l[i:i + n]

def split_seq(iterable, size):
	it = iter(iterable)
	item = list(itertools.islice(it, size))
	while item:
		yield item
		item = list(itertools.islice(it, size))
def getCC(cores, cpu_count):
	if(cores.__len__() > 64):
		coreCH = split_seq(cores, cpu_count)
	else:
		coreCH = split_seq(cores,2)

	cc = 0
	for i in coreCH:
		cc += 1
	return cc

def createTNNeMoConfig(filename):

	model, neuronTypes, crossbars, cores = readTN(filename)
	crossbarSize = model['crossbarSize']

	neuronTemplates = getNeuronModels(neuronTypes)

	nullNeuron = TN(256,4)
	nullNeuron.S =[0,0,0,0]

	neurons = []
	#set up neurons for cores:
	ntmp = {}
	for nt in neuronTemplates.values():
		ntmp[nt.nnid] = nt

	neuronTemplates = ntmp
	nc = 0
	cfgFile = ConfigFile()
	#mp.set_start_method('spawn')
	q = mp.Queue()
	count = int (cores.__len__() / mp.cpu_count())
	cc = getCC(cores,mp.cpu_count())

	if(cores.__len__() > 64):
		coreCH = split_seq(cores, count)

	else:
		coreCH = split_seq(cores, 2)

	data = ""


	print("Importing JSON file and generating csv...")
	bar = progressbar.ProgressBar(max_value=cc+1)
	bar.update(1)
	pid = 0
	result = ""
	# temp = []
	# for ch in coreCH:
	# 	temp.append(neuronCSVFut(ch,crossbars,nc,neuronTemplates))
	# for i in temp:
	# 	data = data + i

	with concurrent.futures.ProcessPoolExecutor(max_workers=mp.cpu_count()) as e:
		f = []
		for ch in coreCH:
			f.append( e.submit(neuronCSVFut,ch,crossbars,nc,neuronTemplates))
		done = False
		while(not done):
			done = True
			for proc in f:
				if proc.done():
					bar.update(1)
				else:
					done = False

		for i in f:
			data = data + i.result()


	# for p in procs:
	# 	p.join()
	# 	bar.update(pid)
	# 	pid += 1;
	#
	print("Combining CSV text...")
	cfgFile.neuron_text = cfgFile.neuron_text + data
	# while not q.empty():
	#
	# 	cfgFile.neuron_text = cfgFile.neuron_text + q.get()


	#neuronCSVGen(cores, crossbars, nc, neuronTemplates, q)
	return cfgFile

@jit
def neuronCSVGen(cores, crossbars, nc, neuronTemplates, q):
	for core in cores:
		crossbar = crossbars[core['crossbar']['name']]
		coreNeuronTypes = getNeuronTypeList(core['neurons']['types'])
		dendriteCons = getNeuronDendrites(core['neurons']['dendrites'])
		destCores = getNeuronDestCores(core['neurons']['destCores'])
		destAxons = getNeuronDestAxons(core['neurons']['destAxons'])
		destDelays = getNeuronDelays(core['neurons']['destDelays'])

		coreID = core['id']

		# synapse types:
		tl = crossbar[0]
		nc += 1

		# core data configured, create neurons for this core:
		# genNeuronBlock(coreID, coreNeuronTypes, crossbar, destAxons, destCores, destDelays, neuronTemplates, neurons, tl)
		for i in range(0, 256):
			# get synaptic connectivity col for this neuron:
			connectivity = np.array(crossbar[1])[:, i]
			if coreNeuronTypes[i] in neuronTemplates.keys():
				neuron = createNeuronfromNeuronTemplate(neuronTemplates[coreNeuronTypes[i]], coreID, i, connectivity,
														tl)
			else:
				neuron = TN(256, 4)
			neuron.destCore = destCores[i]
			neuron.destLocal = destAxons[i]
			neuron.signalDelay = destDelays[i]
			# cfgFile.add_neuron(neuron)
			q.put(neuron.to_csv())


@jit
def neuronCSVFut(cores, crossbars, nc, neuronTemplates):
	d = ""
	for core in cores:
		crossbar = crossbars[core['crossbar']['name']]
		coreNeuronTypes = getNeuronTypeList(core['neurons']['types'])
		dendriteCons = getNeuronDendrites(core['neurons']['dendrites'])
		destCores = getNeuronDestCores(core['neurons']['destCores'])
		destAxons = getNeuronDestAxons(core['neurons']['destAxons'])
		destDelays = getNeuronDelays(core['neurons']['destDelays'])

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
				neuron = createNeuronfromNeuronTemplate(neuronTemplates[coreNeuronTypes[i]], coreID, i, connectivity,
														tl)
			else:
				neuron = TN(256, 4)
			neuron.destCore = destCores[i]
			neuron.destLocal = destAxons[i]
			neuron.signalDelay = destDelays[i]
			#d = d + neuron.to_csv()
			nrs.append(neuron)
		for i in nrs:
			d = d + i.to_csv()
			# cfgFile.add_neuron(neuron)
	return d


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
	if type=='json':
		return readSpikeJSON(filename)

def readAndSaveSpikeFile(filename, type,saveFile="spikes.csv"):
	spikes = readSpikeFile(filename, type)

	out = [s.toCSV().replace('\n','') for s in spikes]
	with open(saveFile, 'w') as f:
		f.writelines(out)




if __name__ == '__main__':
	mdl = Model()

	nt = NeuronType(model=mdl, name="Tester", sigmas=[1, 2, 3, 4], s=[1, 2, 3, 4], b=[False, False, False, False],
					sigma_lambda=-1,
					lambd=0, c_lambda=0, epsilon=False, alpha=10, beta=0, TM=18, gamma=2, kappa=False, sigmaVR=1, VR=4,
					V=0, cls="TestNeuron")

	ns = createTNNeMoConfig('./test/patternMatch.json')
	print("PatternMatch Created")
	ns = createTNNeMoConfig('./sobel/sobelTiles.json')
	print("sobel created")

	spks = readSpikeFile('./sobel/sobelTiles_inputSpikes.sfti')
	print("sobel spikes created")

	ns.save_csv('test.csv')

import io

from tqdm import tqdm

from tn_api.genRange import populateArray

from tn_api.tn_nemo_api import TN,ConfigFile,Spike,vnames,TNFunct

import json
from collections import OrderedDict
import copy

import concurrent.futures
import multiprocessing as mp
from concurrent.futures import as_completed
import itertools
import numpy as np

import jsoncomment.package.comments as comments

try:

	from numba import jit
	print("NB")
except:

	print('no numba;')


	def jit(func):
		def _decorator(*args):
			return func(*args)

		return _decorator


@jit
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



def readTN(filename):
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
		neuronTypes[nk]['nid'] = int(n_num)

	crossbarDat = parseCrossbar(crossbarDat, mdl['crossbarSize'])
	return (mdl, neuronTypes, crossbarDat, coreDat)

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

CNTS = []
def createNeMoCFGFromJson(filename):
	model, neuronTypes, crossbars, cores = readTN(filename)

	neuronTemplates = getNeuronModels(neuronTypes)
	nullNeuron = TN(256,4)
	nullNeuron.S = [0,0,0,0]
	ntmp = {}
	for nt in neuronTemplates.values():
		ntmp[nt.nnid] = nt

	neuronTemplates = ntmp
	nc = 0
	cfgFile = ConfigFile()

	count = int(cores.__len__() / mp.cpu_count())

	print("Generating CSV...")
	data = ""
	# for ch in cores:
	# 	ch = [ch]
	# 	data = data + neuronCSVFut(ch,crossbars,nc,neuronTemplates)

	with concurrent.futures.ProcessPoolExecutor(max_workers=mp.cpu_count()) as e:
		f = []
		for ch in cores:
			ch = [ch]
			f.append( e.submit(neuronCSVFut,ch,crossbars,nc,neuronTemplates))

		kwargs = {
			'total': len(f),
			'unit' : 'nap',
			'unit_scale' : True,
			'leave': True
		}
		print("Running JSON neuron conversion...")

		for dti in tqdm(as_completed(f), **kwargs):
			data = data + dti.result()


	cfgFile.neuron_text = cfgFile.neuron_text + data
	return cfgFile




def neuronTemplatesToArr(neuronTemplates):
	ks = []
	vs = []
	for key,value in zip(neuronTemplates.keys(), neuronTemplates.values()):
		ks.append(key)
		vs.append(value)

@jit
def neuronCSVFut(cores, crossbars, nc, neuronTemplates):

		d = ""
		coreNeuronTypes = np.zeros(256, dtype=np.int32)
		destCores = np.zeros(256, dtype=np.int32)
		destAxons = np.zeros(256, dtype=np.int32)
		destDelays = np.zeros(256, dtype=np.int32)


		for core in cores:
			crossbar = crossbars[core['crossbar']['name']]



			tp = ""


			populateArray(coreNeuronTypes,	  (core['neurons']['types'])	)

			populateArray(destCores,		  (core['neurons']['destCores'])	)
			populateArray(destAxons,		  (core['neurons']['destAxons'])	)
			populateArray(destDelays, 		  (core['neurons']['destDelays'])	)

			# coreNeuronTypes = getNeuronTypeList (core['neurons']['types'])
			# dendriteCons = getNeuronDendrites	(core['neurons']['dendrites'])
			# destCores = getNeuronDestCores		(core['neurons']['destCores'])
			# destAxons = getNeuronDestAxons		(core['neurons']['destAxons'])
			# destDelays = getNeuronDelays		(core['neurons']['destDelays'])

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
				else:
					neuron = TN(256, 4)
				neuron.destCore = destCores[i]
				neuron.destLocal = destAxons[i]
				neuron.signalDelay = destDelays[i]
				# d = d + neuron.to_csv()
				if neuron.destCore < 0 or neuron.destLocal < 0:
					neuron.selfFiring=1
				nrs.append(neuron)

			for i in nrs:
				ic = i.to_csv()
				d = "{}{}".format(d,ic) #d + i.to_csv()
			#cfgFile.add_neuron(neuron)
			coreNeuronTypes.fill(0)
			destCores.fill(0)
			destAxons.fill(0)
			destDelays.fill(0)
		return d

# @jit
# def neuronCSVGen(coreID,coreConnectivity,coreSyanpseTypeList, coreNTs, destCores,destAxons,destDelays,neuronTemplates):
#
# 	neurons = np.array([""] * 256)
#
# 	for i in range(0, 256):
# 		connectivity = np.array(crossbar[1])[:, i]
# 		if coreNTs[i] in neuronTemplates.keys():
# 			neuron = createNeuronfromNeuronTemplate(neuronTemplates[coreNTs[i]], coreID, i, connectivity,
# 													tl)
# 		else:
#


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

	ns = createNeMoCFGFromJson('../test/patternMatch.json')
	print("PatternMatch Created")
	ns = createNeMoCFGFromJson('../sobel/sobelTiles.json')
	print("sobel created")

	spks = readSpikeFile('./sobel/sobelTiles_inputSpikes.sfti')
	print("sobel spikes created")

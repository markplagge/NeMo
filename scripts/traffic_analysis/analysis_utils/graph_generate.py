import networkx as nx
import click
import matplotlib.pyplot as plt
import tqdm
try:
    from numba import jit
except:
    click.secho('No NUMBA installed. Disabling JIT features',fg='red',underline=True)
    def jit(the_function):
        the_function()


import multiprocessing as mp
import threading as tp
"""
Generates networkx graphs from inputs 
Needs to generate the following graphs:

nemo neuron -> neuron connection graph (digraph - weights are number of spikes)
                ndes are neurons, with core and chip attributes

nemo core -> core connection graph (digarph - weights on edges are counts of spike activity)
            nodes are cores. chip # and layer ID are attributes

nemo chip -> chip connection graph (digraph - weights on edges are counts.
            nodes represent chips. layer ID is the node attribute.

"""


def gatherLayerInfo(layer_info_file):
    pass


def createCoreGraph(connection_dict, count_dict, layer_mapping_dict, num_cores_per_chip=1024):
    pass


class NeMoGraph:

    def __init__(self, counts, connections, type='directed', fbase='nemo_graph', num_chips=3, cores_per_chip=1024,
                 doGen=True):
        self.num_chips = num_chips
        self.cores_per_chip = cores_per_chip
        self.fbase = fbase

        if type == 'directed':
            G = nx.OrderedMultiDiGraph()
        else:
            G = nx.OrderedMultiGraph()
        self.G = G
        if doGen:
            self.genGraph(G, counts, connections)

    def get_chip_num(self, src):
        return src // self.cores_per_chip

    def __create_weight_edges(self, con_ct, con):

        for con_k in con_ct.keys():
            v1 = 'ore'
            bad = v1 in con_k
            bad = bad or v1 in con[con_k]
            if not bad:
                weightmt = (con_k, con[con_k], con_ct[con_k])
                yield weightmt

    def genGraph(self, G, counts, connections):
        for edge in self.__create_weight_edges(counts, connections):
            src = edge[0]
            dt = edge[1]
            wt = edge[2]
            G.add_node(src, chip=self.get_chip_num(src))
            G.add_node(dt, chip=self.get_chip_num(src))
            G.add_edge(src, dt, weight=wt)
        self.G = G

    def saveGraph(self, doGML=True, doGEX=True, doPickle=True):
        if doGEX:
            nx.write_gexf(self.G, self.fbase + ".gexf")
        if doGML:
            nx.write_graphml(self.G, self.fbase + ".graphml")
        if doPickle:
            nx.write_gpickle(self.G, self.fbase + ".gpick")


class NeMoGraphFromFiles(NeMoGraph):
    def __init__(self, file_list, type='directed', fbase='nemo_graph', num_chips=3, cores_per_chip=1024, test=False):
        self.file_list = file_list
        super().__init__(0, 0, type, fbase, num_chips=cores_per_chip, cores_per_chip=cores_per_chip, doGen=False)
        self.test = test
        self.pos = 2
        # self.G = super().G

    @jit
    def adn(self, src_id, dest_id, src_chip, dest_chip, src_core, dest_core, src_neuron, dest_neuron):
        self.G.add_node(src_id, chip=src_chip, neuron=src_neuron, core=src_core)
        self.G.add_node(dest_id, chip=dest_chip, neuron=dest_neuron, core=dest_core)
        self.G.add_edge(src_id, dest_id, weight=1)
        # if (self.G.has_edge(src_id, dest_id)):
        #     # edg = self.G.edges[src_core,dest_core]['weight']
        #     # edg += 1
        #     self.G.edges([src_id, dest_id])['weight'] += 1
        # else:
        #     self.G.add_edge(src_id, dest_id, weight=1)


    def gronk_file(self,l):
        sr = l.split(',')
        src = f"{sr[1]},{sr[2]}"
        dst = f"{sr[4]},{sr[5]}"
        src_core = int(sr[1])
        dst_core = int(sr[4])
        src_neuron = int(sr[2])
        dst_neuron = int(sr[5])
        sc = super().get_chip_num(src_core)
        dc = super().get_chip_num(dst_core)

        self.adn(src, dst, sc, dc, src_core, dst_core, src_neuron, dst_neuron)

    def populateGraphFromFile(self, fileName):
        nl = 0
        tl = 1024
        with open(fileName, 'r') as f:
            # with click.progressbar(f,label='file parsing...') as pbar:
            # for l in click.progressbar(f,label='File parsing...'):
            for l in tqdm.tqdm(f, desc='loading file...',position=self.pos):
                if not 'timestamp' in l:
                    self.gronk_file(l)
                    nl += 1
                    if self.test and nl > tl:
                        click.echo("Testing mode complete")
                        break
    @jit
    def genGraphFromFiles(self):
        thds = []
        for file in self.file_list:
            x = tp.Thread(target=self.populateGraphFromFile,args=(file,))
            #x = mp.Process(target=self.populateGraphFromFile,args=(file,))
            x.start()
            thds.append(x)

            #self.populateGraphFromFile(file)
        return thds


    def genGraphFromMemory(self, fileObject):
        nl = 0
        tl = 1024
        for l in tqdm.tqdm(fileObject,desc='parsing...', position=self.pos):
            if not 'timestamp' in l:
                nl += 1
                if self.test and nl > tl:
                    click.echo("testing mode complete")
                self.gronk_file(l)
        return self.G


class NeMoGraphFromFilesCore(NeMoGraphFromFiles):

    def __init__(self,*args,**kwargs):

        super().__init__(*args,**kwargs)
        self.pos = 3

    def adn(self, src_id, dest_id, src_chip, dest_chip, src_core, dest_core, src_neuron, dest_neuron):
        src_id = src_core
        dest_id = dest_core
        # print("Core network generation")
        super().adn(src_core, dest_core, src_chip, dest_chip, src_core, dest_core, src_neuron, dest_neuron)

        # self.G.add_node(src_core,chip=src_chip)
        # self.G.add_node(dest_core,chip=dest_chip)



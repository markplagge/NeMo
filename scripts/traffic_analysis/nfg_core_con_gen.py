import collections
import multiprocessing as mp
from collections import OrderedDict

import click
import networkx as nx
import tqdm
# from scripts.traffic_analysis.analysis_utils.mp_file_parse import MultiFileWorker
from analysis_utils import MultiFileWorker
from analysis_utils.graph_generate import NeMoGraphFromFilesCore

try:
    from numba import jit
except:
    click.secho('No NUMBA installed. Disabling JIT features', fg='red', underline=True)


    def jit(the_function):
        the_function()


class MultiFileNFG(MultiFileWorker):

    def __init__(self, file_list, split_size, nworkers):
        src_core_re = r"(coreID\s+=\s+)(.\d*)"
        src_core_full = r"(?<=coreID\s=\s)\s*.\d*"
        dst_core_re = r"(destCore\s+=\s+)(.\d*)"
        dst_core_full = r"(?<=destCore\s=\s)\s*.\d*"

        self.src_core_reg = re.compile(src_core_re, re.MULTILINE)
        self.dest_core_reg = re.compile(dst_core_re, re.MULTILINE)
        self.src_core_re_f = re.compile(src_core_full, re.MULTILINE)
        self.dst_core_re_f = re.compile(dst_core_full, re.MULTILINE)
        super().__init__(file_list, split_size, nworkers)

    def line_parser(self, lines):
        connections = collections.defaultdict(list)
        for l in lines:
            if 'coreID' in l:
                s_core, d_core = self.extract_core_data(l)
                connections[s_core].append(d_core)
        return connections

    def result_aggregate(self, worker_results):
        connections = collections.defaultdict(list)
        for proc in tqdm.tqdm(worker_results, desc='connection aggregation...'):
            presult = proc.get()
            connections.update(presult)

        self.result = connections
        return connections

    def extract_core_data(self, line):
        # Line wise switching to regex:

        sc_matches = self.src_core_reg.findall(line)
        dc_matches = self.dest_core_reg.findall(line)
        if not sc_matches:
            click.secho("err on sc", fg="red", bold=True)
        elif not dc_matches:
            click.secho("err on dc", bold=True)
        else:
            s_core = int(sc_matches[0][1])
            d_core = int(dc_matches[0][1])
            return s_core, d_core
        click.secho("attempting non re method", fg="red")

        s_core_start = line.find('coreID')
        s_core_end = line.find(',localID')
        s_core_txt = line[s_core_start:s_core_end]
        s_core = s_core_txt.split('=')[1]

        d_core_start = line.find('destCore')
        d_core_end = line.find(',destLocal')
        d_core_txt = line[d_core_start:d_core_end]
        d_core = d_core_txt.split('=')[1]

        return s_core, d_core  ## STI2LL STRINGS


class NFGParser(NeMoGraphFromFilesCore):

    def __init__(self, input_file, out_grph='nfg_cores', split_size=50 * 1024 * 1024, nworkers=mp.cpu_count() // 2,
                 doFullReg=False, debug_num_cores=True):
        super().__init__(0, type='directed', fbase=out_grph, num_chips=1, cores_per_chip=4096, test=False)
        self.doFReg = doFullReg
        self.pos = 3
        self.input_file = input_file
        self.split_size = split_size
        self.nworkers = nworkers
        self.file_results = None
        self.mworker = None
        self.debug_num_cores = debug_num_cores
        self.new_worker()
        self.out_grph = out_grph

    def load_files(self):

        if self.doFReg:
            click.secho("Using full regex match for parsing! ", fg="yellow", underline=True)
            # regex = r"((?<=coreID\s=\s)\s*\d).+?((?<=destCore\s=\s)\s*.\d*)"
            regex = r"((?<=coreID\s=\s)\s*\d*).+?((?<=destCore\s=\s)\s*.\d*)"
            self.G = nx.MultiGraph()
            with open(self.input_file, 'r') as f:
                with open(self.out_grph + "d.csv", 'w') as out_csv:
                    out_csv.write("Source,Target\n")
                    full_data = f.read()
                    matches = re.finditer(regex, full_data)
                    with tqdm.tqdm(desc="regex search", total=4096 * 256) as pbar:
                        for m_obj in matches:
                            src = m_obj.group(1)
                            dst = m_obj.group(2)
                            out_csv.write(src)
                            out_csv.write(',')
                            out_csv.write(dst)
                            out_csv.write('\n')
                            self.G.add_edge(src, dst, weight=1)
                            pbar.update(1)

            return self.G
        else:
            load_results = self.mworker.parse_files()
            self.file_results = load_results
            return load_results

    def new_worker(self):
        self.mworker = MultiFileNFG(self.input_file, self.split_size, self.nworkers)

    def clean_worker(self):
        v = self.mworker
        self.mworker = None
        del v

    @jit
    def add_core(self, src_core, dest_core):
        self.G.add_node(src_core)
        self.G.add_node(dest_core)
        self.G.add_edge(src_core, dest_core)

    def saveGraph(self, **kwargs):
        nx.write_graphml(self.G, self.fbase + ".graphml")
        nx.write_gexf(self.G, self.fbase + ".gexf")
        if not self.doFReg:
            nx.write_adjlist(self.G, self.fbase + ".csv", delimiter=',')

    def generate_graph(self):
        if self.doFReg:
            click.secho("Full Regex mode", color='blue')
        else:
            for s_core in tqdm.tqdm(self.file_results.keys()):
                [self.add_core(int(s_core), int(x)) for x in self.file_results[s_core]]
                # self.add_core(int(s_core),int(self.file_results[s_core]))


def make_unique(key, dct):
    """
	Makes each element of a JSON file unique - for parsing the TN JSON files since they have multiple identical keys/
	:param key:
	:param dct:
	:return:
	"""
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


import re


def parse_dest_cores(dest_core_split):
    ## Cases for dest cores are:
    # 1. int x int == core number x n neurons connected to that core.
    # 2. int == axon is connected to that core
    # 3. int:int Cores starting at first int, ending at second int, are conencted to n neurons
    count = 0  # sanity checker
    dest_core_list = []
    for dest_core in dest_core_split:
        # dest_core = dest_core.replace('"','') #remove quotes
        if 'x' in dest_core:
            ## case 1
            dest_core_id, number = dest_core.split('x')
            dest_core_list = dest_core_list + [int(dest_core_id)] * int(number)
            # dest_core_list = dest_core_list + dest_cores

        elif dest_core.count(':') >= 1:
            vals = dest_core.split(":")
            if dest_core.count(':') == 1:
                start = int(vals[0])
                end = int(vals[1]) + 1
                for i in range(int(start), int(end)):
                    dest_core_list.append(i)
            else:
                start = int(vals[0])
                end = int(vals[2]) + 1
                increment = int(vals[1])
                for i in range(int(start), int(end), int(increment)):
                    dest_core_list.append(i)

        else:
            dest_core_list.append(int(dest_core))
        count += 1
    return dest_core_list


def parse_tn_core(core_str, core_id):
    ##got a CORE PARAMETERS thing.
    regex = r'(?<="destCores":\S)(.+?)(])'
    matches = re.search(regex, core_str)
    core_str = matches.group(1)

    core_str = core_str.replace('"', '').split(',')
    dcl = parse_dest_cores(core_str)
    return core_id, dcl


def parse_tn_core_list(core_id_list):
    res = []
    for core_id_tuple in core_id_list:
        core_id, dcl = parse_tn_core_tuple(core_id_tuple)
        res.append((core_id, dcl))
    return res


def parse_tn_core_tuple(core_id_tuple):
    return parse_tn_core(core_id_tuple[0], core_id_tuple[1])


class tnJSONCoreParser():
    def __init__(self, tn_file_path):
        self.tn_file = open(tn_file_path, 'r')
        self.tn_data = ''
        self.graph = nx.MultiDiGraph()

    def find_core_pos(self):
        cline = self.tn_file.readline()
        while '/* Core parameters' not in cline:
            cline = self.tn_file.readline()
        self.tn_data = cline + self.tn_file.read()
        self.tn_file.close()
        self.tn_data = self.tn_data + "|||"

    def parse_tn_json(self):

        # group_match = re.compile('"core":.+(Core\sparameters)',re.DOTALL|re.MULTILINE)
        # gm2 = re.compile('(?=Core\sparameters \*\/\n).+(\/\*\sCore\sparameters \*\/)')
        # gm3 = re.compile('((?=Core\sparameters \*\/\n).+(\/\*\sCore\sparameters \*\/))')
        # gm4 = re.compile('(?<=\/\*\sCore\sparameters\s\*\/)[\s\S\w\W\v\n]+(?=,\n.+\/\*\sCore\sparameters\s\*\/)')
        # matches = gm4.findall(self.tn_data)
        # regex = r"(/\*\sCore\sparameters\s\*/)(.*?)/\*\sCore\sparameters\s\*/"
        regex = r'\"core\":{(.*?)(/\*\sCore\sparameters\s\*/|\|)'

        my_core_regex = r'(?<="id":)\d+'

        # matches = re.search(regex, self.tn_data, re.DOTALL)
        # matches = re.findall(regex,self.tn_data,re.DOTALL)
        matches = re.finditer(regex, self.tn_data, re.DOTALL | re.MULTILINE)
        ## each match is a core data chunk.
        core_conn_strs = []

        with mp.Pool(processes=mp.cpu_count() / 2) as pool:
            num_cores = 0
            for matchNum, match in enumerate(matches):
                matchNum = matchNum + 1
                start = match.start()
                end = match.end()
                # core_str = self.tn_data[start:end]
                core_str = '"core":{ ' + match.group(1)
                core_str = core_str.rstrip().rstrip(',')
                my_core = re.search(my_core_regex, core_str).group(0)
                core_conn_strs.append((str(core_str), str(my_core)))
                num_cores += 1
                #
                # core_id,dcl = self.parse_tn_core(core_str)
            # now have a list of tuples - core_string, source core id.
            # run them through
            # connections = Parallel(n_jobs=2)(delayed(self.parse_tn_core_tuple)(x) for x in core_conn_strs)
            # con_worker = pool.map_(parse_tn_core_list,core_conn_strs)
            # core_conn_strs = [core_conn_strs[1] , core_conn_strs[2], core_conn_strs[3]]
            click.secho("Found " + str(num_cores) + " cores. ", fg="blue")
            con_worker = pool.imap_unordered(parse_tn_core_tuple, core_conn_strs, 128)
            # with click.progressbar(con_worker,length=num_cores,label='loading cores...',show_percent=True,color='green') as bar:
            with open('tn_con_d.csv', 'w') as out_csv:
                out_csv.write("Source,Target\n")
                with tqdm.tqdm(con_worker, desc='Cores...', total=int(num_cores), leave=True, position=0) as bar:
                    for i in bar:
                        # for i in tqdm.tqdm(con_worker,desc="Loading cores...", total=num_cores):
                        source_core = int(i[0])
                        dest_cores = i[1]
                        # self.graph.add_node(source_core)
                        for dc in tqdm.tqdm(dest_cores, desc='sub cores...', position=1, leave=False):
                            self.graph.add_edge(source_core, dc)
                            out_csv.write(str(source_core) + "," + str(dc) + '\n')

                            # self.graph.add_node(dc)
                            # self.graph.add_edge(source_core,dc)
                    # print(len(connections)
            click.secho("Saving graph...", fg='blue')
            nx.write_graphml(self.graph, 'tn_con.graphml')
            nx.write_adjlist(self.graph, "tn_con.csv", delimiter=',')
            nx.write_gexf(self.graph, 'tn_con.gexf')


def testimap(core_conn_strs):
    print(core_conn_strs)

    # Rather than use the make_unique system, we will extract core data.

    #
    #
    #
    # def line_parser(self, lines):
    #     connections = collections.defaultdict(list)
    #     for l in lines:
    #
    #         if 'coreID' in l:
    #             s_core, d_core = self.extract_core_data(l)
    #             connections[s_core].append(d_core)
    #     return connections
    #
    # def result_aggregate(self, worker_results):
    #     connections = collections.defaultdict(list)
    #     for proc in tqdm.tqdm(worker_results, desc='connection aggregation...'):
    #         presult = proc.get()
    #         connections.update(presult)
    #
    #     self.result = connections
    #     return connections
    #
    # def extract_core_data(self, line):
    #     s_core_start = line.find('coreID')
    #     s_core_end = line.find(',localID')
    #     s_core_txt = line[s_core_start:s_core_end]
    #     s_core = s_core_txt.split('=')[1]
    #
    #     d_core_start = line.find('destCore')
    #     d_core_end = line.find(',destLocal')
    #     d_core_txt = line[d_core_start:d_core_end]
    #     d_core = d_core_txt.split('=')[1]
    #
    #     return s_core, d_core  ## STILL STRINGS


@click.group()
def cli():
    click.secho("generate core connectivity debug info.")


@cli.command()
@click.option('-d', is_flag=True, default=False, help="use full regex pattern matching to generate.")
@click.argument("nfg_file", type=click.Path(exists=True))
def loadNFG(d, nfg_file):
    nf = nfg_file
    click.secho("Starting core connectivity data gen from NFG file...", fg="green")
    # if d:
    #     parser = NFGParser(nfg_file,doFullReg=True)
    #
    #
    # else:
    parser = NFGParser(nfg_file, doFullReg=d)
    click.secho("Loading core con. from file", fg="blue")
    load_results = parser.load_files()
    click.secho("Cleaning up...")
    parser.clean_worker()
    click.secho("Generating core con. graph")
    parser.generate_graph()
    click.secho("Saving graph", fg="green")
    parser.saveGraph()
    click.secho("Cleaning up nfg", fg="green")
    del (parser)


@cli.command()
@click.argument("tn_json", type=click.Path(exists=True))
def loadTNJ(tn_json):
    click.secho("Now generating TN core connectivity graph.")
    tnp = tnJSONCoreParser(tn_json)
    tnp.find_core_pos()
    with open('/shared/share/superneuro/NeMoRem/scripts/model/out_data.json', 'w') as f:
        f.write(tnp.tn_data)
    tnp.parse_tn_json()



if __name__ == "__main__":
    print("Starting data gen.")
    cli()

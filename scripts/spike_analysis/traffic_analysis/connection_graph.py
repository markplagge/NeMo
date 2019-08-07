import glob
import json
from collections import defaultdict

import click
import matplotlib.pyplot as plt
import networkx as nx
import numpy as np
import tqdm
from analysis_utils import MultiFileWorker
from analysis_utils import NeMoGraphLin, NeMoGraphLinCore
from numpy.core import double
from scipy.io import mmwrite

nemo_csv_format = ['timestamp','core','local','destGID','destCore','destNeuron','isOutput?']
nemo_csv_src_flds = [1,2]
nemo_csv_dest_flds = [4,5]

nemo_csv_used_cols = [0,1,2,4,5]
dt2 = [(f'f{x}',np.int) for x in range(1,len(nemo_csv_used_cols))]
#dt2 = ()
nemo_csv_used_dtypes = [('f0',double)] +  dt2 # * (len(nemo_csv_used_cols) -1)
#source/dests are in 'core-local' format. data is stored in a hashtable with soruce/dest as key, and value is count


import os
import multiprocessing as mp


#process file function
def processfilemulti(filename, start=0, stop=0):
    if start == 0 and stop == 0:
        with open(filename,'r') as f:
            counts,connections = readCSVChunk(f.readlines())
    else:
        with open(filename, 'r') as fh:
            fh.seek(start)
            lines = fh.readlines(stop - start)
            counts,connections = readCSVChunk(lines)

    results = (counts,connections)
    return results

def updateCounts(count_d, newcount_d):
    for key in newcount_d.keys():
        count_d[key] += newcount_d[key]
    return count_d

def readCSVChunk(lines,pre_list=False):
    counts = defaultdict(int)
    connections = {}#defaultdict(0)
    for l in lines:
        if pre_list:
            sr = l
        else:
            sr = l.split(',')
            src = f"{sr[1]},{sr[2]}"
            dst = f"{sr[4]},{sr[5]}"




        connections[src] = dst
        counts[src] +=1
    return (counts,connections)

    #rdr = csv.DictReader()

def parseFileNP(file_name):
    data = np.genfromtxt(file_name,delimiter=',',dtype=dt2,usecols=nemo_csv_used_cols,skip_header=1)
    return data

def lf_mp(file_list):
    ##MP PROCESS CSV FILE
    counts = defaultdict(int)
    connections = {}

    for filename in file_list:
        filesize = os.path.getsize(filename)
        split_size = 50*1024*1024

        # determine if it needs to be split
        if filesize > split_size:
            click.echo("Split - using MP")
            click.echo(f"File size is {filesize}, chunk size is {split_size}")
            # create pool, initialize chunk start location (cursor)
            pool = mp.Pool(mp.cpu_count() // 2)
            cursor = 0
            results = []
            with open(filename, 'r') as fh:

                # for every chunk in the file...
                for chunk in range(filesize // split_size):
                    # determine where the chunk ends, is it the last one?
                    if cursor + split_size > filesize:
                        end = filesize
                    else:
                        end = cursor + split_size
                    # seek to end of chunk and read next line to ensure you
                    # pass entire lines to the processfile function
                    fh.seek(end)
                    fh.readline()

                    # get current file location
                    end = fh.tell()

                    # add chunk to process pool, save reference to get results
                    proc = pool.apply_async(processfilemulti, args=[filename, cursor, end])
                    results.append(proc)
                    # setup next chunk
                    cursor = end
                    ## process anything that is ready
                    # for p in results:
                    #     if p.ready():
                    #         res = p.get()
                    #         #counts.update(res[0])
                    #         counts = updateCounts(counts,res[0])
                    #         connections.update(res[1])


            pool.close()
            pool.join()
            # iterate through results
            for proc in results:
                processfile_result = proc.get()
                #counts.update(processfile_result[0])
                counts = updateCounts(counts,processfile_result[0])
                connections.update(processfile_result[1])
            result = (counts,connections)
        else:
            ct,cc = processfilemulti(filename)
            counts.update(ct)
            connections.update(cc)
            result = (counts,connections)
    return result

def loadFiles(file_list,use_chunky=True,use_mp=True):
    dbl = 50
    v = []
    if use_chunky:
        if use_mp:
            click.secho('using chunky mp data loader', fg='green')
            mf = MultiFileWorker(file_list)
            result = mf.parse_files()

            #result = mf.result

        else:
            for file in file_list:
                result = processfilemulti(file)
    else:
        counts = defaultdict(int)
        connections = {}
        for file in file_list:
            with open(file,'r') as fh:
            #file_data = parseFileNP(file)
            #print("loaded data from CSV")
                for l in fh:


                    if 'timest' not in l:
                        ct,cn = readCSVChunk([l])
                        #counts.update(ct)
                        counts = updateCounts(counts,ct)
                        connections.update(cn)
                    dbl -= 1
                    if dbl <= 0:
                        r = (counts,connections)
                        return r

        result = (counts,connections)

    return result


def conn_parser(dct):
    #print(list(dct)[0:10])
    for v in dct.keys():
        #print(v)
        if not 'core' in v:
            #d = v.replace('|',',')
            #d[0] = int(d[0])
            #d[1] = int(d[1])
            #d = d + dct[v].split('-') # [int(x) for x in dct[v].split('-')]
            d = f"{v},{dct[v]}"
            #print(d)
            yield d
def count_parser(dct):
    for v in dct.keys():
        if not 'core' in v:
            #d = v.replace('|',',')
            d = f"{v},{dct[v]}"
            yield d

def create_core_counts(connections, counts,ncores=4096,neurons_per_core=256):
    #each chip has 4096 cores, each core has 256 neurons
    nneurons=neurons_per_core * ncores
    #0-255 <- core 0
    #BUT - we don't need this, just recreate the dictionary
    #Dictionary structure:
    #<core> -> <destcore> : count
    #so:
    for c_key in counts.keys():
        v1 = 'ore'
        bad = v1 in c_key
        bad = bad or v1 in connections[c_key]
        if not bad:
            core = int(c_key.split(',')[0])
            destcore = int(connections[c_key].split(',')[0])
            count = counts[c_key]
            yield (core,destcore,count)

def get_chip_num(src_core, cores_per_chip=1024):
    return src_core // cores_per_chip

def get_core_num(src_neuron, neurons_per_core=256):
    return src_neuron // neurons_per_core


def do_stats(Gs,gunp,mmname='mm_g'):
    click.secho('stats',fg='blue')
    click.secho('doing pagerank', fg='green')

    click.secho("Doing networkx based stats.", fg='green', underline=True)
    click.echo("starting pagerank.")
    pr_w = nx.pagerank_scipy(Gs)
    nx.set_node_attributes(Gs, pr_w, 'pagerank')
    #first, get a sparse matrix form of the graph:
    pr_path = gunp + 'pr'
    tempfn = gunp + mmname
    jsonfn = gunp + 'mm_g.json'
    #pr_cmd = pr_path + " market " + tempfn + ".mtx " + " --quick --max-iter=100 --iteration-num=1000 --json --jsonfile=" + jsonfn

    #mmwrite(tempfn,nx.to_scipy_sparse_matrix(Gs))

    #pr2 = pr_w

#    #bw = nx.betweenness_centrality(G)
#    #nx.set_node_attributes(Gs[0],bw1,'betweenness')



def do_graph_via_class(save_file_name,file_list,doGML=True,doGex=True,num_chips=3,do_eigen=True):
    click.echo(click.style("Not implemented yet.", fg='red'))
    pass

dairy_q = mp.Queue()
def generate_file_graph(loaded_data,doGML=True,doGEX=True,num_chips=3,do_eigen=True,type='core'):
    tmpfn = 'tmp_core_data'
    if type == 'core':
        nemo_g_obj = NeMoGraphLinCore([''],num_chips=num_chips,fbase='tmp'+ "_dir_core",
                                    cores_per_chip=num_chips,test=False)
        nemo_g_obj.pos = 3
    else:
        nemo_g_obj = NeMoGraphLin([''],num_chips=num_chips,fbase='tmp' + "_dir_core",
                                    cores_per_chip=num_chips,test=False)
        nemo_g_obj.pos = 4

        tmpfn = 'tmp_neuron_data'
    nemo_g_obj.genGraphFromMemory(loaded_data)
    G = nemo_g_obj.G
    ## Do stats if needed...

    nx.write_gpickle(G,tmpfn)
    dairy_q.put(tmpfn)


def do_graph_via_serial_class(save_file_name,file_list,doGML=True,doGEX=True,num_chips=3,
                              do_eigen=True,gun=''):

    par = True



    click.echo(click.style("Using new serial mode with networkX duplication management.",fg='green'))
    click.secho("files are: \n" + '\n'.join(file_list), fg='green')

    n_g_directed = NeMoGraphLin(file_list,num_chips=num_chips,fbase=save_file_name + "_directed",
                                cores_per_chip=num_chips,test=True)
    n_g_core = NeMoGraphLinCore(file_list,num_chips=num_chips,fbase=save_file_name + "_dir_core",
                                cores_per_chip=num_chips,test=False)


    if isinstance(file_list,list):
        fd = []
        for f in file_list:
            with open(f, 'r') as infile:
                fd = fd + infile.readlines()

        if par:
            worker1 = mp.Process(target=generate_file_graph,args=(fd,doGML,doGEX,num_chips,do_eigen,'core',))
            worker1.start()
            worker2 = mp.Process(target=generate_file_graph,args=(fd,doGML,doGEX,num_chips,do_eigen,'core',))
            worker2.start()

            with tqdm.tqdm(desc="Waiting on workers...",position=0,total=2) as pbar:
                working = True
                itms = 0
                while itms < 2:
                    if not dairy_q.empty():
                        temp_fn = dairy_q.get()
                        itms += 1
                        if 'core' in temp_fn:
                            click.secho('got core temp file.. loading')
                            coreQ = nx.read_gpickle(temp_fn)
                            n_g_core.G = coreQ
                            os.remove(temp_fn)
                        else:
                            coreN = nx.read_gpickle(temp_fn)
                            click.secho('got neuron temp file.. loading')
                            n_g_directed.G = nx.read_gpickle(coreN)
                            os.remove(coreN)
                            itms += 1

                        pbar.update(1)

        else:
            n_g_core.genGraphFromMemory(fd)
            n_g_directed.genGraphFromMemory(fd)
        del(fd)
        # with mp.Pool() as pool:
        #     g1=pool.apply_async(n_g_directed.genGraphFromFiles)
        #     g2=pool.apply_async(n_g_core.genGraphFromFiles)

        #t1 =  n_g_directed.genGraphFromFiles()
        #t2 = n_g_core.genGraphFromFiles()
        # t1 = n_g_directed.genGraphFromMemory(fd)
        # t2 = n_g_core.genGraphFromMemory(fd)
        # for t in t1:
        #     t.join()
        # for t in t2:
        #     t.join()



    # v1=g1.get()
    # v2=g2.get()
    click.echo("Generated digraph. ")
    # n_g_directed.fbase = 'nemo_neuron_digraph'
    # n_g_directed.saveGraph(doGML,doGEX)
    # n_g_directed.fbase=save_file_name
    # n_g_core.fbase = 'nemo_core_digraph'
    # n_g_core.saveGraph(doGML,doGEX)
    # n_g_core.fbase =save_file_name
    click.echo("saving mm version of graphs...")
    import threading

    wp1 = threading.Thread(target=mmwrite,args=('neuron.mm',nx.to_scipy_sparse_matrix(n_g_directed.G)))
    wp1.start()

    #mmwrite('neuron.mm', nx.to_scipy_sparse_matrix(n_g_directed.G))

    wp2 = threading.Thread(target=mmwrite,args=('core.mm',nx.to_scipy_sparse_matrix(n_g_core.G)))
    wp2.start()

    #mmwrite('core.mm', nx.to_scipy_sparse_matrix(n_g_core.G))
    # if do_eigen:
    #     click.echo("Computing some stats...")
    #     do_stats(n_g_core.G,gun,'core.mm')

    click.secho('saving gex version of graphs...',fg='blue')
    n_g_directed.saveGraph(False,doGEX)
    n_g_core.saveGraph(False,doGEX)
    click.secho("waiting on mm writer threads...")
    wp1.join()
    wp2.join()








@click.command()
@click.option('--nemo_csv_folder',default='./')
@click.option('--nemo_csv_name', default='fire_record_rank*.csv')
@click.option('--save_file_name', default='connection_data.csv')
@click.option('--save_count_name', default='spike_count.csv')
@click.option('--do_save_csv', default=False, is_flag=True)
@click.option('--nemo_g_version',default=0,help='0 = original, 1 = use NeMoG classes, 2 = use serial nemoG.')
@click.option('--cores_per_chip',default=1024, help='number of cores per chip')
@click.option('--do_eigen',default=False, is_flag=True,help="Do eigenvalue centrality analysis along with other analysis.")
@click.option('--gun', default='/home/plaggm/Downloads/gunrock/build/bin/', help='path to gun')
@click.option('-n', default=False, is_flag=True, help='do NSCS tee json spike to graph processing')
@click.option('-nf', default='', help='nscs tee json file location.')
def cli(nemo_csv_folder, nemo_csv_name,save_file_name,save_count_name,
        do_save_csv,nemo_g_version,cores_per_chip,do_eigen,gun,n,nf):
    click.echo("Starting...")

    if n:
        click.secho('loading nscs spike file and generating graphml')
        G = nx.MultiDiGraph()
        with open(nf,'r') as nscs_json:
            for l in nscs_json:
                if 'srcCore' in l:
                    sp = json.loads(l)['spike']
                    src_node = f"{sp['srcCore']}"
                    dst_node = f"{sp['destCore']}"

                    G.add_node(src_node)
                    G.add_node(dst_node)
                    G.add_edge(src_node,dst_node,weight=1)
            nx.write_graphml(G,'./nscs_graph.graphml')
        return

    fns = nemo_csv_folder + "./" + nemo_csv_name
    files = glob.glob(fns)
    click.echo(click.style("Loading spike data...", fg='blue'))
    click.secho("Found the following files: \n " + "\n".join([x for x in files]), fg='green')
    # click.secho(str(files), fg='green')
    # print(len(files))
    if not isinstance(files, list) or len(files) == 0:
        click.secho("No files found. Exiting...", fg='red')
        exit(1)

    if nemo_g_version == 1:
        do_graph_via_class(save_file_name,files)
        return 0
    if nemo_g_version == 2:
        do_graph_via_serial_class(save_file_name,files,do_eigen=do_eigen,gun=gun,num_chips=cores_per_chip)
        return

    counts,connections = loadFiles(files)
    #counts = data[0]
    #connections = data[1]
    if(do_save_csv):
        click.echo("Saving connection CSV file (srccore,srcneuron,destcore,destneuron)")
        with open(save_file_name,'w') as sf:

            sf.write('srccore,srcneuron,destcore,destneuron\n')
            #for line in conn_parser(connections):
            for line in click.progressbar(conn_parser(connections),label="Saving conn. csv..."):
                sf.write(line)
                sf.write('\n')

        with open(save_count_name,'w') as sf:
            sf.write('srccore,srcneuron,count\n')
            #for line in count_parser(counts):
            for line in click.progressbar(count_parser(connections), label="Saving count csv..."):
                sf.write(line)
                sf.write('\n')

    click.echo(click.style("Saving graph data",fg='green'))




    def createWeightEdges(con_ct,con):
        for con_k in con_ct.keys():
            v1 = 'ore'
            bad = v1 in con_k
            bad = bad or v1 in con[con_k]
            if not bad:
                weightmt = (con_k,con[con_k],con_ct[con_k])
                yield weightmt
    G=nx.DiGraph()
    #G=nx.Graph()
    #G.add_weighted_edges_from([(1,2,0.5), (3,1,0.75)])
    for edge in createWeightEdges(counts,connections):
        sr = edge[0]
        dt = edge[1]
        wt = edge[2]

        src_core = int(sr.split(',')[0])
        #sr =int( sr.split(',')[1])
        dest_core =int( dt.split(',')[0])
        #dt = int(dt.split(',')[1])
        src_chip = get_chip_num(src_core)
        dest_chip = get_chip_num(dest_core)

        #G.add_weighted_edges_from([edge])
        G.add_node(sr,core=src_core,chip=src_chip)
        G.add_node(dt,core=dest_core,chip=dest_chip)
        G.add_edge(sr,dt,weight=wt)

    GC = nx.DiGraph()
    #GC = nx.Graph()
    for edge in create_core_counts(connections, counts):
        sr = edge[0]
        dt = edge[1]
        wt = edge[2]
        GC.add_node(sr, chip=get_chip_num(sr))
        GC.add_node(dt, chip=get_chip_num(dt))
        GC.add_edge(sr, dt, weight=wt)
        # GC[sr]['chip'] = get_chip_num(sr)
        # GC[dt]['chip'] = get_chip_num(dt)

    #test
    ## Let's do some stats on the graph here
    ## Let's find the clustering coef, and save that as a parameter to the nodes:
    # click.echo("Calculating cluster data")
    # node_clusters = nx.clustering(G)
    # for k in node_clusters:
    #     G.nodes[k]['clust_coef'] = node_clusters[k]
    #
    # click.echo("calculating cluster data for core graph")
    # node_clusters = nx.clustering(GC)
    # for k in node_clusters:
    #     GC.nodes[k]['clust_coef'] = node_clusters[k]






    nx.write_graphml(G,'traffic_pat_neu_graph.graphml')
    nx.write_gexf(G, "traffic_pat_neu_graph.gexf")





    nx.write_graphml(GC,'traffic_pat_core_graph.graphml')
    nx.write_gexf(GC,'traffic_pat_core_graph.gexf')

    click.echo(click.style("Write Graph data to file. \n Generate MatPlotLib graphs? [y/n] "), nl=False)
    c = 'x'
    doG = True
    if not doG:
        while c != 'y' or c != 'n':
            c = click.getchar(False)
            if c == 'y':
                doG = True
                click.echo()
                break

            if c == 'n':
                doG = False
                click.echo()
                break

            else:
                click.echo(click.style("\n Must pick y or n.",fg='red'))
                click.echo("Generate MatPlotLib graphs? [y/n] ",nl=False)

        click.echo()

    if doG:
        click.echo(click.style("Generating matplotlib graphs...",fg='blue'))
        #nx.draw(G)

        #spring layout
        #pos = nx.spring_layout(G)
        #colors = range(1048576)
        #nx.draw(G,pos,node_color="#A0CBE2", edge_colors=colors, width=1, edge_cmape=plt.cm.Blues,with_labels=False)
        #nx.draw(G,with_labels=False)
        #nx.draw_spectral(G,with_labels=False)

        #pos = nx.kamada_kawai_layout(GC)
        nx.draw_kamada_kawai(GC)
        plt.savefig("con_graph_core.png")

    else:
        print("done!")
    #pd.DataFrame.from_dict(connections).to_csv(save_file_name)
    #pd.DataFrame.from_dict(counts).to_csv(save_count_name)
    return (G,GC)




if __name__ == '__main__':
    cli()

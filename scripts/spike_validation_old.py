import concurrent.futures
import gc
import json


import click
import multiprocessing
import numpy as np
import pandas as pd
from joblib import Parallel,delayed
from pymongo import MongoClient,cursor
import bson
from bson.codec_options import CodecOptions


try:
    #from numba import jit
    from numba import jit, vectorize
    print("Numba JIT enabled")
except:
    print('no numba;')


    def jit(func):
        def _decorator(*args):
            return func(*args)

        return _decorator



    def vectorize(func):
        def _decorator(*args):
            return func(*args)
        return _decorator

#ncols = ["type","srcTime","srcCore","srcNeuron","destCore","destAxon"]
#ncols = ["srcTime","srcCore","srcNeuron","destCore","destAxon"]
ncols = ["srcCore","srcNeuron","destCore","destAxon"]

use_cuda = True
def _connect_mongo(host,port,db):
    return MongoClient(host,port)[db]

@jit
def load_tn_and_find(data_chunk):
    hosts = (['localhost'] * 1) + (['129.161.1.11'] * 1)
    host = np.random.choice(hosts)
    db = "spikev"
    #db = _connect_mongo("localhost",port=27017,db=db)
    db = _connect_mongo(host,port=27017,db=db)
    missing = [0]
    r = 0
    for l in data_chunk:
        if(len(l) > 5):
            r += 1
            dat = json.loads(l)
            ts = dat["srcTime"]
            assert(ts > -1)
            E = .1

            trange = bson.BSON.encode({"$gt":ts - E, "$lt": ts + E})
            query = {"timestamp":trange,"destCore":dat["destCore"], "destNeuron":dat["destNeuron"], "core":dat["srcCore"], "local":dat["srcNeuron"] }
            cursor = db["cf_nemo"].count(query)
            if(cursor == 0):
                #missing.append([x for x,y in dat])
                missing.append(1)
    return len(missing)


@vectorize(['boolean(int64,int64)','boolean(float64,float64)'],target="parallel")
def spikeEQP(source_spike, foreign_spike):
    return np.equal(source_spike,foreign_spike)
    #return np.equal(source_spike,foreign_spike)[1]
    #return np.isclose(source_spike,foreign_spike)
    #return np.array([1],dtype=np.float32)


@vectorize(['boolean(int64,int64)','boolean(float64,float64)'],target="cuda")
def spikeEQC(source_spike, foreign_spike):
    return (abs(foreign_spike - source_spike) > 0.1)


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






def parse_nemo_csv(n_filename):

    #dat = pd.DataFrame(columns=ncols)

    #with open(n_filename, 'r') as file:
    #    for l in file:
    print("NeMo Load")
    dat = pd.read_csv(n_filename,sep=',',header=None,names=ncols,index_col=False,
                      skiprows=1, usecols=[1,2,4,5]) #usecols=[0,1,2,4,5])
    print("NeMo Load Complete.")
    return dat

def js_to_pd(js_list):
    data = pd.DataFrame(columns=ncols)
    for l in js_list:
        if(len(l) > 5):
            dat = json.loads(l)
            data = data.append(dat,ignore_index=True)
    return data



def parse_tn_json(tn_filename):
    #We need src_time, src_core, src_neuron, dest_neruon, dest_axon info
    #Time is going to be a double, rest are long ints
    #tn json is two lines ignore, then
    #{"spike":{"srcTime":0,"srcCore":0,"srcNeuron":1,"destCore":874,"destAxon":222,"destDelay":1}}
    ## load json file into memory(ditch first two lines)
    ## then loop through the file, generating data
    ln = 0
    data = pd.DataFrame(columns=ncols)
    print("loading TN JSON")
    file_dat = []
    with open(tn_filename, 'r') as file:
        for l in file:
            if len(l) > 5 and ":-" in l:
                file_dat.append(l)
        #file_dat = file.readlines()
    chunk_size = len(file_dat) / multiprocessing.cpu_count()

    dchunks = []
    futes = []
    print(f"Total elements: {len(file_dat)} ")
    #datas = Parallel(n_jobs=40,verbose=51,batch_size=1000,pre_dispatch="all")(delayed(js_to_pd)(f) for f in file_dat)
    datas = Parallel(n_jobs=-1,verbose=50)(delayed(js_to_pd)(f) for f in file_dat)
    for d in datas:
        data = data.append(d)
    return data

    with concurrent.futures.ProcessPoolExecutor(max_workers = 40) as executor:
        with click.progressbar(file_dat) as bar:
            for pag in executor.map(js_to_pd,file_dat):
                data.append(pag,ignore_index=True)
                bar.update(1)
    return data




    #for l in file_dat:
    with click.progressbar(file_dat) as bar:
        for l in bar:
            if len(l) > 5:
                dat = json.loads(l)
                #dat['type'] = 'tn'

                #dat = {x:dat[x] for x in ncols}
                data = data.append(dat,ignore_index=True)
    print("TN load complete")
    return data




def spikeMatch(source_spike, foreign_spikes):
    if(use_cuda):
        return np.all(spikeEQC(source_spike,foreign_spikes))
    else:
        return np.all(spikeEQP(source_spike,foreign_spikes))
    #return spikeEQ(source_spike,foreign_spikes)
    #speq = np.vectorize(spikeEQ,excluded=['source_spike'])




@jit
def findSpikes(tn_data_np, nemo_data_np):
    #spikes_in_nemo = np.zeros(shape=tn_data_np.shape[1])
    missing_in_nemo = []
    futs = []
    for tn_spike in tn_data_np:
        sm = spikeMatch(tn_spike,nemo_data_np)
        if not (np.any(sm)):
            missing_in_nemo.append(tn_spike)

        #if not spikeMatch(tn_spike,nemo_data_np):
        #    missing_in_nemo.append(tn_spike)
    return missing_in_nemo
    #with concurrent.futures.ProcessPoolExecutor(max_workers=2) as executor:
    #    for tn_spike in tn_data_np:


@jit
def findOutSpikes(tn_data_np, nemo_data_np):
    missing_in_nemo = []
    for tn_spike in tn_data_np:
        if(tn_spike[2] < 0 or tn_spike[3] < 0):
            sm = spikeMatch(tn_spike, nemo_data_np)
            if not np.any(sm):
                missing_in_nemo.append(sm)

    return missing_in_nemo


def pd_to_np(nemo_data, tn_data):
    #nd = nemo_data.astype(np.float64).as_matrix()
    #td = tn_data.astype(np.float64).as_matrix()
    nd = nemo_data.astype(np.int64).as_matrix()
    td = nemo_data.astype(np.int64).as_matrix()
    return (nd,td)





def load_data(tn_filename, nemo_filename):
    con = False
    if(con):
        with concurrent.futures.ProcessPoolExecutor(max_workers=4) as executor:

            nemo_proc = executor.submit(parse_nemo_csv,nemo_filename)
            tn_proc = executor.submit(parse_tn_json,tn_filename)
            print("Starting data loading")
            nemo_data = nemo_proc.result()
            tn_data = tn_proc.result()
    else:

        tn_data = parse_tn_json(tn_filename)

        nemo_data = parse_nemo_csv(nemo_filename)

    print(f"loaded data. {len(nemo_data)} nemo spikes and {len(tn_data)} spikes loaded.")
    return (nemo_data, tn_data)








if __name__ == '__main__':
    #pre-click test:
    tn_filename = "/shared/share/superneuro/validation_models/pytests/tn.json"
    nemo_filenae = "/shared/share/superneuro/validation_models/pytests/tn.json"

    tn_filename = "/shared/share/superneuro/validation_models/tee_cf_for_mongo.json"
    nemo_filename = "/shared/share/superneuro/validation_models/cf100_nemo_out_final/nemo_cf100_out.csv"


    print("trying query method")
    #Parallel(n_jobs=-1, verbose=50)(delayed(js_to_pd)(f) for f in file_dat)
    with open(tn_filename, 'r') as tnf:
        dat = []
        ct = 0
        for l in tnf:
            ct = ct + 1
            if len(l) > 5 and ":-" in l:
                 dat.append(l)

        print(f"Found {len(dat)} output spikes.")

        #missing = Parallel(n_jobs=20,verbose=51)(delayed(load_tn_and_find)(l) for l in tnf)
    missing = Parallel(n_jobs=20, verbose=51)(delayed(load_tn_and_find)(chunk) for chunk in dat)


    print(f"found {len(missing)} vals!")

    np.savetxt("missing_out_1.csv", np.array(missing),delimiter=',')
    input("Press enter to get dump")
    print(missing)
    exit(0)


    print("loading from files")
    nemo_data, tn_data = load_data(tn_filename, nemo_filename)
    gc.collect()
    print("create numpy arrays")
    nemo_data,tn_data = pd_to_np(nemo_data, tn_data)
    gc.collect()
    print("generating comparisons")
    missing_in_nemo = findSpikes(tn_data,nemo_data)
    print(f"found {len(missing_in_nemo)} missing spikes.")
    missing_out = findOutSpikes(tn_data,nemo_data)
    print(f"found {len(missing_out)} output spikes")

    np.savetxt("missing_full.csv",np.array(missing_in_nemo,dtype=np.int64),delimiter=',')
    np.savetxt("missing_out.csv", np.array(missing_out), delimiter=',')






#def findSpike(tn_spike_row, nemo_spike_list):
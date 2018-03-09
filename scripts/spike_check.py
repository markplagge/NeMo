import concurrent
import concurrent.futures


import multiprocessing
import numpy as np
from collections import defaultdict

from multiprocessing import Process, Manager
from pymongo import MongoClient,cursor
import pandas as pd
import click
import matplotlib.pyplot as plt
import seaborn as sns

# Possible Numba support:
try:
    #from numba import jit
    from numba import jit, prange
    print("Numba JIT enabled")
except:
    print('no numba;')


    def jit(func):
        def _decorator(*args):
            return func(*args)

        return _decorator

    def prange(a,b):
        return range(a,b)


from tqdm import tqdm
from multiprocessing import *

from joblib import Parallel, delayed

def keyGen(type='nemo'):
    if (type == 'nemo'):
        keys= ["timestamp",
        "core",
        "local",
        "destGID",
        "destCore",
        "destNeuron",
        "isOutput?"]
    else:
        keys = ["srcTime",
                "srcCore",
                "srcNeuron",
                "destCore",
                "destAxon",
                "destDelay"]
    return keys

def keyMatch():
    nemo_keys = keyGen()
    tn_keys = keyGen("")
    kvp = {}
    kvp['timestamp'] = 'srcTime'
    kvp['core'] = 'srcCore'
    kvp['local'] = 'srcNeuron'
    kvp['destGID'] = 'destGID'
    kvp['destCore'] = 'destCore'
    kvp['destNeuron'] = 'destAxon'
    kvp['isOutput?'] = 'isOutput'

    return kvp

def _connect_mongo(host, port, username, password, db):
    """ A util for making a connection to mongo """

    if username and password:
        mongo_uri = 'mongodb://%s:%s@%s:%s/%s' % (username, password, host, port, db)
        conn = MongoClient(mongo_uri)
    else:
        conn = MongoClient(host, port)


    return conn[db]

def read_mongo(db='spikev', collection='mnist_tn', query={}, host='128.213.23.222', port=27017, username=None, password=None,
               no_id=True, kvo=None):
    """ Read from Mongo and Store into DataFrame """

    # Connect to MongoDB
    db = _connect_mongo(host=host, port=port, username=username, password=password, db=db)

    # Make a query to the specific DB and Collection
    cursor = db[collection].find(query)


    # Expand the cursor and construct the DataFrame
    df =  pd.DataFrame(list(cursor))
    if no_id:
        del df['_id']
    cols = list(df.columns.values)
    df[cols] = df[cols].astype(int)

    if kvo != None:
        df = df.rename(index=str, columns=kvo)
        df = df.drop(['isOutput','destGID'],axis=1)

    else:
        df = df.drop('destDelay', axis=1)

    # Delete the _id


    return df


def createPandasData(host):
    #Do tn first, then nemo
    #Generate two pandas dataframes
    nemo_query = { "destCore": {"$lte": -10}, "timestamp": {"$lte": 278}}
    mnist_tn_df = read_mongo(host=host)
    mnist_nemo_df = read_mongo(host=host,collection='mnist_nemo',kvo=keyMatch(),query=nemo_query)
    return (mnist_tn_df,mnist_nemo_df)

@click.group()
def cli():
    print("spike comparison test generation")


@click.command()
@click.option('--host', default='128.213.23.222')
def query(host):
    tn, nemo = createPandasData(host)
    # print(tn)
    # print(nemo)
    nemo = nemo.ix[:, ['destAxon', 'destCore', 'srcCore', 'srcNeuron', 'srcTime']]
    tn = tn.ix[:, ['destAxon', 'destCore', 'srcCore', 'srcNeuron', 'srcTime']]
    # for c in cols:
    #     nemo[c] = nemo[c] - 1

    nemo['srcTime'] = nemo['srcTime'] - 34
    nemo['srcTime'] = nemo.round(0)

    df1 = tn
    df2 = nemo

    df1.to_pickle('tn_data.dat')
    df2.to_pickle('nemo_data.dat')

    df1.to_csv('tn_sp.csv')
    df2.to_csv('nemo_sp.csv')
    return (df1, df2)

#@jit(parallel=True) #nopython=True
def compareSpikeRow(left_row, right_row):
    res = True
    for i in prange(0, len(left_row)):
        if(left_row[i] != right_row[i]):
            res = False
    return res

def huntForSpike(row, otherData):
    foundMatch = False
    for r in otherData:
        foundMatch = foundMatch or compareSpikeRow(row, r)
        if foundMatch:
            return False
    return foundMatch

#@jit(nopython=True)
def huntForSpikeTuple(row_chunk, fullData):

    fm = False
    result = []
    for row in row_chunk:
        for d in fullData:
            if compareSpikeRow(row,d):
                result.append(row)
                break

    return result

#@jit()
def huntForSpikeMP(row_chunk, fullData,results):

    for row in row_chunk:
        v = True
        for d in fullData:
            if compareSpikeRow(row,d):
                v = False
        if v:
            results.append(row)




def chunks(l, n):
    """Yield successive n-sized chunks from l."""
    for i in range(0, len(l), n):
        yield l[i:i + n]



def compareSpikeDF(df1, df2):
    #df1 = TrueNorth, df2 = NeMo
    print("saving diffs")
    assert(isinstance(df1, pd.DataFrame))
    assert (isinstance(df2, pd.DataFrame))
    d1 = list(df1.values)
    d2 = list(df2.values)

    mgr = Manager()
    d1g = mgr.list(d2)
    return_list = mgr.list()

    ddict = df1.to_dict()
    num_workers = int(multiprocessing.cpu_count() / 2)
    chunk_size = int(len(d2) / num_workers)
    workers = []
    for chk in chunks(d2,chunk_size):
        p = multiprocessing.Process(target=huntForSpikeMP,args=(chk,d1g,return_list))
        workers.append(p)
        p.start()

    running = True
    finished = []
    with tqdm(total=num_workers) as pbar:
        while(running):
            if len(finished) == num_workers:
                running = False
            for w in workers:
                if not w.is_alive():
                    w.join()
                    finished.append(w)
                    pbar.update()
                    #tqdm.write("Completed one process.")
        #print(return_list)
        missing_flds = return_list

    #print(missing_flds)
    #np.savetxt('missing.csv', missing_flds, delimiter=',',header=",".join(cols))
    cols = ['destAxon', 'destCore', 'srcCore', 'srcNeuron', 'srcTime']
    mf = np.array(return_list)
    np.savetxt('missing.csv',mf,delimiter=',',header=','.join(cols))
    # with open('missing.csv', 'w') as f:
    #     f.write(','.join(cols))
    #     f.write(','.join(['nemo_' + c for c in cols]))
    #     f.write('\n')
    #     for row in missing_flds:
    #             ','.join([str(x) for x in row])
    #             f.write('\n')





def mrfunct(df1, df2, how, s,fn):
    merged = df1.merge(df2,indicator=True, how=how)
    if(s):
        merged.to_csv(fn)




@click.command()
@click.option('--svm', help="Save merge files as csv", is_flag=True)
def findmissing(svm,df1 = None, df2 = None):

    cols = ['destAxon', 'destCore', 'srcCore', 'srcNeuron', 'srcTime']
    if df1 == None:
        df1 = pd.read_pickle('tn_data.dat')
    if df2 == None:
        df2 = pd.read_pickle('nemo_data.dat')

    hows = ["outer","left","inner"]
    p1 = multiprocessing.Process(target=mrfunct,args=[df1,df2,hows[0],svm,f"spike_merge_{hows[0]}.csv"])
    p2 = multiprocessing.Process(target=mrfunct, args=[df1, df2, hows[1], svm, f"spike_merge_{hows[1]}.csv"])
    p3 = multiprocessing.Process(target=mrfunct,args=[df1,df2,hows[2],svm,f"spike_merge_{hows[2]}.csv"])
    #mergedO = df1.merge(df2, indicator=True, how='outer')
    #mergedL = df1.merge(df2, indicator=True, how='left')
    #mergedI = df1.merge(df2, indicator=True, how='inner')




    #if svm:
    #    mergedO.to_csv('spike_merge_outer.csv')
    #    mergedL.to_csv('spike_merge_left.csv')
    #    mergedI.to_csv('spike_merge_iner.csv')

    compareSpikeDF(df1, df2)




cli.add_command(query)
cli.add_command(findmissing)

# @click.command()
# def doAll():
#     query()
#     findmissing(True)




if __name__ == '__main__':


    ide = False
    useClick = True
    alreadySaved = False

    if ide:
        df1 = None
        df2 = None
        if not alreadySaved:
            df1, df2 = query('128.213.23.222')
        findmissing(True, df1, df2)
        exit(0)

    elif useClick:
        cli()
    cols = ['destAxon', 'destCore', 'srcCore', 'srcNeuron', 'srcTime']


    tn,nemo = createPandasData()
    #print(tn)
    #print(nemo)
    cols = ['destAxon', 'destCore', 'srcCore', 'srcNeuron','srcTime']
    nemo = nemo.ix[:, ['destAxon', 'destCore', 'srcCore', 'srcNeuron', 'srcTime']]
    tn = tn.ix[:, ['destAxon', 'destCore', 'srcCore', 'srcNeuron', 'srcTime']]
    # for c in cols:
    #     nemo[c] = nemo[c] - 1

    nemo['srcTime'] = nemo['srcTime'] - 34
    nemo['srcTime'] = nemo.round(0)




    df1 = tn
    df2 = nemo

    df1.to_pickle('tn_data.dat')
    df2.to_pickle('nemo_data.dat')


    df1.to_csv('tn_sp.csv')
    df2.to_csv('nemo_sp.csv')


    df1 = pd.read_pickle('tn_data.dat')
    df2 = pd.read_pickle('nemo_data.dat')
    missing = {}
    for c in cols:
        missing[c] = []


    for index1, tnRow in df1.iterrows():
        for index2, nemRow in df2.iterrows():
            for c in cols:
                if tnRow[c] != nemRow[c]:
                    missing[c].append(tnRow[c])




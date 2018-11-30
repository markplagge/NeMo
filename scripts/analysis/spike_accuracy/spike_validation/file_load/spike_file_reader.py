import numpy as np
import pandas as pd
import pathlib
from concurrent.futures import ThreadPoolExecutor as thread_pool
from concurrent.futures import ProcessPoolExecutor as process_pool
import dask.dataframe as dd
import dask.bag as db
import dask.array as da
from dask.delayed import delayed
import tempfile
import os
import glob
import subprocess
from subprocess import Popen,PIPE

import json

import dask
import dask.bag as db
import dask.array as da
import dask.dataframe as df
import pandas as pd
import numpy as np
from joblib import Parallel, delayed


def parse_and_save(nscs_file_path, nemo_file_path, nemo_pattern='fire_record_rank_*.csv'):
    print(nemo_file_path)
    nscs_spikes = read_nscs_spikes(nscs_file_path)
    nemo_spikes = read_nemo_spike_files(nemo_file_path,nemo_pattern)
    nscs_spikes.compute()
    nscs_spikes.to_json("nscs_spikes.json")
    nemo_spikes.compute()
    nemo_spikes.to_json("nemo_spikes.json")
    nscs_spikes.to_csv("nscs_spikes.csv")
    nemo_spikes.to_csv("nemo_spikes.json")



error_message = ""
_names_ = ["timestamp","core","local","destGID","destCore","destNeuron","isOutput?"]
_names_ = ['timestamp','srcCore','srcNeuron','destGID','destCore','destAxon','isOutput?']
g_blocksize = 128e6
def file_reader(filename):
    skip_first = False
    if "_0.csv" in filename:
        skip_first = True

    with open(filename, 'r') as f:
        if skip_first:
            f.readline()
        yield f.readline()

def read_nemo_spike_file(filename):
    #reads in the nemo csv file

    dat = pd.read_csv(filename,delimiter=",",names=_names_,comment="t")
    return dat

def find_spike_files(root_path,filename_pattern):
    pth = pathlib.Path(root_path)
    if pth.is_dir():
        if pth.exists():
            matched_files = pth.glob(filename_pattern)
            data_files = []
            for f in matched_files:
                fp = pathlib.Path(f)
                size = fp.stat().st_size
                if size > 0:
                    data_files.append(f)
            if len(data_files) > 0:
                return data_files
            else:
                error_message = "No data found. File list was: \n"
                error_message =error_message  + "\n".join([str(x) for x in data_files])
                return False #not needed, but kept for clarity.
    else:
        error_message = "Path " + root_path + " not found or not valid."
    return False


def np_read_spikes(matched_files,mt=True):

    dts = [(x, np.int64) for x in _names_[1:]]
    dts = [("timestamp", np.float128)] + dts
    dtp = np.dtype(dts)

    def np_parser(filename):
        # f = open(filename, 'r')
        # if s:
        #     f.readline()
        # data = np.frombuffer(f, dtype=dtp)
        data = np.genfromtxt(fname=filename,dtype=dtp,comments="timestamp",delimiter=',')
        if len(data) > 0:
            return data
        else:
            return None
        return data
    if mt:
        with thread_pool(max_workers=4) as tp:
            data_map = tp.map(np_parser,matched_files)
            fd  = (x for x in data_map)
            full_data = []
            for d in fd:
                if isinstance(d,np.array):
                    full_data.append(d)

            data = np.concatenate(full_data)
    else:
        data = np.concatenate((np_parser(x) for x in matched_files))
    return data



def get_ranks(root_path,filename_pattern):
    p = pathlib.Path(root_path)
    g = p.glob(filename_pattern)
    ranks = []
    for pth in g:
        if pth.stat().st_size > 0:
            ranks.append(pth.parts[1].split(".csv")[0].split("_")[-1])
        return ranks






def read_files_dsk(root_path,filename_pattern,comp=True):
    rp = pathlib.Path(root_path)
    dts = {x: np.int64 for x in _names_[1:]}
    dts["timestamp"] = np.float64
    if comp:
        print("using compression based loader.")
        filenames = rp.glob(filename_pattern)
        dfs = [delayed(pd.read_csv)(fn) for fn in filenames ]
        df = dd.from_delayed(dfs)
    else:
        df = dd.read_csv(root_path + filename_pattern, blocksize=g_blocksize, names=_names_, dtype=dts, delimiter=',', comment="t")
    return df


def read_nemo_spike_files(root_path="./",filename_pattern="fire_record_rank_*.csv", dask=True,mt=True,np=True):
    # pth = pathlib.Path(root_path)
    # if pth.is_dir():
    #     if pth.exists():
    #         matched_files = pth.glob(filename_pattern)
    #         #print("Found " + str(len(matched_files)) + " files ")
    #         ## Enable threading here.
    print("ROOT PATH: " + root_path)
    matched_files = find_spike_files(root_path,filename_pattern)
    if matched_files:
        if dask:
            if ".gz" in filename_pattern:
                compression = True
            else:
                compression = False
            print("Chunk Size is " + str(g_blocksize))
            return read_files_dsk(root_path,filename_pattern,compression)

        if np:
            return np_read_spikes(matched_files,mt)
        if mt:
            full_data = []
            with thread_pool(max_workers=4) as tp:
                data_map = tp.map(read_nemo_spike_file,matched_files)
                full_data = [d for d in data_map]
        else:
            full_data = [read_nemo_spike_file(fn) for fn in matched_files]

        main_dat = pd.concat(full_data)



        return main_dat
    else:
        print(error_message)
        return -1


### Reader for NSCS spike files
def popen_method(call):
    subprocess_call = Popen([call], shell=True, stdout=PIPE, stderr=PIPE)
    out, err = subprocess_call.communicate()
    if err:
        raise FileNotFoundError(
            '\n============= Could not find file or other error ==============='
            '\n{}\n===========================================\n'.format(
                err.rstrip()))
    return out


def json_callback(line):
    sp = json.loads(line)
    dt = sp['spike']
    dt['timestamp'] = dt['srcTime'] + dt['destDelay']
    return dt



def read_nscs_spikes(nscs_file_name):

    #p = pathlib.Path(tn_file_name)
    print("Loading " + nscs_file_name)
    #if p.exists():
    with tempfile.NamedTemporaryFile(delete=False) as tmp:
        fn = tmp.name
        print("temp: ")
        print(fn)
        cmd = "tail -n +3 " + nscs_file_name + " > " + fn
        print("running " + cmd)
        popen_method(cmd)
        print("removed header in temp file. Using dask to load json")
        data = db.read_text(fn,blocksize=int(g_blocksize)).map(json_callback).to_dataframe()

    return data



    #else:
    #    error_message = "Could not open NSCS spike file " + tn_file_name
    #    return False


if __name__ == '__main__':
    ##test file read.
    pth = "/Users/markplagge/dev/NeMo/libs/spike_validation_tools/test_data"
    print("testing dask")
    dx = read_nscs_spikes("/Users/markplagge/dev/NeMo/libs/spike_validation_tools/test_data/test_nscs.json")
    dk = read_nemo_spike_files(pth)






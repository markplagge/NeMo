import argparse
import concurrent
import multiprocessing

import sys

from itertools import islice
from tqdm import tqdm
import random
import progressbar
import os
import time
from concurrent.futures.thread import ThreadPoolExecutor
from concurrent.futures import ProcessPoolExecutor, as_completed

import psycopg2
from psycopg2 import extras
from pymongo import MongoClient
from sqlalchemy import Table, Column, Integer, String, MetaData, ForeignKey
from sqlalchemy import create_engine

import pymongo
import tempfile
import pandas as pd
import numpy as np
import io

# Possible Numba support:
try:
    from numba import jit

    print("Numba JIT enabled")
except:
    print('no numba;')


    def jit(func):
        def _decorator(*args):
            return func(*args)

        return _decorator


def getConRead(db_name="dumpdb", table='data', host='128.213.23.222', mode=1):
    if mode == 0:
        print("mongo not implemented.")
        return None
    elif mode == 1:
        conn = psycopg2.connect("dbname=postgres user=postgres password=mysecretpassword host=" + host)
        return conn
    else:
        print("mode error!!!")
        return -1


def initPG(host='localhost', table='sumtbl'):
    try:
        conn = psycopg2.connect("dbname=postgres user=postgres password=mysecretpassword host=" + host)
        c = conn.cursor()
    except:
        raise psycopg2.Error
    try:
        c.execute(
            "CREATE  UNLOGGED TABLE {} (source int, dest int, cstart int, cend int) ".format(
                table))
    except:
        print("DB Create Error. Attempting to create new table.")
        conn.commit()
        conn = psycopg2.connect("dbname=postgres user=postgres password=mysecretpassword host=" + host)
        c = conn.cursor()
        c.execute("DROP TABLE {} ".format(table))
        c.execute(
            "CREATE  UNLOGGED TABLE {} (source int, dest int, cstart int, cend int) ".format(
                table))
        # arg.table = "tmp" + str(random.randint()) + str(random.randint()) + str(random.randint())
        # tnt = arg.table
    c.execute("CREATE INDEX rnk_{} ON {} (source,dest)".format(table, table))
    conn.commit()
    # sqlalch:
    engine = create_engine('postgresql://postgres:mysecretpassword@{}/'.format(host, 'postgres'))
    return engine


def getConnetion(db_name="dumpdb", table_name="data", host='128.213.23.222', mode=1):
    if mode == 0:
        client = MongoClient(host)
        db = client[db_name]
        col = db[table_name]
        return col
    elif mode == 1:

        return initPG(host=host, table=table_name)


def saveData(data, host, table, cursCol, mode=1):
    if mode == 0:
        pass
    elif mode == 1:
        sends = data.query('type == "MPI_Isend"')
        engine = create_engine('postgresql://postgres:mysecretpassword@{}/'.format(host, "postgres"))
        engine.connect()
        sends.round({"start": 1, "end": 1})
        sends[['cstart', 'cend']] = sends[['cstart', 'cend']].astype(int)

        sends = sends.filter(items=['source', 'dest', 'cstart', 'cend'])
        # con = initPG(host,table)

        # dq = "INSERT INTO {} VALUES(%s,%s,%s,%s,%s,%s)".format(table)
        # newdat = [sends['source'],sends['dest'],sends['wcstart'],sends['wcend'],sends['cstart'],sends['cend']]
        # psycopg2.extras.execute_batch(con,dq,newdat)
        sends.to_sql(table, engine, if_exists='append', index=False, chunksize=1024)


def loadFile(filename, host, table, mode=0):
    ec = 0
    cols = ["type", "source", "dest", "wcstart", "wcend", "cstart",
            "cend", "dat1", "dat2", "dat3", "dat4"]
    with open(filename, 'r') as f:
        with tempfile.TemporaryFile(mode="r+") as tmp:
            # go line by line - ensure that the last line isn't cut off and remove the extra ,
            for line in f:
                line = line.split(',')
                try:
                    if (len(line[1].split(' ')) < 11):
                        pass
                    else:
                        tmp.write(line[1])
                except:
                    ec = 1
            tmp.seek(0)

            data = pd.read_table(tmp, sep=" ", header=None, names=cols, index_col=None)  # index_col=1)
    saveData(data, host, table, None, mode=mode)

    return ec


def processChunk(lines, host, table, mode=1):
    table = []
    linedat = io.StringIO()
    cols = ["type", "source", "dest", "wcstart", "wcend", "cstart",
            "cend", "dat1", "dat2", "dat3", "dat4"]
    for l in lines:
        try:
            ltxt = l.split(',')[1]
            linedat.write(ltxt)
            linedat.write('\n')
        except:
            print("Linedat error")
    linedat.seek(0)
    data = pd.read_table(linedat, sep=" ", header=None, names=cols, index_col=None)  # index_col=1)
    saveData(data, host, table, None, mode=mode)

    # dq = "INSERT INTO {} VALUES(%s,%s,%s,%s,%s,%s)".format(table)
chunkQ = multiprocessing.Queue()
workingQ = multiprocessing.Queue()
processedQ = multiprocessing.Queue()

class procWorker(multiprocessing.Process):
    def __init__(self,task_q, result_q):
        multiprocessing.Process.__init__(self)
        self.task_q = task_q
        return
    def run(self):
        proc_name = self.name
        while True:
            pass
import threading

class fileProcessor():
    chunkQ = None
    fileQ = None
    procQ = None
    def __init__(self, host, table, fileList, mode, np):
        self.chunkQ = multiprocessing.Queue()
        self.fileQ = multiprocessing.Queue()
        self.strQ = multiprocessing.Queue()
        np = int(np/3)
        self.fileList = fileList
        self.host = host
        self.table = table
        self.numchunks = np
        self.chunkGirth = 5000

        initPG(host,table)
        for file in self.fileList:
            self.fileQ.put(file)
        self.filepos = 1
        self.chunkpos = self.filepos + len(fileList)
        self.chunkProcs = [multiprocessing.Process(target=self.chunkFile,args=(i + 1,)) for i in range(len(fileList))]
        self.mergeProcs = [threading.Thread(target=self.mergeWork) for i in range(np)]
        self.writeProcs = [multiprocessing.Process(target=self.writeWork) for i in range(np)]
        for p1,p2,p3 in zip(self.chunkProcs, self.mergeProcs, self.writeProcs):
            p1.start()
            p2.start()
            p3.start()
        for p1 in tqdm(self.chunkProcs,position=0, desc="file loading procs."):
            p1.join()
        self.chunkQ.put(None)
        for p1 in tqdm(self.mergeProcs):
            p1.join()
        self.strQ.put(None)
        for p1 in tqdm(self.writeProcs):
            p1.join



    def monitor(self):
        #check chunk output queue first
        pass

    def chunkFile(self,pos):
        def chunkput(chunko):
            self.chunkQ.put(chunko)
        filePos = self.filepos
        self.filepos += 1
        filePos = pos
        while(self.fileQ.empty() != True):
            fileName = self.fileQ.get()
            #chunks = [0] * self.numchunks
            chunks = ["" for i in range(self.numchunks)]
            with open(fileName, 'r') as f:
                #file = mmap.mmap(f.fileno(), 0, access=mmap.ACCESS_READ)
                file = f
                ctr = 0
                i = 0
                for line in tqdm(f,position=filePos,desc="file loading..."):
                    chunks[ctr] = f"{chunks[ctr]}{line}"
                    ctr = i % self.numchunks

                    i += 1
                    if i % self.chunkGirth == 0:
                        chunkput(chunks)
                        chunks = ["" for i in range(self.numchunks)]
            chunkput(chunks)


    def mergeWork(self):
        mpos = self.chunkpos
        self.chunkpos += 1

        cols = ["type", "source", "dest", "wcstart", "wcend", "cstart",
                "cend", "dat1", "dat2", "dat3", "dat4"]
        while True:
            lines = self.chunkQ.get()
            if lines == None:
                break
            dat = io.StringIO()
            for l in tqdm(lines,position=mpos, desc="MergeWork"):
                #for l in ll.split('\n'):
                    try:
                        ltxt = l.split(',')[1]
                        if (len(ltxt.split(' ')) < 11):
                            pass
                        else:
                            dat.write(f"{ltxt}\n")
                    except:
                        print("line truncated")
            time.sleep(2)
        dat.seek(0)
        self.strQ.put(dat)

    def writeWork(self):

        cols = ["type", "source", "dest", "wcstart", "wcend", "cstart",
                "cend", "dat1", "dat2", "dat3", "dat4"]
        while(True):
            lines = self.strQ.get()
            if lines == None:
                break
            data = pd.read_table(lines, sep=" ", header=None, names=cols, index_col=None)  # index_col=1)
            saveData(data, self.host, self.table, None, mode=1)






def chunkFiles(fileList, host, table, mode):
    dt = fileProcessor(host,table,fileList,1,arg.nump)

    return dt


def loadChunkIntoDB(lines, host, table, mode=1):
    cols = ["type", "source", "dest", "wcstart", "wcend", "cstart",
            "cend", "dat1", "dat2", "dat3", "dat4"]


def fastLoadDB(file_list, host='128.213.23.222', table="sumpd", mode=1):
    eng1 = initPG(host, table)
    rp = []
    # with concurrent.futures.ProcessPoolExecutor() as executor:
    with concurrent.futures.ThreadPoolExecutor() as executor:
        for worker_file in tqdm(file_list):
            rp.append(executor.submit(loadFile, worker_file, host, table, mode))
        for res in tqdm(concurrent.futures.as_completed(rp)):
            if res.result() != 0:
                tqdm.write("Found a truncated line.")

    print("loaded into DB")
    return



def aggQuery(host, table, mode=1):
    q2 = """SELECT source,
         dest,
         COUNT(cstart)
FROM     {}
GROUP BY source,
         dest
ORDER BY source,dest"""

    conn = psycopg2.connect("dbname=postgres user=postgres password=mysecretpassword host=" + host)
    c = conn.cursor()
    gendq = "SELECT MAX(cstart) FROM {};".format(table)
    c.execute(gendq)
    maxTick = c.fetchone()[0]

    with open("agg.csv", "w") as f:
        q = q2.format(table)
        c.execute(q)
        comm_lines = c.fetchall()
        cpos = 0
        for c in comm_lines:
            dst_cnt = c[2]
            dst_cnt = int(dst_cnt / maxTick)
            f.write(f"{c[0]},{c[1]},{dst_cnt}\n")
        return
        for src_chip in tqdm(range(0, int(arg.numc)), desc="Input Chip Read",position=0):
            dest_count = np.zeros(arg.numc)
            for dest_chip in tqdm(range(0, arg.numc), desc="Output Chip Read",position=1):
                if comm_lines[cpos][0] == src_chip and comm_lines[cpos][1] == dest_chip:
                    dest_count=comm_lines[cpos][2]
                    dest_count[dest_chip] = int(dest_count[dest_chip] / maxTick)
                    cpos += 1
                if len(comm_lines) >= cpos:
                    return
            [f.write(f"{src_chip},{i},{dest_count[i]}\n") for i in range(0, arg.numc)]


def aggQueryGenerateZeros(host,table, mode=1):
    q2 = """SELECT source,
             dest,
             COUNT(cstart)
    FROM     {}
    GROUP BY source,
             dest
    ORDER BY source,dest"""

    conn = psycopg2.connect("dbname=postgres user=postgres password=mysecretpassword host=" + host)
    c = conn.cursor()
    gendq = "SELECT MAX(cstart) FROM {};".format(table)
    c.execute(gendq)
    maxTick = c.fetchone()[0]

    with open("agg.csv", "w") as f:
        q = q2.format(table)
        c.execute(q)
        commLines = c.fetchall()
        sourceDestCount= np.zeros([arg.numc,arg.numc,0],dtype=np.long)
        cline = 0
        for src in range(0,arg.numc):
            for dest in range(0, arg.numc):
                try:
                    if commLines[cline][0] == src and commLines[cline][1] == dest:
                        ns = commLines[cline][2]
                        ns = int(ns / maxTick)
                        sourceDestCount[src][dest] = ns
                        cline = cline + 1
                    if cline >= len(commLines):
                        print(cline)
                        break
                except:

                    break;
        sourceDestCount.tofile('agg.csv',sep=',')






def saveAggData(host, table, mode=1):
    aggQueryGenerateZeros(host,table,mode)
    #aggQuery(host,table,mode)
    return
    if mode == 1:
        pgquery = """SELECT 
         COUNT(cstart)
FROM     {}
WHERE    (source = {} ) AND (dest = {})"""
        conn = psycopg2.connect("dbname=postgres user=postgres password=mysecretpassword host=" + host)
        c = conn.cursor()
        gendq = "SELECT MAX(cstart) FROM {};".format(table)
        c.execute(gendq)
        maxTick = c.fetchone()[0]


        with open("agg.csv", "w") as f:
            for src_chip in tqdm(range(0, int(arg.numc)), desc="Input Chip Read",position=0):
                dest_count = np.zeros(arg.numc)
                for dest_chip in tqdm(range(0, arg.numc), desc="Output Chip Read"):
                    q = pgquery.format(table, src_chip, dest_chip)
                    c.execute(q)
                    line = c.fetchone()
                    #with tqdm(desc=f"parsing chip {src_chip} to {dest_chip}") as tq:
                    while (line is not None):
                        dest_count[dest_chip] = int(line[0])
                        dest_count[dest_chip] = int(dest_count[dest_chip] / maxTick)
                        line = c.fetchone()
                    [f.write(f"{src_chip},{i},{dest_count[i]}\n") for i in range(0, arg.numc)]



def getCount(host, table, db_name='dt', mode=1):
    if mode == 0:
        print("mode error mongo")
    elif mode == 1:
        conn = getConRead(db_name, table, host, mode)
        c = conn.cursor()
        c.execute(f"SELECT COUNT(*) FROM {table}")
        line = c.fetchone()
        if (line is not None):
            return line[0]
        else:
            return -1


if __name__ == '__main__':
    desc = """Loads and generates average spike pairs for CODES synthetic workload integration.
    Currently requires a running Postgresql server. 
    """
    eplg = """Check usage for more details..."""
    parser = argparse.ArgumentParser(description=desc, epilog=eplg)
    parser.add_argument("--table", help="table name", default='sumpd')
    parser.add_argument("--host", help="host db", default="128.213.23.222")
    parser.add_argument("--dbname", help="Mongo DB name - only used in Mongo mode", default="mongdat")
    parser.add_argument("--nall", action="store_true", help="do not process all files with extension found in dir",
                        default=False)
    parser.add_argument('--ext', nargs=1, help="filename extension", default=".txt")
    parser.add_argument('--numc', help="number of chips", default=4096)

    parser.add_argument('--noload', action='store_true', default=False, help="Don't load files into database.")
    parser.add_argument('--nosave', action='store_true', default=False,
                        help="Don't load from db and create parsed files.")

    parser.add_argument('--nump', default=0, help="Use this many threads/processes.")
    parser.add_argument('--exmp', default=False, action='store_true', help="Use extreme MP for loading.")
    parser.add_argument("files", nargs="*", help="list of files to load")
    arg = parser.parse_args()
    if (not arg.noload and (arg.nall and len(arg.files) <= 0)):
        parser.error("If loading files non-automatically (--nall), you must specify one or more files to load.")


    ## Arg cleanup:
    if (isinstance(arg.numc, list)):
        arg.numc = arg.numc[0]
    arg.numc = int(arg.numc)
    if (isinstance(arg.nump, list)):
        arg.nump = arg.nump[0]
    arg.nump = int(arg.nump)

    if arg.nump == 0:
        arg.nump = multiprocessing.cpu_count()
        if sys.platform == 'darwin':
            r = os.popen('sysctl hw').readlines()[1:20]
            for i in r:
                if "physicalcpu" in i:
                    nps = i.split(':')[1]
                    arg.nump = int(nps)

    mode = 1  # mode 1 is pgsql mode - no mongo yet

    if (not arg.noload):
        file_list = []

        if (not arg.nall):
            print("All " + arg.ext + " files.")
            for file in os.listdir("."):
                if file.endswith(arg.ext):
                    file_list.append(os.getcwd() + "/" + file)
        else:
            print(f"Not loading all files. Using explicit file list: \n {arg.files}")
        if (len(arg.files) > 0):
            for file in arg.files:
                file_list.append(file)
        print(f"Reading in data into table {arg.table} on database host {arg.host}")
        #try:
        if (arg.exmp):
                chunkFiles(file_list, arg.host, arg.table,1)



        else:
                fastLoadDB(file_list, arg.host, arg.table)
        #except psycopg2.Error as e:
            #parser.error(f"Unable to connect to database or other DB error.\n Host: {arg.host}"
#                         f"\t Table: {arg.table}\n Files: {file_list}\n PG Error: {e.pgerror}")
        #except:
            #parser.error(f"General load error. Host: {arg.host}\t Table: {arg.table}\n Files: {file_list}\n")

    else:
        print("Skipping creation of db and data load")

    ct = getCount(arg.host, arg.table, arg.dbname, mode)
    print(f"Selected table has {ct} rows.")

    if (not arg.nosave):
        print(f"generating aggregate csv file from table {arg.table} on host {arg.host}")
        saveAggData(arg.host, arg.table)
    else:
        print("Skipping creation of aggregate CSV data")

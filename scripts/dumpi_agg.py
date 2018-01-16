import pandas as pd
import os
import numpy as np
import argparse
import concurrent
import multiprocessing
from multiprocessing import Pool

import psycopg2
import time
from sqlalchemy import Column, ForeignKey, Integer, String
from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy.orm import relationship
from sqlalchemy import create_engine
Base = declarative_base()
import io
#from concurrent.futures.thread import ThreadPoolExecutor
from multiprocessing import Queue,Manager

from concurrent.futures import ProcessPoolExecutor
from concurrent.futures import *
from tqdm import tqdm

# SQL Table Creation:
def initPG(host='localhost', table='sumtbl'):
    cq = "CREATE  UNLOGGED TABLE {} (source int, dest int, cstart real, cend real) "
    try:
        conn = psycopg2.connect("dbname=postgres user=postgres password=mysecretpassword host=" + host)
        c = conn.cursor()
    except:
        raise psycopg2.Error
    try:
        c.execute(
            cq.format(
                table))
    except psycopg2.Error as e:
        print(e)
        print("DB Create Error. Attempting to create new table.")
        conn.commit()
        conn = psycopg2.connect("dbname=postgres user=postgres password=mysecretpassword host=" + host)
        c = conn.cursor()
        c.execute("DROP TABLE {} ".format(table))
        c.execute(
           cq.format(
                table))
        # arg.table = "tmp" + str(random.randint()) + str(random.randint()) + str(random.randint())
        # tnt = arg.table
    c.execute("CREATE INDEX rnk_{} ON {} (source,dest)".format(table, table))
    conn.commit()


class FileParser():

    def __init__(self, host,table,file_list,chunk_size = 1000000):


        self.data = None

        self.chunk_size = chunk_size
        self.host = host
        self.table = table
        self.file_list = file_list
        self.db_type = 'postgres'
        self.fileProcs = []
        self.writerProcs = []
        initPG(host, table)
        self.engine = self.createDBConnection()


        m = multiprocessing.Manager()
        self.loadedFileQ = multiprocessing.Queue()
        self.loadedFileQ = m.Queue(100000000)

    def processFiles(self, np):
        #with ProcessPoolExecutor() as thdex:
        #with ThreadPoolExecutor() as thdex:
            #with Pool(processes=np) as pool:
            #    self.fileProcs = pool.imap_unordered(self.loadFilePdPy,file_list)
            #    time.sleep(1)
                for i in file_list:
                    p = multiprocessing.Process(target=self.loadFilePdPy,args=(i,))
                    p.start()
                    self.fileProcs.append(p)
                for i in range(np):
                    p = multiprocessing.Process(target=self.writerProcess,args=(i,))
                    p.start()


                    self.writerProcs.append(p)
                #list(tqdm(self.fileProcs, total=len(file_list), position=0))
                for p in self.fileProcs:
                    p.join()

                self.loadedFileQ.put(None)

                with tqdm(total=np) as bar:
                    ttl = np
                    completep = []
                    while(True):
                        for p in self.writerProcs:
                            p.join()
                            bar.update(1)

                        # for i in range(len(self.writerProcs)):
                        #     if self.writerProcs[i].is_alive():
                        #
                        #         pass
                        #     else:
                        #         if not self.writerProcs[i] in completep:
                        #             completep.append(self.writerProcs[i])
                        #             bar.update(1)
                        # if [p.is_alive() for p in self.writerProcs]:
                        #     break




#                 i = 0
#                 for file in self.file_list:
#                     self.fileProcs.append(thdex.submit(self.loadFilePdPy,file))
#                     i += 1
#                 time.sleep(2)
#
#                 #self.writerProcs = [pool.apply_async(self.writerProcess, i) for i in np]
#                 #self.fileProcs = pool.map(self.loadedFileQ, self.file_list)
#                 #self.writerProcs = pool.imap_unordered(self.writerProcess, range(np))
#                # for i in range(np):
#                 #    px = multiprocessing.Process(target=self.writerProcess,args=i)
# #
#  #                   self.writerProcs.append(px)
#                 for i in range(np):
#                     self.writerProcs.append(thdex.submit(self.writerProcess,i))
#
#                 #for _ in tqdm(as_completed(self.fileProcs), position=0):
#                 #    pass
#                 list(tqdm(self.fileProcs, total=len(file_list), position=0))
#                 self.loadedFileQ.put(None)
#                 list(tqdm(self.writerProcs, total=np, position=0))

    def createDBConnection(self):
        if self.db_type is 'postgres':
            self.engine = create_engine('postgresql://postgres:mysecretpassword@{}/'.format(self.host, "postgres"))
            self.engine.connect()

    def initDB(self):
        if self.db_type == 'postgres':
            initPG(self.host, self.table)


    def writerProcess(self,i):
        with tqdm(position=i+1) as tq:
            engine = create_engine('postgresql://postgres:mysecretpassword@{}/'.format(self.host, "postgres"))
            engine.connect()
            while True:
                pdata = self.loadedFileQ.get()
                if( pdata is not None ):
                    pdata.to_sql(self.table, engine, if_exists='append', index=False)
                else:
                    return
                tq.update(1)

    def loadFilePdPy(self, filename):
        cols = ["type", "source", "dest", "wcstart", "wcend", "cstart",
                "cend", "dat1", "dat2", "dat3", "dat4"]
        cols = [cols[1],cols[2],cols[5],cols[6]]
        #x = pd.read_table(filename, sep=' ', usecols=[1, 2, 5, 6],header=None, names=cols, index_col=None)
        x = pd.read_table(filename, sep=' ', usecols=[1, 2, 5, 6], chunksize=self.chunk_size,header=None, names=cols, index_col=None)
        for chunk in x:
            self.loadedFileQ.put(chunk)


def loadAllIntoPandas(file_list,chunk_size, host, table):
    cols = ["type", "source", "dest", "wcstart", "wcend", "cstart",
            "cend", "dat1", "dat2", "dat3", "dat4"]
    cols = [cols[1], cols[2], cols[5], cols[6]]

    str = io.StringIO()
    for filename in tqdm(file_list):
        with open(filename, 'r') as f:
            for line in f:
                line = line.split(',')
                try:
                    if (len(line[1].split(' ')) < 11):
                        pass
                    else:
                        str.write(line[1])
                except:
                    ec = 1
    str.seek(0)
    engine = create_engine('postgresql://postgres:mysecretpassword@{}/'.format(host, 'postgres'))
    engine.connect()
    data = pd.read_table(str, sep=' ', usecols=[], chunksize=chunk_size)
    for chunk in tqdm(data):
        chunk.to_sql(table, engine, if_exists='append', index=False)


#### DATA AGGREGATION
def aggQuery(host,table,mode=1):
    print("AGG QUERY SPARSE")
    print("AGG Query")
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
    print("Max ticks: " + str(maxTick))
    with open("agg.csv", "w") as f:
        q = q2.format(table)
        print("running agg query...")
        c.execute(q)
        line = c.fetchone()
        ttl = c.rowcount
        with tqdm(total=ttl) as bar:
            while line is not None:
                mtv = int(line[2] / maxTick)
                f.write(f"{line[0]},{line[1]},{mtv}\n")
                line = c.fetchone()
                bar.update(1)

def aggQueryGenerateZeros(host,table, mode=1):
    print("AGG Query")
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
        for src in tqdm(range(0,arg.numc),position=0, desc="sender chip"):
            for dest in tqdm(range(0, arg.numc),position=1, desc="recvr chip"):
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


def doAgg(host,table,dbname,nump,mp,cs):
    if arg.add0:

        aggQueryGenerateZeros(host,table,1)
    else:
        aggQuery(host,table,1)

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
    parser.add_argument('--add0', help="add zeros to csv?", default=False, action='store_true')
    parser.add_argument('--noload', action='store_true', default=False, help="Don't load files into database.")
    parser.add_argument('--nosave', action='store_true', default=False,
                        help="Don't load from db and create parsed files.")

    parser.add_argument('--nump', default=0, help="Use this many threads/processes.")
    parser.add_argument('--mp', default=False, action='store_true', help="Use multiprocessing")
    parser.add_argument('--cs', default=131070, help="chunk size")


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

    if(isinstance(arg.cs, list)):
        arg.cs = arg.cs[0]
    arg.cs = int(arg.cs)

    if arg.nump == 0:
        arg.nump = multiprocessing.cpu_count()

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
        if arg.mp:
            reader = FileParser(arg.host, arg.table, file_list)
            reader.createDBConnection()
            reader.processFiles(arg.nump)
        else:
            loadAllIntoPandas(file_list,arg.cs,arg.host,arg.table)
        print("Completed loading files into database.")

    if (not arg.nosave):
        print("saving into agg. csv file")
        doAgg(arg.host,arg.table,arg.dbname,arg.nump,arg.mp,arg.cs)
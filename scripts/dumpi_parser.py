import argparse
import concurrent
from concurrent.futures.thread import ThreadPoolExecutor
from concurrent.futures import ProcessPoolExecutor, as_completed
import sqlite3
import numpy as np
import pandas
import psycopg2
from tqdm import tqdm
import os
import time
import progressbar

def initsqlite(dbname="dumps.sqlite"):
    conn = sqlite3.connect(dbname)
    tblname = arg.table
    sql = """BEGIN;
-- CREATE TABLE "table1" ---------------------------------------
CREATE TABLE "table1"(
	"sc" Integer,
	"type" Text,
	"source" Integer,
	"dest" Integer,
	"wcstart" Real,
	"wcend" Real,
	"cstart" Real,
	"cend" Real,
	"dat1" Integer,
	"dat2" Integer,
	"dat3" Integer,
	"dat4" Integer );
-- -------------------------------------------------------------

-- CREATE INDEX "index_sc" -------------------------------------
CREATE INDEX "index_sc" ON "table1"( "sc" );
-- -------------------------------------------------------------
-- CREATE INDEX "index_wcstart" --------------------------------
CREATE INDEX "index_wcstart" ON "table1"( "wcstart" );
-- -------------------------------------------------------------
COMMIT;

"""
    sql = sql.replace('table1', tblname)
    c = conn.cursor()
    c.executescript(sql)
    conn.commit()
    return conn

def initpgsql(dbname='dumpi.db', host='localhost'):
    print("table set to " + str(arg.table))
    tnt = arg.table
    # else:
    # 	r = random.randrange(1,10000)
    # 	tnt = "dload_" + str(r)
    # 	print("table set to " + tnt)
    conn = psycopg2.connect("dbname=postgres user=postgres password=mysecretpassword host=" + host)
    c = conn.cursor()
    try:
        c.execute("CREATE TABLE {} (rank int , sort numeric, line text) ".format(tnt))
    except:
        print("DB Create Error")
        c.execute("DROP TABLE {} ".format(tnt))
        c.execute("CREATE TABLE {} (rank int, sort numeric, line text)".format(tnt))
    # c.execute("CREATE INDEX rank ON dumpi (rank)")
    conn.commit()
    return conn


def getConSQLite(dbname="dumps.sqlite"):
    conn = sqlite3.connect(dbname)
    return conn

def getConPG():
    return psycopg2.connect("dbname=postgres user=postgres password=mysecretpassword host=localhost")

def getCon(dbname="dumps.sqlite", mode=0):
    #if mode == 0:
    return getConSQLite(dbname)


def addLineSL(data, cursor, tblname):
    cursor.executemany("INSERT INTO " + tblname + "VALUES (?,?,?,?,?,?,?,?,?,?,?,?",data)


def addLine(data,cursor,tblname,mode):
    if mode==0:
        return addLineSL(data,cursor,tblname)


def commit(cursor):
    cursor.commit()

def initSQL(mode=0, dbname="dumps.sqlite"):
    if mode == 0:
        return initsqlite(dbname)
    if mode == 1:
        return initpgsql(dbname)



def fastLoadPushRunner(localFileList,mode=0):
    #conn = psycopg2.connect("dbname=postgres user=postgres password=mysecretpassword host=localhost")
    conn = getCon(mode=mode)
    #c = conn.cursor()

    cols = ["sc", "type", "source", "dest","wcstart","wcend","cstart",
            "cend","dat1","dat2","dat3","dat4"]
    delimits = [','] + [" "] * 10
    dts = [np.str, int, int] + [np.double] * 4
    dts = dts + [np.int] * 4
    lndat = []

    for filename in localFileList:
        dat = pandas.read_table(filename,sep=",|\s",header=None, names=cols,engine='python',index_col=False)
        dat.to_sql(arg.table, conn, if_exists= 'append', index=False)
    conn.commit()

def stdLoad(file_list):
    mode = 0
    initSQL(mode=mode)
    for i in tqdm(file_list):
        fastLoadPushRunner([i],0)
    return

def fastLoadDB(file_list):

    mode = 1
    if arg.sqlite:
        mode = 0

    initSQL(mode=mode)
    chunks = 54
    worker_file_lists = []
    cworker = 0
    for i in range(0, chunks):
        worker_file_lists.append([])

    for fn in file_list:
        worker_file_lists[cworker].append(fn)
        cworker += 1
        if cworker >= chunks:
            cworker = 0
            #	print("FL:")
            #	print(file_list)
            #	print("WFL:")
            #	print(worker_file_lists)

    print("Arming & Starting FILE->DB ")
    running_procs = []
    # tester
    #	for worker_files in worker_file_lists:
    #		fastLoadPushRunner(worker_files)

    with concurrent.futures.ProcessPoolExecutor(max_workers=40) as executor:
        i = 0

        for worker_files in tqdm(worker_file_lists,position=0):
            running_procs.append(executor.submit(fastLoadPushRunner, worker_files))

            i += 1
        print("Loading DB")
        for f in tqdm(concurrent.futures.as_completed(running_procs),position=1):
            pass
            #		with progressbar.ProgressBar(max_value=40) as bar:
            #			bar.update(0)
            #			while(not all([x.done() for x in running_procs])):
            #				bar.update(sum([x.done() for x in running_procs]))

    print("loaded into DB")
    return
def newfile(rank):
    f = open(arg.outfile_tmplt + str(rank).zfill(4) + ".dat", 'w')
    return f

def get_and_save_from_db():

    rnkCmd = "SELECT * from " \
             + arg.table + " ORDER BY sc,wcstart"

    conn = getCon()
    c = conn.cursor()
    crank = 0
    f = newfile(crank)
    v = c.execute("SELECT MAX("+ arg.table + ".sc) FROM  " + arg.table  + ";")
    mx = v.fetchone()[0]
    c = conn.cursor()
    bar = progressbar.ProgressBar(max_value=int(mx))
    i = 0


    c.execute(rnkCmd)
    line = c.fetchone()
    while line is not None:  # for line in c.execute(rnkCmd):
        nrank = line[0]
        if crank != nrank:
            f.close()
            f = newfile(nrank)
            crank = nrank
            bar.update(i)
            i += 1
        f.write(" ".join([str(x) for x in line[1:]]))
        f.write("\n")
        line = c.fetchone()
    f.close()


if __name__=='__main__':
    ctimes = time.strftime("%y.%m.%d.%H.%M.%S")
    parser = argparse.ArgumentParser(description='Process NEMO dumpi files')
    parser.add_argument('--ext', nargs=1, help="filename extension", default=".txt")
    parser.add_argument("--noall", action="store_true", help="do not process all files with extension found in dir",
                        default=False)
    parser.add_argument('--table', nargs=1, help="table name", default="dumpi")
    parser.add_argument("--dbname", default="dumpi.sqlite")
    parser.add_argument('outfile_tmplt', nargs='?', help="save to this file", default="dumpi--" + ctimes +
                                                                                      "-processed-")
    parser.add_argument("--sqlite", default=True, action="store_true", help="SQLITE breakdown")
    parser.add_argument("files", nargs="*", help="process these files", type=argparse.FileType('r'))


    arg = parser.parse_args()


    print("parser")
    print("table: " + arg.table)
    print("dbname:" + arg.dbname)
    file_list = []
    if (not arg.noall):
        print("All " + arg.ext + " files.")
        for file in os.listdir("."):
            if file.endswith(arg.ext):
                file_list.append(os.getcwd() + "/" + file)

    if (len(arg.files) > 0):
        for file in arg.files:
            file_list.append(file)
    print("Reading in:")


    #arg.table = arg.table[0]
    fastLoadDB(file_list)
    get_and_save_from_db()
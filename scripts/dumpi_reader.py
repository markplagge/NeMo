import argparse
import concurrent
from tqdm import tqdm
import random
import progressbar
import os
import time
from concurrent.futures.thread import ThreadPoolExecutor
from concurrent.futures import ProcessPoolExecutor, as_completed
import psycopg2
from psycopg2 import extras

try:
    import pymongo
    from pymongo import MongoClient
    from pymongo import IndexModel, ASCENDING, DESCENDING


    mongo_ok = True
except:
    mongo_ok = False

try:
    import couchdb
    from couchdb import client
    from couchdb.client import Server, Database
    couch_ok = True

except:
    couch_ok = False

import joblib
from joblib import delayed, memory, Memory, Parallel


def readL(line):
    return line.split(",")


def readFile(filename):
    with open(filename, 'r') as f:
        lines = f.readlines()

    return lines


def compareLine(line1, line2):
    l1 = line1.split(' ')
    l2 = line2.split(' ')
    st1 = float(l1[3])
    st2 = float(l2[3])
    if st1 == st2:
        st1 = float(l1[5])
        st2 = float(l2[5])
        if st1 == st2:
            return 0

    if st1 > st2:
        return 1

    return -1


def sortKey(fullLine=[]):
    fullLine.sort(key=lambda x: float(x.split(' ')[3]))
    return fullLine


def saveRank(data, key):
    with open(arg.outfile_tmplt + str(key).zfill(4) + ".dat", 'w') as f:
        f.write("<mpiType> <src> <dst> <wallStart> <wallStop> <cpuStart> <cpuStop> <count> <dataType> <comm> <tag>\n")
        for v in data:
            f.write(v)


def initSQL(dname='dumpi.db', host="localhost"):
    # conn = sqlite3.connect(dname)
    #
    # c = conn.cursor()
    # if(arg.table):
    print("table set to " + str(arg.table))
    tnt = arg.table
    # else:
    # 	r = random.randrange(1,10000)
    # 	tnt = "dload_" + str(r)
    # 	print("table set to " + tnt)

    conn = psycopg2.connect("dbname=postgres user=postgres password=mysecretpassword host=" + host)
    c = conn.cursor()
    try:
        c.execute("CREATE  UNLOGGED TABLE {} (rank int , sort numeric, line text) ".format(tnt))
    except:
        print("DB Create Error")
        conn.commit()
        conn = psycopg2.connect("dbname=postgres user=postgres password=mysecretpassword host=" + host)
        c = conn.cursor()
        c.execute("DROP TABLE {} ".format(tnt))

        c.execute("CREATE TABLE {} (rank int, sort numeric, line text)".format(tnt))
        #arg.table = "tmp" + str(random.randint()) + str(random.randint()) + str(random.randint())
        #tnt = arg.table
    c.execute("CREATE INDEX rnk_{} ON {} (rank)".format(tnt,tnt))
    conn.commit()
    return conn


def getCon(dbname='dumpi.db', host='localhost'):
    tnt = arg.table
    conn = psycopg2.connect("dbname=postgres  user=postgres password=mysecretpassword host=localhost")
    return conn


def getSQL(dname='dumpi.db'):
    # conn = sqlite3.connect(dname)
    conn = psycopg2.connect("dbname=postgres user=postgres password=mysecretpassword host=localhost")
    return conn


def addLine(rank, time, line, cursor):
    # cursor = getSQL()
    line = line.rstrip()
    cmd = f"INSERT INTO {arg.table} VALUES ({rank},{time},'{line}')"
    cursor.execute(cmd)
    return cursor


def commit(cursor):
    cursor.commit()


def fastLoadPushRunner(localFileList):
    conn = psycopg2.connect("dbname=postgres user=postgres password=mysecretpassword host=localhost")
    c = conn.cursor()
    lndat = []
    ec = 0
    for filename in localFileList:
        with open(filename, "r") as f:
            lines = f.readlines()
            for line in lines:
                line = line.split(',')
                #
                #
                # else:
                try:
                    if(len(line[1].split(' ')) < 11):
                        ec = 1
                    else:
                        lndat.append([int(line[0]), float(line[1].split(' ')[3]), line[1]])
                except:
                    ec = 1


    psycopg2.extras.execute_batch(c, "INSERT INTO " + arg.table + " VALUES (%s, %s, %s)", lndat)
    conn.commit()
    return ec


def fastLoadDB(file_list):
    initSQL()

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

    with concurrent.futures.ProcessPoolExecutor() as executor:
        i = 0

        for worker_files in tqdm(worker_file_lists):
            running_procs.append(executor.submit(fastLoadPushRunner, worker_files))


        print("Loading DB")
        for res in tqdm(concurrent.futures.as_completed(running_procs)):
            if res.result() != 0:
                tqdm.write("Found a truncated line!")

        #		with progressbar.ProgressBar(max_value=40) as bar:
        #			bar.update(0)
        #			while(not all([x.done() for x in running_procs])):
        #				bar.update(sum([x.done() for x in running_procs]))

    print("loaded into DB")
    return


def bulkLoadForDB(file_list):
    i = 0
    lndat = []
    maxFiles = 1024
    cfiles = 0
    if type == "pg":
        pass
    print(" loading into postgres...")
    conn = initSQL()
    c = conn.cursor()

    with progressbar.ProgressBar(max_value=len(file_list)) as bar:
        for filename in file_list:
            cfiles = cfiles + 1
            with open(filename, 'r') as f:
                lines = f.readlines()
                # lndat = []
                for line in lines:
                    line = line.split(',')
                    # p2 = line[1].split("|")
                    # lndat.append([int(line[0]), int(p2[0]), float(p2[1].split(" ")[3]), p2[1] ])


                    # float(line[1].split(' ')[X] is the key to sort on. 3 is wall clock start, 5 is CPU start
                    # if arg.join:
                    # 	line = removeDelay(line)
                    lndat.append([int(line[0]), float(line[1].split(' ')[3]), line[1]])

                # addLine(line[0], line[1].split(' ')[5], line[1],c)

                # psycopg2.extras.execute_batch(c,"INSERT INTO "+arg.table + " VALUES (%s, %s, %s)",lndat)
            bar.update(i)
            i += 1
            if cfiles >= maxFiles:
                psycopg2.extras.execute_batch(c, "INSERT INTO " + arg.table + " VALUES (%s, %s, %s)", lndat)
                cfiles = 0
                lndat = []

    psycopg2.extras.execute_batch(c, "INSERT INTO " + arg.table + " VALUES (%s, %s, %s)", lndat)
    conn.commit()


###############3###COUCHDB
def createCDB(table="dumpi_run", host="http://localhost:5984"):
    print("create/load couch con")
    return getdb(getsrv(host), table)


def getsrv(host):
    couch = couchdb.Server(host)
    return couch


def getdb(couch, table):
    try:
        db = couch[table]
    except:
        db = couch.create(table)
    return db


##INSERT into ol' couchy:
def addDataToCouchWC(couchdb, data):
    couchdb.save_bulk(data)


from uuid import uuid4


def addDataToCouchNC(db, data):
    doc_id = uuid4().hex
    db[doc_id] = data


#	db = createCDB(table,host)
#	addDataToCouchWC(db,data)

import requests


def cdb(host, table):
    requests.put(host + table)


def insertrawblk(host, table, data):
    r = requests.post(host + table, json=data)
    if r.status_code != 201:
        print("NON CODE ERRRR")
        print(r)
        print("-data-")
        print(data)


def insertrawfile(filename, host, table):
    data = createObjFromFile(filename)
    Parallel(n_jobs=8, backend="threading")(delayed(insertrawblk)(host, table, d) for d in data)


import uuid


def insertrawthreader(host, table, file_list):
    with concurrent.futures.ThreadPoolExecutor() as exe:
        ftrs = []
        for fn in tqdm(file_list):
            data = createObjFromFile(fn)
            for d in data:
                d["_id"] = str(uuid.uuid5(uuid.NAMESPACE_OID, str(d["rank"]) + str(d["sort"])).hex)
                ftrs = ftrs + [exe.submit(insertrawblk, host, table, d) for d in data]
        print("Threads started & primed")
        for f in tqdm(as_completed(ftrs), leave=False):
            pass


# with concurrent.futures.ProcessPoolExecutor(max_workers=10) as executor:
#	with concurrent.futures.ThreadPoolExecutor() as executor:
#		futures = [executor.submit(insertrawfile, filename,host,table) for filename in file_list]
#		print("couch proc primed, starting writes...")
#		for f in tqdm(as_completed(futures),leave=False):
#			pass


#: Multitproc okay couch
def addFilesToCouch(file_list, table, host):
    print("couch table " + table)
    print("couch host " + host)

    db = createCDB(table, host)
    ## create the view for getting merged results:
    viewdef = {"language": "javascript", "views": {
        "main": {"map": "function(doc) { rank = doc.rank; txt = doc.line; sort = doc.sort; emit([rank,sort] ,txt); }"}}}
    requests.put(host + table + "/_design/" + table, json=viewdef)
    print("direct curl mode: " + str(arg.cr))
    if arg.cr:
        insertrawthreader(host, table, file_list)
        return db

    dat = []
    print("loading files")
    with concurrent.futures.ThreadPoolExecutor() as executor:
        ftrs = []
        for filename in file_list:
            dat = dat + createObjFromFile(filename)
            ftrs = ftrs + [executor.submit(addDataToCouchNC, db, dt) for dt in dat]
        for f in tqdm(as_complete(ftrs), leave=False):
            pass
        #		Parallel(n_jobs=10)(delayed(addDataToCouchNC)(table,host,dt) for dt in dat)
    return db


def getFilesFromCouch(table, host, numRanks, dbx):
    db = dbx  # createCDB(table,host)
    vr = db.iterview("_design/" + table + "/_view/main", batch=1)
    crank = -1
    i = 0
    print("Saving COUCH data")
    # assume view has been created already.
    for v in tqdm(vr):
        rank = v["key"][0]
        time = v["key"][1]
        line = v["value"]
        if crank == -1:
            f = newfile(rank)
            crank = rank
        elif rank != crank:
            f.close()
            f = newfile(rank)
            crank = rank
        f.write(line)


#################MONGO 
def getMongoCol(table="dumpi_run", host="localhost"):
    db = gdb(host)
    col = db[table]
    return col


def gdb(host="localhost"):
    client = MongoClient(host)
    db = client.dumpi
    return db


def createObjFromFile(fileName):
    dat = []
    with open(fileName, 'r') as f:
        lines = f.readlines()
        for line in lines:
            line = line.split(',')
            dat.append({"rank": int(line[0]),
                        "sort": float(line[1].split(' ')[3]),
                        "line": line[1]})
    return dat


def addDataToMongo(dat, col):
    return (col.insert_many(dat)).inserted_ids


def addFileToMongo(filename, table):
    col = getMongoCol(table)
    return addDataToMongo(createObjFromFile(filename), col)


def addFilesToMongo(file_list, table):
    dat = []
    for fn in file_list:
        dat = dat + addFileToMongo(fn, table)

    return dat


def chunkData(file_list, chunks=10):
    worker_file_lists = []
    cworker = 0
    for i in range(0, chunks):
        worker_file_lists.append([])

    for fn in file_list:
        worker_file_lists[cworker].append(fn)
        cworker += 1
        if cworker >= chunks:
            cworker = 0
    return worker_file_lists


def mongoLoadFiles(file_list, table="dumpi_run", workers=10, chunks=10, chunked=True):
    ########CHUNK THIS?
    workers = arg.mwkrs
    print(workers)
    chunks = workers
    col = getMongoCol(table)
    db = gdb()
    db[table].create_index([('rank', pymongo.ASCENDING), ('sort', pymongo.ASCENDING)])
    if arg.nommt:
        print("single threaded mongo database loading")
        docids = []
        for filename in tqdm(file_list, dynamic_ncols=True, leave=False):
            data = createObjFromFile(filename)
            docids = docids + addDataToMongo(data, col)

        return docids

    print("reading into Mongo")
    docids = []
    i = 0
    threads = []
    #	with concurrent.futures.ThreadPoolExecutor() as executor:
    with concurrent.futures.ProcessPoolExecutor(max_workers=workers) as executor:
        #		with progressbar.ProgressBar(max_value = len(file_list) + 1) as bar:
        #			bar.update(i)
        if chunked:
            cfs = chunkData(file_list, chunks)
            #				inserts = {executor.submit(addFilesToMongo,chunkfiles,table): chunkfiles for chunkfiles in tqdm(cfs,dynamic_ncols=True)}
            futures = [executor.submit(addFilesToMongo, fl, table) for fl in cfs]
        else:
            #				inserts = {executor.submit(addFileToMongo,filename,table): filename for filename in tqdm(file_list, dynamic_ncols=True)}
            futures = [executor.submit(addfileToMongo, fl, table) for fl in
                       tqdm(file_list, dynamic_ncols=True, leave=False)]

        #			i+=1
        #			bar.update(i)
        #			for filename in file_list:
        #				threads.append(executor.submit(addFileToMongo(filename, col)
        #				lndat = createObjFromFile(filename)
        #				idocids = docids + addDataToMongo(lndat, col)

        #			for future in tqdm(concurrent.futures.as_completed(inserts)):
        print("mongo processes primed, starting writes...")
        for f in tqdm(as_completed(futures), leave=False):
            pass

    for i, future in tqdm(enumerate(futures), leave=False):
        #				bar.update(i)
        #				i += 1
        docids = docids + future.result()

    return docids


def saveMongoData(docIDs, table):
    col = getMongoCol(table)
    crank = -1
    i = 0
    #	with progressbar.ProgressBar(max_value = len(file_list)) as bar:
    for entry in tqdm(col.find().sort([("rank", pymongo.ASCENDING), ("sort", pymongo.ASCENDING)])):
        if crank == -1:
            f = newfile(entry["rank"])
            crank = entry["rank"]
        elif entry["rank"] != crank:
            crank = entry["rank"]
            f.close()
            f = newfile(entry["rank"])
            crank = entry["rank"]

        f.write(entry["line"])
    #			bar.update(i)
    #			i += 1

    f.close()


##########
def loadFiles(file_list, tablename, type="pg"):
    if arg.bulk == True:
        bulkLoadForDB(file_list)
        return

    # Used to do this line by line.
    i = 0
    lndat = []
    # lndat = bulkLoadForDB(file_list)
    if type == "pg":
        pass
    print(" loading into postgres...")
    conn = initSQL()
    c = conn.cursor()

    #	with progressbar.ProgressBar(max_value = len(file_list)) as bar:
    for filename in tqdm(file_list):
        with open(filename, 'r') as f:
            lines = f.readlines()
            # lndat = []
            for line in tqdm(lines, leave=False):
                line = line.split(',')
                # p2 = line[1].split("|")
                # lndat.append([int(line[0]), int(p2[0]), float(p2[1].split(" ")[3]), p2[1] ])


                # float(line[1].split(' ')[X] is the key to sort on. 3 is wall clock start, 5 is CPU start

                lndat.append([int(line[0]), float(line[1].split(' ')[3]), line[1]])

            # addLine(line[0], line[1].split(' ')[5], line[1],c)
            psycopg2.extras.execute_batch(c, "INSERT INTO " + arg.table + " VALUES (%s, %s, %s)", lndat)
        #			bar.update(i)
        i += 1

    # psycopg2.extras.execute_batch(c, "INSERT INTO " + arg.table + " VALUES (%s, %s, %s)", lndat)



    # commit(c)
    conn.commit()
    return i


def parseFiles(file_list, db=False):
    datum = {}

    for filename in file_list:
        print("File: " + filename)
        lines = readFile(filename)
        for line in lines:
            line = line.split(',')
            if (line[0] not in datum.keys()):
                datum[line[0]] = []

            datum[line[0]].append(line[1])
    print("Sort starting...")
    bc = 0

    with concurrent.futures.ProcessPoolExecutor(max_workers=16) as executor:
        fts = []
        with progressbar.ProgressBar(max_value=len(datum.keys())) as bar:
            for key in datum:
                fts.append(executor.submit(sortKey, datum[key]))
                bar.update(bc)
                bc += 1
        print("Sorting files...")

        with progressbar.ProgressBar(max_value=len(datum.keys())) as bar:
            while (all([x.running() for x in fts])):
                bar.update(sum([x.done() for x in fts]))

    print("File sorted")
    return datum


def newfile(rank):
    f = open(arg.outfile_tmplt + str(rank).zfill(4) + ".dat", 'w')
    return f


def getMaxRank():
    rnkCmd = "SELECT MAX(rank) FROM dumpi"
    conn = getSQL()
    c = conn.cursor()
    v = c.execute(rnkCmd)
    print(v)
    return v[0]


def get_and_save_from_db(i=0):
    rnkCmd = "SELECT rank,line FROM " + str(arg.table) + " ORDER BY rank,sort"
    conn = getSQL()
    c = conn.cursor()
    crank = 0
    f = newfile(crank)

    bar = progressbar.ProgressBar(max_value=progressbar.UnknownLength)
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

        f.write(line[1])
        line = c.fetchone()
    f.close()


    for i in tqdm(range(0, int(arg.numc)),desc="Finding missing core files..."):
        c = conn.cursor()
        c.execute("SELECT rank from str(arg.table) WHERE rank == " + str(i))
        line = c.fetchone()
        if line is None:
            f = newfile(i)
            f.close()



#### Band-aid fix for delay ####

def removeDelay(textLine):
    tl = textLine.split(" ")
    if tl[0] == "MPI_Irecv":
        tl[3] = str(float(tl[3]) - 1)
    return " ".join(tl)


if __name__ == '__main__':
    ctimes = time.strftime("%y.%m.%d.%H.%M.%S")
    parser = argparse.ArgumentParser(description='Process NEMO dumpi files')
    parser.add_argument('--ext', nargs=1, help="filename extension", default=".txt")
    parser.add_argument("--all", action="store_false", help="process all files with extension found in dir",
                        default=True)
    parser.add_argument("--db", action="store_true", help="use database", default=True)
    parser.add_argument('--table', nargs=1, help="table name", default="dumpi")
    parser.add_argument('outfile_tmplt', nargs='?', help="save to this file", default="dumpi--" + ctimes +
                                                                                      "-processed-")
    parser.add_argument("--numc", default=0, help="Number of chips in simulation - will generate empty files to fill.")
    # parser.add_argument("--join", action="store_true", default=False)
    parser.add_argument("--bulk", action="store_true", default=False, help="use sequental bulk updates")
    parser.add_argument("--mp", action="store_true", default=False, help="enable pgsql multithread")
    parser.add_argument("--mongo", action="store_true", default=False, help="enable Mongo")
    parser.add_argument("--nommt", action="store_true", default=False, help="disable Mongo Multithread")
    parser.add_argument("--mwkrs", default=10, help="Mongo MultiProc Num Workers")
    parser.add_argument("--couch", default=False, action="store_true", help="CouchDB it!")
    parser.add_argument("--sqlite", default=False, action="store_true", help="SQLITE breakdown")
    parser.add_argument("--cr", default=False, action="store_true", help="couch raw request mode")
    parser.add_argument("files", nargs="*", help="process these files", type=argparse.FileType('r'))
    arg = parser.parse_args()

    # generate file list:
    file_list = []
    if (arg.all):
        print("All " + arg.ext + " files.")
        for file in os.listdir("."):
            if file.endswith(arg.ext):
                file_list.append(os.getcwd() + "/" + file)

    if (len(arg.files) > 0):
        for file in arg.files:
            file_list.append(file)
    print("Reading in:")
    if arg.couch:
        couch_host = "http://localhost:5984/"
        print("COUCHDB testing....")
        db = addFilesToCouch(file_list, arg.table[0], couch_host)
        print("Done COUCH add.")
        print("saving files...")
        getFilesFromCouch(arg.table[0], couch_host, 60000, db)

        exit()

    elif arg.mongo:
        print("mongo status: " + str(mongo_ok))
        if (mongo_ok):
            #			try:
            arg.mwkrs = int(arg.mwkrs)
            arg.table = arg.table[0]
            ids = mongoLoadFiles(file_list, arg.table)
            #			except e:
            #				print("Exception during mongo loadup.")
            #				quit()
            r = saveMongoData(ids, arg.table)
        else:
            print("Mongo Lib not loaded. Exiting")
            exit()



    elif arg.db:
        arg.table = arg.table[0]
        if arg.mp:
            fastLoadDB(file_list)
        else:
            loadFiles(file_list, arg.table)
        print("saving from db")
        get_and_save_from_db()

    else:
        datum = parseFiles(file_list)
        print("saving files")
        with concurrent.futures.ThreadPoolExecutor() as executor:
            dats = []
            for k in datum.keys():
                dats.append(executor.submit(saveRank, datum[k], k))
            print("Writing.....")
            executor.shutdown()


        # for k in datum.keys():
        # 	with open(arg.outfile_tmplt + str(k).zfill(4) + ".dat", 'w') as f:
        # 		f.write("<mpiType> <src> <dst> <wallStart> <wallStop> <cpuStart> <cpuStop> <count> <dataType> <comm> <tag>\n")
        # 		for v in datum[k]:
        # 			f.write(v )

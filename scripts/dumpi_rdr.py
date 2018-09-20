import argparse
import concurrent
import progressbar
import os
import time
from concurrent.futures.thread import ThreadPoolExecutor
import psycopg2
from psycopg2 import extras

import joblib
from joblib import delayed,memory,Memory,Parallel

def readL(line):
	return line.split(",")

def readFile(filename):

	with open(filename,'r') as f:
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
		if st1==st2:
			return 0


	if st1 > st2:
		return 1

	return -1

def sortKey(fullLine = []):
	fullLine.sort(key=lambda x: float(x.split(' ')[3]))
	return fullLine


def saveRank(data,key):
	with open(arg.outfile_tmplt+str(key).zfill(4) + ".dat", 'w') as f:
		f.write("<mpiType> <src> <dst> <wallStart> <wallStop> <cpuStart> <cpuStop> <count> <dataType> <comm> <tag>\n")
		for v in data:
			f.write(v)


def initSQL(dname='dumpi.db'):
	# conn = sqlite3.connect(dname)
	#
	# c = conn.cursor()
	tnt = arg.table
	conn = psycopg2.connect("dbname=postgres user=postgres password=mysecretpassword host=localhost")
	c = conn.cursor()
	try:
		c.execute("CREATE TABLE " + tnt + " (rank int , sort numeric, line text) ")
	except:
		print("DB Create Error")
		c.execute('''DROP TABLE dumpi''')
		c.execute('''CREATE TABLE dumpi (rank int, sort numeric, line text)''')
	#c.execute("CREATE INDEX rank ON dumpi (rank)")
	conn.commit()
	return conn


def getSQL(dname='dumpi.db'):
	#conn = sqlite3.connect(dname)
	conn = psycopg2.connect("dbname=postgres user=postgres password=mysecretpassword host=localhost")
	return conn

def addLine(rank,time,line,cursor):
	#cursor = getSQL()
	line = line.rstrip()
	cmd = f"INSERT INTO {arg.table} VALUES ({rank},{time},'{line}')"
	cursor.execute(cmd)
	return cursor

def commit(cursor):
	cursor.commit()

def loadFiles(file_list, tablename):
	conn = initSQL()
	c = conn.cursor()
	i = 0
	with progressbar.ProgressBar(max_value = len(file_list)) as bar:
		for filename in file_list:
			with open(filename, 'r') as f:
				lines = f.readlines()
				lndat = []
				for line in lines:
					line = line.split(',')
					#p2 = line[1].split("|")
					#lndat.append([int(line[0]), int(p2[0]), float(p2[1].split(" ")[3]), p2[1] ])


					#float(line[1].split(' ')[X] is the key to sort on. 3 is wall clock start, 5 is CPU start

					lndat.append([int(line[0]), float(line[1].split(' ')[3]), line[1]])

					#addLine(line[0], line[1].split(' ')[5], line[1],c)
				psycopg2.extras.execute_batch(c,"INSERT INTO "+arg.table + " VALUES (%s, %s, %s)",lndat)
			bar.update(i)
			i += 1



	#commit(c)
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

def get_and_save_from_db(i):
	rnkCmd = "SELECT rank,line FROM " + str(arg.table) + " ORDER BY rank,sort"
	conn = getSQL()
	c = conn.cursor()
	crank = 0
	f = newfile(crank)

	bar = progressbar.ProgressBar(max_value=progressbar.UnknownLength)
	i = 0
	c.execute(rnkCmd)
	line = c.fetchone()
	while line is not None: #for line in c.execute(rnkCmd):
		nrank = line[0]
		if crank != nrank:
			f.close()
			f = newfile(nrank)
			crank = nrank
			bar.update(i)
			i += 1

		f.write(line[1] )
		line = c.fetchone()
	f.close()



if __name__ == '__main__':
	ctimes = time.strftime("%y.%m.%d.%H.%M.%S")
	parser = argparse.ArgumentParser(description='Process NEMO dumpi files')
	parser.add_argument('--ext', nargs=1, help="filename extension",default=".txt")
	parser.add_argument("--all", action="store_false", help="process all files with extension found in dir",
						default=True)

	parser.add_argument("--db", action="store_true", help="use database", default=False)
	parser.add_argument('--table', nargs=1, help="table name", default="dumpi")

	parser.add_argument('outfile_tmplt', nargs='?', help="save to this file", default="dumpi--" + ctimes +
						"-processed-")
	parser.add_argument("files", nargs="*", help="process these files", type=argparse.FileType('r'))


	arg = parser.parse_args()

	#generate file list:
	file_list = []
	if(arg.all):
		print("All " + arg.ext  + " files.")
		for file in os.listdir("."):
			if file.endswith(arg.ext):
				file_list.append(os.getcwd() + "/" + file)

	if(len(arg.files) > 0):
		for file in arg.files:
			file_list.append(file)
	print("Reading in:")
	if arg.db:
		
		arg.table = arg.table[0]
		i = loadFiles(file_list, arg.table)
		print("saving from db")
		get_and_save_from_db(i)

	else:
		datum = parseFiles(file_list)
		print("saving files")
		with concurrent.futures.ThreadPoolExecutor() as executor:
			dats = []
			for k in datum.keys():
				dats.append(executor.submit(saveRank,datum[k],k))
			print("Writing.....")
			executor.shutdown()


	# for k in datum.keys():
	# 	with open(arg.outfile_tmplt + str(k).zfill(4) + ".dat", 'w') as f:
	# 		f.write("<mpiType> <src> <dst> <wallStart> <wallStop> <cpuStart> <cpuStop> <count> <dataType> <comm> <tag>\n")
	# 		for v in datum[k]:
	# 			f.write(v )
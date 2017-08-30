import argparse

import os
import time
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



def parseFiles(file_list):
	datum = {}


	for filename in file_list:
		print("File: " + filename)
		lines = readFile(filename)
		for line in lines:
			line = line.split(',')
			if(line[0] not in datum.keys()):
				datum[line[0]] = []

			datum[line[0]].append(line[1])
	for key in datum:
		#float(x.split(' ')[X] is the key to sort on. 3 is wall clock start, 5 is CPU start
		datum[key].sort(key=lambda x: float(x.split(' ')[3]))

	return datum



if __name__ == '__main__':
	ctimes = time.strftime("%y.%m.%d.%H.%M.%S")
	parser = argparse.ArgumentParser(description='Process NEMO dumpi files')
	parser.add_argument('--ext', nargs=1, help="filename extension",default=".txt")
	parser.add_argument("--all", action="store_false", help="process all files with extension found in dir",
						default=True)
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

	datum = parseFiles(file_list)

	for k in datum.keys():
		with open(arg.outfile_tmplt + str(k).zfill(4) + ".dat", 'w') as f:
			f.write("<mpiType> <src> <dst> <wallStart> <wallStop> <cpuStart> <cpuStop> <count> <dataType> <comm> <tag>\n")
			for v in datum[k]:
				f.write(v )

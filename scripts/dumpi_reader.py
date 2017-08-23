import argparse

import os
import time
def readL(line):
	return line.split(",")

def readFile(filename):

	with open(filename,'r') as f:
		lines = f.readline()

	return lines


def parseFiles(file_list):
	datum = {}
	for filename in file_list:
		lines = readFile(filename)
		for line in lines:
			if(line[0] not in datum.keys()):
				datum[line[0]].append(line[1])
			datum[line[0]] = []
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

	datum = parseFiles(file_list)

	for k in datum.keys():
		with open(arg.outfile_tmplt + str(k).zfill(4) + ".dat", 'w') as f:
			for v in datum[k]:
				f.write(v + "\n")

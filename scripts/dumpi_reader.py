import argparse



def readL(line):
	return line.split(",")

def readFile(filename):

	with open(filename,'r') as f:
		lines = f.readline()

	return lines


def parseFiles(filenameList):



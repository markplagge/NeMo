import argparse



def readL(line):
	return line.split(",")

def readFile(filename):
	coreData = []

	with open(filname,'r') as f:
		

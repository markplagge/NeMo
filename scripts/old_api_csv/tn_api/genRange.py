import numpy as np
try:

	from numba import jit
	print("NB")
except:

	print('no numba;')
	def jit(func):
		def _decorator(*args):
			return func(*args)
		return _decorator

@jit
def getArrayFromMulti(jItem):

	increment = 1
	if jItem.count(':') >= 1:
		vals = jItem.split(":")

		if jItem.count(':') == 1:
			start = int(vals[0])
			end = int(vals[1] ) + start + -1
		else:
			start = int(vals[0])
			end = int(vals[2])  + start + -1
			increment = int(vals[1])

		rng  = np.arange(start,end,increment,dtype=np.int32)
		return rng
	elif jItem.count('x') == 1:
		sp = jItem.split('x')
		num = int(sp[1])
		val = int(sp[0])
		its = [val] * (num-1)
		return np.array(its, dtype=np.int32)
	else:
		return [int(jItem)]
		return np.array([int(jItem)], dtype=np.int32)



def populateArray(inputArr, lines):
	pos = 0
	for typeItem in lines:
		itms = getArrayFromMulti(typeItem)
		for itm in itms:
			inputArr[pos] = itm
			pos += 1
			if pos > 255:
				print("Err - line item {} pushed past the end of the array from {}".format(itm,lines))
				#raise IndexError("Error OOB from lineitem")

				return inputArr

	return inputArr



if __name__ == '__main__':
	typeTests = [
		["0:255"],
		["1x256"],
		["1x16", "-2x240"],
		["0:15", "-1x240"],
		['1x256']
	]

	res = []
	for testList in typeTests:
		itmArry = np.zeros(256)
		res.append(populateArray(itmArry,testList))

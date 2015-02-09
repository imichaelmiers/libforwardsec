import numpy as np
from sys import argv
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import sim
def main(argv):
	data = np.load(argv[1])
	plot_sizezvsrate(data)
def plot(data):
	windowIndex = 0
	msgrateIndex = 1
	intervalLengthIndex = 3
	dirCPUTOTALIndex=11+6
	DecCPUTotalIndex=23+6
	PunCPUToTalIndex=35+6
	MaxSize=36+6
	subRows=[]
	other=[]
	oo= []
	target = 0.001
	for r in data:
		if r[msgrateIndex] == target:
			subRows.append(r)
		if r[msgrateIndex] == 0.01:
			other.append(r)
		if r[msgrateIndex] == 0.0001:
			oo.append(r)

	subRows = np.array(subRows)
	other = np.array(other)
	oo = np.array(oo)
	dirTimes = subRows[:,dirCPUTOTALIndex]
	decTime = subRows[:,DecCPUTotalIndex]
	PunTime = subRows[:,PunCPUToTalIndex]
	totalTime = np.add(dirTimes,decTime)
	totalTime = np.add(totalTime,PunTime)
	rate = subRows[:,intervalLengthIndex]

	sizes = subRows[:,MaxSize]
	osize = other[:,MaxSize]
	orate = other[:,intervalLengthIndex]
	oor = oo[:,intervalLengthIndex]
	oos = oo[:,MaxSize]

	# plt.plot(oor,oos,'go-',label='0.0001 msgs per second')
	# plt.plot(rate,sizes,'bv-',label='0.001 msgs per second')
	# plt.plot(orate,osize,'rs-',label='0.01 msgs per second')

	plt.plot(rate,dirTimes,'bv-',label='Key Derivation ')
	plt.plot(rate,decTime,'rs-',label='Decrypt Times')
	plt.plot(rate,PunTime,'gp-',label='Pun Times')
	plt.plot(rate,totalTime,'ko-',label='Total')
	plt.title('Keysize vs interval size for %s seconds, window of %s'%(data[0,4],data[0,0]))
	plt.gca().legend(loc='upper right',shadow=True)
	plt.gca().set_xscale('log')
	plt.xlabel('interval size(s)')
	plt.ylabel('keySize KB')
	plt.show()
def plot_sizezvsrate(data):
	windowIndex = 0
	msgrateIndex = 1
	intervalLengthIndex = 3
	dirCPUTOTALIndex=11+6
	DecCPUTotalIndex=23+6
	PunCPUToTalIndex=35+6
	MaxSize=36+6
	subRows=[]
	target = 0.001
	for r in data:
		print r[intervalLengthIndex]
		print r[msgrateIndex]
		print " "
		if r[intervalLengthIndex] == r[msgrateIndex]:
			subRows.append(r)
	print(len(subRows))

	subRows = np.array(subRows)

	dirTimes = subRows[:,dirCPUTOTALIndex]
	decTime = subRows[:,DecCPUTotalIndex]
	PunTime = subRows[:,PunCPUToTalIndex]
	totalTime = np.add(dirTimes,decTime)
	totalTime = np.add(totalTime,PunTime)
	rate = subRows[:,intervalLengthIndex]

	sizes = subRows[:,MaxSize]


	plt.plot(rate,sizes,'bv-')

	plt.title('Keysize vs msg rate, window of %s'%(data[0,0]))
	plt.gca().legend(loc='upper right',shadow=True)
	plt.gca().set_xscale('log')
	plt.xlabel('interval size(s)')
	plt.ylabel('keySize KB')
	plt.show()
if __name__ == "__main__":
	main(argv)

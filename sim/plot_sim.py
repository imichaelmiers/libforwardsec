import numpy as np
from sys import argv
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import sim
def main(argv):
	data = np.load(argv[1])
	plot(data)
def plot(data):
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
		if r[msgrateIndex] == target:
			subRows.append(r)
	subRows = np.array(subRows)
	dirTimes = subRows[:,dirCPUTOTALIndex]
	decTime = subRows[:,DecCPUTotalIndex]
	PunTime = subRows[:,PunCPUToTalIndex]
	totalTime = np.add(dirTimes,decTime)
	totalTime = np.add(totalTime,PunTime)
	rate = subRows[:,intervalLengthIndex]
	sizes = subRows[:,MaxSize]
	plt.plot(rate,sizes,'bv-',label='size')

	# plt.plot(rate,dirTimes,'bv-',label='Key Derivation ')
	# plt.plot(rate,decTime,'rs-',label='Decrypt Times')
	# plt.plot(rate,PunTime,'gp-',label='Pun Times')
	# plt.plot(rate,totalTime,'ko-',label='Total')
	plt.title('Performance vs interval size for %s at %s message a second'%(data[0,4],target))
	plt.gca().legend(loc='upper right',shadow=True)
	plt.gca().set_xscale('log')
	plt.xlabel('interval size(s)')
	plt.ylabel('total time(s) ')
	plt.show()

if __name__ == "__main__":
	main(argv)

import numpy as np
from sys import argv
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import sim
def main(argv):
	data = sim.sim_intervals(1)
	print "got data"
	plot(data)
def plot(data):
	print (data.shape)
	decTime = data[:,4]
	dirTimes = data[:,5]
	PunTime = data[:,6]
	maxDer = data[:,7]
	maxDec = data[:,8]

	totalTime = np.add(dirTimes,decTime)
	totalTime = np.add(totalTime,PunTime)
	totalMax = np.add(maxDer,maxDec)
	windows = data[:,0]
	rate = data[:,1]
	# fig = plt.figure()
	# ax = fig.add_subplot(111, projection='3d')

	# ax.scatter(rate,windows,totalTime)
	# ax.set_xlabel('Average message rate (msg/interval)')
	# ax.set_ylabel('Window size (intervals)')
	# ax.set_zlabel('time (ms)')

	# plt.plot(rate,dirTimes,'b-',label='% Key Derivation Times')
	# plt.plot(rate,decTime,'r-',label='% Decrypt Times')
	# plt.plot(rate,PunTime,'g-',label='% Puncture Times')

	plt.plot(rate,dirTimes,'bo-',label='Key Derivation ')
	plt.plot(rate,decTime,'ro-',label='Decrypt Times')
	plt.plot(rate,PunTime,'go-',label='Pun Times')
	plt.plot(rate,totalTime,'ko-',label='Total')

	plt.gca().legend(loc='upper right',shadow=True)
	plt.xlabel('interval size(ms)')
	plt.ylabel('total time(ms) ')
	plt.show()
if __name__ == "__main__":
	main(argv)

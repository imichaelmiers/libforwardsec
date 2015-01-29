import numpy as np
from sys import argv
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
def main(argv):
	print "trying to load data"
	data = np.load(argv[1])
	print "data lodaed"
	plot(data)
def plot(data):
	dirTimes = data[:,2]
	dirStdev = data[:,3]
	decTime = data[:,4]
	decStdev = data[:,5]
	PunTime = data[:,6]
	PunStdev  = data[:,7]
	maxSize = data[:,8]
	totalTime = np.add(dirTimes,decTime)
	totalTime = np.add(totalTime,PunTime)
	
	windows = data[:,0]
	rate = data[:,1]
	fig = plt.figure()
	ax = fig.add_subplot(111, projection='3d')

	ax.scatter(rate,windows,totalTime)
	ax.set_xlabel('Average message rate (msg/interval)')
	ax.set_ylabel('Window size (intervals)')
	ax.set_zlabel('time (ms)')
	plt.show()
if __name__ == "__main__":
	main(argv)

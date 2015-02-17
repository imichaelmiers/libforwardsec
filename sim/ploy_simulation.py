import numpy as np
from sys import argv
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import sim
def main(argv):
	tex=False
	if tex:
		plt.rc('text', usetex=True)
		plt.rc('font', family='serif')
	rate = 1
	data = sim.sim_intervals(rate)
	plt.figure(figsize=(20,5))
	plt.subplot(131)
	plot(data,rate)
	rate = .1
	data = sim.sim_intervals(rate)
	plt.subplot(132)
	plot(data,rate)
	rate = .01
	data = sim.sim_intervals(rate)
	plt.subplot(133)
	lines = plot(data,rate)
	plt.gca().legend(loc='upper right',shadow=True)
	plt.gcf().suptitle("Performance vs Interval Size", fontsize=14)
	#plt.figlegend(lines,('Key Derivation','Decrypt','Puncture','total'),'upper right')
	plt.savefig('perf_v_interval_3fold.pdf',format='pdf',bbox_inches='tight')
	plt.show()
def plot(data,r):
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


	a=plt.plot(rate,dirTimes,'bv-',label='Key Derivation ')
	b=plt.plot(rate,decTime,'rs-',label='Decrypt ')
	c=plt.plot(rate,PunTime,'gp-',label='Puncture')
	d=plt.plot(rate,totalTime,'ko-',label='Total')
	plt.title('%s message/second'%r)
	plt.gca().set_xscale('log')
	plt.xlabel('interval size(s)')
	plt.ylabel('total time(s) ')
	return a,b,c,d
if __name__ == "__main__":
	main(argv)

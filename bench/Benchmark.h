#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <sys/time.h>
#include <string>
#include <sstream>
using namespace std;

typedef struct timeval timeval_t;

class Benchmark
{
public:
	Benchmark() { initBench = false; sum = 0.0; iterationCount = 0;k=1;M=0.0;S=0.0; };
	~Benchmark() { };
	void start();
	void stop();
	double computeTimeInMilliseconds();
	int getTimeInMicroseconds();
	double getStandardDeviation() const;
	string getAverageAndStddev() ;
	double getAverage() const;
private:
	timeval_t startT, endT;
	double sum;
	int iterationCount;
	bool initBench;
	double M = 0.0;
    double S = 0.0;
    int k =1;
    friend ostream& operator<<(ostream& os, const Benchmark& b){
    	os << "average " << b.getAverage() << "\t stdev " << b.getStandardDeviation();
    	return os;
    }
};

#endif

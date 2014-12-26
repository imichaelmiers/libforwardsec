#include <stdexcept>
#include <math.h>       /* sqrt */
#include "Benchmark.h"
int sec_in_microsecond = 1000000;
int ms_in_microsecond = 1000;

void Benchmark::start()
{
	initBench = true;
	gettimeofday(&startT, NULL);
}

void Benchmark::stop()
{
	gettimeofday(&endT, NULL);
}

int Benchmark::getTimeInMicroseconds()
{
	if(initBench) {
		return ((endT.tv_sec - startT.tv_sec) * sec_in_microsecond) + (endT.tv_usec - startT.tv_usec);
	}
		throw invalid_argument("Must call start first " );
	return -1.0;
}

double Benchmark::computeTimeInMilliseconds()
{
	if (initBench) {
		double microsec_result = (double) this->getTimeInMicroseconds();
		double rawResult = microsec_result / ms_in_microsecond;
		//ss << rawResult << ", ";
		sum += rawResult;
		iterationCount++;
		double value =  rawResult;
  		 double tmpM = M;
        M += (value - tmpM) / k;
        S += (value - tmpM) * (value - M);
        k++;
		return rawResult;
	}
	throw invalid_argument("Must call start first " );

	return -1.0; // didn't call start
}
double Benchmark::getStandardDeviation() const {
	if(k <3 || !initBench) throw invalid_argument("k too small or you did not call start.");
	return sqrt(S/(k-2));
}
string Benchmark::getAverageAndStddev(){
	stringstream sss;
	sss << "average " << getAverage() << " stdev " << getStandardDeviation() << endl;
	return sss.str();
}


double Benchmark::getAverage() const
{
	return sum / iterationCount;
}


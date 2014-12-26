#include <iostream>
#include <vector>
#include <tuple>
#include "gmpfse.h"
#include "Benchmark.h"
using namespace forwardsec;
using namespace std;
bytes testVector = {{0x3a, 0x5d, 0x7a, 0x42, 0x44, 0xd3, 0xd8, 0xaf, 0xf5, 0xf3, 0xf1, 0x87, 0x81, 0x82, 0xb2,
						  0x53, 0x57, 0x30, 0x59, 0x75, 0x8d, 0xe6, 0x18, 0x17, 0x14, 0xdf, 0xa5, 0xa4, 0x0b,0x43,0xAD,0xBC}};
std::vector<string>makeTags(unsigned int n){
	std::vector<string> tags(n);
	for(unsigned int i=0;i<n;i++){
		tags[i] = "tag"+std::to_string(i);
	}
	return tags;
}
Benchmark benchKeygen(const unsigned int iterations, const unsigned int d =32,
		const unsigned int n = 1){
	Benchmark b;
    for(unsigned int i=0;i < iterations;i++){
		Pfse test(d,n);
		b.start();
		test.keygen();
		b.stop();
		b.computeTimeInMilliseconds();
	}
	return b;
}
Benchmark benchEnc(const unsigned int iterations, const unsigned int d =32,
		const unsigned int n = 1){
	Pfse test(d,n);
    test.keygen();
    Benchmark benchE;
    for(unsigned int i=0;i < iterations;i++){
        benchE.start();
        test.encrypt(test.pk,testVector,1,makeTags(n));
        benchE.stop();
        benchE.computeTimeInMilliseconds();
    }
	   return benchE;
}
Benchmark benchDec(const unsigned int iterations, const unsigned int d =32,
		const unsigned int n = 1){
	Pfse test(d,n);
    test.keygen();
    Benchmark benchD;
    PseCipherText ct = test.encrypt(test.pk,testVector,1,makeTags(n));;
    for(unsigned int i=0;i < iterations;i++){
        benchD.start();
        test.decrypt(ct);
        benchD.stop();
        benchD.computeTimeInMilliseconds();
    }
	   return benchD;
}
Benchmark benchPuncFirst(const unsigned int iterations, const unsigned int d =32,
		const unsigned int n = 1){

    Benchmark benchP;
    for(unsigned int i=0;i < iterations;i++){
    	Pfse test(d,n);
        test.keygen();
        benchP.start();
        test.puncture("punc"+std::to_string(i));
        benchP.stop();
        benchP.computeTimeInMilliseconds();
    }
	   return benchP;
}

Benchmark benchPunc(const unsigned int iterations, const unsigned int d =32,
		const unsigned int n = 1){

    Benchmark benchP;
    for(unsigned int i=0;i < iterations;i++){
    	Pfse test(d,n);
        test.keygen();
        test.puncture("punc");
        benchP.start();
        test.puncture("punc"+std::to_string(i));
        benchP.stop();
        benchP.computeTimeInMilliseconds();
    }
	   return benchP;
}

Benchmark benchNextInterval(const unsigned int iterations, const unsigned int d =32,
		const unsigned int n = 1){

    Benchmark benchN;
    for(unsigned int i=0;i < iterations;i++){
    	Pfse test(d,n);
        test.keygen();
        benchN.start();
        test.prepareNextInterval();
        benchN.stop();
        benchN.computeTimeInMilliseconds();
    }
	   return benchN;

}
std::vector<std::tuple<unsigned int ,Benchmark>> benchDecPunctured(const unsigned int iterations,
		const unsigned int punctures, const unsigned int puncture_steps,
		const unsigned int d =32,
		const unsigned int n = 1){
	std::vector<std::tuple<unsigned int ,Benchmark>>  b;
	for(unsigned int p = 0; p < punctures;p+=puncture_steps){
        Benchmark benchDP;
    	Pfse test(d,n);
        test.keygen();
        PseCipherText ct = test.encrypt(test.pk,testVector,1,makeTags(n));;
        for(unsigned int i =0;i<p;i++){
            test.puncture("punc"+std::to_string(i));
        }
        for(unsigned int i=0;i < iterations;i++){
           benchDP.start();
		   test.decrypt(ct);
		   benchDP.stop();
		   benchDP.computeTimeInMilliseconds();
        }
        b.push_back(std::make_tuple (punctures,benchDP));
	}
	return b;
}
int main()
{
	relicResourceHandle h;
	unsigned int i = 10;
	unsigned int d = 32;
	unsigned int n = 1;
    Benchmark K,E,D,PF,PS,N,DP;
    cout << "Benchmarking " << i << " iterations of Puncturable forward secure encryption with depth " <<
    		d << " and " << n << " tags" << endl;
    K = benchKeygen(i,d,n);
    cout << "keygen:\t\t\t" << K << endl;

    E = benchEnc(i,d,n);
    cout << "Enc:\t\t\t" << E << endl;

    D = benchDec(i,d,n);
    cout << "Dec:\t\t\t" << D << endl;

    PF = benchPuncFirst(i,d,n);
    cout << "Initial Puncture:\t" << PF << endl;
    PS = benchPunc(i,d,n);
    cout << "Subsequent Puncture:\t" << PS << endl;

    N = benchNextInterval(i,d,n);
    cout << "NextInterval:\t\t" << N << endl;
}

#ifndef SRC_UTIL_H_
#define SRC_UTIL_H_
#include <vector>
#include"relic_wrapper/relic_api.h"

std::vector<ZR>  indexToPath(const unsigned int &index,const unsigned int & treeDepth);
unsigned int pathToIndex(const std::vector<ZR> & path, const unsigned int & treeDepth);


ZR LagrangeBasisCoefficients(const PairingGroup & group, const unsigned int & j,const ZR &x , const std::vector<ZR> & polynomial_xcordinates);

ZR LagrangeInterp(const PairingGroup & group, const ZR &x , const std::vector<ZR> & polynomial_xcordinates,
		const std::vector<ZR> & polynomial_ycordinates);

template <class type> type LagrangeInterpInExponent( const PairingGroup & group,const ZR &x, const std::vector<ZR> & polynomial_xcordinates,
		const std::vector<type> & exp_polynomial_ycordinates){
	assert(polynomial_xcordinates.size() ==exp_polynomial_ycordinates.size());
	type prod;
	unsigned int k = exp_polynomial_ycordinates.size();
	for(uint j = 0; j < k;j++){
			ZR lagrangeBasisPolyatX = LagrangeBasisCoefficients(group,j,x,polynomial_xcordinates);
			prod =  group.mul(prod,group.exp(exp_polynomial_ycordinates[j],lagrangeBasisPolyatX));
	}
	return prod;
}


#endif /* SRC_UTIL_H_ */

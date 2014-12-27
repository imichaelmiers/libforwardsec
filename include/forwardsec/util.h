#ifndef SRC_UTIL_H_
#define SRC_UTIL_H_
#include <vector>
#include <array>
#include"relic_wrapper/relic_api.h"
namespace forwardsec{
std::vector<relicxx::ZR>  indexToPath(const unsigned int &index,const unsigned int & treeDepth);
unsigned int pathToIndex(const std::vector<relicxx::ZR> & path, const unsigned int & treeDepth);

typedef  std::vector<uint8_t> bytes;
bytes xorarray(const bytes & l,const bytes & r);


relicxx::ZR LagrangeBasisCoefficients(const relicxx::PairingGroup & group, const unsigned int & j,const relicxx::ZR &x , const std::vector<relicxx::ZR> & polynomial_xcordinates);

relicxx::ZR LagrangeInterp(const relicxx::PairingGroup & group, const relicxx::ZR &x , const std::vector<relicxx::ZR> & polynomial_xcordinates,
		const std::vector<relicxx::ZR> & polynomial_ycordinates);

template <class type> type LagrangeInterpInExponent( const relicxx::PairingGroup & group,const relicxx::ZR &x, const std::vector<relicxx::ZR> & polynomial_xcordinates,
		const std::vector<type> & exp_polynomial_ycordinates){
	assert(polynomial_xcordinates.size() ==exp_polynomial_ycordinates.size());
	type prod;
	unsigned int k = exp_polynomial_ycordinates.size();
	for(uint j = 0; j < k;j++){
		relicxx::ZR lagrangeBasisPolyatX = LagrangeBasisCoefficients(group,j,x,polynomial_xcordinates);
			prod =  group.mul(prod,group.exp(exp_polynomial_ycordinates[j],lagrangeBasisPolyatX));
	}
	return prod;
}
}

#endif /* SRC_UTIL_H_ */

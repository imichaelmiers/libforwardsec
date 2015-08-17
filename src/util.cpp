#include<assert.h>
#include"util.h"
#include <cmath>
using namespace std;
namespace forwardsec{
using namespace relicxx;

bytes xorarray(const bytes & l,const bytes & r){
	if(l.size()!=r.size()){
		throw invalid_argument("Arrays must be be same size. Instead  l:" +std::to_string(l.size()) +  " r:" +std::to_string(r.size()));
	}
	bytes result(l.size());
	for(unsigned int i=0;i<l.size();i++){
		result.at(i) = l.at(i) ^ r.at(i);
	}
	return result;
}

unsigned int treeSize(unsigned int k){
    if(k>31){
        throw invalid_argument ("tree depth must be less than 32, not " + std::to_string(k));
    }else if(k==31){
         return 4294967295; // 2^32 -1 = maxint.
    }
    return pow(2.0,k+1)-1;
}

std::vector<ZR>  indexToPath(const unsigned int &index,const unsigned int & treeDepth){
    std::vector<ZR> path;
    unsigned int nodesSoFar = 0;
    unsigned int level;
    if(index >= treeSize(treeDepth)){
        throw invalid_argument ("index out of bounds of tree: tree supports at most " + std::to_string(treeSize(treeDepth)) +" -1 nodes, not " + std::to_string(index));
    }
    for(level =0 ; level < treeDepth ; level++){
        unsigned int subtree_height = treeSize(treeDepth-level -1);
        if (nodesSoFar == index){
            return path;
        }else if(index <= (subtree_height + nodesSoFar)){
            path.push_back(ZR(0));
            nodesSoFar++;
        }else{
            path.push_back(ZR(1));
            nodesSoFar += (subtree_height +1);
        }

    }

    return path;
}

unsigned int pathToIndex(const std::vector<ZR> & path, const unsigned int & treeDepth){
    unsigned int index = 0;
    unsigned int pathsize = path.size();
    if(pathsize > treeDepth){
        throw invalid_argument("path too long for tree depth");
    }
    for(unsigned int level =0 ; level < pathsize ; level++){
        if (path.at(level) == 0){
            index ++;
        }else if(path.at(level) == 1){

          unsigned int left_subtree_level = level + 1;
          unsigned int left_subtree_height = treeDepth - left_subtree_level;
          unsigned int left_subtree_size = treeSize(left_subtree_height);

          index += left_subtree_size;

          index++;
        }
    }
    return index;
}

ZR LagrangeBasisCoefficients(const PairingGroup & group, const unsigned int & j,const ZR &x , const vector<ZR> & polynomial_xcordinates){
    unsigned int k = polynomial_xcordinates.size();
    ZR prod = 1;
    for(unsigned int  m=0;m<k;m++){
        if(j != m){
			try{
			ZR interim = group.div(group.sub(x,polynomial_xcordinates.at(m)),group.sub(polynomial_xcordinates.at(j),polynomial_xcordinates.at(m)));
			prod = group.mul(prod,interim);
			}catch(const RelicDividByZero & t){
				throw logic_error("LagrangeBasisCoefficient calculation failed. RelicDividByZero"
						" Almost certainly a duplicate x-coordinate: ");// FIXME give cordinate
			}
        }
    }
    return prod;
}

ZR LagrangeInterp(const PairingGroup & group, const ZR &x , const vector<ZR> & polynomial_xcordinates,
		const vector<ZR> & polynomial_ycordinates){
    unsigned int k =polynomial_ycordinates.size();
    assert(polynomial_xcordinates.size()==polynomial_ycordinates.size());
    ZR prod = 0;
    for(unsigned int j = 0; j < k;j++){
            ZR lagrangeBasisPolyatX = LagrangeBasisCoefficients(group,j,x,polynomial_xcordinates);
         //   cout << "y_ " << j << "= "<<polynomial_ycordinates[j] << " coef = " << lagrangeBasisPolyatX << " prod = " << prod<< endl;
            prod =  group.add(prod,group.mul(lagrangeBasisPolyatX,polynomial_ycordinates.at(j)));

    }

    // cout << "final prod =" << prod << endl;
    return prod;
}
}

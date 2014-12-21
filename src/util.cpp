#include<assert.h>
#include"util.h"

unsigned int treeSize(unsigned int k){
    return (2 <<(k)) -1;
}

std::vector<ZR>  indexToPath(const unsigned int &index,const unsigned int & treeDepth){
    std::vector<ZR> path;
    unsigned int nodesSoFar = 0;
    ZR zero = 0;
    for(unsigned int level =0 ; level < treeDepth ; level++){
        unsigned int subtree_height = treeSize(treeDepth-level-1);
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
    if(nodesSoFar < index){
        throw invalid_argument ("index out of bounds of tree");
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
        if (path[level] == 0){
            index ++;
        }else if(path[level] == 1){

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
			ZR interim = group.div(group.sub(x,polynomial_xcordinates[m]),group.sub(polynomial_xcordinates[j],polynomial_xcordinates[m]));
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
		const vector<ZR> & polynomial_ycordinates, const  unsigned int& degree){
    unsigned int k = degree + 1;
    assert(polynomial_ycordinates.size()==k);
    assert(polynomial_xcordinates.size()==k);
    ZR prod = 0;
    for(unsigned int j = 0; j < k;j++){
            ZR lagrangeBasisPolyatX = LagrangeBasisCoefficients(group,j,x,polynomial_xcordinates);
         //   cout << "y_ " << j << "= "<<polynomial_ycordinates[j] << " coef = " << lagrangeBasisPolyatX << " prod = " << prod<< endl;
            prod =  group.add(prod,group.mul(lagrangeBasisPolyatX,polynomial_ycordinates[j]));

    }

    // cout << "final prod =" << prod << endl;
    return prod;
}

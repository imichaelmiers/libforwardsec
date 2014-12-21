#include <iostream>
#include <sstream>
#include <string>
#include <list>
#include <vector>
#include <assert.h>
#include <stdexcept>
#include <bitset>
#include "gmpfse.h"
using namespace std;
#define splitkey 0
uint d = 1;
#define NULLTAG 42
#ifdef DDDEBUG
#define DBGG(x) x
#else
#define DBGG(x)
#endif
PfsePuncturedPrivateKey PfseKeyStore::getKey(unsigned int i)  const{
	PfsePuncturedPrivateKey p;
	auto x = puncturedKeys.find(i);
	if(x == puncturedKeys.end()){
		auto y = unpucturedHIBEKeys.find(i);

		if(y == unpucturedHIBEKeys.end() ){
			DBGG(cout << "did not find   keys for interval " << i << endl;)

  			  throw invalid_argument("No key for this interval: " + std::to_string(i));
		}
		DBGG(cout << "found key in unpunctured keys for interval " << i << endl;)
		p.hibeSK = y->second;
		p.ppkeSK = unpucturedPPKEKey;
	}else{
		DBGG(cout << "found key in punctured keys for interval " << i << endl;)
		p = x->second;
	}
	return p;
}

void PfseKeyStore::updateKey(unsigned int i, const PfsePuncturedPrivateKey & p){
//	if(p.ppkeSK == unpucturedPPKEKey){
//		throw invalid_argument("Key not punctured");
//	}
	puncturedKeys[i] = p;
	unpucturedHIBEKeys.erase(i);
}
void PfseKeyStore::erase(unsigned int i){
	DBGG(cout << "Erasing key for inteval " << i << endl;)

	puncturedKeys.erase(i); //FIXME secure erase.
	unpucturedHIBEKeys.erase(i);
}
bool PfseKeyStore::hasKey(const unsigned int i) const{
	return puncturedKeys.count(i) || unpucturedHIBEKeys.count(i);
}


void PfseKeyStore::addkey(unsigned int i, const BbghPrivatekey & h){
	DBGG(cout << "added key for interval " << i << endl;);
	unpucturedHIBEKeys[i] = h;
}

bool PfseKeyStore::needsChildKeys(const unsigned int i, const unsigned int d) const{
	vector<ZR> path = indexToPath(i,d);
	if(path.size()==d){ // don't need keys if it's a leaf node.
		return false;
	}
	vector<ZR> lpath(path);
	vector<ZR> rpath(path);
	lpath.push_back(ZR(0));
	rpath.push_back(ZR(1));
	return !(hasKey(pathToIndex(lpath,d)) && hasKey(pathToIndex(rpath,d)));
}

Pfse::Pfse(uint d):hibe(),ppke(),depth(d){
	this->nextParentInterval = 1 ;
    // group.setCurve(BN256);
    // cout << "depth" << depth << endl;
}


void Pfse::keygen(){
    G2 msk;
    hibe.setup(depth,this->pk,msk);


    std::vector<ZR> left, right;
    BbghPrivatekey sklefthibe,  skrighthibe;
    GmppkePrivateKey ppkeSK;
    left.push_back(ZR(0));
    right.push_back(ZR(1));

    hibe.keygen(this->pk,msk,left,sklefthibe);
    hibe.keygen(this->pk,msk,right,skrighthibe);
    int l = pathToIndex(left,depth);
    int r = pathToIndex(right,depth);
    ZR gamma = group.random(ZR_t);
    ZR gamma1 = group.random(ZR_t); // XXX FIXME . this should break. we are randomizing the left key w/ the wrong alpha
    assert(!(gamma1 == gamma));
    sklefthibe.a0 = group.mul(sklefthibe.a0,group.exp(this->pk.g2G2,group.neg(gamma)));
    skrighthibe.a0 = group.mul(skrighthibe.a0,group.exp(this->pk.g2G2,group.neg(gamma1)));

    ppke.keygen(pk,gamma,pk,ppkeSK);


    this->privatekeys.unpucturedPPKEKey = ppkeSK;
    this->privatekeys.addkey(l,sklefthibe);
    this->privatekeys.addkey(r,skrighthibe);
    nextParentInterval = 1;
    this->prepareNextInterval();
}

void Pfse::prepareNextInterval(){

    std::vector<ZR> path = indexToPath(nextParentInterval,depth);
    uint pathlength = path.size();
    const PfsePuncturedPrivateKey & k = privatekeys.getKey(nextParentInterval); //FIXME refrence could be deleted
    if(k.punctured()){
         throw logic_error("The parent tag is already punctured. You must call prepareNextInterval before starting");
    }

    if (pathlength  < depth){ // Not a leaf node, so derive new hibe keys.
        HIBEkey sklefthibe, skrighthibe;

        // compute left key
        path.push_back(ZR(0));
        int leftChildIndex = pathToIndex(path,depth);
        hibe.keygen(pk,k.hibeSK,path,sklefthibe);

        // compute right key;
        path[pathlength]=ZR(1);
        int rightChildIndex = pathToIndex(path,depth);
        hibe.keygen(pk,k.hibeSK,path,skrighthibe);

        //store keys
        privatekeys.addkey(leftChildIndex,sklefthibe);
        privatekeys.addkey(rightChildIndex,skrighthibe);
    }
   	nextParentInterval ++;
}

void Pfse::bindKey(PfsePuncturedPrivateKey & k) {
    const ZR gamma = group.random(ZR_t);
    GmppkePrivateKey puncturedKey;

	k.hibeSK.a0 = group.mul(k.hibeSK.a0,group.exp(pk.g2G2,group.neg(gamma)));

	GmppkePrivateKeyShare boundShare = ppke.skgen(pk,gamma);
	const GmppkePrivateKeyShare & oldShare = k.ppkeSK.shares[0];
	boundShare.sk1 = group.mul(boundShare.sk1,oldShare.sk1);
	boundShare.sk2 = group.mul(boundShare.sk2,oldShare.sk2);
	boundShare.sk3 = group.mul(boundShare.sk3,oldShare.sk3);

	puncturedKey.shares.push_back(boundShare);

	k.ppkeSK = puncturedKey;
}


void Pfse::eraseKey(unsigned int interval) {
	if(privatekeys.needsChildKeys(interval,depth)){
		throw invalid_argument("Cannot delete key for interval "+std::to_string(interval)+
				" , haven't derived keys yet.");
	}else{
		privatekeys.erase(interval);
	}
}

void Pfse::puncture(string tag){
	// The current active interval is one behind the nextParentInterval
    puncture(nextParentInterval-1,tag);
}

void Pfse::puncture(uint interval, string tag){
	if(privatekeys.needsChildKeys(interval,depth)){
		throw invalid_argument("Cannot puncture key for  interval "+std::to_string(interval)+
				" , haven't derived keys yet. Last interval with keys is " +std::to_string(nextParentInterval-1));
	}

    PfsePuncturedPrivateKey k = privatekeys.getKey(interval);

    //if the key is unpunctured, we need to bind in a new punctured key
    if(!k.punctured()){
    	DBGG(cout << interval << "not already punctured" << endl;)
		bindKey(k);
    }
    ppke.puncture(pk,k.ppkeSK,group.hashListToZR(tag));
	privatekeys.updateKey(interval,k);

//privatekeys[interval] = sk;

}
PseCipherText Pfse::encrypt(const pfsepubkey & pk, const AESKey aes_key, const uint interval,const vector<string> tags) const {
    vector<ZR> tagsZR;

    for(uint i=0;i<tags.size();i++){

        ZR tag =  group.hashListToZR(tags[i]);
        tagsZR.push_back(tag);
    }
    return encryptFO(pk,aes_key,interval,tagsZR);
}
PseCipherText Pfse::encryptFO(const pfsepubkey & pk,const AESKey  & aes_key
		, const uint interval, const vector<ZR>  & tags ) const {
    GT x = group.random(GT_t);
    return encryptFO(pk,aes_key,x,interval,tags);

}
PseCipherText Pfse::encryptFO(const pfsepubkey & pk,  const AESKey  & aes_key,const  GT & x,
		const uint interval, const vector<ZR>  & tags ) const {
    std::stringstream ss; //FIXME the << operator returns "BROKEN"
    ss << x;
    ss << aes_key;
    ZR s = group.hashListToZR(ss.str());
   
    PseCipherText ct = encrypt(pk,x,s,interval,tags);

    // since we don't have a different hash function, we simply prefix it
    std::stringstream sss;
    sss << "0xDEADBEEF";
    sss << x;
    ZR xorash = group.hashListToZR(sss.str().c_str());
    AESKey bits = intToBits(xorash);

    ct.xorct = aes_key ^ bits;
    return ct;

}
PseCipherText Pfse::encrypt(const pfsepubkey & pk,const  GT & M, const uint interval, const vector<ZR>  & tags) const{
        ZR s = group.random(ZR_t);
        return Pfse::encrypt(pk,M,s,interval,tags);
}

PseCipherText Pfse::encrypt(const pfsepubkey & pk, const GT & M,  const ZR & s, const uint interval, const vector<ZR>  & tags) const{
    PseCipherText ct;
    
    ct.interval = interval;

    std::vector<ZR> id= indexToPath(interval,depth);

    ct.hibeCT = hibe.encrypt(pk,M,s,id);
    ct.ppkeCT =  ppke.encrypt(pk,M,s,tags);
    ct.ct0 =  group.mul(group.exp(group.pair(pk.g2G1, pk.hibeg1), s), M);

    return ct;
}
AESKey Pfse::decrypt(const PseCipherText &ct) const{
    const PfsePuncturedPrivateKey & sk = privatekeys.getKey(ct.interval);

    return decryptFO(sk,ct);
}

AESKey Pfse::decryptFO(const PfsePuncturedPrivateKey & sk,const PseCipherText &ct) const{
    GT x = decryptGT(sk,ct);

    // since we don't have a different hash function, we simply prefix it
    std::stringstream sss;
    sss << "0xDEADBEEF";
    sss << x;
    ZR xorash = group.hashListToZR(sss.str());

    AESKey bits = intToBits(xorash);
    AESKey aes_key = ct.xorct ^ bits;
    PseCipherText cttest = encryptFO(pk,aes_key,x,ct.interval,ct.ppkeCT.tags);
    if(ct == cttest ){
        return aes_key;
    }else{
      throw BadCiphertext("Fujisaki Okamoto integrety check failed ");
    }
}



GT Pfse::decryptGT(const PfsePuncturedPrivateKey & sk,const PseCipherText &ct) const {
    GT b1 = hibe.decrypt(sk.hibeSK,ct.hibeCT);
   // assert(b1== group.exp(group.exp(group.pair(g2G1,gG2),group.mul(ss,group.sub(aa,gam1))),neg));
    GT b2 = ppke.decrypt(pk,sk.ppkeSK,ct.ppkeCT);
   // assert(b2 ==group.exp(group.pair(g2G1,gG2),group.mul(ss,gam1)));
    return group.div(group.mul(ct.ct0,b1),b2);
}

uint treeSize(uint k){
    return (2 <<(k)) -1;
}


ZR Gmppke::LagrangeBasisCoefficients(uint j,const ZR &x , const vector<ZR> & polynomial_xcordinates) const{
    uint k = polynomial_xcordinates.size();
    assert(k==d+1);
    ZR prod = 1;
    for(uint  m=0;m<k;m++){
        if(j != m){
        // cout << "<<<<<<<<<<<<<<<<<<<<<<<" << endl;
        // cout  << "j:" << j << " m:" << m << endl;
        // cout << "(" << x << "-" <<  polynomial_xcordinates[m] << ")/" << endl;
        // cout << "(" << polynomial_xcordinates[j] << "-" << polynomial_xcordinates[m] << ")" << endl;
        ZR interim = group.div(group.sub(x,polynomial_xcordinates[m]),group.sub(polynomial_xcordinates[j],polynomial_xcordinates[m]));
     //   cout << "interim " << interim << endl;
        prod = group.mul(prod,interim);
        // cout << "prod = " << prod << endl;
        // cout << ">>>>>>>>>>>>>>>>>>>>>>>\n\n" << endl;
        }
    }
    return prod;
}
ZR Gmppke::LagrangeInterp(const ZR &x , const vector<ZR> & polynomial_xcordinates,
    const vector<ZR> & polynomial_ycordinates, uint degree) const{
    uint k = degree + 1;
    assert(k == d+1);
    assert(polynomial_ycordinates.size()==k);
    assert(polynomial_xcordinates.size()==k);
    ZR prod = 0;
    for(uint j = 0; j < k;j++){
            ZR lagrangeBasisPolyatX = LagrangeBasisCoefficients(j,x,polynomial_xcordinates);
         //   cout << "y_ " << j << "= "<<polynomial_ycordinates[j] << " coef = " << lagrangeBasisPolyatX << " prod = " << prod<< endl;
            prod =  group.add(prod,group.mul(lagrangeBasisPolyatX,polynomial_ycordinates[j]));

    }

    // cout << "final prod =" << prod << endl;
    return prod;
}

template <class type> type Gmppke::LagrangeInterpInExponent(const ZR &x , const vector<ZR> & polynomial_xcordinates,
    const vector<type> & exp_polynomial_ycordinates,const  uint degree) const{
    uint k = degree + 1;
    assert(k == d+1);
    assert(exp_polynomial_ycordinates.size()==k);
    assert(polynomial_xcordinates.size()==k);
    type prod;
    for(uint j = 0; j < k;j++){
            ZR lagrangeBasisPolyatX = LagrangeBasisCoefficients(j,x,polynomial_xcordinates);
            prod =  group.mul(prod,group.exp(exp_polynomial_ycordinates[j],lagrangeBasisPolyatX));
    }
    return prod;
}

G1  Gmppke::vG1(const std::vector<G1> & gqofxG1, const ZR & x) const{
    vector<ZR> xcords;
    //     cout << "\n\n XXXX" << endl;

    // cout << "length of gqofx in VG1 " << gqofxG1.length() << endl;
    int size = gqofxG1.size() ;
    for(int i=0;i<size;i++){
        ZR xcord = i;
        xcords.push_back(xcord);
    }
        
    //     cout << "length of gqofx in VG1 " << gqofxG1.length() << endl;
    // cout << "length of gqofxv in VG1 " << gqofxv.size() << endl;
    //     cout << "XXXX\n\n" << endl;

    return LagrangeInterpInExponent(x,xcords,gqofxG1,d);

}
G2 Gmppke::vG2(const std::vector<G2> & gqofxG2,const ZR & x) const{
    vector<ZR> xcords;
        // cout << "\n\n XXXX" << endl;

    // cout << "length of gqofx in VG2 " << gqofxG2.length() << endl;
        int size = gqofxG2.size() ;

    for(int i=0; i <size; i++){
   //     cout <<  "step " << i << endl;

        ZR xcord = i;
        xcords.push_back(xcord);

    }
    //     cout << "length of gqofx in VG2 " << gqofxG2.length() << endl;
    // cout << "length of gqofxv in VG2 " << gqofxv.size() << endl;
    // cout << "XXXX\n\n" << endl;

    return LagrangeInterpInExponent(x,xcords,gqofxG2,d);

}
void Gmppke::keygen(const BbhHIBEPublicKey & pkhibe,ZR & gamma, GmppkePublicKey & pk, GmppkePrivateKey & sk) const
{
    pk.d = d;


    pk.gG1 = pkhibe.gG1;
    pk.gG2 = pkhibe.gG2;
    pk.ppkeg1 =  group.exp(pk.gG2,gamma);
    pk.g2G1 = pkhibe.g2G1;
    pk.g2G2 = pkhibe.g2G2;

    // Select a random polynomial of degree d subject to q(0)= beta. We do this
    // by selecting d+1 points. Because we don't actually  care about the 
    // polynomial, only g^q(x), we merely select points as (x,g^q(x)).

    vector<ZR> polynomial_xcordinates;

    // the first point is (x=0,y=beta) so  x=0, g^beta.
    polynomial_xcordinates.push_back(ZR(0));
    pk.gqofxG1.push_back(pk.g2G1); // g^beta
    pk.gqofxG2.push_back(pk.g2G2); // g^beta

    // the next d points' y values  are random
    // we use x= 1...d because this has the side effect
    // of easily computing g^q(0).... g^q(d).
    for (uint i = 1; i <= d; i++)
    {
        const ZR ry = group.random(ZR_t);

        polynomial_xcordinates.push_back(ZR(i));
        pk.gqofxG1.push_back(group.exp(pk.gG1,ry));
        pk.gqofxG2.push_back(group.exp(pk.gG2,ry));

    }
    assert(polynomial_xcordinates.size()==pk.gqofxG1.size());

    // Sanity check that Lagrange interpolation works to get us g^beta on q(0).
    assert(pk.g2G1 == LagrangeInterpInExponent<G1>(0,polynomial_xcordinates,pk.gqofxG1,d));
    assert(pk.g2G2 == LagrangeInterpInExponent<G2>(0,polynomial_xcordinates,pk.gqofxG2,d));


    sk.shares.push_back(skgen(pk,gamma));

    return;
}
GmppkePrivateKeyShare Gmppke::skgen(const GmppkePublicKey &pk,const ZR & alpha  ) const{
	GmppkePrivateKeyShare share;
    share.sk4 = ZR(NULLTAG);
    const ZR r = group.random(ZR_t);
    share.sk1 = group.exp(pk.g2G2, group.add(r,alpha));
    G2 vofx = vG2(pk.gqofxG2,share.sk4); // calculate v(t0).
    share.sk2 = group.exp(vofx, r);// v(t0)^r
    share.sk3 = group.exp(pk.gG2, r);
    return share;
}

void Gmppke::puncture(const GmppkePublicKey & pk, GmppkePrivateKey & sk, const ZR & tag) const{

    GmppkePrivateKeyShare skentryn;
    GmppkePrivateKeyShare & skentry0 = sk.shares[0];

    const ZR r0 = group.random(ZR_t);
    const ZR r1 = group.random(ZR_t);
    const ZR lambda = group.random(ZR_t);

    assert(skentry0.sk4 == ZR(NULLTAG));

    skentry0.sk1 = group.mul(skentry0.sk1,group.exp(pk.g2G2,group.sub(r0,lambda))); // sk1 * g2g2^{r0- lambda}
    const G2 vofx = vG2(pk.gqofxG2,skentry0.sk4);
    skentry0.sk2 = group.mul(skentry0.sk2,group.exp(vofx,r0));  // sk2 * V(t0)^r0
    skentry0.sk3 = group.mul(skentry0.sk3,group.exp(pk.gG2,r0));  // sk3 * g2G2^r0

    skentryn.sk1=group.exp(pk.g2G2,group.add(r1,lambda));  // gG2 ^ (r1+lambda)
    const G2 vofx2 = vG2(pk.gqofxG2,tag);
    skentryn.sk2 = group.exp(vofx2,r1); // V(tag) ^ r1
    skentryn.sk3 = group.exp(pk.gG2,r1);  // G^ r1
    skentryn.sk4 = tag;

    sk.shares.push_back(skentryn);
}
GmmppkeCT Gmppke::encrypt(const GmppkePublicKey & pk, const GT & M, const ZR & s, const std::vector<ZR> & tags ) const
{
    assert(tags.size()==d);
    GmmppkeCT  ct;
    ct.ct1 = group.mul(group.exp(group.pair(pk.g2G1, pk.ppkeg1), s), M);
    ct.ct2 = group.exp(pk.gG1, s);

    for (uint i = 0; i < pk.d; i++)
    {
        G1 vofx = vG1(pk.gqofxG1,tags[i]);
        ct.ct3.push_back(group.exp(vofx, s));
    }
    ct.tags = tags;
    return ct;
}

GT Gmppke::decrypt(const GmppkePublicKey & pk, const GmppkePrivateKey & sk, const GmmppkeCT & ct) const
{
    assert(ct.tags.size()==d);
    assert(d==pk.d);

    vector<ZR> shareTags(ct.tags.size()+1);// allow one more tag for share the private key holds
    for(uint i =0; i < ct.tags.size();i++){
            shareTags[i]=ct.tags[i];
    }
    assert(shareTags.size() == pk.d+1);

    // Compute w_i coefficients for recovery
    // FIXME check that points are unique.

    const uint numshares = sk.shares.size();

    vector<GT> z(numshares);

    for (uint i = 0; i < numshares; i++)
    {
        const GmppkePrivateKeyShare & s0 = sk.shares[i];

        // FIXME DO NOT COPY an entire  vector  if possible.

        shareTags[shareTags.size()-1] = s0.sk4; 

        vector<ZR> w;

        for(uint j=0;j < shareTags.size(); j++){
            w.push_back(LagrangeBasisCoefficients(j,0,shareTags));
        }
        const ZR wstar = w[w.size() - 1];

        G1 ct3prod_j;
        for (uint j = 0; j < d; j++)
        {
            ct3prod_j = group.mul(ct3prod_j, group.exp(ct.ct3[j],w[j])); // w[0] = wstar

        }
       GT denominator = group.mul(group.pair(ct3prod_j, s0.sk3), group.pair(group.exp(ct.ct2,wstar), s0.sk2));
       GT nominator = group.pair(ct.ct2, s0.sk1);
       z[i]=group.div(nominator, denominator);
    }

    GT zprod;
    for (uint i = 0; i < numshares; i++)
    {
        zprod = group.mul(zprod, z[i]);
    }
    return zprod;
}



std::vector<ZR>  indexToPath(uint index,uint l){
    std::vector<ZR> path;
    uint nodesSoFar = 0;
    ZR zero = 0;
    for(uint level =0 ; level < l ; level++){
        uint subtree_height = treeSize(l-level-1);
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

uint pathToIndex(std::vector<ZR> & path, uint l){
    uint index = 0;
    uint pathsize = path.size();
    if(pathsize > l){
        throw invalid_argument("path too long for tree depth");
    }
    for(uint level =0 ; level < pathsize ; level++){
        if (path[level] == 0){
            index ++;
        }else if(path[level] == 1){

          uint left_subtree_level = level + 1;
          uint left_subtree_height = l - left_subtree_level;
          uint left_subtree_size = treeSize(left_subtree_height);

          index += left_subtree_size;

          index++;
        }
    }
    return index;
}



void Bbghibe::setup(const unsigned int & l, BbhHIBEPublicKey & pk, G2 & msk) const
{
    ZR alpha = group.random(ZR_t);
    pk.gG1 = group.random(G1_t);
    pk.gG2 = group.random(G2_t);
    pk.hibeg1 = group.exp(pk.gG2, alpha);
    pk.l = l;
    const ZR r = group.random(ZR_t);
    pk.g2G1 = group.exp(pk.gG1, r);
    pk.g2G2 = group.exp(pk.gG2, r);
    const ZR r1 = group.random(ZR_t);
    pk.g3G1 = group.exp(pk.gG1, r1);
    pk.g3G2 = group.exp(pk.gG2, r1);
    for (unsigned int i = 0; i < l; i++)
    {
        ZR h = group.random(ZR_t);
        pk.hG1.push_back(group.exp(pk.gG1, h));
        pk.hG2.push_back(group.exp(pk.gG2, h));
    }
    msk = group.exp(pk.g2G2, alpha);

    return;
}

void Bbghibe::keygen(const BbhHIBEPublicKey & pk, const G2 & msk, const std::vector<ZR> & id, BbghPrivatekey & sk) const
{
    const ZR r = group.random(ZR_t);
    const unsigned int k = id.size();
    for (unsigned int i = 0; i < k; i++)
    {
        sk.a0 = group.mul(sk.a0, group.exp(pk.hG2[i], id[i]));
    }
    sk.a0 = group.exp(group.mul(sk.a0, pk.g3G2), r);
    sk.a0 = group.mul(msk, sk.a0);
    sk.a1 = group.exp(pk.gG2, r);

    sk.b.resize(pk.l);
    sk.bG2.resize(pk.l);
    for (unsigned int i =  k;i < pk.l; i++)
    {
        sk.b[i] = group.exp(pk.hG1[i], r);

        sk.bG2[i] = group.exp(pk.hG2[i], r);
    }
    return;
}

void Bbghibe::keygen(const BbhHIBEPublicKey & pk,const  BbghPrivatekey & sk, const std::vector<ZR> &id,BbghPrivatekey & skout) const{
    const unsigned int k = id.size();
    ZR t = group.random(ZR_t);

    G2 hprod;
    for (unsigned int i = 0; i < k ; i++)
    {
        hprod = group.mul(hprod, group.exp(pk.hG2[i], id[i]));
    }
    hprod = group.exp(group.mul(hprod, pk.g3G2), t);
    hprod = group.mul(hprod,group.exp(sk.bG2[k-1],(id[k-1])));
    skout.a0 = group.mul(hprod,sk.a0);
    skout.a1 = group.mul(sk.a1,group.exp(pk.gG2,t));
    skout.b.resize(pk.l);
    skout.bG2.resize(pk.l);
    assert(skout.b.size() == sk.b.size());
    assert(skout.bG2.size() == sk.bG2.size());

    for (unsigned int i = k ; i < pk.l; i++)
    {
        skout.b[i] = group.mul(sk.b[i],group.exp(pk.hG1[i],t));
        skout.bG2[i] = group.mul(sk.bG2[i],group.exp(pk.hG2[i],t));

    }
    return;
}

BbghCT Bbghibe::encrypt(const BbhHIBEPublicKey & pk, const GT & M, const std::vector<ZR> & id) const{
    ZR r = group.random(ZR_t);
    return encrypt(pk,M,r,id);
}

BbghCT Bbghibe::encrypt(const BbhHIBEPublicKey & pk,const GT & M, const ZR & s,const std::vector<ZR> & id) const
{
	BbghCT ct;
    const unsigned int k = id.size();
    assert(k<=pk.l);

    ct.A = group.mul(group.exp(group.pair(pk.g2G1, pk.hibeg1), s), M);
    ct.B = group.exp(pk.gG1, s);

    G1 dotProd2;
    for (unsigned int i = 0; i < k; i++)
    {
        dotProd2 = group.mul(dotProd2,group.exp(pk.hG1[i], id[i]));
    }
    ct.C = group.exp(group.mul(dotProd2, pk.g3G1), s);
    return ct;
}

GT Bbghibe::decrypt(const BbghPrivatekey & sk, const BbghCT & ct) const{
    return group.div(group.pair(ct.C, sk.a1), group.pair(ct.B, sk.a0));
}

GT Bbghibe::decrypt_(const BbghPrivatekey & sk, const BbghCT & ct) const
{

    return group.mul(ct.A, group.div(group.pair(ct.C, sk.a1), group.pair(ct.B, sk.a0)));
}

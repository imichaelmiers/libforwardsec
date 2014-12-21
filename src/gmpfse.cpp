#include <iostream>
#include <sstream>
#include <string>
#include <list>
#include <vector>
#include <assert.h>
#include <stdexcept>
#include <bitset>
#include <set>

#include "gmpfse.h"
#include "util.h"
using namespace std;
#define splitkey 0
unsigned int d = 1;
#define NULLTAG 42
#ifdef DDDEBUG
#define DBGG(x) x
#else
#define DBGG(x)
#endif

/** Checks if you can decrypt a pfse ciphertext
 *
 * @param sk
 * @param ct
 * @return
 */
bool canDecrypt(const GmppkePrivateKey & sk,const PartialGmmppkeCT & ct){ // FIXME

	if(sk.shares.size()< ct.tags.size()){
		std::set<ZR> tags;
		for(auto t : sk.shares){
			tags.insert(t.sk4);
		}
		for(auto t: ct.tags){
			if(tags.count(t)>0){
				return false;
			}
		}
	}else{
		std::set<ZR> tags(ct.tags.begin(),ct.tags.end());
		for(auto t : sk.shares){
			if(tags.count(t.sk4)>0){
		    	 return false;
			}
		}
    }
    return true;
}
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

Pfse::Pfse(unsigned int d):hibe(),ppke(),depth(d){
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
    unsigned int pathlength = path.size();
    const PfsePuncturedPrivateKey & k = privatekeys.getKey(nextParentInterval); //FIXME refrence could be deleted
    if(k.punctured()){
         throw logic_error("The parent tag is already punctured. You must call prepareNextInterval before starting");
    }

    if (pathlength  < depth){ // Not a leaf node, so derive new hibe keys.
    	BbghPrivatekey sklefthibe, skrighthibe;

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

void Pfse::puncture(unsigned int interval, string tag){
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
PseCipherText Pfse::encrypt(const pfsepubkey & pk, const AESKey aes_key, const unsigned int interval,const vector<string> tags) const {
    vector<ZR> tagsZR;

    for(unsigned int i=0;i<tags.size();i++){

        ZR tag =  group.hashListToZR(tags[i]);
        tagsZR.push_back(tag);
    }
    return encryptFO(pk,aes_key,interval,tagsZR);
}
PseCipherText Pfse::encryptFO(const pfsepubkey & pk,const AESKey  & aes_key
		, const unsigned int interval, const vector<ZR>  & tags ) const {
    GT x = group.random(GT_t);
    return encryptFO(pk,aes_key,x,interval,tags);

}
PseCipherText Pfse::encryptFO(const pfsepubkey & pk,  const AESKey  & aes_key,const  GT & x,
		const unsigned int interval, const vector<ZR>  & tags ) const {
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
PseCipherText Pfse::encrypt(const pfsepubkey & pk,const  GT & M, const unsigned int interval, const vector<ZR>  & tags) const{
        ZR s = group.random(ZR_t);
        return Pfse::encrypt(pk,M,s,interval,tags);
}

PseCipherText Pfse::encrypt(const pfsepubkey & pk, const GT & M,  const ZR & s, const unsigned int interval, const vector<ZR>  & tags) const{
    PseCipherText ct;
    
    ct.interval = interval;

    std::vector<ZR> id= indexToPath(interval,depth);

    ct.hibeCT = hibe.blind(pk,M,s,id);
    ct.ppkeCT =  ppke.blind(pk,M,s,tags);
    ct.ct0 =  group.mul(group.exp(group.pair(pk.g2G1, pk.hibeg1), s), M);

    return ct;
}
AESKey Pfse::decrypt(const PseCipherText &ct) const{
    const PfsePuncturedPrivateKey & sk = privatekeys.getKey(ct.interval);
    if(!canDecrypt(sk.ppkeSK,ct.ppkeCT)){
    	throw PuncturedCiphertext("cannot decrypt. Duplicate tags");
    }
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
    GT b1 = hibe.recoverBlind(sk.hibeSK,ct.hibeCT);
   // assert(b1== group.exp(group.exp(group.pair(g2G1,gG2),group.mul(ss,group.sub(aa,gam1))),neg));
    GT b2 = ppke.recoverBlind(pk,sk.ppkeSK,ct.ppkeCT);
   // assert(b2 ==group.exp(group.pair(g2G1,gG2),group.mul(ss,gam1)));
    return group.div(ct.ct0,group.mul(b1,b2));
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

    return LagrangeInterpInExponent(group,x,xcords,gqofxG1,d);

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

    return LagrangeInterpInExponent(group,x,xcords,gqofxG2,d);

}
void Gmppke::keygen(GmppkePublicKey & pk, GmppkePrivateKey & sk) const
{
   baseKey bpk;
   const ZR alpha = group.random(ZR_t);
   bpk.gG1 = group.random(G1_t);
   bpk.gG2 = group.random(G2_t);
   const ZR beta = group.random(ZR_t);
   bpk.g2G1 = group.exp(bpk.gG1, beta);
   bpk.g2G2 = group.exp(bpk.gG2, beta);
   pk.gG1 = bpk.gG1;
   pk.gG2 = bpk.gG2;
   pk.g2G1 = bpk.g2G1;
   pk.g2G2 = bpk.g2G2;
   keygen(bpk,alpha,pk,sk);
}
void Gmppke::keygen(const baseKey & pkhibe,const ZR & gamma, GmppkePublicKey & pk, GmppkePrivateKey & sk) const
{
    pk.d = d;

    pk.ppkeg1 =  group.exp(pk.gG2,gamma);



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
    for (unsigned int i = 1; i <= d; i++)
    {
        const ZR ry = group.random(ZR_t);

        polynomial_xcordinates.push_back(ZR(i));
        pk.gqofxG1.push_back(group.exp(pk.gG1,ry));
        pk.gqofxG2.push_back(group.exp(pk.gG2,ry));

    }
    assert(polynomial_xcordinates.size()==pk.gqofxG1.size());

    // Sanity check that Lagrange interpolation works to get us g^beta on q(0).
    assert(pk.g2G1 == LagrangeInterpInExponent<G1>(group,0,polynomial_xcordinates,pk.gqofxG1,d));
    assert(pk.g2G2 == LagrangeInterpInExponent<G2>(group,0,polynomial_xcordinates,pk.gqofxG2,d));


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

GmmppkeCT Gmppke::encrypt(const GmppkePublicKey & pk,const GT & M,const std::vector<ZR> & tags) const{
	const ZR s = group.random(ZR_t);
//	const ZR alpha(42);
//	const ZR beta = ZR(747);//group.random(ZR_t);

	GmmppkeCT ct = blind(pk,M,s,tags);
//    assert(pk.ppkeg1 ==  group.exp(pk.gG2,alpha));
//    assert(pk.g2G1 == group.exp(pk.gG1, beta));
//    GT p = group.pair(pk.g2G1, pk.ppkeg1);
//
//    GT t = group.exp(group.pair(pk.gG1,pk.gG2),group.mul(alpha,beta));
//    assert(p== t);

	ct.ct1 = group.mul(group.exp(group.pair(pk.g2G1, pk.ppkeg1), s), M);
//	assert(ct.ct2==group.exp(pk.gG1, s));
//	GT b = group.pair(ct.ct2, group.exp(pk.gG2,alpha)  );
//	//GT bb  = group.exp(p,group.mul(alpha,group.mul(beta,s)));
//	GT bb = group.exp(t,s);
//	//assert(b==group.exp(group.pair(pk.g2G1, pk.ppkeg1), s));
//	assert(M==group.div(ct.ct1,bb));
	return ct;

}
PartialGmmppkeCT Gmppke::blind(const GmppkePublicKey & pk, const GT & M, const ZR & s, const std::vector<ZR> & tags ) const
{
    assert(tags.size()==d);
    PartialGmmppkeCT  ct;
    ct.ct2 = group.exp(pk.gG1, s);

    for (unsigned int i = 0; i < pk.d; i++)
    {
        G1 vofx = vG1(pk.gqofxG1,tags[i]);
        ct.ct3.push_back(group.exp(vofx, s));
    }
    ct.tags = tags;
    return ct;
}



GT Gmppke::decrypt(const GmppkePublicKey & pk, const GmppkePrivateKey & sk, const GmmppkeCT & ct ) const{
    if(!canDecrypt(sk,ct)){
    	throw PuncturedCiphertext("cannot decrypt. Duplicate tags");
    }
    return decrypt_unchecked(pk,sk,ct);
}
GT Gmppke::decrypt_unchecked(const GmppkePublicKey & pk, const GmppkePrivateKey & sk, const GmmppkeCT & ct ) const{
	return group.div(ct.ct1,recoverBlind(pk,sk,ct));
}

GT Gmppke::recoverBlind(const GmppkePublicKey & pk, const GmppkePrivateKey & sk, const PartialGmmppkeCT & ct) const
{
    assert(ct.tags.size()==d);
    assert(d==pk.d);



    vector<ZR> shareTags(ct.tags);
    const unsigned int numshares = sk.shares.size();

    shareTags.resize( ct.tags.size()+1);// allow one more tag for share the private key holds

    assert(shareTags.size() == pk.d+1);

    // Compute w_i coefficients for recovery
    // FIXME check that points are unique.


    vector<GT> z(numshares);

    for (unsigned int i = 0; i < numshares; i++)
    {
        const GmppkePrivateKeyShare & s0 = sk.shares[i];

        // FIXME DO NOT COPY an entire  vector  if possible.

        shareTags[shareTags.size()-1] = s0.sk4; 

        vector<ZR> w;

        for(unsigned int j=0;j < shareTags.size(); j++){
            w.push_back(LagrangeBasisCoefficients(group,j,0,shareTags));
        }
        const ZR wstar = w[w.size() - 1];

        G1 ct3prod_j;
        for (unsigned int j = 0; j < d; j++)
        {
            ct3prod_j = group.mul(ct3prod_j, group.exp(ct.ct3[j],w[j])); // w[0] = wstar

        }
       GT denominator = group.mul(group.pair(ct3prod_j, s0.sk3), group.pair(group.exp(ct.ct2,wstar), s0.sk2));
       GT nominator = group.pair(ct.ct2, s0.sk1);
       z[i]=group.div(nominator, denominator);
    }

    GT zprod;
    for (unsigned int i = 0; i < numshares; i++)
    {
        zprod = group.mul(zprod, z[i]);
    }
    return zprod;
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
    ZR s = group.random(ZR_t);

     BbghCT ct =blind(pk,M,s,id);
     ct.A = group.mul(group.exp(group.pair(pk.g2G1, pk.hibeg1), s), M);
     return ct;
}

PartialBbghCT Bbghibe::blind(const BbhHIBEPublicKey & pk,const GT & M, const ZR & s,const std::vector<ZR> & id) const
{
	PartialBbghCT ct;
    const unsigned int k = id.size();
    assert(k<=pk.l);

    ct.B = group.exp(pk.gG1, s);

    G1 dotProd2;
    for (unsigned int i = 0; i < k; i++)
    {
        dotProd2 = group.mul(dotProd2,group.exp(pk.hG1[i], id[i]));
    }
    ct.C = group.exp(group.mul(dotProd2, pk.g3G1), s);
    return ct;
}

GT Bbghibe::recoverBlind(const BbghPrivatekey & sk, const PartialBbghCT & ct) const{
	// This is inverted so that decrypt is ct/result. I.e. it aligns with the ppke scheme.
    return group.div(group.pair(ct.B, sk.a0),group.pair(ct.C, sk.a1));
}

GT Bbghibe::decrypt(const BbghPrivatekey & sk, const BbghCT & ct) const
{

    return group.div(ct.A, recoverBlind(sk,ct));
}

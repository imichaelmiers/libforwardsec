#include <iostream>
#include <assert.h>
#include <set>

#include "gmpfse.h"
#include "util.h"
using namespace std;

#ifdef DDDEBUG
#define libforwardsec_DBG(x) x
#else
#define libforwardsec_DBG(x)
#endif
namespace forwardsec{
const static string THE_HASH_CONSTANT = "Do not meddle in the affairs of dragons for you are crunchy and taste good with ketchup.";
PfseKeyStore::PfseKeyStore(const GmppkePrivateKey & unpuncturedKey){
	this->unpucturedPPKEKey = unpuncturedKey;
}
PfsePuncturedPrivateKey PfseKeyStore::getKey(unsigned int i)  const{
	PfsePuncturedPrivateKey p;
	auto x = puncturedKeys.find(i);
	if(x == puncturedKeys.end()){
		auto y = unpucturedHIBEKeys.find(i);

		if(y == unpucturedHIBEKeys.end() ){
			libforwardsec_DBG(cout << "did not find   keys for interval " << i << endl;)

  			  throw invalid_argument("No key for this interval: " + std::to_string(i));
		}
		libforwardsec_DBG(cout << "found key in unpunctured keys for interval " << i << endl;)
		p.hibeSK = y->second;
		p.ppkeSK = unpucturedPPKEKey;
	}else{
		libforwardsec_DBG(cout << "found key in punctured keys for interval " << i << endl;)
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
	libforwardsec_DBG(cout << "Erasing key for inteval " << i << endl;)

	puncturedKeys.erase(i); //FIXME secure erase.
	unpucturedHIBEKeys.erase(i);
}
bool PfseKeyStore::hasKey(const unsigned int i) const{
	return puncturedKeys.count(i) || unpucturedHIBEKeys.count(i);
}


void PfseKeyStore::addkey(unsigned int i, const BbghPrivatekey & h){
	libforwardsec_DBG(cout << "added key for interval " << i << endl;);
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
    ZR gamma = group.randomZR();
    ZR gamma1 = group.randomZR(); // XXX FIXME . this should break. we are randomizing the left key w/ the wrong alpha
    assert(!(gamma1 == gamma));
    sklefthibe.a0 = group.mul(sklefthibe.a0,group.exp(this->pk.g2G2,group.neg(gamma)));
    skrighthibe.a0 = group.mul(skrighthibe.a0,group.exp(this->pk.g2G2,group.neg(gamma1)));

    ppke.keygenPartial(gamma,pk,ppkeSK);


    this->privatekeys = PfseKeyStore(ppkeSK);
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
    const ZR gamma = group.randomZR();
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
    	libforwardsec_DBG(cout << interval << "not already punctured" << endl;)
		bindKey(k);
    }
    ppke.puncture(pk,k.ppkeSK,tag);
	privatekeys.updateKey(interval,k);

//privatekeys[interval] = sk;

}
PseCipherText Pfse::encrypt(const pfsepubkey & pk, const bytes msg, const unsigned int interval,const vector<std::string> tags) const {
    return encryptFO(pk,msg,group.randomGT(),interval,tags);
}

/** Fujisaki Okomoto CCA2 secure encryption
 *
 */
PseCipherText Pfse::encryptFO(const pfsepubkey & pk,  const bytes  & msg,const  GT & x,
		const unsigned int interval, const vector<std::string>  & tags ) const {
    std::stringstream ss; //FIXME the << operator returns "BROKEN"
    ss << x;
    for(auto a : msg){
    	ss << a;
    }
    ZR s = group.hashListToZR(ss.str());

    PseCipherText ct = encryptGT(pk,x,s,interval,tags);
    bytes bytestohash = x.getBytes();
    // since we don't have a different hash function, we simply postfix it with a magic constant;
    bytestohash.insert(bytestohash.begin(),THE_HASH_CONSTANT.begin(),THE_HASH_CONSTANT.end());
    bytes bits(32);
	SHA_FUNC(&bits[0], &bytestohash[0], bytestohash.size());
    ct.xorct = xorarray(msg, bits);
    return ct;

}

/** Encryption of group elements. used by encryptFO
 *
 */
PseCipherText Pfse::encryptGT(const pfsepubkey & pk, const GT & M,  const ZR & s, const unsigned int interval, const vector<std::string>  & tags) const{
    PseCipherText ct;

    ct.interval = interval;

    std::vector<ZR> id= indexToPath(interval,depth);

    ct.hibeCT = hibe.blind(pk,s,id);
    ct.ppkeCT =  ppke.blind(pk,s,tags);
    ct.ct0 =  group.mul(group.exp(group.pair(pk.g2G1, pk.hibeg1), s), M);

    return ct;
}
bytes Pfse::decrypt(const PseCipherText &ct) const{
    const PfsePuncturedPrivateKey & sk = privatekeys.getKey(ct.interval);
	vector<string> intersect =sk.ppkeSK.puncturedIntersect(ct.ppkeCT.tags);
    if(intersect.size()>0){
    	string duplicates = "";
    	bool first = true;
    	for(auto e: intersect){
    		if(!first){
    			duplicates +=", ";
    		}
    		duplicates += e;
    		first = false;
    	}
    	throw PuncturedCiphertext("cannot decrypt. The key is punctured on the following tags in the ciphertext: " + duplicates + ".");
    }
    return decryptFO(sk,ct);
}

bytes Pfse::decryptFO(const PfsePuncturedPrivateKey & sk,const PseCipherText &ct) const{
    GT x = decryptGT(sk,ct);
    bytes bytestohash = x.getBytes();
    // since we don't have a different hash function, we simply postfix it with a magic constant;
    bytestohash.insert(bytestohash.begin(),THE_HASH_CONSTANT.begin(),THE_HASH_CONSTANT.end());
    bytes bits(32);
	SHA_FUNC(&bits[0], &bytestohash[0], bytestohash.size());

    bytes aes_key = xorarray(ct.xorct, bits);
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
}

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
using namespace relicxx;
const static string THE_HASH_CONSTANT = "Do not meddle in the affairs of dragons for you are crunchy and taste good with ketchup.";
PfseKeyStore::PfseKeyStore(const GmppkePrivateKey & unpuncturedKey,unsigned int depth):depth(depth){
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
	libforwardsec_DBG(cout << "Erasing key for interval " << i << endl;)
	if(needsChildKeys(i)){
		 throw invalid_argument("Cannot delete key for interval "+std::to_string(i)+
	                              " , haven't derived keys yet.");
	}
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

bool PfseKeyStore::needsChildKeys(const unsigned int i) const{
	vector<ZR> path = indexToPath(i,depth);
	if(path.size()==depth){ // don't need keys if it's a leaf node.
		return false;
	}
	vector<ZR> lpath(path);
	vector<ZR> rpath(path);
	lpath.push_back(ZR(0));
	rpath.push_back(ZR(1));
	return !(hasKey(pathToIndex(lpath,depth)) && hasKey(pathToIndex(rpath,depth)));
}

Pfse::Pfse(unsigned int d, unsigned int numtags):hibe(),ppke(),depth(d){
	this->numtags = numtags;
    // group.setCurve(BN256);
    // cout << "depth" << depth << endl;
}


void Pfse::keygen( pfsepubkey & pk,  PfseKeyStore &sk) const{
    G2 msk;
    hibe.setup(depth,pk,msk);


    std::vector<ZR> left, right;
    BbghPrivatekey sklefthibe,  skrighthibe;
    GmppkePrivateKey ppkeSK;
    left.push_back(ZR(0));
    right.push_back(ZR(1));

    hibe.keygen(pk,msk,left,sklefthibe);
    hibe.keygen(pk,msk,right,skrighthibe);
    int l = pathToIndex(left,depth);
    int r = pathToIndex(right,depth);
    ZR gamma = group.randomZR();
    sklefthibe.a0 = group.mul(sklefthibe.a0,group.exp(pk.g2G2,group.neg(gamma)));
    skrighthibe.a0 = group.mul(skrighthibe.a0,group.exp(pk.g2G2,group.neg(gamma)));

    ppke.keygenPartial(gamma,pk,ppkeSK,numtags);


    sk = PfseKeyStore(ppkeSK,depth);
    sk.addkey(l,sklefthibe);
    sk.addkey(r,skrighthibe);
    sk.nextParentInterval = 1;
    this->prepareNextInterval(pk,sk);
}

void Pfse::prepareNextInterval(const pfsepubkey & pk, PfseKeyStore &sk)const {

	prepareIntervalAfter(pk,sk,sk.nextParentInterval);
   	sk.nextParentInterval ++;
}

void Pfse::prepareIntervalAfter(const pfsepubkey & pk, PfseKeyStore &sk,const unsigned int& i) const{
    std::vector<ZR> path = indexToPath(i,depth);
    unsigned int pathlength = path.size();
    const PfsePuncturedPrivateKey & k = sk.getKey(i); //FIXME refrence could be deleted
    if(k.punctured()){
         throw logic_error("The parent key is already punctured. The software should never allow this to happen"
        		 " You must call prepareNextInterval before starting");
    }
    if(!sk.needsChildKeys(i)){
    	return;
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
        sk.addkey(leftChildIndex,sklefthibe);
        sk.addkey(rightChildIndex,skrighthibe);
    }
}

void Pfse::deriveKeyFor(const pfsepubkey & pk, PfseKeyStore &sk,const unsigned int& i,
		const bool& storeIntermediateKeys) const{
	std::vector<ZR> path = indexToPath(i,depth);
	std::vector<ZR> ancestor;
	while(ancestor.size()<path.size() && sk.hasKey(pathToIndex(ancestor,depth)+1)){
		ancestor = std::vector<ZR>(path.begin(),path.begin()+ancestor.size()+1);

	}
    const PfsePuncturedPrivateKey & k = sk.getKey(pathToIndex(ancestor,depth));
    if(k.punctured()){
         throw logic_error("The parent tag is already punctured. The software should never allow this to happen");
    }

    BbghPrivatekey  curk= k.hibeSK;
    while(ancestor.size()<path.size()){
		ancestor = std::vector<ZR>(path.begin(),path.begin()+ancestor.size()+1);
		hibe.keygen(pk,curk,ancestor,curk);
		if(storeIntermediateKeys || ancestor.size()==path.size()){
			 sk.addkey(pathToIndex(ancestor,depth),curk);
		}
	}

}


void Pfse::bindKey(const pfsepubkey & pk,PfsePuncturedPrivateKey & k) const{
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


void Pfse::puncture(const pfsepubkey & pk, PfseKeyStore &sk,string tag) const{
	// The current active interval is one behind the nextParentInterval
    puncture(pk,sk,sk.nextParentInterval-1,tag);
}

void Pfse::puncture(const pfsepubkey & pk, PfseKeyStore &sk,unsigned int interval, string tag) const{
	if(sk.needsChildKeys(interval)){
		throw invalid_argument("Cannot puncture key for  interval "+std::to_string(interval)+
				" , haven't derived keys yet. Last interval with keys is " +std::to_string(sk.nextParentInterval-1));
	}

    PfsePuncturedPrivateKey k = sk.getKey(interval);

    //if the key is unpunctured, we need to bind in a new punctured key
    if(!k.punctured()){
    	libforwardsec_DBG(cout << interval << "not already punctured" << endl;)
		bindKey(pk,k);
    }
    ppke.puncture(pk,k.ppkeSK,tag);
	sk.updateKey(interval,k);

//privatekeys[interval] = sk;

}
PseCipherText Pfse::encrypt(const pfsepubkey & pk, const bytes msg, const unsigned int interval,const vector<std::string> tags) const {
	if(msg.size()>32){
		throw invalid_argument("msg must be at most 32 bytes.");
	}
    return encryptFO(pk,msg,group.randomGT(),interval,tags);
}

/** Fujisaki Okomoto CCA2 secure encryption
 *
 */
PseCipherText Pfse::encryptFO(const pfsepubkey & pk,  const bytes  & msg,const  GT & x,
		const unsigned int interval, const vector<std::string>  & tags ) const {
    std::stringstream ss;
    bytes b = x.getBytes();
    b.reserve(msg.size());
    b.insert(b.begin(),msg.begin(),msg.end());
    ZR s = group.hashListToZR(b);

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
bytes Pfse::decrypt(const pfsepubkey & pk, const PfseKeyStore &sk,const PseCipherText &ct) const{
    const PfsePuncturedPrivateKey & ski = sk.getKey(ct.interval);
	vector<string> intersect =ski.ppkeSK.puncturedIntersect(ct.ppkeCT.tags);
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
    return decryptFO(pk,ski,ct);
}

bytes Pfse::decryptFO(const pfsepubkey & pk,const PfsePuncturedPrivateKey & ski,const PseCipherText &ct) const{
    GT x = decryptGT(pk,ski,ct);
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

GT Pfse::decryptGT(const pfsepubkey & pk,const PfsePuncturedPrivateKey & ski,const PseCipherText &ct) const {
    GT b1 = hibe.recoverBlind(ski.hibeSK,ct.hibeCT);
   // assert(b1== group.exp(group.exp(group.pair(g2G1,gG2),group.mul(ss,group.sub(aa,gam1))),neg));
    GT b2 = ppke.recoverBlind(pk,ski.ppkeSK,ct.ppkeCT);
   // assert(b2 ==group.exp(group.pair(g2G1,gG2),group.mul(ss,gam1)));
    return group.div(ct.ct0,group.mul(b1,b2));
}
}

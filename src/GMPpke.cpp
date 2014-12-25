
#include <assert.h>
#include <unordered_set>
#include "GMPpke.h"
#include "util.h"
namespace forwardsec{

using namespace std;
static const string  NULLTAG = "42"; // the reserved tag

using namespace std;
std::vector<std::string> GmppkePrivateKey::puncturedIntersect(const std::vector<std::string> & tags)const {
	unordered_set<std::string> tagset(tags.begin(),tags.end());
	vector<string> duplicates;
	for(auto share : shares){
		if(tagset.count(share.sk4) >0){
			duplicates.push_back(share.sk4);
		}
	}
    return duplicates;
}
//G1  Gmppke::vG1(const std::vector<G1> & gqofxG1, const std::string & x) const{
//    vector<ZR> xcords;
//    int size = gqofxG1.size() ;
//    for(int i=0;i<size;i++){
//        ZR xcord = i;
//        xcords.push_back(xcord);
//    }
//    return LagrangeInterpInExponent(group,group.hashListToZR(x),xcords,gqofxG1);
//
//}
//G2 Gmppke::vG2(const std::vector<G2> & gqofxG2,const std::string  & x) const{
//    vector<ZR> xcords;
//        int size = gqofxG2.size() ;
//
//    for(int i=0; i <size; i++){
//        ZR xcord = i;
//        xcords.push_back(xcord);
//    }
//    return LagrangeInterpInExponent(group,group.hashListToZR(x),xcords,gqofxG2);
//}
void Gmppke::keygen(GmppkePublicKey & pk, GmppkePrivateKey & sk,const unsigned int & d) const
{
   GmppkePublicKey bpk;
   const ZR alpha = group.randomZR();
   bpk.gG1 = group.randomG1();
   bpk.gG2 = group.randomG2();
   const ZR beta = group.randomZR();
   bpk.g2G1 = group.exp(bpk.gG1, beta);
   bpk.g2G2 = group.exp(bpk.gG2, beta);
   pk.gG1 = bpk.gG1;
   pk.gG2 = bpk.gG2;
   pk.g2G1 = bpk.g2G1;
   pk.g2G2 = bpk.g2G2;
   keygenPartial(alpha,pk,sk,d);
}
void Gmppke::keygenPartial(const ZR & alpha, GmppkePublicKey & pk, GmppkePrivateKey & sk, const unsigned int & d) const
{
    pk.d = d;
    pk.ppkeg1 =  group.exp(pk.gG2,alpha);

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
        const ZR ry = group.randomZR();

        polynomial_xcordinates.push_back(ZR(i));
        pk.gqofxG1.push_back(group.exp(pk.gG1,ry));
        pk.gqofxG2.push_back(group.exp(pk.gG2,ry));

    }
    assert(polynomial_xcordinates.size()==pk.gqofxG1.size());

    // Sanity check that Lagrange interpolation works to get us g^beta on q(0).
    assert(pk.g2G1 == LagrangeInterpInExponent<G1>(group,0,polynomial_xcordinates,pk.gqofxG1));
    assert(pk.g2G2 == LagrangeInterpInExponent<G2>(group,0,polynomial_xcordinates,pk.gqofxG2));


    sk.shares.push_back(skgen(pk,alpha));

    return;
}
GmppkePrivateKeyShare Gmppke::skgen(const GmppkePublicKey &pk,const ZR & alpha  ) const{
	GmppkePrivateKeyShare share;
    share.sk4 = NULLTAG;
    const ZR r = group.randomZR();
    share.sk1 = group.exp(pk.g2G2, group.add(r,alpha));
    G2 vofx = vx(pk.gqofxG2,NULLTAG); // calculate v(t0).
    share.sk2 = group.exp(vofx, r);// v(t0)^r
    share.sk3 = group.exp(pk.gG2, r);
    return share;
}

void Gmppke::puncture(const GmppkePublicKey & pk, GmppkePrivateKey & sk, const string & tag) const{

	if(tag == NULLTAG){
		throw invalid_argument("Invalid tag "+tag +". The tag " + NULLTAG + " is reserved and cannot be used.");
	}
    GmppkePrivateKeyShare skentryn;
    GmppkePrivateKeyShare & skentry0 = sk.shares[0];

    const ZR r0 = group.randomZR();
    const ZR r1 = group.randomZR();
    const ZR lambda = group.randomZR();

    assert(skentry0.sk4 == NULLTAG);

    skentry0.sk1 = group.mul(skentry0.sk1,group.exp(pk.g2G2,group.sub(r0,lambda))); // sk1 * g2g2^{r0- lambda}
    const G2 vofx = vx(pk.gqofxG2,NULLTAG);
    skentry0.sk2 = group.mul(skentry0.sk2,group.exp(vofx,r0));  // sk2 * V(t0)^r0
    skentry0.sk3 = group.mul(skentry0.sk3,group.exp(pk.gG2,r0));  // sk3 * g2G2^r0

    skentryn.sk1=group.exp(pk.g2G2,group.add(r1,lambda));  // gG2 ^ (r1+lambda)
    const G2 vofx2 = vx(pk.gqofxG2,tag);
    skentryn.sk2 = group.exp(vofx2,r1); // V(tag) ^ r1
    skentryn.sk3 = group.exp(pk.gG2,r1);  // G^ r1
    skentryn.sk4 = tag;

    sk.shares.push_back(skentryn);
}


GmmppkeCT Gmppke::encrypt(const GmppkePublicKey & pk,const GT & M,const std::vector<std::string> & tags) const{
	const ZR s = group.randomZR();
	GmmppkeCT ct = blind(pk,s,tags);
	ct.ct1 = group.mul(group.exp(group.pair(pk.g2G1, pk.ppkeg1), s), M);
	return ct;

}
PartialGmmppkeCT Gmppke::blind(const GmppkePublicKey & pk, const ZR & s, const std::vector<string> & tags ) const
{
	//simple duplicate check without modifying  tags by sorting
	if(	unordered_set<string>(tags.begin(),tags.end()).size() != tags.size()){
		throw invalid_argument("Tags must be unique. You have provided at least one duplicate tag.");
	}
	if(tags.size() != pk.d){
		throw invalid_argument("You must provide exactly " +std::to_string(pk.d) + " tags. You provided " +std::to_string(tags.size())+" tag.");
	}
    PartialGmmppkeCT  ct;
    ct.ct2 = group.exp(pk.gG1, s);

    for (unsigned int i = 0; i < pk.d; i++)
    {
        G1 vofx = vx(pk.gqofxG1,tags[i]);
        ct.ct3.push_back(group.exp(vofx, s));
    }
    ct.tags = tags;
    return ct;
}



GT Gmppke::decrypt(const GmppkePublicKey & pk, const GmppkePrivateKey & sk, const GmmppkeCT & ct ) const{
	vector<string> intersect =sk.puncturedIntersect(ct.tags);
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
    return decrypt_unchecked(pk,sk,ct);
}
GT Gmppke::decrypt_unchecked(const GmppkePublicKey & pk, const GmppkePrivateKey & sk, const GmmppkeCT & ct ) const{
	return group.div(ct.ct1,recoverBlind(pk,sk,ct));
}

GT Gmppke::recoverBlind(const GmppkePublicKey & pk, const GmppkePrivateKey & sk, const PartialGmmppkeCT & ct) const
{
    assert(ct.tags.size()==pk.d);
    vector<ZR> shareTags(ct.tags.size());
    for(unsigned int i=0;i<ct.tags.size();i++){
    	shareTags[i] = group.hashListToZR(ct.tags[i]);
    }
    const unsigned int numshares = sk.shares.size();

    shareTags.resize( ct.tags.size()+1);// allow one more tag for share the private key holds

    assert(shareTags.size() == pk.d+1);

    // Compute w_i coefficients for recovery
    vector<GT> z(numshares);

    for (unsigned int i = 0; i < numshares; i++)
    {
        const GmppkePrivateKeyShare & s0 = sk.shares[i];

        // FIXME DO NOT COPY an entire  vector  if possible.

        shareTags[shareTags.size()-1] = group.hashListToZR(s0.sk4);

        vector<ZR> w;

        for(unsigned int j=0;j < shareTags.size(); j++){
            w.push_back(LagrangeBasisCoefficients(group,j,0,shareTags));
        }
        const ZR wstar = w[w.size() - 1];

        G1 ct3prod_j;
        for (unsigned int j = 0; j < pk.d; j++)
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
}

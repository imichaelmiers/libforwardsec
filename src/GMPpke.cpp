#include <string>
#include <list>
#include <vector>
#include <assert.h>
#include <stdexcept>
#include <set>
#include "GMPpke.h"
#include "util.h"
using namespace std;
unsigned int d = 1;
#define NULLTAG 42

using namespace std;

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


G1  Gmppke::vG1(const std::vector<G1> & gqofxG1, const ZR & x) const{
    vector<ZR> xcords;
    int size = gqofxG1.size() ;
    for(int i=0;i<size;i++){
        ZR xcord = i;
        xcords.push_back(xcord);
    }
    return LagrangeInterpInExponent(group,x,xcords,gqofxG1,d);

}
G2 Gmppke::vG2(const std::vector<G2> & gqofxG2,const ZR & x) const{
    vector<ZR> xcords;
        int size = gqofxG2.size() ;

    for(int i=0; i <size; i++){
        ZR xcord = i;
        xcords.push_back(xcord);
    }
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

	GmmppkeCT ct = blind(pk,s,tags);
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
PartialGmmppkeCT Gmppke::blind(const GmppkePublicKey & pk, const ZR & s, const std::vector<ZR> & tags ) const
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

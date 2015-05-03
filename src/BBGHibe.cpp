/*
 * BBGHHibe.cpp
 *
 *  Created on: Dec 21, 2014
 *      Author: imiers
 */
#include <assert.h>
#include "BBGHibe.h"
namespace forwardsec{

using namespace std;
using namespace relicxx;

void BBGHibePrivateKey::neuter(){
    b.resize(0); //FIXME secure erase
    bG2.resize(0);
}
void BBGHibe::setup(const unsigned int & l, BBGHibePublicKey & pk, G2 & msk) const
{
	if(l>31){
		throw invalid_argument("max id length must be less than 32.");
	}
    ZR alpha = group.randomZR();
    pk.gG1 = group.randomG1();
    pk.gG2 = group.randomG2();
    pk.hibeg1 = group.exp(pk.gG2, alpha);
    pk.l = l;
    const ZR r = group.randomZR();
    pk.g2G1 = group.exp(pk.gG1, r);
    pk.g2G2 = group.exp(pk.gG2, r);
    const ZR r1 = group.randomZR();
    pk.g3G1 = group.exp(pk.gG1, r1);
    pk.g3G2 = group.exp(pk.gG2, r1);
    for (unsigned int i = 0; i < l; i++)
    {
        ZR h = group.randomZR();
        pk.hG1.push_back(group.exp(pk.gG1, h));
        pk.hG2.push_back(group.exp(pk.gG2, h));
    }
    msk = group.exp(pk.g2G2, alpha);

    return;
}

void BBGHibe::keygen(const BBGHibePublicKey & pk, const G2 & msk, const std::vector<ZR> & id, BBGHibePrivateKey & sk) const
{
    const ZR r = group.randomZR();
    const unsigned int k = id.size();
    for (unsigned int i = 0; i < k; i++)
    {
        sk.a0 = group.mul(sk.a0, group.exp(pk.hG2[i], id[i]));
    }
    sk.a0 = group.exp(group.mul(sk.a0, pk.g3G2), r);
    sk.a0 = group.mul(msk, sk.a0);
    sk.a1 = group.exp(pk.gG2, r);
    const unsigned int s = pk.l - k;

    sk.b.resize(s);
    sk.bG2.resize(s);
    for (unsigned int i =  k;i < pk.l; i++)
    {
        sk.b[i - k] = group.exp(pk.hG1[i], r);

        sk.bG2[i - k] = group.exp(pk.hG2[i], r);
    }
    return;
}

void BBGHibe::keygen(const BBGHibePublicKey & pk,const  BBGHibePrivateKey & sk, const std::vector<ZR> &id,BBGHibePrivateKey & skout) const{
    const unsigned int k = id.size();
    ZR t = group.randomZR();

    G2 hprod;
    for (unsigned int i = 0; i < k ; i++)
    {
        if(id.at(i)==1){
           hprod = group.mul(hprod, pk.hG2[i]);
        }
    }
    hprod = group.exp(group.mul(hprod, pk.g3G2), t);
    hprod = group.mul(hprod,group.exp(sk.bG2[0],(id[k-1])));
    skout.a0 = group.mul(hprod,sk.a0);
    skout.a1 = group.mul(sk.a1,group.exp(pk.gG2,t));
    const unsigned int s = pk.l - k;
    skout.b.resize(s);
    skout.bG2.resize(s);
   // assert(skout.b.size() == sk.b.size());
   // assert(skout.bG2.size() == sk.bG2.size());

    for (unsigned int i = k ; i < pk.l; i++)
    {
        skout.b[i - k] = group.mul(sk.b[i-k+1],group.exp(pk.hG1[i],t));
        skout.bG2[i -k] = group.mul(sk.bG2[i -k+1],group.exp(pk.hG2[i],t));

    }
    return;
}

BBGHibeCiphertext BBGHibe::encrypt(const BBGHibePublicKey & pk, const GT & M, const std::vector<ZR> & id) const{
    ZR s = group.randomZR();

     BBGHibeCiphertext ct =blind(pk,s,id);
     //FIXME remove pairing.
     ct.A = group.mul(group.exp(group.pair(pk.g2G1, pk.hibeg1), s), M);
     return ct;
}

BBGHibePartialCiphertext BBGHibe::blind(const BBGHibePublicKey & pk, const ZR & s,const std::vector<ZR> & id) const
{
	BBGHibePartialCiphertext ct;
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

GT BBGHibe::recoverBlind(const BBGHibePrivateKey & sk, const BBGHibePartialCiphertext & ct) const{
	// This is inverted so that decrypt is ct/result. I.e. it aligns with the ppke scheme.
    return group.div(group.pair(ct.B, sk.a0),group.pair(ct.C, sk.a1));
}

GT BBGHibe::decrypt(const BBGHibePrivateKey & sk, const BBGHibeCiphertext & ct) const
{

    return group.div(ct.A, recoverBlind(sk,ct));
}

}



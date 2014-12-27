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
void Bbghibe::setup(const unsigned int & l, BbhHIBEPublicKey & pk, G2 & msk) const
{
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

void Bbghibe::keygen(const BbhHIBEPublicKey & pk, const G2 & msk, const std::vector<ZR> & id, BbghPrivatekey & sk) const
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

    sk.b.resize(pk.l);
    sk.bG2.resize(pk.l);
    for (unsigned int i =  k;i < pk.l; i++)
    {
        sk.b[i-(k-1)] = group.exp(pk.hG1[i], r);

        sk.bG2[i-(k-1)] = group.exp(pk.hG2[i], r);
    }
    return;
}

void Bbghibe::keygen(const BbhHIBEPublicKey & pk,const  BbghPrivatekey & sk, const std::vector<ZR> &id,BbghPrivatekey & skout) const{
    const unsigned int k = id.size();
    ZR t = group.randomZR();

    G2 hprod;
    for (unsigned int i = 0; i < k ; i++)
    {
        hprod = group.mul(hprod, group.exp(pk.hG2[i], id[i]));
    }
    hprod = group.exp(group.mul(hprod, pk.g3G2), t);
    hprod = group.mul(hprod,group.exp(sk.bG2[k-1],(id[k-1])));
    skout.a0 = group.mul(hprod,sk.a0);
    skout.a1 = group.mul(sk.a1,group.exp(pk.gG2,t));

    const unsigned int  kk = k-2;
    skout.b.resize(pk.l-kk);
    skout.bG2.resize(pk.l-kk);
    for (unsigned int i = k ; i < pk.l; i++)
    {
        skout.b[i-kk] = group.mul(sk.b[i-kk],group.exp(pk.hG1[i],t));
        skout.bG2[i-kk] = group.mul(sk.bG2[i-kk],group.exp(pk.hG2[i],t));

    }
    return;
}

BbghCT Bbghibe::encrypt(const BbhHIBEPublicKey & pk, const GT & M, const std::vector<ZR> & id) const{
    ZR s = group.randomZR();

     BbghCT ct =blind(pk,s,id);
     ct.A = group.mul(group.exp(group.pair(pk.g2G1, pk.hibeg1), s), M);
     return ct;
}

PartialBbghCT Bbghibe::blind(const BbhHIBEPublicKey & pk, const ZR & s,const std::vector<ZR> & id) const
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

}



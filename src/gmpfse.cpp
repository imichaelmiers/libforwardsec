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
int d = 1;
int tag0=42;



#define BENCH
Pfse::Pfse(uint d):hibe(),ppke(),depth(d){
    // group.setCurve(BN256);
    // cout << "depth" << depth << endl;
}
void Pfse::keygen(){
    G2 msk;
    hibe.setup(depth,this->pk.hibe,msk);


    std::vector<ZR> left, right;
    BbghPrivatekey sklefthibe,  skrighthibe;
    GmppkePrivateKey skleftppke,skrightppke;
    left.push_back(ZR(0));
    right.push_back(ZR(1));

    hibe.keygen(this->pk.hibe,msk,left,sklefthibe);
    hibe.keygen(this->pk.hibe,msk,right,skrighthibe);
    int l = hibe.pathToIndex(left,depth);
    int r = hibe.pathToIndex(right,depth); 
    ZR gamma1 = group.random(ZR_t);
    ZR gamma2 = group.random(ZR_t);

    sklefthibe.a0 = group.mul(sklefthibe.a0,group.exp(this->pk.hibe.g2G2,group.neg(gamma1)));
    skrighthibe.a0 = group.mul(skrighthibe.a0,group.exp(this->pk.hibe.g2G2,group.neg(gamma2)));

    ppke.keygen(pk.hibe,gamma1,pk.ppke,skleftppke);
   // ppke.skgen(pk.ppke,gamma2,skrightppkeentry);
   // skrightppke.insert(0,skrightppkeentry);

    this->unpucturedKey = skleftppke;
    this->activeKey = skleftppke;
    // skright.hibe = skrighthibe;
    // skright.ppke = skrightppke;
    this->Hibeprivatekeys[l] = sklefthibe;
    this->Hibeprivatekeys[r] = skrighthibe;
    latestInterval = 1;
}
// void Pfse::deleteInterval(uint interval){
//     if(interval < 1){
//         throw invalid_argument("Interval < 1. No previous interval to delete yet" );
//     }
//     if(privatekeys.count(interval) != 1){
//         throw invalid_argument("We don't have that interval. Maybe it was already deleted" );
//     }
//     if(privatekeys.count(interval+1) != 1){
//         throw invalid_argument("Can't delete this interval yet. First run prepareNextInterval " );
//     }
//     PfseIntervalKey empty;

//     privatekeys[interval] = empty;
// }
void Pfse::prepareNextInterval(){
    assert(Hibeprivatekeys.count(latestInterval) ==1);

    std::vector<ZR> path = hibe.indexToPath(latestInterval,depth);
    int pathlength = path.size();
    HIBEkey skparent = Hibeprivatekeys[latestInterval];
    // if(skparent.ppke.length()>1){
    //     throw logic_error("The parent tag is already punctured. You must call prepareNextInterval before starting");
    // }
   // CharmList ppkeparententry0 = skparent.ppke[0].getList();
    if (pathlength  < depth){ // Not a leaf node, so derive new keys.
        HIBEkey sklefthibe; 
        HIBEkey skrighthibe;

        path.push_back(ZR(0)); // FIXME
        int leftChildIndex = hibe.pathToIndex(path,depth);
        hibe.keygen(pk.hibe,skparent,path,sklefthibe);
       // cout << "deriving key for " << leftChildIndex << " ";
#ifndef BENCH
        cout << "derived  hibe left key for " << leftChildIndex <<  endl;
       for(int i = 0; i <=pathlength ; i++){
            cout <<  path[i];
        }

        cout << endl;
#endif

        // compute right key;
        path[pathlength]=ZR(1);
        int rightChildIndex = hibe.pathToIndex(path,depth);
        hibe.keygen(pk.hibe,skparent,path,skrighthibe);


#ifndef BENCH

        cout << "derived hibe right key for " << rightChildIndex <<  endl;
        for(int i = 0; i <=pathlength ; i++){
            cout << path[i];
        }
        cout << endl;
#endif
        Hibeprivatekeys[leftChildIndex] = sklefthibe;
        Hibeprivatekeys[rightChildIndex] = skrighthibe;


        // G2 a0left = group.mul(sklefthibe[0].getG2(),group.exp(g2G2,group.neg(gamma1)));
        // G2 a0right = group.mul(skrighthibe[0].getG2(),group.exp(g2G2,group.neg(gamma2)));
        // sklefthibe[0] = a0left;
        // skrighthibe[0] =a0right;
 // FIXME secure delete;
    }
        // NOW that we have derived the hibe key for the next intervals if necessary
       


        ZR gamma = group.random(ZR_t);
         for(auto& x: this->Hibeprivatekeys){
             HIBEkey s;
             s = x.second;
             s.a0 = group.mul(s.a0,group.exp(pk.hibe.g2G2,group.neg(gamma)));
             Hibeprivatekeys[x.first]=s;
         }

        Hibeprivatekeys.erase(latestInterval);
        latestInterval ++;
        HIBEkey nextIntervalHIBEKey = Hibeprivatekeys[latestInterval];


        Hibeprivatekeys[latestInterval]=nextIntervalHIBEKey;
        GmppkePrivateKeyShare newActiveKeyPPKEKeyEntry;
        ppke.skgen(pk.ppke,gamma,newActiveKeyPPKEKeyEntry);
        GmppkePrivateKey newActiveKeyPPKEKey;
        GmppkePrivateKeyShare pristineKeyEntry = unpucturedKey.shares[0];
        updateppkesk(newActiveKeyPPKEKeyEntry,pristineKeyEntry);
        newActiveKeyPPKEKey.shares.push_back(newActiveKeyPPKEKeyEntry);
        this->unpucturedKey = newActiveKeyPPKEKey;
        this->activeKey = newActiveKeyPPKEKey;
        // updateppkesk(skleftppkeentry,ppkeparententry0);
        // updateppkesk(skrightppkeentry,ppkeparententry0);

        // skleftppke.insert(0,skleftppkeentry);
        // skrightppke.insert(0,skrightppkeentry);
        // skleft.ppke= skleftppke;
        // skleft.hibe = sklefthibe;
        // skright.ppke = skrightppke;
        // skright.hibe = skrighthibe;
        // privatekeys[leftChildIndex] = skleft;
        // privatekeys[rightChildIndex] = skright;
        
}
void Pfse::updateppkesk(GmppkePrivateKeyShare & skentry,GmppkePrivateKeyShare & skentryold){
    skentry.sk1 = group.mul(skentry.sk1,skentryold.sk1);
    skentry.sk2 = group.mul(skentry.sk2,skentryold.sk2);
    skentry.sk3 = group.mul(skentry.sk3,skentryold.sk3);
}
void Pfse::puncture(string tag){
    puncture(latestInterval,tag);
}
void Pfse::puncture(uint interval, string tag){
    ZR tagZR = group.hashListToZR(tag);
    // if(interval >= latestInterval){
    //     throw invalid_argument("We cannot puncture on this interval. First run prepareNextInterval ");
    // }
    // if(privatekeys.count(interval) != 1){
    //     cout << "we don't have key for " <<interval << endl;
    //     throw invalid_argument("Hipster !!!. We don't have that key yet. Current interval is earlier. " + interval);
    // }
    if(interval != latestInterval){
        throw invalid_argument("can only puncture on the current interval");
    }
 //   PfseIntervalKey sk = [interval];
    ppke.puncture(pk.ppke,activeKey,tagZR);
//privatekeys[interval] = sk;

}
PseCipherText Pfse::encrypt(pfsepubkey & pk, AESKey aes_key,uint interval,vector<string> tags){
    vector<ZR> tagsZR;
    // for(int i=0;i<aes_key.size();i++){
    //     ZR zrz(0);
    //     ZR zro(1);
    //     bitmsg[i]=  aes_key[i] ? zro :zrz;
    // }
    for(int i=0;i<tags.size();i++){
        ZR tag =  group.hashListToZR(tags[i]);
        tagsZR.push_back(tag);
    }
    return encryptFO(pk,aes_key,interval,tagsZR);
}
PseCipherText Pfse::encryptFO(pfsepubkey & pk,  AESKey  & aes_key ,uint interval, vector<ZR>  & tags ){
    GT x = group.random(GT_t);
    return encryptFO(pk,aes_key,x,interval,tags);

}
PseCipherText Pfse::encryptFO(pfsepubkey & pk,  AESKey  & aes_key, GT & x, uint interval, vector<ZR>  & tags ){
    std::stringstream ss;
    ss << x;
    ss << aes_key;
    ZR s = group.hashListToZR(ss.str());
   
    PseCipherText ct = encrypt(pk,x,s,interval,tags);

    std::stringstream sss;
    sss << "0xDEADBEEF";
    sss << x;
    ZR xorash = group.hashListToZR(sss.str().c_str());
    AESKey bits = intToBits(xorash);

    ct.xorct = aes_key ^ bits;
    return ct;

}
PseCipherText Pfse::encrypt(pfsepubkey & pk, GT & M,uint interval, vector<ZR>  & tags){
        ZR s = group.random(ZR_t);
        return Pfse::encrypt(pk,M,s,interval,tags);
}

PseCipherText Pfse::encrypt(pfsepubkey & pk, GT & M, ZR & s,uint interval, vector<ZR>  & tags){
    PseCipherText ct;
    GmmppkeCT ctppke;
    BbghCT cthibe;
    std::vector<ZR> id= hibe.indexToPath(interval,depth);
    GT ct0 =  group.mul(group.exp(group.pair(pk.hibe.g2G1, pk.hibe.g1), s), M);

    hibe.encrypt(pk.hibe,M,s,id,cthibe);
    ppke.encrypt(pk.ppke,M,s,tags,ctppke);
    ct.interval = interval;

    ct.ct0= ct0;
    ct.hibeCT = cthibe;
    ct.ppkeCT = ctppke;
    ct.interval = interval;
    return ct;
}
AESKey Pfse::decrypt(PseCipherText &ct){
    //AESKey k;

    return decryptFO(ct);
    // for(int i=0;i<k.size();i++){
    //     k[i]= r[i].getZR() == ZR(1)? true : false;
    // }
    //return k;
}

AESKey Pfse::decryptFO(PseCipherText &ct){
    GT x = decryptGT(ct);

    std::stringstream sss;
    sss << "0xDEADBEEF";
    sss << x;
    ZR xorash = group.hashListToZR(sss.str());

    AESKey bits = intToBits(xorash);
    AESKey aes_key = ct.xorct ^ bits;
    PseCipherText cttest = encryptFO(pk,aes_key,x,ct.interval,ct.ppkeCT.tags);
    if(
    cttest.ppkeCT == ct.ppkeCT && 
    cttest.hibeCT == ct.hibeCT
    ){ 
        return aes_key;
    }else{
      throw BadCiphertext("Fujisaki Okamoto integrety check failed ");
    }
}


GT Pfse::decryptGT(PseCipherText &ct){
    GT b1,b2,result;
    int interval = ct.interval;

    // if(privatekeys.count(interval) != 1){
    //     cout << "we don't have key for that interval. Maybe it was deleted " <<interval << endl;
    //     throw invalid_argument("No key for this interval. Cannot decrypt" + interval);
    // }
    if(Hibeprivatekeys.count(interval) != 1){
        cout << "we don't have key for that interval. Maybe it was deleted " <<interval << endl;
        throw invalid_argument("No key for this interval. Cannot decrypt" + interval);
    }

    HIBEkey sk = Hibeprivatekeys[interval];
    hibe.decrypt(sk,ct.hibeCT,b1);
    ZR neg = -1;
   // assert(b1== group.exp(group.exp(group.pair(g2G1,gG2),group.mul(ss,group.sub(aa,gam1))),neg));
    ppke.decrypt(pk.ppke,activeKey,ct.ppkeCT,b2);
   // assert(b2 ==group.exp(group.pair(g2G1,gG2),group.mul(ss,gam1)));
    result = group.div(group.mul(ct.ct0,b1),b2);
    return result;
}

uint treeSize(uint k){
    return (2 <<(k)) -1;
}


ZR Gmppke::LagrangeBasisCoefficients(uint j,const ZR &x , const vector<ZR> & polynomial_xcordinates){
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
    const vector<ZR> & polynomial_ycordinates, uint degree){
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
    const vector<type> & exp_polynomial_ycordinates, uint degree, const type & g){
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

void  Gmppke::vG1(const std::vector<G1> & gqofxG1,const G1 & gG1, const ZR & x, G1 & result){
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

    result = LagrangeInterpInExponent(x,xcords,gqofxG1,d,gG1);

}
void Gmppke::vG2(const std::vector<G2> & gqofxG2,const G2 & gG2,const ZR & x, G2 & result){
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

    result = LagrangeInterpInExponent(x,xcords,gqofxG2,d,gG2);

}
void Gmppke::keygen(const BbhHIBEPublicKey & pkhibe,ZR & gamma, GmppkePublicKey & pk, GmppkePrivateKey & sk)
{
    pk.d = d;

    ZR r;
    G2 vofx;
    GmppkePrivateKeyShare skentry0;

    ZR t0 = tag0; // The special zero tag;
    ZR sk4 = t0;
    ZR zero = 0;

    pk.gG1 = pkhibe.gG1;
    pk.gG2 = pkhibe.gG2;
    pk.g1 =  group.exp(pk.gG2,gamma);
    pk.g2G1 = pkhibe.g2G1;
    pk.g2G2 = pkhibe.g2G2;

    // Select a random polynomial of degree d subject to q(0)= beta. We do this
    // by selecting d+1 points. Because we don't actually  care about the 
    // polynomial, only g^q(x), we merely select points as (x,g^q(x)).

    vector<ZR> polynomial_xcordinates;

    // the first point is (x=0,y=beta) so  x=0, g^beta.
    polynomial_xcordinates.push_back(zero);
    pk.gqofxG1.push_back(pk.g2G1); // g^beta
    pk.gqofxG2.push_back(pk.g2G2); // g^beta

    // the next d points y values  are random
    // we use x= 1...d becuase the has the side effect
    // of easily computing g^q(0).... g^q(d).
    for (int i = 1; i <= d; i++)
    {
        ZR x = i;
        ZR ry = group.random(ZR_t); // 17
        G1 qofrxG1 = group.exp(pk.gG1,ry);
        G2 qofrxG2 = group.exp(pk.gG2,ry);
        polynomial_xcordinates.push_back(x);
        pk.gqofxG1.push_back(qofrxG1);
        pk.gqofxG2.push_back(qofrxG2);

    }
    assert(polynomial_xcordinates.size()==pk.gqofxG1.size());


        // cout << "\n\n polynomial cordinates:" << polynomial_xcordinates.size() <<  endl;
        // for (int i = 0; i <= d; i++){
        //     cout << "{" << polynomial_xcordinates[i] << "," << polynomial_ycordinates[i] << "}"<< endl;

        // }
        // cout << "\n\n" << endl;
//    cout << "random points picked"<< endl;
    // Sanity check that Lagrant interpt works to get us g^beta on q(0).
    G2 test2 = LagrangeInterpInExponent<G2>(0,polynomial_xcordinates,pk.gqofxG2,d,pk.gG2);
    assert(pk.g2G1 == LagrangeInterpInExponent<G1>(0,polynomial_xcordinates,pk.gqofxG1,d,pk.gG1));
    assert(pk.g2G2 == test2); // FIXME i should be able to put the extression for test2 here, but that doesn't compile;



    skgen(pk,gamma,skentry0);

    sk.shares.push_back(skentry0);
    // if(splitkey ==1){

    // sk.insert(1,skentry1);
    // }
    return;
}
void Gmppke::skgen(const GmppkePublicKey &pk,const ZR & alpha, GmppkePrivateKeyShare & skentry0){
    G2 vofx;
    // gG2 = pk[1].getG2();
    // g2G2 = pk[4].getG2();
    // gqofxG2 = pk[7].getListG2();
    ZR t0 = tag0; // The special zero tag;
    skentry0.sk4 = t0; 
    ZR r = group.random(ZR_t);
    skentry0.sk1 = group.exp(pk.g2G2, group.add(r,alpha));
    vG2(pk.gqofxG2,pk.gG2 ,t0,vofx); // calculate v(t0).
    skentry0.sk2 = group.exp(vofx, r);// v(t0)^r
    skentry0.sk3 = group.exp(pk.gG2, r);
}

void Gmppke::puncture(const GmppkePublicKey & pk, GmppkePrivateKey & sk, const ZR & tag){

    GmppkePrivateKeyShare skentryn;
    GmppkePrivateKeyShare & skentry0 = sk.shares[0];

    ZR r0 = group.random(ZR_t);
    ZR r1 = group.random(ZR_t);
    ZR lambda = group.random(ZR_t);

    assert(skentry0.sk4 == ZR(tag0));

   // G2 laminv = group.inv(group.exp(gG2,lambda));
  //  sk1p = group.mul(group.mul(sk1,group.exp(g2G2,r0)),laminv); // sk1 * g2g2^{r0- lambda}
    skentry0.sk1 = group.mul(skentry0.sk1,group.exp(pk.g2G2,group.sub(r0,lambda))); // sk1 * g2g2^{r0- lambda}
    G2 vofx; 
    vG2(pk.gqofxG2,pk.gG2,skentry0.sk4,vofx);
    skentry0.sk2 = group.mul(skentry0.sk2,group.exp(vofx,r0));  // sk2 * V(t0)^r0
    skentry0.sk3 = group.mul(skentry0.sk3,group.exp(pk.gG2,r0));  // sk3 * g2G2^r0

    //vG2(gqofxG2,gG2 ,t1,vofx); // calculate v(t0).
 //   G2 tsk1n = group.exp(g2G2, group.add(group.add(alphat,rt),group.sub(r0,lambda)));
    //sk1p == tsk1n;
    //assert(tsk1n == sk1p);
    //G2 tsk2n = group.exp(vofx,r1);
    //G2 tsk3n = group.exp(gG2,r1);
    //ZR tsk4n = t1;

    skentryn.sk1=group.exp(pk.g2G2,group.add(r1,lambda));  // gG2 ^ (r1+lambda)
    G2 vofx2;   
    vG2(pk.gqofxG2,pk.gG2,tag,vofx2);  
    skentryn.sk2 = group.exp(vofx2,r1); // V(tag) ^ r1
    skentryn.sk3 = group.exp(pk.gG2,r1);  // G^ r1
    skentryn.sk4 = tag;


    sk.shares[0]=skentry0;
    sk.shares.push_back(skentryn);
}
void Gmppke::encrypt(const GmppkePublicKey & pk, const GT & M, const ZR & s, const std::vector<ZR> & tags, GmmppkeCT & ct)
{
    G1 vofx;
    assert(tags.size()==d);
    ct.ct1 = group.mul(group.exp(group.pair(pk.g2G1, pk.g1), s), M);
    ct.ct2 = group.exp(pk.gG1, s);

    for (int i = 0; i < pk.d; i++)
    {
#ifndef BENCH

        cout << "\t encrypting with tag tag_" << i <<" = " << tags[i] << endl;
#endif
        vG1(pk.gqofxG1,pk.gG1,tags[i], vofx);
        // G1 hardresult = group.exp(gG1,89);
        // cout <<"testing of v(x) was computed correctly " << endl;
        // assert(vofx == hardresult);

        ct.ct3.push_back(group.exp(vofx, s));
    }
    ct.tags = tags;
    return;
}

void Gmppke::decrypt(const GmppkePublicKey & pk, const GmppkePrivateKey & sk, const GmmppkeCT & ct, GT & b)
{


    ZR wstar;

    assert(ct.tags.size()==d);
    assert(d==pk.d);

    vector<ZR> shareTags(ct.tags.size()+1);// allow one more tag for share the private key holds
    for(int i =0; i < ct.tags.size();i++){
            shareTags[i]=ct.tags[i];//(hashresult);
    }
    assert(shareTags.size() == pk.d+1);

    // Compute w_i coefficients for recovery
    // FIXME check that points are unique.

    int numshares = sk.shares.size();
#ifndef BENCH

    cout << "total number of shares " << numshares << endl;
#endif

    // G2 sk1prod = group.init(G2_t);
    // G2 sk2prod = group.init(G2_t);
    // G2 sk3prod = group.init(G2_t);

    // G1 ct2prod = group.init(G1_t);
    // G1 ct3prod = group.init(G1_t);
    // G1 ct2prodexp = group.init(G1_t);

    // GT nomprod= group.init(GT_t);
    // GT dnomprod= group.init(GT_t);
    // ZR sumwstar = 0;
    vector<GT> z(numshares);

    for (int i = 0; i < numshares; i++)
    {
        const GmppkePrivateKeyShare & s0 = sk.shares[i];

        // FIXME DO NOT COPY an entire fucking vector for christ sakes 
       // vector<ZR> tagv(hashtags);
        //tagv.push_back(s0.sk4);
        shareTags[shareTags.size()-1] = s0.sk4; 
        //assert(hashtags.size() == pk.d+1);
        //assert(tagv.size()==pk.d+1);
        vector<ZR> w;

        for(uint j=0;j < shareTags.size(); j++){
            w.push_back(LagrangeBasisCoefficients(j,0,shareTags));
      //      cout << "w[" << j << "]= " << w[j] << endl; 
        }
        wstar = w[w.size() - 1];

        G1 ct3prod_j;
        for (int j = 0; j < d; j++)
        {
            ct3prod_j = group.mul(ct3prod_j, group.exp(ct.ct3[j],w[j])); // w[0] = wstar

        }

      //  denom = group.mul(group.pair(ct3prod_j, sk3), group.exp(group.pair(ct2, sk2), wstar));
       GT denom = group.mul(group.pair(ct3prod_j, s0.sk3), group.pair(group.exp(ct.ct2,wstar), s0.sk2));
       GT nom = group.pair(ct.ct2, s0.sk1);
       z[i]=group.div(nom, denom);

        // sk1prod = group.mul(sk1prod,sk1);
        // sk2prod = group.mul(sk2prod,sk2);
        // sk3prod =group.mul(sk3prod,sk3);
        // ct2prod = group.mul(ct2prod,ct2);
        // ct2prodexp = group.mul(ct2prodexp,group.exp(ct2,wstar));
        // ct3prod = group.mul(ct3prod,ct3prod_j);

        // nomprod = group.mul(nomprod,nom);
        // dnomprod = group.mul(dnomprod,denom);
    }

    GT zprod;
#ifndef BENCH
    cout << "\tComputeding zprod . numshares =" << numshares << endl;
#endif
    for (int i = 0; i < numshares; i++)
    {
        zprod = group.mul(zprod, z[i]);
    }
    b =  zprod;

   //  GT nominator = group.pair(ct2prod,sk1prod);
   //  GT denominator = group.mul(group.pair(ct3prod,sk3prod),group.pair(sk2prod,ct2prodexp));
   // assert(nominator == nomprod);

   // b = group.div(nominator,denominator);
    return;
}



std::vector<ZR>  Bbghibe::indexToPath(uint index,uint l){
    std::vector<ZR> path;
    uint nodesSoFar = 0;
    ZR zero = 0;
    for(uint level =0 ; level < l ; level++){
        uint subtree_height = treeSize(l-level-1);
    //    cout << "l " << l << " level " << level  <<" subtee " << (l-level-1) << " size " << subtree_height << endl;
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

uint Bbghibe::pathToIndex(std::vector<ZR> & path, uint l){
    uint index = 0;
    int pathsize = path.size();
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



void Bbghibe::setup(int l, BbhHIBEPublicKey & pk, G2 & msk)
{
    ZR alpha = group.random(ZR_t);
    pk.gG1 = group.random(G1_t);
    pk.gG2 = group.random(G2_t);
    pk.g1 = group.exp(pk.gG2, alpha);
    pk.l = l;
    ZR r = group.random(ZR_t);
    pk.g2G1 = group.exp(pk.gG1, r);
    pk.g2G2 = group.exp(pk.gG2, r);
    ZR r1 = group.random(ZR_t);
    pk.g3G1 = group.exp(pk.gG1, r1);
    pk.g3G2 = group.exp(pk.gG2, r1);
    for (int i = 0; i < l; i++)
    {
        ZR h = group.random(ZR_t);
        pk.hG1.push_back(group.exp(pk.gG1, h));
        pk.hG2.push_back(group.exp(pk.gG2, h));
    }
    msk = group.exp(pk.g2G2, alpha);

    return;
}
void Bbghibe::keygen(BbhHIBEPublicKey & pk, G2 & msk, int ID, int k, BbghPrivatekey & sk){
  //  CharmListZR id = intToBits(ID, k);
   // keygen(pk,msk,id,sk);
}

void Bbghibe::keygen(BbhHIBEPublicKey & pk, G2 & msk, std::vector<ZR> & id, BbghPrivatekey & sk)
{
    ZR r = group.random(ZR_t);    
    int k = id.size();
    for (int i = 0; i < k; i++)
    {
        sk.a0 = group.mul(sk.a0, group.exp(pk.hG2[i], id[i]));
    }
    sk.a0 = group.exp(group.mul(sk.a0, pk.g3G2), r);
    sk.a0 = group.mul(msk, sk.a0);
    sk.a1 = group.exp(pk.gG2, r);

    sk.b.resize(pk.l);
    sk.bG2.resize(pk.l);
    for (int i =  k;i < pk.l; i++)
    {
        sk.b[i] = group.exp(pk.hG1[i], r);

        sk.bG2[i] = group.exp(pk.hG2[i], r);
    }
    return;
}
void Bbghibe::keygen(BbhHIBEPublicKey & pk, BbghPrivatekey & sk, int ID, int k,BbghPrivatekey & skout){
  // CharmListZR id = intToBits(ID, k);
   // keygen(pk,sk,id,skout);
}
void Bbghibe::keygen(BbhHIBEPublicKey & pk, BbghPrivatekey & sk, std::vector<ZR> &id,BbghPrivatekey & skout){

    ZR t;
    G2 hprod;


    int k = id.size();



    t = group.random(ZR_t);

    for (int i = 0; i < k ; i++)
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

    for (int i = k ; i < pk.l; i++)
    {
        skout.b[i] = group.mul(sk.b[i],group.exp(pk.hG1[i],t));
        skout.bG2[i] = group.mul(sk.bG2[i],group.exp(pk.hG2[i],t));

    }
    return;
}

void Bbghibe::encrypt(const BbhHIBEPublicKey & pk, GT & M, int ID, int k, BbghCT & ct){
    // ZR s = group.random(ZR_t);
    // CharmListZR id = intToBits(ID, k);
    // encrypt(pk,M,s,id,ct);
}
void Bbghibe::encrypt(const BbhHIBEPublicKey & pk, GT & M, std::vector<ZR> & id, BbghCT & ct){
    ZR r = group.random(ZR_t);
    encrypt(pk,M,r,id,ct);
}

void Bbghibe::encrypt(const BbhHIBEPublicKey & pk, GT & M, ZR & s, std::vector<ZR> & id, BbghCT & ct)
{

 
    G1 dotProd2 = group.init(G1_t, 1);
    int k = id.size();
    assert(k<=pk.l);

    ct.A = group.mul(group.exp(group.pair(pk.g2G1, pk.g1), s), M);
    ct.B = group.exp(pk.gG1, s);
    group.init(dotProd2, 1);
    for (int i = 0; i < k; i++)
    {
        dotProd2 = group.mul(dotProd2,group.exp(pk.hG1[i], id[i]));
    }
    ct.C = group.exp(group.mul(dotProd2, pk.g3G1), s);
    return;
}

void Bbghibe::decrypt(BbghPrivatekey & sk, BbghCT & ct, GT & b){

    b = group.div(group.pair(ct.C, sk.a1), group.pair(ct.B, sk.a0));
    return;
}
GT Bbghibe::decrypt(BbghPrivatekey & sk, BbghCT & ct)
{

    return group.mul(ct.A, group.div(group.pair(ct.C, sk.a1), group.pair(ct.B, sk.a0)));
}

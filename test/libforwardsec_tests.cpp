#include <string>

#include <vector>
#include <stdexcept>
#include <bitset>
#include <assert.h>
#include <cmath>
#include <cereal/archives/binary.hpp>
#include <cereal/types/bitset.hpp>

#include "gtest/gtest.h"

#include "forwardsec.h"
#include "gmpfse.h"
#include "GMPpke.h"
#include "BBGHibe.h"
#include "Util.h"
using namespace std;
using namespace forwardsec;
using namespace relicxx;
class Gmmppketest : public ::testing::Test {
protected:
	 virtual void SetUp(){
	 	test.keygen(pk,sk,3);
	 }
     PairingGroup group;
	 Gmppke test;
	 GmppkePublicKey pk ;
	 GmppkePrivateKey sk;
};

class PFSETests : public ::testing::Test {
protected: 
	 virtual void SetUp(){
	 	test.keygen(pk,sk);
	 } 
	 unsigned int d=3;
	 PFSETests():test(d){}
 	 //PairingGroup group;
	 GMPfse test;
	 GMPfsePublicKey pk ;
	 GMPfsePrivateKey sk;
	 bytes testkey = {{0x3a, 0x5d, 0x7a, 0x42, 0x44, 0xd3, 0xd8, 0xaf, 0xf5, 0xf3, 0xf1, 0x87, 0x81, 0x82, 0xb2,
						  0x53, 0x57, 0x30, 0x59, 0x75, 0x8d, 0xe6, 0x18, 0x17, 0x14, 0xdf, 0xa5, 0xa4, 0x0b,0x43,0xAD,0xBC}};
	// static PairingGroup _group;
};

class BbghhibeTests : public ::testing::Test {
protected: 
     PairingGroup group;

     Bbghibe test;
     BbhHIBEPublicKey pk;

     std::vector<ZR> id0, id1, id00, id01,
                 id10, id11, id111;

     BbghPrivatekey sk0, sk1, sk00, sk01,
                    sk10, sk11, sk111;

     virtual void SetUp(){
        G2 msk;
        test.setup(3,pk,msk);
        ZR z(0);
        ZR o(1);

        id0.push_back(z);
        id1.push_back(o);

        id00.push_back(z);
        id00.push_back(z);

        id01.push_back(z);
        id01.push_back(o);

        id10.push_back(o);
        id10.push_back(z);

        id11.push_back(o);
        id11.push_back(o);

        test.keygen(pk,msk,id0,sk0);
        test.keygen(pk,msk,id1,sk1);

       test.keygen(pk,sk0,id00,sk00);
       test.keygen(pk,sk0,id01,sk01);

       test.keygen(pk,sk1,id10,sk10);
       test.keygen(pk,sk1,id11,sk11);

       id111.push_back(o);
       id111.push_back(o);
       id111.push_back(o);

       test.keygen(pk,sk11,id111,sk111);
     } 


    // static PairingGroup _group;
};


TEST_F(PFSETests,Decrypt){
    vector<string> tags;
    tags.push_back("9");
    GMPfseCiphertext ct1 = test.encrypt(pk,testkey,1,tags);

    //test.puncture(1,eight);

    bytes result = test.decrypt(pk,sk,ct1);
    ASSERT_EQ(testkey,result);
}

TEST_F(PFSETests,FailWhenPunctured){
    vector<string> tags;
    tags.push_back("9");
    GMPfseCiphertext ct1 = test.encrypt(pk,testkey,1,tags);
    test.puncture(pk,sk,"9");
    //test.puncture(1,eight);
   // test.decrypt(ct1);
   EXPECT_THROW(test.decrypt(pk,sk,ct1),PuncturedCiphertext);
}

TEST_F(PFSETests,DecryptOnPuncture){
    vector<string> tags;
    tags.push_back("9");

    GMPfseCiphertext ct = test.encrypt(pk,testkey,1,tags);

    test.puncture(pk,sk,1,"8");

    bytes result = test.decrypt(pk,sk,ct);

    EXPECT_EQ(testkey,result);

    // test multiple punctures;
    test.puncture(pk,sk,1,"1");
    test.puncture(pk,sk,1,"2");
    test.puncture(pk,sk,1,"3");

    EXPECT_EQ(testkey,test.decrypt(pk,sk,ct));
	test.prepareNextInterval(pk,sk);

    GMPfseCiphertext ct2 = test.encrypt(pk,testkey,2,tags);

    bytes result1 = test.decrypt(pk,sk,ct2);

    EXPECT_EQ(testkey,result1) ;
}

//
TEST_F(PFSETests,PassOnPunctureNextInterval){
	test.prepareNextInterval(pk,sk);

    vector<string> tags;
    tags.push_back("9");

    test.puncture(pk,sk,2,"8");
    test.puncture(pk,sk,2,"10");

	test.prepareNextInterval(pk,sk);
    GMPfseCiphertext ct1 = test.encrypt(pk,testkey,3,tags);

    bytes result = test.decrypt(pk,sk,ct1);
    EXPECT_EQ(testkey,result);

}
TEST_F(PFSETests,PunctureAndDeriveAll){
	// there are 2^d =1 nodes in a tree of depth d.
	// we don't have the root, so we subtrct one more.
	unsigned int intervals = std::pow(2,d+1)-1;
	for(unsigned int i =1;i< intervals; i++){
	    vector<string> tags;
	    tags.push_back("9");
	    test.puncture(pk,sk,i,"8");
	    test.puncture(pk,sk,i,"10");
	    test.puncture(pk,sk,"11");
	    test.puncture(pk,sk,"12");
	    GMPfseCiphertext ct1 = test.encrypt(pk,testkey,i,tags);
	    bytes result = test.decrypt(pk,sk,ct1);
	    EXPECT_EQ(testkey,result);
	    if(i+1 < intervals){
			test.prepareNextInterval(pk,sk);
	    }
	}
}
TEST(Util,IndexToPath){
	EXPECT_EQ(pathToIndex(indexToPath(13,3),3),(unsigned int)13);
	EXPECT_EQ(pathToIndex(indexToPath(14,3),3),(unsigned int)14);

	EXPECT_THROW(indexToPath(15,3),invalid_argument);
}

TEST_F(PFSETests,Delete){

    vector<string> tags;
    tags.push_back("9");

    test.puncture(pk,sk,1,"8");
    test.puncture(pk,sk,1,"10");

    GMPfseCiphertext ct = test.encrypt(pk,testkey,1,tags);

    bytes result = test.decrypt(pk,sk,ct);
    EXPECT_EQ(testkey,result);
    sk.erase(1);
    EXPECT_THROW(test.decrypt(pk,sk,ct),invalid_argument); // no key
    EXPECT_THROW(sk.erase(2),invalid_argument); // no child keys so count delete
}
TEST_F(PFSETests,PunctureWrongInterval){
    EXPECT_THROW( test.puncture(pk,sk,2,"8");,invalid_argument); // can't puncture key we don't have children for.
}
TEST_F(PFSETests,DeriveKeyInFuture){
    GMPfseCiphertext ct1 = test.encrypt(pk,testkey,11,{"1"});
    test.deriveKeyFor(pk,sk,11);
    bytes result = test.decrypt(pk,sk,ct1);
    ASSERT_EQ(testkey,result);
}
TEST_F(PFSETests,DeriveAllKeyInFuture){
	unsigned int intervals = std::pow(2,d+1)-1;
	for(unsigned int i =1;i<intervals;i++){
	GMPfse test1(d);
	GMPfsePublicKey pk1;
	GMPfsePrivateKey sk1;
	test1.keygen(pk1,sk1);
    GMPfseCiphertext ct1 = test1.encrypt(pk1,testkey,i,{"1"});
    test1.deriveKeyFor(pk1,sk1,i);
    bytes result = test1.decrypt(pk1,sk1,ct1);
    ASSERT_EQ(testkey,result);
	}
}

TEST_F(PFSETests,serializePseCipherText){
	std::stringstream ss;
    vector<std::string> tags;
    tags.push_back("2");
    GMPfseCiphertext ctnew, ct = test.encrypt(pk,testkey,1,tags);
	EXPECT_NE(ct,ctnew);
	{
		cereal::BinaryOutputArchive oarchive(ss);
		oarchive(ct);
	}
	//cout << "PseCipherText size " << size << endl;
	{
	    cereal::BinaryInputArchive iarchive(ss); // Create an input archive
	    iarchive(ctnew);
	}
	EXPECT_EQ(ct,ctnew);
}

TEST_F(PFSETests,serializePfsepubkey){
	GMPfsePublicKey pknew;
	std::stringstream ss;
	EXPECT_NE(pk,pknew);
	{
		cereal::BinaryOutputArchive oarchive(ss);
		oarchive(pk);
	}
//	int size = ss.tellp();
//	cout << "pfsepubkey size " << size << endl;
	{
	    cereal::BinaryInputArchive iarchive(ss); // Create an input archive
	    iarchive(pknew);
	}
	EXPECT_EQ(pk,pknew);
}
TEST_F(PFSETests,serializeGmppkePrivateKey){
	std::stringstream ss;
	GMPfsePrivateKey storen;
	EXPECT_NE(sk,storen);
	{
		cereal::BinaryOutputArchive oarchive(ss);
		oarchive(sk);
	}
//	int size = ss.tellp();
//	cout << "serializeBbhHIBEPublicKey size " << size << endl;
	{
	    cereal::BinaryInputArchive iarchive(ss); // Create an input archive
	    iarchive(storen);
	}
	EXPECT_EQ(sk,storen);
}


TEST_F(PFSETests,testSeperateDecryptandSerialize){

    vector<string> tags;
    tags.push_back("9");
    GMPfseCiphertext ct1 = test.encrypt(pk,testkey,1,tags);

    //test.puncture(1,eight);


	std::stringstream ss;
	 GMPfsePublicKey pksender;

	GMPfse testsender(d);

	EXPECT_NE(pk,pksender);
	{
		cereal::BinaryOutputArchive oarchive(ss);
		oarchive(pk);
	}
	{
	    cereal::BinaryInputArchive iarchive(ss); // Create an input archive
	    iarchive(pksender);
	}
	GMPfseCiphertext ctnew,ct = testsender.encrypt(pksender,testkey,1,tags);
	{
		cereal::BinaryOutputArchive oarchive(ss);
	//	oarchive(a);
		oarchive(ct);
	}
	{
	    cereal::BinaryInputArchive iarchive(ss); // Create an input archive
	  //  iarchive(a);
	    iarchive(ctnew);
	}
	EXPECT_EQ(ct,ctnew);
    EXPECT_EQ(testkey,test.decrypt(pk,sk,ctnew));

}






TEST_F(BbghhibeTests,basic){
    GT m = group.randomGT();

    BbghCT ct = test.encrypt(pk,m,id1);
    EXPECT_EQ(m,test.decrypt(sk1, ct));

}


TEST_F(BbghhibeTests,basicFail){
    GT m = group.randomGT();

    BbghCT ct = test.encrypt(pk,m,id1);

    EXPECT_NE(m,test.decrypt(sk0,ct));
    EXPECT_NE(m,test.decrypt(sk11,ct));


}
TEST_F(BbghhibeTests,derived){
    GT m = group.randomGT();


    BbghCT ct = test.encrypt(pk,m,id11);
    EXPECT_EQ(m,test.decrypt(sk11,ct));
    EXPECT_NE(m,test.decrypt(sk00,ct));
    EXPECT_NE(m,test.decrypt(sk10,ct));
    EXPECT_NE(m,test.decrypt(sk01,ct));

}
TEST_F(BbghhibeTests,derivedFurther){
    GT m = group.randomGT();

    BbghCT ct = test.encrypt(pk,m,id111);
    EXPECT_EQ(m,test.decrypt(sk111,ct));
    EXPECT_NE(m,test.decrypt(sk00,ct));
    EXPECT_NE(m,test.decrypt(sk10,ct));
    EXPECT_NE(m,test.decrypt(sk01,ct));

}

TEST_F(BbghhibeTests,serializeBbghCT){
	std::stringstream ss;
    GT m = group.randomGT();

    BbghCT ctnew;
    BbghCT ct = test.encrypt(pk,m,id1);
	EXPECT_NE(ct,ctnew);
	{
		cereal::BinaryOutputArchive oarchive(ss);
		oarchive(ct);
	}
//	int size = ss.tellp();
	//cout << "BbhHIBEPublicKey size " << size << endl;
	{
	    cereal::BinaryInputArchive iarchive(ss); // Create an input archive
	    iarchive(ctnew);
	}
	EXPECT_EQ(ct,ctnew);
}

TEST_F(BbghhibeTests,serializeBbhHIBEPublicKey){
	BbhHIBEPublicKey pknew;
	std::stringstream ss;
	EXPECT_NE(pk,pknew);
	{
		cereal::BinaryOutputArchive oarchive(ss);
		oarchive(pk);
	}
//	int size = ss.tellp();
//	cout << "serializeBbhHIBEPublicKey size " << size << endl;
	{
	    cereal::BinaryInputArchive iarchive(ss); // Create an input archive
	    iarchive(pknew);
	}
	EXPECT_EQ(pk,pknew);
}
TEST_F(BbghhibeTests,serializeBbghPrivatekey){
	BbhHIBEPublicKey pknew;
	std::stringstream ss;
	BbghPrivatekey skn;
	EXPECT_NE(sk0,skn);
	{
		cereal::BinaryOutputArchive oarchive(ss);
		oarchive(sk0);
	}
//	int size = ss.tellp();
//	cout << "serializeBbhHIBEPublicKey size " << size << endl;
	{
	    cereal::BinaryInputArchive iarchive(ss); // Create an input archive
	    iarchive(skn);
	}
	EXPECT_EQ(skn,sk0);
	EXPECT_NE(sk111,skn);

	{
		cereal::BinaryOutputArchive oarchive(ss);
		oarchive(sk111);
	}
	{
	    cereal::BinaryInputArchive iarchive(ss); // Create an input archive
	    iarchive(skn);
	}
	EXPECT_EQ(skn,sk111);
}

TEST_F(BbghhibeTests,testSeperateDecryptandSerialize){
	std::stringstream ss;
	BbhHIBEPublicKey pksender;

	Bbghibe testsender;
	EXPECT_NE(pk,pksender);
	{
		cereal::BinaryOutputArchive oarchive(ss);
		oarchive(pk);
	}
	{
	    cereal::BinaryInputArchive iarchive(ss); // Create an input archive
	    iarchive(pksender);
	}

    GT m = group.randomGT();

    BbghCT ctnew,ct = testsender.encrypt(pksender,m,id1);
	{
		cereal::BinaryOutputArchive oarchive(ss);
		oarchive(ct);
	}
	{
	    cereal::BinaryInputArchive iarchive(ss); // Create an input archive
	    iarchive(ctnew);
	}
    EXPECT_EQ(m,test.decrypt(sk1, ctnew));

}


TEST_F(Gmmppketest,basic){
    GT m = group.randomGT();


    GmmppkeCT ct = test.encrypt(pk,m,{{"1","2","3"}});
    EXPECT_EQ(m,test.decrypt(pk,sk,ct));

}
TEST_F(Gmmppketest,puncture){
    GT m = group.randomGT();
    GmmppkeCT ct = test.encrypt(pk,m,{{"1","2","3"}});
    test.puncture(pk,sk,"4");
    EXPECT_EQ(m,test.decrypt(pk,sk,ct));
    test.puncture(pk,sk,"5");
    test.puncture(pk,sk,"6");
    EXPECT_EQ(m,test.decrypt(pk,sk,ct));
}

TEST_F(Gmmppketest,punctureFailWithPuncturedCiphertext){
    GT m = group.randomGT();
    GmmppkeCT ct = test.encrypt(pk,m,{{"1","2","3"}});
    test.puncture(pk,sk,"2");
    EXPECT_THROW(test.decrypt(pk,sk,ct),PuncturedCiphertext);

    test.puncture(pk,sk,"5");
    test.puncture(pk,sk,"6");
    EXPECT_THROW(test.decrypt(pk,sk,ct),PuncturedCiphertext);
}

// we disable this test when using OPENMP b/c googls
// catch mechanism appears to not work across threads.
#ifndef RELICXX_USE_OPENMP
// checks tha the system actually fails when handed a
TEST_F(Gmmppketest,punctureFail){
    GT m = group.randomGT();
    GmmppkeCT ct = test.encrypt(pk,m,{{"1","2","3"}});
    test.puncture(pk,sk,"2");
    EXPECT_THROW(test.decrypt_unchecked(pk,sk,ct),std::logic_error);
    test.puncture(pk,sk,"5");
    test.puncture(pk,sk,"6");
    EXPECT_THROW(test.decrypt_unchecked(pk,sk,ct),std::logic_error);
}
#endif
TEST_F(Gmmppketest,serializeGmmppkeCT){
	std::stringstream ss;
    GT m = group.randomGT();

    GmmppkeCT ctnew, ct = test.encrypt(pk,m,{{"1","2","3"}});
	EXPECT_NE(ct,ctnew);
	{
		cereal::BinaryOutputArchive oarchive(ss);
		oarchive(ct);
	}
	//cout << "BbghCT size " << size << endl;
	{
	    cereal::BinaryInputArchive iarchive(ss); // Create an input archive
	    iarchive(ctnew);
	}
	EXPECT_EQ(ct,ctnew);
}

TEST_F(Gmmppketest,serializeGmppkePublicKey){
	GmppkePublicKey pknew;
	std::stringstream ss;
	EXPECT_NE(pk,pknew);
	{
		cereal::BinaryOutputArchive oarchive(ss);
		oarchive(pk);
	}
//	int size = ss.tellp();
//	cout << "GmppkePublicKey size " << size << endl;
	{
	    cereal::BinaryInputArchive iarchive(ss); // Create an input archive
	    iarchive(pknew);
	}
	EXPECT_EQ(pk,pknew);
}
TEST_F(Gmmppketest,serializeGmppkePrivateKey){
	std::stringstream ss;
	GmppkePrivateKey skn;
	EXPECT_NE(sk,skn);
	{
		cereal::BinaryOutputArchive oarchive(ss);
		oarchive(sk);
	}
//	int size = ss.tellp();
//	cout << "serializeBbhHIBEPublicKey size " << size << endl;
	{
	    cereal::BinaryInputArchive iarchive(ss); // Create an input archive
	    iarchive(skn);
	}
	EXPECT_EQ(sk,skn);
}

TEST_F(Gmmppketest,testSeperateDecryptandSerialize){
	std::stringstream ss;
	GmppkePublicKey pksender;

	Gmppke testsender;

	EXPECT_NE(pk,pksender);
	{
		cereal::BinaryOutputArchive oarchive(ss);
		oarchive(pk);
	}
	{
	    cereal::BinaryInputArchive iarchive(ss); // Create an input archive
	    iarchive(pksender);
	}

    GT m = group.randomGT();

    GmmppkeCT ctnew,ct = testsender.encrypt(pk,m,{{"1","2","3"}});
	{
		cereal::BinaryOutputArchive oarchive(ss);
		oarchive(ct);
	}
	{
	    cereal::BinaryInputArchive iarchive(ss); // Create an input archive
	    iarchive(ctnew);
	}
    EXPECT_EQ(m,test.decrypt(pk,sk,ctnew));

}

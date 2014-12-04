#include "gtest/gtest.h"
#include "relic_wrapper/relic_api.h"

class pg : public ::testing::Test {
protected: 
	 virtual void SetUp() {int a = 1+1;}
	 PairingGroup group;
};


TEST_F(pg,G1Identity){
	G1 a,b,c;
	G1 d = group.random(G1_t);
	ASSERT_EQ(a,b);
	ASSERT_EQ(a+b,c);
	ASSERT_NE(a,d);
	ASSERT_EQ(a+d,d);
}

TEST_F(pg,G2Identity){
	G2 a,b,c;
	G2 d = group.random(G2_t);
	ASSERT_EQ(a,b);
	ASSERT_EQ(a+b,c);
	ASSERT_NE(a,d);
	ASSERT_EQ(a+d,d);
}


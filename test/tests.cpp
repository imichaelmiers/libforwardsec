#include "gtest/gtest.h"
#include "relic_wrapper/relic_api.h"

class pg : public ::testing::Test {
protected: 
	 virtual void SetUp() {int a = 1+1;}
	 PairingGroup group;
};

#define randabcd(T) 		\
T a = group.random( T##_t );\
T b = group.random( T##_t); \
T c = group.random( T##_t); \
T d = group.random( T##_t); \
T e = group.random( T##_t); \
T i;

TEST_F(pg,G1Identity){
	G1 a,b,c;
	G1 d = group.random(G1_t);
	ASSERT_EQ(a,b);
	ASSERT_EQ(a+b,c);
	ASSERT_NE(a,d);
	ASSERT_EQ(a+d,d);
}

TEST_F(pg,G1Comp){
	G1 a = group.random(G1_t);
	G1 b = group.random(G1_t);
	G1 c = group.random(G1_t);
	EXPECT_NE(a,b);
	ASSERT_EQ(a,a);
	ASSERT_EQ(b,b);

}

TEST_F(pg,G2Comp){
	G2 a = group.random(G2_t);
	G2 b = group.random(G2_t);
	G2 c = group.random(G2_t);
	EXPECT_NE(a,b);
	ASSERT_EQ(a,a);
	ASSERT_EQ(b,b);


}
TEST_F(pg,GTComp){
	GT a = group.random(GT_t);
	GT b = group.random(GT_t);
	GT c = group.random(GT_t);
	EXPECT_NE(a,b);
	ASSERT_EQ(a,a);
	ASSERT_EQ(b,b);

}

TEST_F(pg,G2Identity){
	G2 a,b,c;
	G2 d = group.random(G2_t);
	EXPECT_EQ(a,b);
	EXPECT_EQ(a+b,c);
	ASSERT_NE(a,d);
	EXPECT_EQ(a+d,d);
}


TEST_F(pg,GTIdentity){
	GT a,b,c;
	GT d = group.random(GT_t);
	EXPECT_EQ(a,b);
	EXPECT_EQ(a*b,c);
	EXPECT_NE(a,d);
	EXPECT_EQ(a*d,d);
}

TEST_F(pg,G1Add){
	randabcd(G1)
	EXPECT_EQ(a+b,b+a) << "not commutative";
	EXPECT_EQ(a+i,a) << "identity is wrong";
	d= a+b;
	d= d+c;
	e= b+c;
	e = e+a;
	EXPECT_EQ(d,e) << "not associative";
}
TEST_F(pg,G2Add){
	randabcd(G2)
	EXPECT_EQ(a+b,b+a) << "not commutative";
	EXPECT_EQ(a+i,a) << "identity is wrong";
}
TEST_F(pg,GTAdd){
	randabcd(GT)
	EXPECT_EQ(a*b,b*a) << "not commutative";
	EXPECT_EQ(a*i,a) << "identity is wrong";
}



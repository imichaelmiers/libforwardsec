#include <stdio.h>
#include <relic/relic.h>
#include <relic/relic_test.h>
#include <relic/relic_bench.h>
/*============================================================================*/
/* Public definitions                                                         */
/*============================================================================*/


int acp_bls_gen(bn_t d, g2_t q) {
	bn_t n;
	int result = STS_OK;

	bn_null(n);

	TRY {
		bn_new(n);

		g2_get_ord(n);

		do {
			bn_rand(d, BN_POS, bn_bits(n));
			bn_mod(d, d, n);
		} while (bn_is_zero(d));

		g2_mul_gen(q, d);
	}
	CATCH_ANY {
		result = STS_ERR;
	}
	FINALLY {
		bn_free(n);
	}
	return result;
}

int acp_bls_sig(g1_t s, uint8_t *msg, int len, bn_t d) {
	g1_t p;
	int result = STS_OK;

	g1_null(p);

	TRY {
		g1_new(p);
		g1_map(p, msg, len);
		g1_mul(s, p, d);
	}
	CATCH_ANY {
		result = STS_ERR;
	}
	FINALLY {
		g1_free(p);
	}
	return result;
}

int acp_bls_ver(g1_t s, uint8_t *msg, int len, g2_t q) {
	g1_t p;
	g2_t g;
	gt_t e1, e2;
	int result = 0;

	g1_null(p);
	g2_null(g);
	gt_null(e1);
	gt_null(e2);

	TRY {
		g1_new(p);
		g2_new(g);
		gt_new(e1);
		gt_new(e2);

		g2_get_gen(g);

		g1_map(p, msg, len);
		pc_map(e1, p, q);
		pc_map(e2, s, g);

		if (gt_cmp(e1, e2) == CMP_EQ) {
			result = 1;
		}
	}
	CATCH_ANY {
		THROW(ERR_CAUGHT);
	}
	FINALLY {
		g1_free(p);
		g2_free(g);
		gt_free(e1);
		gt_free(e2);
	}
	return result;
}
int abls(void) {
	int code = STS_ERR;
	bn_t d;
	g1_t s;
	g2_t q;
	uint8_t m[5] = { 0, 1, 2, 3, 4 };

	bn_null(d);
	g1_null(s);
	g2_null(q);

	TRY {
		bn_new(d);
		g1_new(s);
		g2_new(q);

		TEST_BEGIN("boneh-lynn-schacham short signature is correct") {
			TEST_ASSERT(acp_bls_gen(d, q) == STS_OK, end);
			TEST_ASSERT(acp_bls_sig(s, m, sizeof(m), d) == STS_OK, end);
			TEST_ASSERT(acp_bls_ver(s, m, sizeof(m), q) == 1, end);
		}
		TEST_END;
	}
	CATCH_ANY {
		ERROR(end);
	}
	code = STS_OK;

  end:
	bn_free(d);
	g1_free(s);
	g2_free(q);
	return code;
}

#include <stdio.h>
#include <relic/relic.h>
#include <relic/relic_test.h>
#include "bls.h"

int main(void) {
	if (core_init() != STS_OK) {
		core_clean();
		return 1;
	}
    if (pc_param_set_any() == STS_OK){
	util_banner("Tests for the CP module", 0);
		if (abls() != STS_OK) {
			core_clean();
			return 1;
			printf("failure\n");
		}
		printf("sucess\n");
	}
	return 1;
	core_clean();
}
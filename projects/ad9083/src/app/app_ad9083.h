#ifndef __APP_AD9083_H
#define __APP_AD9083_H

#include <stdint.h>
#include <stdbool.h>
#include "ad9083.h"

struct app_ad9083_init{
	uint8_t uc;
};

struct app_ad9083{
	struct ad9083_phy *ad9083_phy;
};

int32_t app_ad9083_init(struct app_ad9083 **app, struct app_ad9083_init *init_param);

bool app_ad9083_check_sysref_rate(uint32_t lmfc, uint32_t sysref);

#endif /* __APP_AD9083_H */

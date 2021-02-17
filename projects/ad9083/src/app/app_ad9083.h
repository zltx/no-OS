#ifndef __APP_AD9083_H
#define __APP_AD9083_H

#include <stdint.h>
#include <stdbool.h>
#include "ad9083.h"

struct app_ad9083_init{
	uint8_t uc;
	struct clk *jesd_rx_clk;
};

struct app_ad9083{
	struct ad9083_phy *ad9083_phy;
};

int32_t app_ad9083_init(struct app_ad9083 **app, struct app_ad9083_init *init_param);

bool app_ad9083_check_sysref_rate(uint32_t lmfc, uint32_t sysref);
int32_t app_ad9083_status(struct app_ad9083 *app);

#endif /* __APP_AD9083_H */

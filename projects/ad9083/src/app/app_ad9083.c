#include <stdlib.h>
#include "inttypes.h"
#include "app_ad9083.h"
#include "ad9083.h"
#include "parameters.h"
#include "spi.h"
#include "spi_extra.h"
#include "gpio_extra.h"
#include "error.h"

bool app_ad9083_check_sysref_rate(uint32_t lmfc, uint32_t sysref)
{
	uint32_t div, mod;

	div = lmfc / sysref;
	mod = lmfc % sysref;

	/* Ignore minor deviations that can be introduced by rounding. */
	return mod <= div || mod >= sysref - div;
}

int32_t app_ad9083_status(struct app_ad9083 *app)
{

	int32_t ret;
	uint16_t status;

	ret = adi_ad9083_jesd_tx_link_status_get(&app->ad9083_phy->ad9083, &status);
	if (ret != SUCCESS)
		return FAILURE;

	if (status != 0x7d && status != 0x3d)
		return FAILURE;

	return SUCCESS;
}

int32_t app_ad9083_init(struct app_ad9083 **app, struct app_ad9083_init *init_param)
{
	int32_t status;
	struct app_ad9083 *app_ad9083;
	struct xil_spi_init_param xil_spi_param = {
		.type = SPI_PS,
		.device_id = 0,
	};

	// clock chip spi settings
	struct spi_init_param ad9083_spi_init_param = {
		.max_speed_hz = 10000000,
		.mode = SPI_MODE_0,
		.chip_select = SPI_AD9083_CS,

		.platform_ops = &xil_platform_ops,
		.extra = &xil_spi_param
	};
	struct xil_gpio_init_param  xil_gpio_param = {
		.type = GPIO_PS,
		.device_id = GPIO_DEVICE_ID
	};
	struct gpio_init_param	gpio_phy_resetb = {
		.number = AD9803_RSTB,
		.platform_ops = &xil_gpio_platform_ops,
		.extra = &xil_gpio_param
	};
	struct gpio_init_param	gpio_phy_pd = {
		.number = AD9803_PWDN,
		.platform_ops = &xil_gpio_platform_ops,
		.extra = &xil_gpio_param
	};

	struct ad9083_init_param ad9083_init_param = {
		.spi_init = &ad9083_spi_init_param,
		.gpio_reset = &gpio_phy_resetb,
		.gpio_pd = &gpio_phy_pd,
		.uc = init_param->uc,
		.jesd_rx_clk = init_param->jesd_rx_clk,
	};

	app_ad9083 = (struct app_ad9083 *)calloc(1, sizeof(*app_ad9083));
	if (!app_ad9083)
		return FAILURE;

	status = ad9083_init(&app_ad9083->ad9083_phy, &ad9083_init_param);
	if (status != SUCCESS)
		printf("ad9083_initialize() error: %" PRId32 "\n", status);

	*app = app_ad9083;

	return SUCCESS;
}

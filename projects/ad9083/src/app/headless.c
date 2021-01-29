/**
 * \file adrv9009/src/app/headless.c
 *
 * \brief Contains example code for user integration with their application
 *
 * Copyright 2015-2017 Analog Devices Inc.
 * Released under the AD9378-AD9379 API license, for more information see the "LICENSE.txt" file in this zip file.
 *
 */

/****< Insert User Includes Here >***/

#include <stdio.h>
#include "ad9083.h"
//#include "adi_hal.h"
#include "inttypes.h"
#include "spi.h"
#include "spi_extra.h"
#include "gpio_extra.h"
#include "error.h"
#include "delay.h"
#include "parameters.h"
#include "util.h"
#include "axi_jesd204_tx.h"
#include "axi_adc_core.h"
#include "axi_dmac.h"
#ifndef ALTERA_PLATFORM
#include "xil_cache.h"
#endif
//#include "talise.h"
//#include "talise_config.h"
#include "app_config.h"
#include "app_clocking.h"
#include "app_jesd.h"
#include "app_ad9083.h"
#include "ad9528.h"

#ifdef IIO_SUPPORT
#include "app_iio.h"
#endif

extern struct axi_jesd204_rx *rx_jesd;
int main(void)
{
	uint8_t uc = 7;
	int32_t status;
	struct axi_adc_init rx_adc_init = {
		.name = "rx_adc",
		.base = RX_CORE_BASEADDR,
		.num_channels = 16,
	};
	struct axi_adc *rx_adc;

	struct axi_dmac_init rx_dmac_init = {
		"rx_dmac",
		RX_DMA_BASEADDR,
		DMA_DEV_TO_MEM,
		0
	};
	struct axi_dmac *rx_dmac;


	printf("Hello\n");

	status = app_clocking_init(uc);
	if (status != SUCCESS) {
		printf("app_clock_init() error: %" PRId32 "\n", status);

		return FAILURE;
	}

	status = app_jesd_init(uc);
	if (status != SUCCESS) {
		printf("app_jesd_init() error: %" PRId32 "\n", status);

		return FAILURE;
	}

	status = app_ad9083_init(uc);
	if (status != SUCCESS) {
		printf("app_clock_init() error: %" PRId32 "\n", status);

		return FAILURE;
	}

	status = axi_jesd204_rx_status_read(rx_jesd);
	if (status != SUCCESS) {
		printf("axi_jesd204_tx_status_read() error: %"PRIi32"\n", status);
	}

	axi_adc_init(&rx_adc, &rx_adc_init);

	axi_dmac_init(&rx_dmac, &rx_dmac_init);
#ifdef IIO_SUPPORT
	printf("The board accepts libiio clients connections through the serial backend.\n");

	struct iio_axi_adc_init_param iio_axi_adc_init_par;
	iio_axi_adc_init_par = (struct iio_axi_adc_init_param) {
		.rx_adc = rx_adc,
		.rx_dmac = rx_dmac,
	};


	return iio_server_init(&iio_axi_adc_init_par);
#else
	printf("Bye\n");

	return SUCCESS;
#endif
}

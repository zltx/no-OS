/***************************************************************************//**
 *   @file   ad9083.c
 *   @brief  Implementation of ad9083 Driver.
 *   @author Cristian Pop (cristian.pop@analog.com)
********************************************************************************
 * Copyright 2021(c) Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  - Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  - Neither the name of Analog Devices, Inc. nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *  - The use of this software may or may not infringe the patent rights
 *    of one or more patent holders.  This license does not release you
 *    from the requirement that you obtain separate licenses from these
 *    patent holders to use this software.
 *  - Use of the software either in source or binary form, must be run
 *    on or directly connected to an Analog Devices Inc. component.
 *
 * THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, NON-INFRINGEMENT,
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ANALOG DEVICES BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, INTELLECTUAL PROPERTY RIGHTS, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include "ad9083.h"
#include "error.h"
#include <inttypes.h>
#include "adi_ad9083_hal.h"

extern uint64_t clk_hz[][3];
extern uint32_t vmax[];
extern uint32_t fc[];
extern uint8_t  rterm[];
extern uint32_t en_hp[];
extern uint32_t backoff[];
extern uint32_t finmax[];
extern uint64_t nco_freq_hz[][3];
extern uint8_t  decimation[][4];
extern uint8_t  nco0_datapath_mode[];
extern adi_cms_jesd_param_t jtx_param[];
extern int32_t adi_ad9083_jtx_startup(adi_ad9083_device_t *device,
                               adi_cms_jesd_param_t *jtx_param);

#define CHIPID_AD9083	0x00EA
#define CHIPID_MASK	0xFFFF

/**
 * Spi write and read compatible with ad9083 API
 * @param user_data
 * @param wbuf Pointer to array with the data to be sent on the SPI
 * @param rbuf Pointer to array where the data to which the SPI will be written
 * @param len The size in bytes allocated for each of the indata and outdata arrays.
 * @return 0 for success, any non-zero value indicates an error
 */
static int32_t ad9083_spi_xfer(void *user_data, uint8_t *in_data,
			       uint8_t *out_data, uint32_t size_bytes)
{
	struct ad9083_phy *phy = user_data;
	uint8_t data[6];
	uint16_t bytes_number;
	int32_t ret;
	int32_t i;

	bytes_number = (size_bytes & 0xFF);

	for (i = 0; i < bytes_number; i++)
		data[i] =  in_data[i];

	ret = spi_write_and_read(phy->spi_desc, data, bytes_number);
	if (ret != SUCCESS)
		return FAILURE;

	for (i = 0; i < bytes_number; i++)
		out_data[i] =  data[i];

	return SUCCESS;
}

int32_t ad9083_log_write(void *user_data, int32_t log_type, const char *message,
			 va_list argp)
{
	char logMessage[160];

	vsnprintf(logMessage, sizeof(logMessage), message, argp);

	switch (log_type) {
	case ADI_CMS_LOG_NONE:
		break;
	case ADI_CMS_LOG_MSG:
		break;
	case ADI_CMS_LOG_WARN:
		printf("%s\n", logMessage);
		break;
	case ADI_CMS_LOG_ERR:
		printf("%s\n", logMessage);
		break;
	case ADI_CMS_LOG_SPI:
		break;
	case ADI_CMS_LOG_API:
		break;
	case ADI_CMS_LOG_ALL:
		printf(logMessage);
		break;
	}

	return 0;
}

/**
 * Delay microseconds, compatible with ad9083 API
 * Performs a blocking or sleep delay for the specified time in microseconds
 * @param user_data
 * @param us - time to delay/sleep in microseconds.
 */
static int ad9083_udelay(void *user_data, unsigned int us)
{
	udelay(us);

	return 0;
}

int32_t ad9083_reset_pin_ctrl(void *user_data, uint8_t enable)
{
	struct ad9083_phy *phy = user_data;

	return gpio_set_value(phy->gpio_reset, enable);
}

static int32_t ad9083_setup(struct ad9083_phy *phy, uint8_t uc)
{
	int32_t ret;

	ret = adi_ad9083_device_clock_config_set(&phy->ad9083, clk_hz[uc][2], clk_hz[uc][0]);
	if (ret != 0)
		return ret;

	ret = adi_ad9083_rx_adc_config_set(&phy->ad9083, vmax[uc], fc[uc], rterm[uc], en_hp[uc], backoff[uc], finmax[uc]);
	if (ret != 0)
		return ret;

	ret = adi_ad9083_rx_datapath_config_set(&phy->ad9083, nco0_datapath_mode[uc], decimation[uc], nco_freq_hz[uc]);
	if (ret != 0)
		return ret;

	ret = adi_ad9083_jtx_startup(&phy->ad9083, &jtx_param[uc]);
	if (ret != 0)
		return ret;

	return SUCCESS;
}

/**
 * Initialize the device.
 * @param device - The device structure.
 * @param init_param - The structure that contains the device initial
 * 		       parameters.
 * @return SUCCESS in case of success, negative error code otherwise.
 */
int32_t ad9083_init(struct ad9083_phy **device, struct ad9083_init_param *init_param)
{
	adi_cms_chip_id_t chip_id;
	struct ad9083_phy *phy;
	uint8_t api_rev[3];
	int32_t ret = 0;

	phy = (struct ad9083_phy *)calloc(1, sizeof(*phy));
	if (!phy)
		return -ENOMEM;

	ret = gpio_get_optional(&phy->gpio_reset, init_param->gpio_reset);
	if (ret < 0)
		goto error_1;
	ret = gpio_get_optional(&phy->gpio_pd, init_param->gpio_pd);
	if (ret < 0)
		goto error_1;

	gpio_direction_output(phy->gpio_reset, GPIO_HIGH);

	/* SPI */
	ret = spi_init(&phy->spi_desc, init_param->spi_init);
	if (ret < 0)
		goto error_2;

	phy->ad9083.hal_info.user_data = phy;
	phy->ad9083.hal_info.delay_us = ad9083_udelay;
	phy->ad9083.hal_info.reset_pin_ctrl = ad9083_reset_pin_ctrl;
	phy->ad9083.hal_info.sdo = SPI_SDIO;
	phy->ad9083.hal_info.msb = SPI_MSB_FIRST;
	phy->ad9083.hal_info.addr_inc = SPI_ADDR_INC_AUTO;
	phy->ad9083.hal_info.spi_xfer = ad9083_spi_xfer;
	phy->ad9083.hal_info.log_write = ad9083_log_write;

	if (ret < 0)
		goto error_3;
	/* software reset, resistor is not mounted */
	ret = adi_ad9083_device_reset(&phy->ad9083, AD9083_HARD_RESET);
	if (ret < 0) {
		printf("%s: reset/init failed (%"PRId32")\n", __func__, ret);
		goto error_3;
	}

	ret = adi_ad9083_device_chip_id_get(&phy->ad9083, &chip_id);
	if (ret < 0) {
		printf("%s: chip_id failed (%"PRId32")\n", __func__, ret);
		goto error_3;
	}

	if ((chip_id.prod_id & CHIPID_MASK) != CHIPID_AD9083) {
		printf("%s: Unrecognized CHIP_ID 0x%X\n", __func__,
		       chip_id.prod_id);
		ret = FAILURE;
		goto error_3;
	}

	ret = ad9083_setup(phy, init_param->uc);
	if (ret < 0) {
		printf("%s: ad9081_setup failed (%"PRId32")\n", __func__, ret);
		goto error_3;
	}

	adi_ad9083_device_api_revision_get(&phy->ad9083, &api_rev[0],
					   &api_rev[1], &api_rev[2]);

	printf("AD9083 Rev. %u Grade %u (API %u.%u.%u) probed\n",
	       chip_id.dev_revision, chip_id.prod_grade,
	       api_rev[0], api_rev[1], api_rev[2]);

	return SUCCESS;

error_3:
	spi_remove(phy->spi_desc);
error_2:
	gpio_remove(phy->gpio_reset);
error_1:
	if (phy)
		free(phy);

	return ret;
}

int32_t ad9083_remove(struct ad9083_phy *dev)
{
	int32_t ret = 0;

	spi_remove(dev->spi_desc);
	gpio_remove(dev->gpio_pd);
	gpio_remove(dev->gpio_ref_sel);
	gpio_remove(dev->gpio_reset);

	if (dev)
		free(dev);

	return ret;
}

/***************************************************************************//**
 *   @file   iio_dac_demo.c
 *   @brief  Source file of DAC Demo iio.
 *   @author RNechita (ramona.nechita@analog.com)
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


#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "iio_dac_demo.h"
#include "iio_types.h"
#include "error.h"

static struct scan_type dac_scan_type = {
	.sign = 's',
	.realbits = 16,
	.storagebits = 16,
	.shift = 0,
	.is_big_endian = false
};

struct iio_channel iio_dac_channels[MAX_NR_CHANNELS];

struct iio_attribute dac_channel_attributes[] = {
	DAC_DEMO_ATTR("dac_channel_attr", DAC_CHANNEL_ATTR),
	END_ATTRIBUTES_ARRAY,
};

struct iio_attribute dac_global_attributes[] = {
	DAC_DEMO_ATTR("dac_global_attr", DAC_GLOBAL_ATTR),
	END_ATTRIBUTES_ARRAY,
};

/*****************************************************************//**
 * @brief initialize the device number of channels with the number of
 * 			channels in the init_param
 * @param desc - dac dev descriptor
 * @param mask - unused, for compliance with prepare_transfer
 * 				function format
 * @return SUCCESS in case of success.
*********************************************************************/
int32_t init_dac_channels(struct dac_demo_desc* desc, uint32_t mask)
{
	uint32_t i;

	if(!desc)
		return -ENODEV;

	for(i = 0; i < mask; i++) {
		char buff[12];
		sprintf(buff,"da_channel%"PRIu32"",i);
		struct iio_channel ch = {
			.name = buff,
			.ch_type = IIO_VOLTAGE,
			.channel = i,
			.scan_index = i,
			.indexed = true,
			.scan_type = &dac_scan_type,
			.attributes = dac_channel_attributes,
			.ch_out = true
		};
		iio_dac_channels[i] = ch;
	}

	return SUCCESS;
}

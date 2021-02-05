/***************************************************************************//**
 *   @file   demo_dev.c
 *   @brief  Implementation of iio_demo.c.
 *   @author Cristian Pop (cristian.pop@analog.com)
 *   @author Mihail Chindris (mihail.chindris@analog.com)
********************************************************************************
 * Copyright 2020(c) Analog Devices, Inc.
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

#include "iio_ad9083.h"

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include "error.h"
#include "util.h"
#include "ad9083.h"

enum iio_demo_attributes {
	DEMO_CHANNEL_ATTR,
	DEMO_GLOBAL_ATTR,
};

#define IIO_DEMO_ATTR(_name, _priv) {\
	.name = _name,\
	.priv = _priv,\
	.show = get_demo_attr,\
	.store = set_demo_attr\
}

static struct iio_attribute demo_channel_attributes[] = {
	IIO_DEMO_ATTR("demo_channel_attr", DEMO_CHANNEL_ATTR),
	END_ATTRIBUTES_ARRAY,
};

static struct iio_attribute iio_demo_global_attributes[] = {
	IIO_DEMO_ATTR("demo_global_attr", DEMO_GLOBAL_ATTR),
	END_ATTRIBUTES_ARRAY,
};

static struct scan_type scan_type = {
	.sign = 's',
	.realbits = 16,
	.storagebits = 16,
	.shift = 0,
	.is_big_endian = false
};

static struct iio_channel iio_demo_channels_in[] = {
	{
		.name = "input_channel_0",
		.ch_type = IIO_VOLTAGE,
		.channel = 0,
		.scan_index = 0,
		.indexed = true,
		.scan_type = &scan_type,
		.attributes = demo_channel_attributes,
		.ch_out = false,
	},
	{
		.name = "input_channel_1",
		.ch_type = IIO_VOLTAGE,
		.channel = 1,
		.scan_index = 1,
		.indexed = true,
		.scan_type = &scan_type,
		.attributes = demo_channel_attributes,
		.ch_out = false,
	},
};


static struct iio_device iio_demo_dev_in_descriptor = {
	.num_ch = DEMO_NUM_CHANNELS,
	.channels = iio_demo_channels_in,
	.attributes = iio_demo_global_attributes,
	.debug_attributes = NULL,
	.buffer_attributes = NULL,
	.prepare_transfer = NULL,
	.end_transfer = NULL,
	.read_dev = NULL,
	.debug_reg_read = (int32_t (*)())adi_ad9083_reg_get,
	.debug_reg_write = (int32_t (*)())adi_ad9083_reg_set,
};

/** Local channel for loopback */
uint16_t local_ch[DEMO_NUM_CHANNELS][NB_LOCAL_SAMPLES];

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

int32_t iio_demo_reg_write(struct iio_demo_desc *desc, uint32_t reg,
			   uint32_t writeval)
{
	if (!desc || reg > MAX_REG_ADDR)
		return -EINVAL;

	desc->dummy_regs[reg] = writeval;

	return SUCCESS;
}

int32_t iio_demo_reg_read(struct iio_demo_desc *desc, uint32_t reg,
			  uint32_t *readval)
{
	if (!desc || reg > MAX_REG_ADDR || !readval)
		return -EINVAL;

	*readval = desc->dummy_regs[reg];

	return SUCCESS;
}

/**
 * @brief get_demo_attr().
 * @param device- Physical instance of a iio_demo_device.
 * @param buf - Where value is stored.
 * @param len - Maximum length of value to be stored in buf.
 * @param channel - Channel properties.
 * @param priv - Attribute ID
 * @return Length of chars written in buf, or negative value on failure.
 */
ssize_t get_demo_attr(void *device, char *buf, size_t len,
		      const struct iio_ch_info *channel, intptr_t priv)
{
	struct iio_demo_desc *desc = device;

	if (channel) {
		if (priv == DEMO_CHANNEL_ATTR)
			return snprintf(buf, len, "%"PRIu32"",
					desc->dev_ch_attr);
	} else {
		if (priv == DEMO_GLOBAL_ATTR)
			return snprintf(buf, len, "%"PRIu32"",
					desc->dev_global_attr);
	}

	return -EINVAL;
}

/**
 * @brief set_demo_attr().
 * @param device - Physical instance of a iio_demo_device.
 * @param buf - Value to be written to attribute.
 * @param len -	Length of the data in "buf".
 * @param channel - Channel properties.
 * @param priv - Attribute ID
 * @return: Number of bytes written to device, or negative value on failure.
 */
ssize_t set_demo_attr(void *device, char *buf, size_t len,
		      const struct iio_ch_info *channel, intptr_t priv)
{
	struct iio_demo_desc *desc = device;
	uint32_t val = srt_to_uint32(buf);

	if (channel) {
		if (priv == DEMO_CHANNEL_ATTR) {
			desc->dev_ch_attr = val;
			return len;
		}
	} else {
		if (priv == DEMO_GLOBAL_ATTR) {
			desc->dev_global_attr = val;
			return len;
		}
	}

	return -EINVAL;

}

struct iio_demo_init_param iio_demo_in_init_par = {
	.dev_global_attr = 1100,
	.dev_ch_attr = 1111,
};

/**
 * @brief Get iio device descriptor.
 * @param desc - Descriptor.
 * @param dev_descriptor - iio device descriptor.
 */
void iio_demo_get_dev_descriptor(struct iio_device **dev_descriptor)
{
	*dev_descriptor = &iio_demo_dev_in_descriptor;
}
/**
 * @brief iio demo init function, registers a demo .
 * @param desc - Descriptor.
 * @param init - Configuration structure.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t iio_demo_dev_init(struct iio_demo_desc **desc,
			  struct iio_demo_init_param *init)
{
	struct iio_demo_desc *ldesc;

	ldesc = (struct iio_demo_desc*)calloc(1, sizeof(*ldesc));
	if (!ldesc)
		return -ENOMEM;

	ldesc->dev_global_attr = iio_demo_in_init_par.dev_global_attr;
	ldesc->dev_ch_attr = iio_demo_in_init_par.dev_ch_attr;
	ldesc->ch_mask = 0;

	*desc = ldesc;

	return SUCCESS;
}

/**
 * @brief Release resources.
 * @param desc - Descriptor.
 * @return SUCCESS in case of success, FAILURE otherwise.
 */
int32_t iio_demo_dev_remove(struct iio_demo_desc *desc)
{
	if (!desc)
		return FAILURE;

	free(desc);

	return SUCCESS;
}

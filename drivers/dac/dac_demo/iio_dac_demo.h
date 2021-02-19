/***************************************************************************//**
 *   @file   iio_dac_demo.h
 *   @brief  Header file of DAC Demo iio.
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

#ifndef IIO_DEMO_DAC
#define IIO_DEMO_DAC

#include <stdlib.h>
#include "dac_demo.h"

#define MAX_NR_CHANNELS 10
#define DAC_DEMO_ATTR(_name, _priv) {\
	.name = _name,\
	.priv = _priv,\
	.show = get_dac_demo_attr,\
	.store = set_dac_demo_attr\
}

extern struct iio_channel iio_dac_channels[MAX_NR_CHANNELS];

#define DAC_DEMO_DEV(_numch) {\
	.num_ch = _numch, \
	.channels = iio_dac_channels, \
	.attributes = dac_global_attributes,	\
	.debug_attributes = NULL,	\
	.buffer_attributes = NULL,	\
	.prepare_transfer = update_dac_channels,	\
	.end_transfer = close_dac_channels,	\
	.read_dev = (int32_t (*)())dac_write_samples,	\
	.debug_reg_read = (int32_t (*)()) dac_demo_reg_read,	\
	.debug_reg_write = (int32_t (*)()) dac_demo_reg_write	\
}

enum iio_dac_demo_attributes {
	DAC_CHANNEL_ATTR,
	DAC_GLOBAL_ATTR,
};

extern struct iio_attribute dac_channel_attributes[];

extern struct iio_attribute dac_global_attributes[];

//extern struct iio_device dac_demo_iio_descriptor;

int32_t init_dac_channels(struct dac_demo_desc* desc, uint32_t mask);

/*struct iio_device dac_demo_iio_descriptor = {
		.num_ch = 2,
		.attributes = dac_global_attributes,
		.debug_attributes = NULL,
		.buffer_attributes = NULL,
		.prepare_transfer = init_dac_channels,
		.end_transfer = close_dac_channels,
		.read_dev = (int32_t (*)())dac_write_samples,
		.debug_reg_read = (int32_t (*)()) dac_demo_reg_read,
		.debug_reg_write = (int32_t (*)()) dac_demo_reg_write
};*/


#endif /* IIO_DEMO_DAC */

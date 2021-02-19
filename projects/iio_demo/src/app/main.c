/***************************************************************************//**
 *   @file   iio_demo/src/app/main.c
 *   @brief  Implementation of Main Function.
 *   @author Cristian Pop (cristian.pop@analog.com)
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

#include "demo_dev.h"
#include "iio_demo_dev.h"
#include "app_config.h"
#include "parameters.h"
#include "iio_app.h"
#include "iio_adc_demo.h"
#include "iio_dac_demo.h"

#ifdef XILINX_PLATFORM
#include <xparameters.h>
#include <xil_cache.h>
#include <xil_cache.h>
#endif // XILINX_PLATFORM

#ifdef ADUCM_PLATFORM
#include <sys/platform.h>
#include "adi_initialize.h"
#include <drivers/pwr/adi_pwr.h>
#endif

#ifdef STM32_PLATFORM
#include "stm32_hal.h"
#endif

#include "error.h"

#if defined(ADUCM_PLATFORM) || defined(STM32_PLATFORM)

#define MAX_SIZE_BASE_ADDR	(NB_LOCAL_SAMPLES * DEMO_NUM_CHANNELS *\
					sizeof(uint16_t))

static uint8_t in_buff[MAX_SIZE_BASE_ADDR];
static uint8_t out_buff[MAX_SIZE_BASE_ADDR];

#define DAC_DDR_BASEADDR	((uint32_t)out_buff)
#define ADC_DDR_BASEADDR	((uint32_t)in_buff)

#endif

#ifdef STM32_PLATFORM
void Error_Handler(void)
{
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
	}
}

void SystemClock_Config(void)
{
	RCC_OscInitTypeDef RCC_OscInitStruct = {0};
	RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

	/** Configure the main internal regulator output voltage
	*/
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);
	/** Initializes the RCC Oscillators according to the specified parameters
	* in the RCC_OscInitTypeDef structure.
	*/
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
	RCC_OscInitStruct.PLL.PLLM = 16;
	RCC_OscInitStruct.PLL.PLLN = 336;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
	RCC_OscInitStruct.PLL.PLLQ = 2;
	RCC_OscInitStruct.PLL.PLLR = 2;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}
	/** Initializes the CPU, AHB and APB buses clocks
	*/
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
				      |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
		Error_Handler();
	}
}
#endif

int32_t platform_init()
{
#if defined(ADUCM_PLATFORM)
	if (ADI_PWR_SUCCESS != adi_pwr_Init())
		return FAILURE;

	if (ADI_PWR_SUCCESS != adi_pwr_SetClockDivider(ADI_CLOCK_HCLK, 1u))
		return FAILURE;

	if (ADI_PWR_SUCCESS != adi_pwr_SetClockDivider(ADI_CLOCK_PCLK, 1u))
		return FAILURE;
	adi_initComponents();
#elif defined(STM32_PLATFORM)
	HAL_Init();
	SystemClock_Config();
#endif
	return 0;
}

//uint16_t buf[2][1000];
uint16_t buf[TEST_CHANNEL_NO][MAX_SAMPLES_PER_CHANNEL];

/***************************************************************************//**
 * @brief main
*******************************************************************************/
int main(void)
{
	int32_t status;

	/* iio demo configurations. */
	struct iio_demo_init_param iio_demo_in_init_par;

	/* iio demo configurations. */
	struct iio_demo_init_param iio_demo_out_init_par;

	/* adc demo configurations. */
	struct adc_demo_init_param adc_init_par;

	/* dac demo configurations. */
	struct dac_demo_init_param dac_init_par;

	/* iio instance descriptor. */
	struct iio_demo_desc *iio_demo_in_desc;

	/* iio instance descriptor. */
	struct iio_demo_desc *iio_demo_out_desc;

	/* adc instance descriptor. */
	struct adc_demo_desc *adc_desc;

	/* dac instance descriptor. */
	struct dac_demo_desc *dac_desc;

	status = platform_init();
	if (status != SUCCESS)
		return status;

	iio_demo_out_init_par = (struct iio_demo_init_param) {
		.dev_global_attr = 1100,
		.dev_ch_attr = 1111,
	};
	status = iio_demo_dev_init(&iio_demo_out_desc, &iio_demo_out_init_par);
	if (status < SUCCESS)
		return status;

	iio_demo_in_init_par = (struct iio_demo_init_param) {
		.dev_global_attr = 2200,
		.dev_ch_attr = 2211,
	};
	status = iio_demo_dev_init(&iio_demo_in_desc, &iio_demo_in_init_par);
	if (status != SUCCESS)
		return status;

	struct iio_data_buffer rd_buf = {
		.buff = (void *)ADC_DDR_BASEADDR,
		.size = MAX_SIZE_BASE_ADDR
	};
	struct iio_data_buffer wr_buf = {
		.buff = (void *)DAC_DDR_BASEADDR,
		.size = MAX_SIZE_BASE_ADDR
	};

	struct iio_data_buffer adc_buff = {
		.buff = (void *)ADC_DDR_BASEADDR,
		.size = MAX_SIZE_BASE_ADDR
	};

	struct iio_data_buffer dac_buff = {
		.buff = (void *)DAC_DDR_BASEADDR,
		.size = MAX_SIZE_BASE_ADDR
	};

	adc_init_par = (struct adc_demo_init_param) {
		.loopback_buffers = NULL,
		.channel_no = 2,
		.dev_global_attr = 3300,
		.dev_ch_attr = 3311
	};
	status = adc_demo_init(&adc_desc, &adc_init_par);
	if (status != SUCCESS)
		return status;

	dac_init_par = (struct dac_demo_init_param) {
		.loopback_buffers = NULL,
		.channel_no = 2,
		.dev_global_attr = 4400,
		.dev_ch_attr = 4411
	};
	status = dac_demo_init(&dac_desc, &dac_init_par);
	if (status != SUCCESS)
		return status;

	status = init_adc_channels(adc_desc,adc_desc->active_ch);
	if(status != SUCCESS)
		return status;
	struct iio_device adc_demo_iio_descriptor = ADC_DEMO_DEV(adc_desc->active_ch);

	status = init_dac_channels(dac_desc,dac_desc->active_ch);
	if(status != SUCCESS)
		return status;
	struct iio_device dac_demo_iio_descriptor = DAC_DEMO_DEV(dac_desc->active_ch);

	struct iio_app_device devices[] = {
		IIO_APP_DEVICE("demo_device_output", iio_demo_out_desc,
			       &iio_demo_dev_out_descriptor,NULL, &wr_buf),
		IIO_APP_DEVICE("demo_device_input", iio_demo_in_desc,
			       &iio_demo_dev_in_descriptor,&rd_buf, NULL),
		IIO_APP_DEVICE("adc_demo", adc_desc,
			       &adc_demo_iio_descriptor,&adc_buff, NULL),
		IIO_APP_DEVICE("dac_demo", dac_desc,
			       &dac_demo_iio_descriptor,NULL, &dac_buff)
	};

	return iio_app_run(devices, 4);
}

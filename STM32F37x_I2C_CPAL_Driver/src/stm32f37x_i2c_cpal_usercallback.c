/**
  ******************************************************************************
  * @file    stm32f37x_i2c_cpal_usercallback.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    20-September-2012
  * @brief   This file provides all the CPAL UserCallback functions.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f37x_i2c_cpal.h"

/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
GPIO_TypeDef* const I2C_SCL_GPIO_PORT[2] = {CPAL_I2C1_SCL_GPIO_PORT,CPAL_I2C2_SCL_GPIO_PORT};
extern const uint16_t CPAL_I2C_SCL_GPIO_PIN[];

GPIO_TypeDef* const I2C_SDA_GPIO_PORT[2] = {CPAL_I2C1_SDA_GPIO_PORT,CPAL_I2C2_SDA_GPIO_PORT};
extern const uint16_t CPAL_I2C_SDA_GPIO_PIN[];

/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

static void delay_us(uint32_t delay_us)
{
	uint32_t nb_loop;
	nb_loop = (8 * delay_us) + 1; /* uS (divide by 4 because each loop take about 4 cycles including nop +1 is here to avoid delay of 0 */
	asm volatile(
					"1: " "\n\t"
					" nop " "\n\t"
					" subs.w %0, %0, #1 " "\n\t"
					" bne 1b " "\n\t"
					: "=r" (nb_loop)
					: "0"(nb_loop)
					: "r3"
				);
}

static void i2c_bus_reset(CPAL_DevTypeDef Device)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;

	GPIO_InitStructure.GPIO_Pin = CPAL_I2C_SCL_GPIO_PIN[Device];
	GPIO_Init((GPIO_TypeDef*)I2C_SCL_GPIO_PORT[Device], &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = CPAL_I2C_SDA_GPIO_PIN[Device];
	GPIO_Init((GPIO_TypeDef*)I2C_SDA_GPIO_PORT[Device], &GPIO_InitStructure);

    /* Release both lines */
	I2C_SCL_GPIO_PORT[Device]->BSRR = CPAL_I2C_SCL_GPIO_PIN[Device];
	I2C_SDA_GPIO_PORT[Device]->BSRR = CPAL_I2C_SDA_GPIO_PIN[Device];

    /*
     * Make sure the bus is free by clocking it until any slaves release the
     * bus.
     */
	while (!((I2C_SDA_GPIO_PORT[Device]->IDR) & CPAL_I2C_SDA_GPIO_PIN[Device])) {
        /* Wait for any clock stretching to finish */
		while (!((I2C_SCL_GPIO_PORT[Device]->IDR) & CPAL_I2C_SCL_GPIO_PIN[Device]))
			;
		delay_us(10);

		/* pull low */
		I2C_SCL_GPIO_PORT[Device]->BRR = CPAL_I2C_SCL_GPIO_PIN[Device];
		delay_us(10);

		/* release high again */
		I2C_SCL_GPIO_PORT[Device]->BSRR = CPAL_I2C_SCL_GPIO_PIN[Device];
		delay_us(10);
	}

    /* generate start then stop condition */
	I2C_SDA_GPIO_PORT[Device]->BRR = CPAL_I2C_SDA_GPIO_PIN[Device];
	delay_us(10);
	I2C_SCL_GPIO_PORT[Device]->BRR = CPAL_I2C_SCL_GPIO_PIN[Device];
	delay_us(10);
	I2C_SCL_GPIO_PORT[Device]->BSRR = CPAL_I2C_SDA_GPIO_PIN[Device];
	delay_us(10);
	I2C_SDA_GPIO_PORT[Device]->BSRR = CPAL_I2C_SCL_GPIO_PIN[Device];

	CPAL_I2C_HAL_GPIOInit(Device);
}


/*------------------------------------------------------------------------------
                     CPAL User Callbacks implementations 
------------------------------------------------------------------------------*/


/*=========== Timeout UserCallback ===========*/


/**
  * @brief  User callback that manages the Timeout error
  * @param  pDevInitStruct
  * @retval None.
  */
uint32_t CPAL_TIMEOUT_UserCallback(CPAL_InitTypeDef* pDevInitStruct)
{
	/* handle i2c bus lockup */
	i2c_bus_reset(pDevInitStruct->CPAL_Dev);

	return CPAL_FAIL;
}


/*=========== Transfer UserCallback ===========*/


/**
  * @brief  Manages the End of Tx transfer event
  * @param  pDevInitStruct 
  * @retval None
  */
/*void CPAL_I2C_TXTC_UserCallback(CPAL_InitTypeDef* pDevInitStruct)
{

}*/


/**
  * @brief  Manages the End of Rx transfer event
  * @param  pDevInitStruct
  * @retval None
  */
/*void CPAL_I2C_RXTC_UserCallback(CPAL_InitTypeDef* pDevInitStruct)
{

}*/


/**
  * @brief  Manages Tx transfer event
  * @param  pDevInitStruct
  * @retval None
  */
/*void CPAL_I2C_TX_UserCallback(CPAL_InitTypeDef* pDevInitStruct)
{

}*/


/**
  * @brief  Manages Rx transfer event
  * @param  pDevInitStruct
  * @retval None
  */
/*void CPAL_I2C_RX_UserCallback(CPAL_InitTypeDef* pDevInitStruct)
{

}*/


/**
  * @brief  Manages the End of DMA Tx transfer event
  * @param  pDevInitStruct
  * @retval None
  */
/*void CPAL_I2C_DMATXTC_UserCallback(CPAL_InitTypeDef* pDevInitStruct)
{
    // Update CPAL_State
	pDevInitStruct->CPAL_State = CPAL_STATE_READY;
	pDevInitStruct->wCPAL_DevError = CPAL_I2C_ERR_NONE;

}*/


/**
  * @brief  Manages the Half of DMA Tx transfer event
  * @param  pDevInitStruct
  * @retval None
  */
/*void CPAL_I2C_DMATXHT_UserCallback(CPAL_InitTypeDef* pDevInitStruct)
{

}*/


/**
  * @brief  Manages Error of DMA Tx transfer event
  * @param  pDevInitStruct
  * @retval None
  */
/*void CPAL_I2C_DMATXTE_UserCallback(CPAL_InitTypeDef* pDevInitStruct)
{

}*/


/**
  * @brief  Manages the End of DMA Rx transfer event
  * @param  pDevInitStruct
  * @retval None
  */
/*void CPAL_I2C_DMARXTC_UserCallback(CPAL_InitTypeDef* pDevInitStruct)
{
    // Update CPAL_State
	pDevInitStruct->CPAL_State = CPAL_STATE_READY;
	pDevInitStruct->wCPAL_DevError = CPAL_I2C_ERR_NONE;
}*/


/**
  * @brief  Manages the Half of DMA Rx transfer event
  * @param  pDevInitStruct
  * @retval None
  */
/*void CPAL_I2C_DMARXHT_UserCallback(CPAL_InitTypeDef* pDevInitStruct)
{

}*/


/**
  * @brief  Manages Error of DMA Rx transfer event
  * @param  pDevInitStruct
  * @retval None
  */
/*void CPAL_I2C_DMARXTE_UserCallback(CPAL_InitTypeDef* pDevInitStruct)
{

}*/


/*=========== Error UserCallback ===========*/


/**
  * @brief  User callback that manages the I2C device errors
  * @note   Make sure that the define USE_SINGLE_ERROR_CALLBACK is uncommented in
  *         the cpal_conf.h file, otherwise this callback will not be functional
  * @param  pDevInitStruct
  * @param  DeviceError
  * @retval None
  */
/*void CPAL_I2C_ERR_UserCallback(CPAL_DevTypeDef pDevInstance, uint32_t DeviceError)
{

}*/


/**
  * @brief  User callback that manages BERR I2C device errors
  * @note   Make sure that the define USE_MULTIPLE_ERROR_CALLBACK is uncommented in
  *         the cpal_conf.h file, otherwise this callback will not be functional
  * @param  pDevInstance
  * @retval None
  */
/*void CPAL_I2C_BERR_UserCallback(CPAL_DevTypeDef pDevInstance)
{

}*/


/**
  * @brief  User callback that manages ARLO I2C device errors
  * @note   Make sure that the define USE_MULTIPLE_ERROR_CALLBACK is uncommented in
  *         the cpal_conf.h file, otherwise this callback will not be functional
  * @param  pDevInstance
  * @retval None
  */
/*void CPAL_I2C_ARLO_UserCallback(CPAL_DevTypeDef pDevInstance)
{

}*/


/**
  * @brief  User callback that manages OVR I2C device errors
  * @note   Make sure that the define USE_MULTIPLE_ERROR_CALLBACK is uncommented in
  *         the cpal_conf.h file, otherwise this callback will not be functional
  * @param  pDevInstance
  * @retval None
  */
/*void CPAL_I2C_OVR_UserCallback(CPAL_DevTypeDef pDevInstance)
{

}*/


/**
  * @brief  User callback that manages AF I2C device errors.
  * @note   Make sure that the define USE_MULTIPLE_ERROR_CALLBACK is uncommented in
  *         the cpal_conf.h file, otherwise this callback will not be functional
  * @param  pDevInstance
  * @retval None
  */
/*void CPAL_I2C_AF_UserCallback(CPAL_DevTypeDef pDevInstance)
{

}*/


/*=========== Addressing Mode UserCallback ===========*/


/**
  * @brief  User callback that manage General Call Addressing mode
  * @param  pDevInitStruct
  * @retval None
  */
/*void CPAL_I2C_GENCALL_UserCallback(CPAL_InitTypeDef* pDevInitStruct)
{

}*/


/**
  * @brief  User callback that manage Dual Address Addressing mode
  * @param  pDevInitStruct
  * @retval None
  */
/*void CPAL_I2C_DUALF_UserCallback(CPAL_InitTypeDef* pDevInitStruct)
{

}*/


/*=========== Listen Mode UserCallback ===========*/


/**
  * @brief  User callback that manage slave read operation.
  * @param  pDevInitStruct 
  * @retval None
  */ 
/*void CPAL_I2C_SLAVE_READ_UserCallback(CPAL_InitTypeDef* pDevInitStruct)
{
}*/


/**
  * @brief  User callback that manage slave write operation.
  * @param  pDevInitStruct 
  * @retval None
  */  
/*void CPAL_I2C_SLAVE_WRITE_UserCallback(CPAL_InitTypeDef* pDevInitStruct)
{  
}*/

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

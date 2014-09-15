/**
  ******************************************************************************
  * @file    stm32f37x_i2c_cpal_conf.h
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    20-September-2012
  * @brief   Library configuration file
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32F37X_I2C_CPAL_CONF_H
#define __STM32F37X_I2C_CPAL_CONF_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

/*=======================================================================================================================================
                                       CPAL Firmware Functionality Configuration
=========================================================================================================================================*/

/*-----------------------------------------------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------------------------------*/

/*   -- Section 1 :                   **** I2Cx Device Selection ****

    Description: This section provide an easy way to select I2Cx devices in user application.
                 Choosing device allows to save memory resources.
                 If you need I2C1 device, uncomment relative define: #define CPAL_USE_I2C1.
                 All available I2Cx device can be used at the same time.
                 At least one I2C device should be selected.*/

#define CPAL_USE_I2C1          /*<! Uncomment to use I2C1 device */
#define CPAL_USE_I2C2          /*<! Uncomment to use I2C2 device */

/*-----------------------------------------------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------------------------------*/

/*  -- Section 2 :                **** Transfer Options Configuration ****

    Description: This section allows user to enable/disable some Transfer Options. The benefits of these
                 defines is to minimize the size of the source code */

/* Enable the use of Master Mode */
#define CPAL_I2C_MASTER_MODE

/* Enable the use of Slave Mode */
//#define CPAL_I2C_SLAVE_MODE

/* Enable Listen mode for slave device */
//#define CPAL_I2C_LISTEN_MODE

/* Enable the use of DMA Programming Model */
#define CPAL_I2C_DMA_PROGMODEL

/* Enable the use of IT Programming Model */
//#define CPAL_I2C_IT_PROGMODEL

/* !!!! These following defines are available only when CPAL_I2C_MASTER_MODE is enabled !!!! */

/* Enable the use of 10Bit Addressing Mode */
//#define CPAL_I2C_10BIT_ADDR_MODE

/* Enable the use of Memory Addressing Mode */
#define CPAL_I2C_MEM_ADDR

/* Enable the use of 16Bit Address memory register option */
//#define CPAL_16BIT_REG_OPTION


/*------------------------------------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------------------------------------------*/

/*  -- Section 3 :           **** UserCallbacks Selection and Configuration ****

    Description: This section provides an easy way to enable UserCallbacks and select type of Error UserCallbacks.
                 By default, All UserCallbacks are disabled (UserCallbacks are defined as void functions).
                 To implement a UserCallbacks in your application, comment the relative define and
                 implement the callback body in your application file.*/


/* Error UserCallbacks Type : Uncomment to select UserCallbacks type. One type must be selected */
/* Note : if Error UserCallbacks are not used the two following defines must be commented

   WARNING: These two defines are EXCLUSIVE, only one define should be uncommented !
 */
//#define USE_SINGLE_ERROR_CALLBACK   /*<! select single UserCallbacks type */
//#define USE_MULTIPLE_ERROR_CALLBACK /*<! select multiple UserCallbacks type */

/* Error UserCallbacks : To use an Error UserCallback comment the relative define */

/* Single Error Callback */
#define CPAL_I2C_ERR_UserCallback       (void)

/* Multiple Error Callback */
#define CPAL_I2C_BERR_UserCallback      (void)
#define CPAL_I2C_ARLO_UserCallback      (void)
#define CPAL_I2C_OVR_UserCallback       (void)
#define CPAL_I2C_AF_UserCallback        (void)

/* Transfer UserCallbacks : To use a Transfer callback comment the relative define */
#define CPAL_I2C_TX_UserCallback        (void)
#define CPAL_I2C_RX_UserCallback        (void)
#define CPAL_I2C_TXTC_UserCallback      (void)
#define CPAL_I2C_RXTC_UserCallback      (void)

/* DMA Transfer UserCallbacks : To use a DMA Transfer UserCallbacks comment the relative define */
#define CPAL_I2C_DMATXTC_UserCallback   (void)
#define CPAL_I2C_DMATXHT_UserCallback   (void)
#define CPAL_I2C_DMATXTE_UserCallback   (void)
#define CPAL_I2C_DMARXTC_UserCallback   (void)
#define CPAL_I2C_DMARXHT_UserCallback   (void)
#define CPAL_I2C_DMARXTE_UserCallback   (void)

/* Address Mode UserCallbacks : To use an Address Mode UserCallbacks comment the relative define */
#define CPAL_I2C_GENCALL_UserCallback   (void)
#define CPAL_I2C_DUALF_UserCallback     (void)

/* Listen mode Callback : Used to handle communication in listen mode */
#define CPAL_I2C_SLAVE_READ_UserCallback        (void)
#define CPAL_I2C_SLAVE_WRITE_UserCallback       (void)

/*------------------------------------------------------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------------------------------------------------------------*/

/*  -- Section 4 :         **** Configure Timeout method, TimeoutCallback ****

    Description: This section allows you to implement your own Timeout Procedure.
                 By default Timeout procedure is implemented with Systick timer and
                 CPAL_I2C_TIMEOUT_Manager is defined as SysTick_Handler.
                 */
static inline void timer18_config(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

    RCC->APB1ENR |= RCC_APB1Periph_TIM18;

	TIM_TimeBaseStructure.TIM_Period = 5000 - 1;
	TIM_TimeBaseStructure.TIM_Prescaler = SystemCoreClock / 1000000 - 1;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM18, &TIM_TimeBaseStructure);
	TIM18->CR1 |= TIM_CR1_ARPE;

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	NVIC_InitStructure.NVIC_IRQChannel = TIM18_DAC2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	TIM18->SR = (uint16_t)~TIM_IT_Update;
	TIM18->DIER |= TIM_IT_Update;

	TIM18->CR1 |= TIM_CR1_CEN;
}



#define _CPAL_TIMEOUT_INIT()			timer18_config()

#define _CPAL_TIMEOUT_DEINIT()        	TIM18->CR1 &= (uint16_t)(~((uint16_t)TIM_CR1_CEN))


#define CPAL_I2C_TIMEOUT_Manager       	TIM18_DAC2_IRQHandler         /*<! This callback is used to handle Timeout error.
                                                                     When a timeout occurs CPAL_TIMEOUT_UserCallback
                                                                     is called to handle this error */

#ifndef CPAL_I2C_TIMEOUT_Manager
   void CPAL_I2C_TIMEOUT_Manager(void);
#else   
   void TIM18_DAC2_IRQHandler(void);
#endif /* CPAL_I2C_TIMEOUT_Manager */

/*#define CPAL_TIMEOUT_UserCallback        (void)                  *//*<! Comment this line and implement the callback body in your
                                                                      application in order to use the Timeout Callback.
                                                                      It is strongly advised to implement this callback, since it
                                                                      is the only way to manage timeout errors.*/

/* Maximum Timeout values for each communication operation (preferably, Time base should be 1 Millisecond).
   The exact maximum value is the sum of event timeout value and the CPAL_I2C_TIMEOUT_MIN value defined below */
#define CPAL_I2C_TIMEOUT_TC             2
#define CPAL_I2C_TIMEOUT_TCR            2
#define CPAL_I2C_TIMEOUT_TXIS           2
#define CPAL_I2C_TIMEOUT_BUSY           2
#define CPAL_I2C_TIMEOUT_TXE			2

/* DO NOT MODIFY THESE VALUES ---------------------------------------------------------*/
#define CPAL_I2C_TIMEOUT_DEFAULT        ((uint32_t)0xFFFFFFFF)
#define CPAL_I2C_TIMEOUT_MIN            ((uint32_t)0x00000001)
#define CPAL_I2C_TIMEOUT_DETECTED       ((uint32_t)0x00000000)

/*-----------------------------------------------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------------------------------*/

/*   -- Section 5 :             **** NVIC Priority Group Selection and Interrupt Priority Offset ****
  
  Description: This section allows user to select NVIC Priority Group and configure Interrupt Priority Offset.
               To change CPAL_NVIC_PRIOGROUP uncomment wanted Priority Group and comment others. Only one
               define is possible.
               By default Priority Offset of I2Cx device (ERR, EVT, DMA) are set to 0 */
               
/* !!!! Note : With STM32F37X family NVIC Priority Group Selection is not used !!!! */

/*-----------NVIC Group Priority-------------*/

/* #define CPAL_NVIC_PRIOGROUP      NVIC_PriorityGroup_0 */ /*!< 0 bits for preemption priority
                                                                       4 bits for subpriority */

/* #define CPAL_NVIC_PRIOGROUP      NVIC_PriorityGroup_1 */ /*!< 1 bits for preemption priority
                                                                       3 bits for subpriority */

#define CPAL_NVIC_PRIOGROUP      NVIC_PriorityGroup_2  /*!< 2 bits for preemption priority
                                                                       2 bits for subpriority */

/* #define CPAL_NVIC_PRIOGROUP       NVIC_PriorityGroup_3 */ /*!< 3 bits for preemption priority
                                                                       1 bits for subpriority */

/* #define CPAL_NVIC_PRIOGROUP       NVIC_PriorityGroup_4 */ /*!< 4 bits for preemption priority */

/*-----------Interrupt Priority Offset-------------*/

/* This defines can be used to decrease the Level of Interrupt Priority for I2Cx Device (ERR, EVT, DMA_TX, DMA_RX).
   The value of I2Cx_IT_OFFSET_SUBPRIO is added to I2Cx_IT_XXX_SUBPRIO and the value of I2Cx_IT_OFFSET_PREPRIO
   is added to I2Cx_IT_XXX_PREPRIO (XXX: ERR, EVT, DMATX, DMARX).
   I2Cx Interrupt Priority are defined in stm32f37x_i2c_cpal_hal.h file in Section 3 */

#define I2C1_IT_OFFSET_SUBPRIO          0      /* I2C1 SUB-PRIORITY Offset */
#define I2C1_IT_OFFSET_PREPRIO          1      /* I2C1 PREEMPTION PRIORITY Offset */

#define I2C2_IT_OFFSET_SUBPRIO          0      /* I2C2 SUB-PRIORITY Offset */
#define I2C2_IT_OFFSET_PREPRIO          1      /* I2C2 PREEMPTION PRIORITY Offset */

/*-----------------------------------------------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------------------------------*/

/*  -- Section 6 :                  **** CPAL DEBUG Configuration ****

    Description: This section allow user to enable or disable CPAL Debug option. Enabling this option provide
                 to user an easy way to debug the application code. This option use CPAL_LOG Macro that integrate
                 printf function. User can retarget printf function to USART ( use hyperterminal), LCD Screen
                 on ST Eval Board or development toolchain debugger.
                 In this example, the log is managed through printf function routed to USART peripheral and allowing
                 to display messages on Hyperterminal-like terminals. This is performed through redefining the
                 function PUTCHAR_PROTOTYPE (depending on the compiler) as follows:

                   #ifdef __GNUC__
                // With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
                // set to 'Yes') calls __io_putchar() 
                    #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
                   #else
                    #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
                   #endif 

    WARNING      Be aware that enabling this feature may slow down the communication process, increase the code size
                 significantly, and may in some cases cause communication errors (when print/display mechanism is too slow)*/


/* To Enable CPAL_DEBUG Option Uncomment the define below */
//#define CPAL_DEBUG

#ifdef CPAL_DEBUG
#define CPAL_LOG(Str)                   printf(Str)
#include <stdio.h>                     /* This header file must be included when using CPAL_DEBUG option */
#else
#define CPAL_LOG(Str)                   ((void)0)
#endif /* CPAL_DEBUG */


/*-----------------------------------------------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------------------------------*/

/*********END OF CPAL Firmware Functionality Configuration****************************************************************/

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

#ifdef __cplusplus
}
#endif

#endif /* __STM32F37X_I2C_CPAL_CONF_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

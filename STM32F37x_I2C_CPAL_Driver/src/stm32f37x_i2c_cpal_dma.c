/**
  ******************************************************************************
  * @file    stm32f37x_i2c_cpal.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    20-September-2012
  * @brief   This file provides all the CPAL firmware functions for I2C peripheral.
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

/* Private macro -------------------------------------------------------------*/


/* This macro allows to test on a flag status and to start Timeout procedure if the
   waiting time exceeds the allowed timeout period.
   @note This macro has not been implemented as a function because the entered parameter
   'cmd' itself can be a macro (if it was implemented as a function, the check on the
   flag would be done only once, while the required behavior is to check the flag
   continuously).*/

#define __CPAL_I2C_TIMEOUT_DETECT                (pDevInitStruct->wCPAL_Timeout == CPAL_I2C_TIMEOUT_MIN)

#define __CPAL_I2C_TIMEOUT(cmd, timeout)         __disable_irq();\
												 pDevInitStruct->wCPAL_Timeout = CPAL_I2C_TIMEOUT_MIN + (timeout);\
												 TIM18->EGR = TIM_PSCReloadMode_Immediate;\
												 TIM18->SR &= (uint16_t)~TIM_IT_Update;\
												 NVIC_ClearPendingIRQ(TIM18_DAC2_IRQn);\
												 __enable_irq();\
                                                 while (((cmd) == 0) && (!__CPAL_I2C_TIMEOUT_DETECT))\
                                                 if (__CPAL_I2C_TIMEOUT_DETECT)\
                                                 {\
                                                   return CPAL_I2C_Timeout (pDevInitStruct); \
                                                 }\
                                                 __disable_irq();\
                                                 pDevInitStruct->wCPAL_Timeout = CPAL_I2C_TIMEOUT_DEFAULT;\
                                                 __enable_irq()


/* Private variables ---------------------------------------------------------*/

/*========= Table Exported from HAL =========*/

extern I2C_TypeDef* CPAL_I2C_DEVICE[];

#ifdef CPAL_I2C_DMA_PROGMODEL
extern DMA_TypeDef* CPAL_I2C_DMA[];

extern DMA_Channel_TypeDef* CPAL_I2C_DMA_TX_Channel[];
extern DMA_Channel_TypeDef* CPAL_I2C_DMA_RX_Channel[];

extern const uint32_t CPAL_I2C_DMA_TX_TC_FLAG[];
extern const uint32_t CPAL_I2C_DMA_RX_TC_FLAG[];

extern const uint32_t CPAL_I2C_DMA_TX_HT_FLAG[];
extern const uint32_t CPAL_I2C_DMA_RX_HT_FLAG[];

extern const uint32_t CPAL_I2C_DMA_TX_TE_FLAG[];
extern const uint32_t CPAL_I2C_DMA_RX_TE_FLAG[];
#endif /* CPAL_I2C_DMA_PROGMODEL */

/*========= Local structures used in CPAL_I2C_StructInit() function ==========*/

I2C_InitTypeDef I2C_InitStructure;

__IO uint32_t Num_Data = 0;
uint32_t CR2_tmp = 0;

/* Private function prototypes -----------------------------------------------*/

/*========= Local Master events handlers =========*/
static uint32_t I2C_MASTER_TCR_Handle(CPAL_InitTypeDef* pDevInitStruct);    /* Handle Master TCR Interrupt event */
static uint32_t I2C_MASTER_TC_Handle(CPAL_InitTypeDef* pDevInitStruct);     /* Handle Master TC Interrupt event */
static uint32_t I2C_MASTER_STOP_Handle(CPAL_InitTypeDef* pDevInitStruct);   /* Handle Master STOP Interrupt event */
static uint32_t I2C_MASTER_NACK_Handle(CPAL_InitTypeDef* pDevInitStruct);   /* Handle Master NACK Interrupt event */

/*========= Local DMA Manager =========*/
static uint32_t I2C_Enable_DMA (CPAL_InitTypeDef* pDevInitStruct, CPAL_DirectionTypeDef Direction);

/*========= CPAL Timeout handler =========*/
static uint32_t CPAL_I2C_Timeout (CPAL_InitTypeDef* pDevInitStruct);


/* Private functions ---------------------------------------------------------*/

/*================== USER CPAL Functions ==================*/


/**
  * @brief  Initialize the peripheral and all related clocks, GPIOs, DMA and
  *         Interrupts according to the specified parameters in the
  *         CPAL_InitTypeDef structure.
  * @param  pDevInitStruct : Pointer to the peripheral configuration structure.
  * @retval CPAL_PASS or CPAL_FAIL
  */
uint32_t CPAL_I2C_Init(CPAL_InitTypeDef* pDevInitStruct)
{
  /* If CPAL_State is not BUSY */
  if ((pDevInitStruct->CPAL_State == CPAL_STATE_READY)
      || (pDevInitStruct->CPAL_State == CPAL_STATE_ERROR)
      || (pDevInitStruct->CPAL_State == CPAL_STATE_DISABLED))
  {
    /* 
    - If CPAL_State is CPAL_STATE_ERROR (an Error occurred in transaction):
      Perform the initialization routines (device will be deinitialized during initialization).
    - If CPAL_State is CPAL_STATE_READY:
      Perform the initialization routines 
    - If CPAL_State is CPAL_STATE_DISABLED:
      Perform the Initialization routines
    */
   
    /* Disable I2Cx device */
    __CPAL_I2C_HAL_DISABLE_DEV(pDevInitStruct->CPAL_Dev);
    /* Deinitialize I2Cx GPIO */
    CPAL_I2C_HAL_GPIODeInit(pDevInitStruct->CPAL_Dev);
    /* Deinitialize I2Cx clock */
    CPAL_I2C_HAL_CLKDeInit(pDevInitStruct->CPAL_Dev);
    /* Deinitialize DMA */
    CPAL_I2C_HAL_DMADeInit(pDevInitStruct->CPAL_Dev, pDevInitStruct->CPAL_Direction);
    /* Initialize I2Cx GPIO */
    CPAL_I2C_HAL_GPIOInit(pDevInitStruct->CPAL_Dev);
    /* Initialize I2Cx clock */
    CPAL_I2C_HAL_CLKInit(pDevInitStruct->CPAL_Dev);
    /* Initialize I2Cx device with parameters stored in pCPAL_I2C_Struct */
    I2C_Init(CPAL_I2C_DEVICE[pDevInitStruct->CPAL_Dev], pDevInitStruct->pCPAL_I2C_Struct);
    /* Select automatic end mode */
	CR2_tmp |= I2C_CR2_AUTOEND;
	/* Disable WakeUp from STOP mode */
	__CPAL_I2C_HAL_DISABLE_WAKEUP(pDevInitStruct->CPAL_Dev);

    /*--------------------------------------------------------------------------
    DMA Initialization :
    ---------------------------------------------------------------------------*/
	/* Initialize I2Cx DMA channels */
	CPAL_I2C_HAL_DMAInit(pDevInitStruct->CPAL_Dev, pDevInitStruct->CPAL_Direction, pDevInitStruct->wCPAL_Options);

    /*--------------------------------------------------------------------------
    Peripheral and DMA interrupts Initialization
    ---------------------------------------------------------------------------*/
    /* Initialize I2Cx interrupts */
    CPAL_I2C_HAL_ITInit(pDevInitStruct->CPAL_Dev, pDevInitStruct->wCPAL_Options, pDevInitStruct->CPAL_Direction, pDevInitStruct->CPAL_ProgModel);

    /* Update CPAL_State to CPAL_STATE_READY */
    pDevInitStruct->CPAL_State = CPAL_STATE_READY;

    /* Initialize Timeout procedure */
    _CPAL_TIMEOUT_INIT();

    return CPAL_PASS;
  }
  /* If CPAL_State is BUSY (a transaction is still on going) exit Init function */
  else
  {
    return CPAL_FAIL;
  }
}


/**
  * @brief  Deinitialize the peripheral and all related clocks, GPIOs, DMA and NVIC 
  *         to their reset values.
  * @param  pDevInitStruct: Pointer to the peripheral configuration structure.
  * @retval CPAL_PASS or CPAL_FAIL
  * @note   The Peripheral clock is disabled but the GPIO Ports clocks remains 
  *         enabled after this deinitialization. 
  */
uint32_t CPAL_I2C_DeInit(CPAL_InitTypeDef* pDevInitStruct)
{
  /* If CPAL_State is not BUSY */
  if ((pDevInitStruct->CPAL_State == CPAL_STATE_READY)
      || (pDevInitStruct->CPAL_State == CPAL_STATE_ERROR)
      || (pDevInitStruct->CPAL_State == CPAL_STATE_DISABLED))
  {
    /* 
    - If CPAL_State is CPAL_STATE_ERROR (an Error occurred in transaction):
      Perform the deinitialization routines
    - If CPAL_State is CPAL_STATE_READY:
      Perform the deinitialization routines
    - If CPAL_State is CPAL_STATE_DISABLED:
      Perform the deinitialization routines                                   */

    /*--------------------------------------------------------------------------
    GPIO pins Deinitialization
    Note: The GPIO clock remains enabled after this deinitialization
    ---------------------------------------------------------------------------*/
    /* Deinitialize I2Cx GPIO */
    CPAL_I2C_HAL_GPIODeInit(pDevInitStruct->CPAL_Dev);

    /*--------------------------------------------------------------------------
    Peripheral Deinitialization
    ---------------------------------------------------------------------------*/
    /* Disable I2Cx device */
    __CPAL_I2C_HAL_DISABLE_DEV(pDevInitStruct->CPAL_Dev);

    /*--------------------------------------------------------------------------
    Peripheral Clock Deinitialization
    ---------------------------------------------------------------------------*/
    /* Deinitialize I2Cx clock */
    CPAL_I2C_HAL_CLKDeInit(pDevInitStruct->CPAL_Dev);

    /*--------------------------------------------------------------------------
    DMA Deinitialization : if DMA Programming model is selected
    ---------------------------------------------------------------------------*/
    CPAL_I2C_HAL_DMADeInit(pDevInitStruct->CPAL_Dev, pDevInitStruct->CPAL_Direction);

    /*--------------------------------------------------------------------------
    Interrupts Deinitialization
    ---------------------------------------------------------------------------*/
    CPAL_I2C_HAL_ITDeInit(pDevInitStruct->CPAL_Dev, pDevInitStruct->wCPAL_Options, pDevInitStruct->CPAL_Direction, pDevInitStruct->CPAL_ProgModel);

    /*--------------------------------------------------------------------------
    Structure fields initialization
    ----------------------------------------------------------------------------*/
    /* Initialize pDevInitStruct state parameters to their default values */
    pDevInitStruct-> CPAL_State     = CPAL_STATE_DISABLED;     /* Device Disabled */
    pDevInitStruct-> wCPAL_DevError = CPAL_I2C_ERR_NONE;       /* No Device Error */
    pDevInitStruct-> wCPAL_Timeout  = ((uint32_t)CPAL_I2C_TIMEOUT_DEFAULT);  /* Set timeout value to CPAL_I2C_TIMEOUT_DEFAULT */

    /*----------------------------------------------------------------------------
    Deinitialize Timeout Procedure
    -----------------------------------------------------------------------------*/
    _CPAL_TIMEOUT_DEINIT();

    return CPAL_PASS;
  }
  /* If CPAL_State is BUSY (a transaction is still on going) Exit Init function */
  else
  {
    return CPAL_FAIL;
  }
}


/**
  * @brief  Initialize the peripheral structure with default values according
  *         to the specified parameters in the CPAL_I2CDevTypeDef structure.
  * @param  pDevInitStruct: Pointer to the peripheral configuration structure.
  * @retval CPAL_PASS or CPAL_FAIL.
  */
uint32_t CPAL_I2C_StructInit(CPAL_InitTypeDef* pDevInitStruct)
{
  /* Initialize I2C_InitStructure to their default values */
  I2C_InitStructure.I2C_Timing              = 0;                             /* Initialize the I2C_Timing member */
  I2C_InitStructure.I2C_Mode                = I2C_Mode_I2C;                  /* Initialize the I2C_Mode member */
  I2C_InitStructure.I2C_AnalogFilter        = I2C_AnalogFilter_Enable;       /* Initialize the I2C_AnalogFilter member */
  I2C_InitStructure.I2C_DigitalFilter       = 0x00;                          /* Initialize the I2C_DigitalFilter member */
  I2C_InitStructure.I2C_OwnAddress1         = 0;                             /* Initialize the I2C_OwnAddress1 member */
  I2C_InitStructure.I2C_Ack                 = I2C_Ack_Enable;                /* Initialize the I2C_Ack member */
  I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;  /* Initialize the I2C_AcknowledgedAddress member */

  /* Initialize pDevInitStruct parameter to their default values */
  pDevInitStruct->CPAL_Direction     = CPAL_DIRECTION_TXRX;                  /* Transmitter and Receiver direction selected */
  pDevInitStruct->CPAL_Mode          = CPAL_MODE_MASTER;                     /* Mode Master selected */
  pDevInitStruct->CPAL_ProgModel     = CPAL_PROGMODEL_DMA;                   /* DMA Programming Model selected */
  pDevInitStruct->pCPAL_TransferTx   = pNULL;                                /* Point pCPAL_TransferTx to a Null pointer */
  pDevInitStruct->pCPAL_TransferRx   = pNULL;                                /* Point pCPAL_TransferRx to a Null pointer */ 
  pDevInitStruct->CPAL_State         = CPAL_STATE_DISABLED;                  /* Device Disabled */
  pDevInitStruct->wCPAL_DevError     = CPAL_I2C_ERR_NONE;                    /* No Device Error */
  pDevInitStruct->wCPAL_Options      = ((uint32_t)0x00000000);               /* No Options selected */
  pDevInitStruct->wCPAL_Timeout      = ((uint32_t)CPAL_I2C_TIMEOUT_DEFAULT); /* Set timeout value to CPAL_I2C_TIMEOUT_DEFAULT */
  pDevInitStruct->pCPAL_I2C_Struct   = &I2C_InitStructure;                   /* Point to I2C_InitStructure (with default values) */

  return CPAL_PASS;
}

/**
  * @brief  Allows to send a data or a buffer of data through the peripheral to 
  *         a selected device in a selected location address.
  * @param  pDevInitStruct: Pointer to the peripheral configuration structure.
  * @retval CPAL_PASS or CPAL_FAIL.
  */
uint32_t CPAL_I2C_Write(CPAL_InitTypeDef* pDevInitStruct)
{
  CR2_tmp = 0;

  /* If Device is Busy (a transaction is still on going) Exit Write function */
  if (((pDevInitStruct->CPAL_State & CPAL_STATE_BUSY) != 0)
      || (pDevInitStruct->CPAL_State == CPAL_STATE_READY_TX)
        || (pDevInitStruct->CPAL_State == CPAL_STATE_READY_RX))
  {
    return CPAL_FAIL;
  }
  /* If CPAL_State is CPAL_STATE_DISABLED (device is not initialized) Exit Write function */
  else if (pDevInitStruct->CPAL_State == CPAL_STATE_DISABLED)
  {
    return CPAL_FAIL;
  }
  /* If CPAL_State is CPAL_STATE_ERROR (Error occurred ) */
  else if (pDevInitStruct->CPAL_State == CPAL_STATE_ERROR)
  {
    return CPAL_FAIL;
  }
  /* If CPAL_State is CPAL_STATE_READY ( Start Communication )*/
  else
  {
    /* Update CPAL_State to CPAL_STATE_BUSY */
    pDevInitStruct->CPAL_State = CPAL_STATE_BUSY;
    /* Update CPAL_State to CPAL_STATE_READY_TX */
    pDevInitStruct->CPAL_State = CPAL_STATE_READY_TX;
	/* Configure and enable TX DMA channel */
	I2C_Enable_DMA(pDevInitStruct, CPAL_DIRECTION_TX);

	/* Configure slave address */
	CR2_tmp |= (uint32_t)((pDevInitStruct->pCPAL_TransferTx->wAddr1) & 0x000003FF);

    /* If CPAL_OPT_NO_MEM_ADDR is not selected and master mode selected */
    if (((pDevInitStruct->wCPAL_Options & CPAL_OPT_NO_MEM_ADDR) == 0)
        && (pDevInitStruct->CPAL_Mode == CPAL_MODE_MASTER ))
    {
		/* Enable reload */
		CR2_tmp |= I2C_CR2_RELOAD;
      
		/* Configure Nbytes */
		CR2_tmp |= (uint32_t)((uint32_t)(1) << 16);

		/* Update CR2 Register */
		__CPAL_I2C_HAL_CR2_UPDATE(pDevInitStruct->CPAL_Dev, CR2_tmp);

		/* Generate start */
		__CPAL_I2C_HAL_START(pDevInitStruct->CPAL_Dev);

		/* Wait until TXIS flag is set */
		__CPAL_I2C_TIMEOUT(__CPAL_I2C_HAL_GET_TXIS(pDevInitStruct->CPAL_Dev), CPAL_I2C_TIMEOUT_TXIS);

		/* Send register address */
		__CPAL_I2C_HAL_SEND(pDevInitStruct->CPAL_Dev, (uint8_t)(pDevInitStruct->pCPAL_TransferTx->wAddr2));

		/* Wait until TCR flag is set */
		__CPAL_I2C_TIMEOUT(__CPAL_I2C_HAL_GET_TCR(pDevInitStruct->CPAL_Dev), CPAL_I2C_TIMEOUT_TCR);

		/* Set Nbytes to zero */
		CR2_tmp &= ~I2C_CR2_NBYTES;
    }

    /* Update CPAL_State to CPAL_STATE_BUSY_TX */
    pDevInitStruct->CPAL_State = CPAL_STATE_BUSY_TX;

	/* Enable automatic end mode */
	CR2_tmp |= I2C_CR2_AUTOEND;

	/* If number of data is equal or lower than 255 bytes */
	if (pDevInitStruct->pCPAL_TransferTx->wNumData <= 0xFF )
	{
		/* Update Num_Data */
		Num_Data = pDevInitStruct->pCPAL_TransferTx->wNumData;

		/* Set Nbytes to wNumData */
		CR2_tmp |= (uint32_t)((uint32_t)(Num_Data) << 16);

		/* Disable reload */
		CR2_tmp &= ~I2C_CR2_RELOAD;
	}
	/* If number of data is greater than 255 bytes */
	else
	{
		/* Set Nbytes to 255 */
		CR2_tmp |= (uint32_t)((uint32_t)(255) << 16);

		/* Enable reload */
		CR2_tmp |= I2C_CR2_RELOAD;
	}

	/* If CPAL_OPT_NO_MEM_ADDR is selected */
	if ((pDevInitStruct->wCPAL_Options & CPAL_OPT_NO_MEM_ADDR) != 0)
	{
		/* Generate start */
		CR2_tmp |= I2C_CR2_START;
	}

	/* Update CR2 Register */
	__CPAL_I2C_HAL_CR2_UPDATE(pDevInitStruct->CPAL_Dev, CR2_tmp);

	/* Enable TX DMA request */
	__CPAL_I2C_HAL_ENABLE_TXDMAREQ(pDevInitStruct->CPAL_Dev);

	/* Enable master interrupts */
	__CPAL_I2C_HAL_ENABLE_MASTER_IT(pDevInitStruct->CPAL_Dev);

  }
  return CPAL_PASS;
}



/**
  * @brief  Allows to receive a data or a buffer of data through the peripheral
  *         from a selected device in a selected location address.
  * @param  pDevInitStruct: Pointer to the peripheral configuration structure.
  * @retval CPAL_PASS or CPAL_FAIL. 
  */
uint32_t CPAL_I2C_Read(CPAL_InitTypeDef* pDevInitStruct)
{
  CR2_tmp = 0;
  
  /* If Device is Busy (a transaction is still on going) Exit Read function */
  if (((pDevInitStruct->CPAL_State & CPAL_STATE_BUSY) != 0)
      || (pDevInitStruct->CPAL_State == CPAL_STATE_READY_TX)
        || (pDevInitStruct->CPAL_State == CPAL_STATE_READY_RX))
  {
    return CPAL_FAIL;
  }
  /* If CPAL_State is CPAL_STATE_DISABLED (device is not initialized) Exit Read function */
  else if (pDevInitStruct->CPAL_State == CPAL_STATE_DISABLED)
  {
    return CPAL_FAIL;
  }
  /* If CPAL_State is CPAL_STATE_ERROR (Error occurred ) */
  else if (pDevInitStruct->CPAL_State == CPAL_STATE_ERROR)
  {
    return CPAL_FAIL;
  }
  /* If CPAL_State is CPAL_STATE_READY */
  else
  {
    /* Update CPAL_State to CPAL_STATE_BUSY */
    pDevInitStruct->CPAL_State = CPAL_STATE_BUSY;

    /* Update CPAL_State to CPAL_STATE_READY_RX */
    pDevInitStruct->CPAL_State = CPAL_STATE_READY_RX;

	/* Configure and enable RX DMA channel */
	I2C_Enable_DMA(pDevInitStruct, CPAL_DIRECTION_RX);

	/* Configure slave address */
	CR2_tmp |= (uint32_t)((pDevInitStruct->pCPAL_TransferRx->wAddr1) & 0x000003FF);

    /* If No Memory Address option bit is not selected and master mode selected */
    if (((pDevInitStruct->wCPAL_Options & CPAL_OPT_NO_MEM_ADDR) == 0)
        && (pDevInitStruct->CPAL_Mode == CPAL_MODE_MASTER ))
    {
        /* Configure Nbytes */
        CR2_tmp |= (uint32_t)((uint32_t)(1) << 16);

        /* Update CR2 Register */
        __CPAL_I2C_HAL_CR2_UPDATE(pDevInitStruct->CPAL_Dev, CR2_tmp);

        /* Generate start */
        __CPAL_I2C_HAL_START(pDevInitStruct->CPAL_Dev); 

        /* Wait until TXIS flag is set */
        __CPAL_I2C_TIMEOUT(__CPAL_I2C_HAL_GET_TXIS(pDevInitStruct->CPAL_Dev), CPAL_I2C_TIMEOUT_TXIS);

        /* Send register address */
        __CPAL_I2C_HAL_SEND(pDevInitStruct->CPAL_Dev, (uint8_t)(pDevInitStruct->pCPAL_TransferRx->wAddr2));

        /* Wait until TC flag is set */
        __CPAL_I2C_TIMEOUT(__CPAL_I2C_HAL_GET_TC(pDevInitStruct->CPAL_Dev), CPAL_I2C_TIMEOUT_TC);
    }

	/* Set Nbytes to zero */
	CR2_tmp &= ~I2C_CR2_NBYTES;

    /* Update CPAL_State to CPAL_STATE_BUSY_RX */
    pDevInitStruct->CPAL_State = CPAL_STATE_BUSY_RX;

	/* Enable transfer request */
	CR2_tmp |= I2C_CR2_RD_WRN;

	/* Enable automatic end mode */
	CR2_tmp |= I2C_CR2_AUTOEND;

	/* If number of data is equal or lower than 255 bytes */
	if (pDevInitStruct->pCPAL_TransferRx->wNumData <= 0xFF )
	{
		/* Update Num_Data */
		Num_Data = pDevInitStruct->pCPAL_TransferRx->wNumData;

		/* Set Nbytes to wNumData */
		CR2_tmp |= (uint32_t)((uint32_t)(Num_Data) << 16);

		/* Disable reload */
		CR2_tmp &= ~I2C_CR2_RELOAD;
	}
	/* If number of data is greater than 255 bytes */
	else
	{
		/* Set Nbytes to wNumData */
		CR2_tmp |= (uint32_t)((uint32_t)(255) << 16);

		/* Enaable reload */
		CR2_tmp |= I2C_CR2_RELOAD;
	}

	/* Generate start */
	CR2_tmp |= I2C_CR2_START;
      
	/* Enable RX DMA request */
	__CPAL_I2C_HAL_ENABLE_RXDMAREQ(pDevInitStruct->CPAL_Dev);

	/* Update CR2 Register */
	__CPAL_I2C_HAL_CR2_UPDATE(pDevInitStruct->CPAL_Dev, CR2_tmp);

	/* Enable master interrupt */
	__CPAL_I2C_HAL_ENABLE_MASTER_IT(pDevInitStruct->CPAL_Dev);

  }
  return CPAL_PASS;
}

/*================== CPAL_I2C_Interrupt_Handler ==================*/

/**
  * @brief  This function handles I2C interrupt request for preparing communication
  *         and for transfer phase in case of using Interrupt Programming Model.
  * @param  pDevInitStruct: Pointer to the peripheral configuration structure.
  * @retval CPAL_PASS. 
  */
uint32_t CPAL_I2C_EV_IRQHandler( CPAL_InitTypeDef* pDevInitStruct)
{
	__IO uint32_t I2CFlagStatus = 0x00000000;

	/* Read I2C status registers (ISR) */
	I2CFlagStatus = __CPAL_I2C_HAL_GET_EVENT(pDevInitStruct->CPAL_Dev);

	/*----------------------------------------*/
	/*-------------- If TCR event ------------*/
	if ((I2CFlagStatus & CPAL_I2C_EVT_TCR) != 0)
	{
		I2C_MASTER_TCR_Handle(pDevInitStruct);
	}

	/*----------------------------------------*/
	/*------------- If TC event --------------*/
	if ((I2CFlagStatus & CPAL_I2C_EVT_TC ) != 0)
	{
		I2C_MASTER_TC_Handle(pDevInitStruct);
	}

	/*----------------------------------------*/
	/*------------- If STOP event ------------*/
	if ((I2CFlagStatus & CPAL_I2C_EVT_STOP) != 0)
	{
		I2C_MASTER_STOP_Handle(pDevInitStruct);
	}

	/*----------------------------------------*/
	/*------------- If NACK event ------------*/
	if((I2CFlagStatus & CPAL_I2C_EVT_NACK ) != 0)
	{
		I2C_MASTER_NACK_Handle(pDevInitStruct);
	}

  return CPAL_PASS;
}


/**
  * @brief  Allows to handle errors occurred during initialization or communication
  *         in order to recover the correct communication status or call specific
  *         user functions.
  * @param  pDevInitStruct: Pointer to the peripheral configuration structure.
  * @retval CPAL_PASS. 
  */
uint32_t CPAL_I2C_ER_IRQHandler(CPAL_InitTypeDef* pDevInitStruct)
{
  /* Read error register and affect to wCPAL_DevError */
  pDevInitStruct->wCPAL_DevError = __CPAL_I2C_HAL_GET_ERROR(pDevInitStruct->CPAL_Dev);

  /* Set CPAL_State to CPAL_STATE_ERROR */
  pDevInitStruct->CPAL_State = CPAL_STATE_ERROR;

  /* If Bus error occurred ---------------------------------------------------*/
  if ((pDevInitStruct->wCPAL_DevError & CPAL_I2C_ERR_BERR) != 0)
  {
    /* Clear error flag */
    __CPAL_I2C_HAL_CLEAR_BERR(pDevInitStruct->CPAL_Dev);

  }

  /* If Arbitration Loss error occurred --------------------------------------*/
  if ((pDevInitStruct->wCPAL_DevError & CPAL_I2C_ERR_ARLO) != 0)
  {
    /* Clear error flag */
    __CPAL_I2C_HAL_CLEAR_ARLO(pDevInitStruct->CPAL_Dev);

  }

  /* If Overrun error occurred -----------------------------------------------*/
  if ((pDevInitStruct->wCPAL_DevError & CPAL_I2C_ERR_OVR) != 0)
  {
    /* No I2C software reset is performed here in order to allow user to get back
    the last data received correctly */
    /* Clear error flag */
    __CPAL_I2C_HAL_CLEAR_OVR(pDevInitStruct->CPAL_Dev);
  }

  return CPAL_PASS;
}

/**
  * @brief  Handle I2C DMA TX interrupt request when DMA programming Model is
  *         used for data transmission. 
  * @param  pDevInitStruct: Pointer to the peripheral configuration structure.
  * @retval CPAL_PASS.
  */
uint32_t CPAL_I2C_DMA_TX_IRQHandler(CPAL_InitTypeDef* pDevInitStruct)
{
  /* Reinitialize timeout value to default (no timeout initiated) */
  pDevInitStruct->wCPAL_Timeout = CPAL_I2C_TIMEOUT_DEFAULT;

  /*------------- If TC interrupt ------------*/
  if((__CPAL_I2C_HAL_GET_DMATX_TCIT(pDevInitStruct->CPAL_Dev)) != 0)
  {
      /* Update remaining number of data */
      pDevInitStruct->pCPAL_TransferTx->wNumData = 0;

      /* Call DMA TX TC UserCallback */
      CPAL_I2C_DMATXTC_UserCallback(pDevInitStruct);

      /* Disable DMA request and channel */
      __CPAL_I2C_HAL_DISABLE_TXDMAREQ(pDevInitStruct->CPAL_Dev);
      __CPAL_I2C_HAL_DISABLE_DMATX(pDevInitStruct->CPAL_Dev);

  }

  /* Clear DMA interrupt Flag */
  __CPAL_I2C_HAL_CLEAR_DMATX_IT(pDevInitStruct->CPAL_Dev);

  return CPAL_PASS;
}


/**
  * @brief  Handle I2C DMA RX interrupt request when DMA programming Model is
  *         used for data reception.  
  * @param  pDevInitStruct: Pointer to the peripheral configuration structure.
  * @retval CPAL_PASS. 
  */
uint32_t CPAL_I2C_DMA_RX_IRQHandler(CPAL_InitTypeDef* pDevInitStruct)
{
  /* Reinitialize Timeout Value to default (no timeout initiated) */
  pDevInitStruct->wCPAL_Timeout = CPAL_I2C_TIMEOUT_DEFAULT; 

  /*------------- If TC interrupt ------------*/
  if ((__CPAL_I2C_HAL_GET_DMARX_TCIT(pDevInitStruct->CPAL_Dev)) != 0)
  {
      /* Update remaining number of data */
      pDevInitStruct->pCPAL_TransferRx->wNumData = 0;

      /* Disable DMA Request and Channel */
      __CPAL_I2C_HAL_DISABLE_RXDMAREQ(pDevInitStruct->CPAL_Dev);
      __CPAL_I2C_HAL_DISABLE_DMARX(pDevInitStruct->CPAL_Dev);

      /* Call DMA RX TC UserCallback */
      CPAL_I2C_DMARXTC_UserCallback(pDevInitStruct);
  }

  /* Clear DMA interrupt Flag */
  __CPAL_I2C_HAL_CLEAR_DMARX_IT(pDevInitStruct->CPAL_Dev);

  return CPAL_PASS;
}

/*================== CPAL_I2C_Timeout_Function ==================*/

/**
  * @brief  This function Manages I2C Timeouts when waiting for specific events.
  * @param  None
  * @retval CPAL_PASS or CPAL_FAIL. 
  */
void CPAL_I2C_TIMEOUT_Manager(void)
{
  uint32_t index = 0;

  TIM18->SR &= (uint16_t)~TIM_IT_Update;

  /* Manage I2C timeouts conditions */
  for (index = 0; index < CPAL_I2C_DEV_NUM; index ++)
  {
    if (I2C_DevStructures[index] != pNULL)
    {
      /* If Timeout occurred  */
      if (I2C_DevStructures[index]->wCPAL_Timeout == CPAL_I2C_TIMEOUT_DETECTED)
      {
        /* Reinitialize timeout value */
        I2C_DevStructures[index]->wCPAL_Timeout = CPAL_I2C_TIMEOUT_DEFAULT;

        /* Update CPAL_State to CPAL_STATE_ERROR */
        I2C_DevStructures[index]->CPAL_State = CPAL_STATE_ERROR;

        /* In case of Device Error Timeout_Callback should not be called */
        if (I2C_DevStructures[index]->wCPAL_DevError == CPAL_I2C_ERR_NONE)
        {
          /* Update wCPAL_DevError to CPAL_I2C_ERR_TIMEOUT */
          I2C_DevStructures[index]->wCPAL_DevError = CPAL_I2C_ERR_TIMEOUT;

          /* Call CPAL_TIMEOUT_UserCallback */
          CPAL_TIMEOUT_UserCallback(I2C_DevStructures[index]);
        }
      }
      /* If Timeout is triggered (wCPAL_Timeout != CPAL_I2C_TIMEOUT_DEFAULT)*/
      else if (I2C_DevStructures[index]->wCPAL_Timeout != CPAL_I2C_TIMEOUT_DEFAULT)
      {
        /* Decrement the timeout value */
        I2C_DevStructures[index]->wCPAL_Timeout--;
      }
    }
  }

}


/**
  * @brief  This function Manages I2C Timeouts when Timeout occurred.
  * @param  pDevInitStruct: Pointer to the peripheral configuration structure.
  * @retval CPAL_PASS or CPAL_FAIL. 
  */
uint32_t CPAL_I2C_Timeout (CPAL_InitTypeDef* pDevInitStruct)
{
  /* Reinitialize timeout value */
  pDevInitStruct->wCPAL_Timeout = CPAL_I2C_TIMEOUT_DEFAULT;

  /* update CPAL_State to CPAL_STATE_ERROR */
  pDevInitStruct->CPAL_State = CPAL_STATE_ERROR;

  /* update wCPAL_DevError to CPAL_I2C_ERR_TIMEOUT */
  pDevInitStruct->wCPAL_DevError = CPAL_I2C_ERR_TIMEOUT;

  /* Call Timeout Callback and quit current function */
  return (CPAL_TIMEOUT_UserCallback(pDevInitStruct));

}

/*================== CPAL_I2C_Event_Handler ==================*/
/**
  * @brief  Handles Master TCR interrupt event.
  * @param  pDevInitStruct: Pointer to the peripheral configuration structure.
  * @retval CPAL_PASS or CPAL_FAIL. 
  */
static uint32_t I2C_MASTER_TCR_Handle(CPAL_InitTypeDef* pDevInitStruct)
{
  CR2_tmp = 0;

  /* If DMA programming model */
  if (pDevInitStruct->CPAL_ProgModel == CPAL_PROGMODEL_DMA)
  {
    /* If master transmitter */
    if (pDevInitStruct->CPAL_State == CPAL_STATE_BUSY_TX)
    {
      /* Update wNumData */
      pDevInitStruct->pCPAL_TransferTx->wNumData = pDevInitStruct->pCPAL_TransferTx->wNumData - 0xff;
    }
    /* If master receiver */
    else
    {
      /* Update wNumData */
      pDevInitStruct->pCPAL_TransferRx->wNumData = pDevInitStruct->pCPAL_TransferRx->wNumData - 0xff;
    }
  }

  /* If master transmitter */
  if (pDevInitStruct->CPAL_State == CPAL_STATE_BUSY_TX)
  {
    /* If remaining number of data is equal or lower than 255 */
    if (pDevInitStruct->pCPAL_TransferTx->wNumData <= 0xff)
    {
      /* Update Num_Data */
      Num_Data = pDevInitStruct->pCPAL_TransferTx->wNumData;
      
      /* Set Nbytes to wNumData */
      CR2_tmp |= (uint32_t)((uint32_t)(Num_Data) << 16);
      
      /* Disable reload */
      CR2_tmp &= ~I2C_CR2_RELOAD;
    }
    /* If remaining number of data is greater than 255 */
    else
    {
      /* Set Nbytes to wNumData */
      CR2_tmp |= (uint32_t)((uint32_t)(255) << 16);
      
      /* Enaable reload */
      CR2_tmp |= I2C_CR2_RELOAD;
    }
  }
  /* If master receiver */
  else
  {
    /* If remaining number of data is equal or lower than 255 */
    if (pDevInitStruct->pCPAL_TransferRx->wNumData <= 0xff)
    {
      /* Update num data */
      Num_Data = pDevInitStruct->pCPAL_TransferRx->wNumData;

      /* Set Nbytes to wNumData */
      CR2_tmp |= (uint32_t)((uint32_t)(Num_Data) << 16);
      
      /* Disable reload */
      CR2_tmp &= ~I2C_CR2_RELOAD;
      
    }
    /* If remaining number of data is greater than 255 */
    else
    {
      /* Set Nbytes to wNumData */
      CR2_tmp |= (uint32_t)((uint32_t)(255) << 16);
      
      /* Enaable reload */
      CR2_tmp |= I2C_CR2_RELOAD;
    }
  }

  /* Update CR2 Register */
  __CPAL_I2C_HAL_CR2_UPDATE(pDevInitStruct->CPAL_Dev, CR2_tmp);

  return CPAL_PASS;
}

/**
  * @brief  Handles Master TC interrupt event.
  * @param  pDevInitStruct: Pointer to the peripheral configuration structure.
  * @retval CPAL_PASS or CPAL_FAIL. 
  */
static uint32_t I2C_MASTER_TC_Handle(CPAL_InitTypeDef* pDevInitStruct)
{
    /* Disable master interrupts */
    __CPAL_I2C_HAL_DISABLE_MASTER_IT(pDevInitStruct->CPAL_Dev);

    /* If master transmitter */
    if (pDevInitStruct->CPAL_State == CPAL_STATE_BUSY_TX)
    {
      /* Update CPAL_State to CPAL_STATE_READY */
      pDevInitStruct->CPAL_State = CPAL_STATE_READY;

      /* Call TX Transfer complete Callback */
      CPAL_I2C_TXTC_UserCallback(pDevInitStruct);
    }
    /* If master receiver */
    else
    {
      /* Update CPAL_State to CPAL_STATE_READY */
      pDevInitStruct->CPAL_State = CPAL_STATE_READY;

      /* Call RX Transfer complete Callback */
      CPAL_I2C_RXTC_UserCallback(pDevInitStruct);
    }

  return CPAL_PASS;
}

/**
  * @brief  Handles Master STOP interrupt event.
  * @param  pDevInitStruct: Pointer to the peripheral configuration structure.
  * @retval CPAL_PASS or CPAL_FAIL. 
  */
static uint32_t I2C_MASTER_STOP_Handle(CPAL_InitTypeDef* pDevInitStruct)
{
  /* If NACK received by master */
  if (pDevInitStruct->wCPAL_DevError == CPAL_I2C_ERR_AF)
  {
    /* Set CPAL_State to CPAL_STATE_ERROR */
    pDevInitStruct->CPAL_State = CPAL_STATE_ERROR;

    /* Clear STOP flag */
    __CPAL_I2C_HAL_CLEAR_STOP(pDevInitStruct->CPAL_Dev);
  }
  else
  {  
    /* Clear STOP flag */
    __CPAL_I2C_HAL_CLEAR_STOP(pDevInitStruct->CPAL_Dev);

    /* Disable master interrupt */
    __CPAL_I2C_HAL_DISABLE_MASTER_IT(pDevInitStruct->CPAL_Dev);

    /* Wait until BUSY flag is reset */
    __CPAL_I2C_TIMEOUT(!(__CPAL_I2C_HAL_GET_BUSY(pDevInitStruct->CPAL_Dev)), CPAL_I2C_TIMEOUT_BUSY);

    /* If master transmitter */
    if (pDevInitStruct->CPAL_State == CPAL_STATE_BUSY_TX)
    {
      /* Update CPAL_State to CPAL_STATE_READY */
      pDevInitStruct->CPAL_State = CPAL_STATE_READY;

      /* Call TX Transfer complete Callback */
      CPAL_I2C_TXTC_UserCallback(pDevInitStruct);
    }
    /* If master receiver */
    else
    {
      /* Update CPAL_State to CPAL_STATE_READY */
      pDevInitStruct->CPAL_State = CPAL_STATE_READY;

      /* Call RX Transfer complete Callback */
      CPAL_I2C_RXTC_UserCallback(pDevInitStruct);
    }
  }
  return CPAL_PASS;
}

/**
  * @brief  Handles Master NACK interrupt event.
  * @param  pDevInitStruct: Pointer to the peripheral configuration structure.
  * @retval CPAL_PASS or CPAL_FAIL. 
  */
static uint32_t I2C_MASTER_NACK_Handle(CPAL_InitTypeDef* pDevInitStruct)
{
  /* Update wCPAL_DevError */
  pDevInitStruct->wCPAL_DevError = CPAL_I2C_ERR_AF;

  /* Clear NACK flag */
  __CPAL_I2C_HAL_CLEAR_NACK(pDevInitStruct->CPAL_Dev);

  return CPAL_PASS;
}

/*================== Local DMA and IT Manager ==================*/
/**
  * @brief  This function Configures and enables I2C DMA before starting transfer phase.
  * @param  pDevInitStruct: Pointer to the peripheral configuration structure.
  * @param  Direction : Transfer direction.
  * @retval CPAL_PASS or CPAL_FAIL. 
  */
static uint32_t I2C_Enable_DMA (CPAL_InitTypeDef* pDevInitStruct, CPAL_DirectionTypeDef Direction)
{
  /* If data transmission will be performed */
  if ((pDevInitStruct->CPAL_State == CPAL_STATE_BUSY_TX) || (Direction == CPAL_DIRECTION_TX))
  {
    /* Configure TX DMA channels */
    CPAL_I2C_HAL_DMATXConfig(pDevInitStruct->CPAL_Dev, pDevInitStruct->pCPAL_TransferTx, pDevInitStruct->wCPAL_Options);

    /* Enable TX DMA channels */
    __CPAL_I2C_HAL_ENABLE_DMATX(pDevInitStruct->CPAL_Dev);

  }
  /* If data reception will be performed */
  else if ((pDevInitStruct->CPAL_State == CPAL_STATE_BUSY_RX) || (Direction == CPAL_DIRECTION_RX))
  {
    /* Configure RX DMA channels */
    CPAL_I2C_HAL_DMARXConfig(pDevInitStruct->CPAL_Dev, pDevInitStruct->pCPAL_TransferRx, pDevInitStruct->wCPAL_Options);

    /* Enable RX DMA channels */
    __CPAL_I2C_HAL_ENABLE_DMARX(pDevInitStruct->CPAL_Dev);

  }
  return CPAL_PASS;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

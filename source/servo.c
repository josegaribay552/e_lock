/* Freescale includes. */
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "board.h"

#include "pin_mux.h"
#include "clock_config.h"
#include "fsl_ftm.h"

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

extern QueueHandle_t servo_queue;

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* The Flextimer instance/channel used for board */
#define BOARD_FTM_BASEADDR FTM0
#define BOARD_FTM_CHANNEL kFTM_Chnl_0

/* Interrupt number and interrupt handler for the FTM instance used */
#define FTM_INTERRUPT_NUMBER FTM0_IRQn
#define FTM_HANDLER FTM0_IRQHandler

/* Interrupt to enable and flag to read; depends on the FTM channel used */
#define FTM_CHANNEL_INTERRUPT_ENABLE kFTM_Chnl0InterruptEnable
#define FTM_CHANNEL_FLAG kFTM_Chnl0Flag

/* Get source clock for FTM driver */
#define FTM_SOURCE_CLOCK CLOCK_GetFreq(kCLOCK_BusClk)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*!
 * @brief delay a while.
 */
void delay(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile uint8_t updatedDutycycle = 97U;

#define CLOSESERVO   0
#define OPENSERVO    1

/*******************************************************************************
 * Code
 ******************************************************************************/
void FTM_HANDLER(void)
{
    if ((FTM_GetStatusFlags(BOARD_FTM_BASEADDR) & FTM_CHANNEL_FLAG) == FTM_CHANNEL_FLAG)
    {
        /* Clear interrupt flag.*/
        FTM_ClearStatusFlags(BOARD_FTM_BASEADDR, FTM_CHANNEL_FLAG);
    }
    __DSB();
}

void ftm_init(void)
{
    ftm_config_t ftmInfo;
    ftm_chnl_pwm_signal_param_t ftmParam;
    ftm_pwm_level_select_t pwmLevel = kFTM_LowTrue;

    /* Configure ftm params with frequency 24kHZ */
    ftmParam.chnlNumber            = BOARD_FTM_CHANNEL;
    ftmParam.level                 = pwmLevel;
    ftmParam.dutyCyclePercent      = updatedDutycycle;
    ftmParam.firstEdgeDelayPercent = 0U;

    FTM_GetDefaultConfig(&ftmInfo);
    ftmInfo.prescale = kFTM_Prescale_Divide_128;
    /* Initialize FTM module */
    FTM_Init(BOARD_FTM_BASEADDR, &ftmInfo);

    FTM_SetupPwm(BOARD_FTM_BASEADDR, &ftmParam, 1U, kFTM_EdgeAlignedPwm, 50U, FTM_SOURCE_CLOCK); //LEGA for SERVO

    /* Enable channel interrupt flag.*/
    FTM_EnableInterrupts(BOARD_FTM_BASEADDR, FTM_CHANNEL_INTERRUPT_ENABLE);

    /* Enable at the NVIC */
    EnableIRQ(FTM_INTERRUPT_NUMBER);

    FTM_StartTimer(BOARD_FTM_BASEADDR, kFTM_SystemClock);
}

void move_Servo(char value)
{
	ftm_pwm_level_select_t pwmLevel = kFTM_LowTrue;

	if (value == OPENSERVO)
	{
		updatedDutycycle = 97U;
	}
	else
	{
		updatedDutycycle = 88U;
	}

	/* Disable interrupt to retain current dutycycle for a few seconds */
	FTM_DisableInterrupts(BOARD_FTM_BASEADDR, FTM_CHANNEL_INTERRUPT_ENABLE);

	/* Disable channel output before updating the dutycycle */
	FTM_UpdateChnlEdgeLevelSelect(BOARD_FTM_BASEADDR, BOARD_FTM_CHANNEL, 0U);

	/* Update PWM duty cycle */
	//FTM_UpdatePwmDutycycle(BOARD_FTM_BASEADDR, BOARD_FTM_CHANNEL, kFTM_CenterAlignedPwm, updatedDutycycle);
	FTM_UpdatePwmDutycycle(BOARD_FTM_BASEADDR, BOARD_FTM_CHANNEL, kFTM_EdgeAlignedPwm, updatedDutycycle);
	PRINTF("%d.\r\n",updatedDutycycle);

	/* Software trigger to update registers */
	FTM_SetSoftwareTrigger(BOARD_FTM_BASEADDR, true);

	/* Start channel output with updated dutycycle */
	FTM_UpdateChnlEdgeLevelSelect(BOARD_FTM_BASEADDR, BOARD_FTM_CHANNEL, pwmLevel);

	/* Delay to view the updated PWM dutycycle */
	//vTaskDelay(500);

	/* Enable interrupt flag to update PWM dutycycle */
	FTM_EnableInterrupts(BOARD_FTM_BASEADDR, FTM_CHANNEL_INTERRUPT_ENABLE);
}

void servo_task(void *pvParameters)
{
	char cmd[2 + 1];

	ftm_init();

    while (1)
    {
		while (xQueueReceive(servo_queue, cmd, portMAX_DELAY) != errQUEUE_EMPTY)
		{
			switch (cmd[0])
			{
				case 'o':
					PRINTF("Open servo\r\n");
					move_Servo (OPENSERVO);
					vTaskDelay(1000);
					PRINTF("Closing servo...\r\n");
					move_Servo (CLOSESERVO);
				break;
				case 'c':
					PRINTF("Close servo\r\n");
					move_Servo (CLOSESERVO);
				break;
				default:
				break;
			}
		}

    }
}

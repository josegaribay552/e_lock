#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "board.h"
#include "fsl_port.h"
#include "fsl_gpio.h"
#include "fsl_common.h"

#include "pin_mux.h"
#include "clock_config.h"

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

volatile bool g_ButtonPress;
extern QueueHandle_t servo_queue;

/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief Interrupt service function of switch SW3.
 *
 */
void BOARD_SW3_IRQ_HANDLER(void)
{
	BaseType_t xHigherPriorityTaskWoken;

	xQueueSendToBackFromISR( servo_queue, "o", &xHigherPriorityTaskWoken );

    /* Clear external interrupt flag. */
    GPIO_PortClearInterruptFlags(BOARD_SW3_GPIO, 1U << BOARD_SW3_GPIO_PIN);

    /* Now the message was sent, and the interrupt source has been cleared, a context
	switch should be performed if xHigherPriorityTaskWoken is equal to pdTRUE.
	NOTE: The syntax required to perform a context switch from an ISR varies from
	port to port, and from compiler to compiler. Check the web documentation and
	examples for the port being used to find the syntax required for your
	application. */
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );

/* Add for ARM errata 838869, affects Cortex-M4, Cortex-M4F Store immediate overlapping
  exception return operation might vector to incorrect interrupt */
#if defined __CORTEX_M && (__CORTEX_M == 4U)
    __DSB();
#endif
}

void BOARD_InitGPIOInterrupts (void)
{
	int result = 0;

    /* Define the init structure for the input switch pin */
    gpio_pin_config_t sw_config = {
        kGPIO_DigitalInput,
        0,
    };

    PORT_SetPinInterruptConfig(BOARD_SW3_PORT, BOARD_SW3_GPIO_PIN, kPORT_InterruptFallingEdge);
	//PORT_SetPinInterruptConfig(BOARD_SW2_PORT, BOARD_SW2_GPIO_PIN, kPORT_InterruptFallingEdge);

	EnableIRQ(BOARD_SW3_IRQ);
	//EnableIRQ(BOARD_SW2_IRQ);

	GPIO_PinInit(BOARD_SW3_GPIO, BOARD_SW3_GPIO_PIN, &sw_config);
	//GPIO_PinInit(BOARD_SW2_GPIO, BOARD_SW2_GPIO_PIN, &sw_config);


    //Set PORTA Interrupt level to 3 (higher than SYSCALL), configMAX_SYSCALL_INTERRUPT_PRIORITY priority is 2.
    result = NVIC_GetPriority(PORTA_IRQn);
	NVIC_SetPriority(PORTA_IRQn,3);        //PORTA vector is 59
	result = NVIC_GetPriority(PORTA_IRQn);

}

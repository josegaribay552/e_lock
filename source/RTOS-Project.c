/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2019 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "lwip/opt.h"

#if LWIP_NETCONN

#include "tcpecho.h"
#include "lwip/netifapi.h"
#include "lwip/tcpip.h"
#include "netif/ethernet.h"
#include "enet_ethernetif.h"

#include "board.h"

#include "fsl_device_registers.h"
#include "pin_mux.h"
#include "clock_config.h"

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "event_groups.h"

/* Freescale includes. */
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "board.h"

#include "pin_mux.h"
#include "clock_config.h"
#include "nfc_task.h"

#include "lwip/dhcp.h"
#include "lwip/prot/dhcp.h"

#include "SEGGER_SYSVIEW.h"
/* Include internal header to get SEGGER_RTT_CB */
#include "SEGGER_RTT.h"

#define SYSVIEW_DEVICE_NAME "FRDMK64F Cortex-M4"
#define SYSVIEW_RAM_BASE (0x1FFF0000)

extern SEGGER_RTT_CB _SEGGER_RTT;
extern const SEGGER_SYSVIEW_OS_API SYSVIEW_X_OS_TraceAPI;

/* The application name to be displayed in SystemViewer */
#ifndef SYSVIEW_APP_NAME
#define SYSVIEW_APP_NAME "SDK System view example"
#endif

/* The target device name */
#ifndef SYSVIEW_DEVICE_NAME
#define SYSVIEW_DEVICE_NAME "Generic Cortex device"
#endif

/* Frequency of the timestamp. Must match SEGGER_SYSVIEW_GET_TIMESTAMP in SEGGER_SYSVIEW_Conf.h */
#define SYSVIEW_TIMESTAMP_FREQ (configCPU_CLOCK_HZ)

/* System Frequency. SystemcoreClock is used in most CMSIS compatible projects. */
#define SYSVIEW_CPU_FREQ configCPU_CLOCK_HZ

/* The lowest RAM address used for IDs (pointers) */
#ifndef SYSVIEW_RAM_BASE
#define SYSVIEW_RAM_BASE 0x20000000
#endif
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* IP address configuration. */
#define configIP_ADDR0 192
#define configIP_ADDR1 168
#define configIP_ADDR2 0
#define configIP_ADDR3 102

/* Netmask configuration. */
#define configNET_MASK0 255
#define configNET_MASK1 255
#define configNET_MASK2 255
#define configNET_MASK3 0

/* Gateway address configuration. */
#define configGW_ADDR0 192
#define configGW_ADDR1 168
#define configGW_ADDR2 0
#define configGW_ADDR3 100

/* MAC address configuration. */
#define configMAC_ADDR                     \
    {                                      \
        0x02, 0x12, 0x13, 0x10, 0x15, 0x11 \
    }

/* Address of PHY interface. */
#define EXAMPLE_PHY_ADDRESS BOARD_ENET0_PHY_ADDRESS

/* System clock name. */
#define EXAMPLE_CLOCK_NAME kCLOCK_CoreSysClk

/*!
 * @brief System View callback
 */
static void _cbSendSystemDesc(void)
{
    SEGGER_SYSVIEW_SendSysDesc("N=" SYSVIEW_APP_NAME ",D=" SYSVIEW_DEVICE_NAME ",O=FreeRTOS");
    SEGGER_SYSVIEW_SendSysDesc("I#15=SysTick");
}

/*!
 * @brief System View configuration
 */
void SEGGER_SYSVIEW_Conf(void)
{
    SEGGER_SYSVIEW_Init(SYSVIEW_TIMESTAMP_FREQ, SYSVIEW_CPU_FREQ, &SYSVIEW_X_OS_TraceAPI, _cbSendSystemDesc);
    SEGGER_SYSVIEW_SetRAMBase(SYSVIEW_RAM_BASE);
}
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* Task priorities. */
#define keepalive_task_PRIORITY (configMAX_PRIORITIES - 4)
#define servo_task_PRIORITY (configMAX_PRIORITIES - 3)
#define tcpipserver_task_PRIORITY (configMAX_PRIORITIES - 3)


extern void tcp_webclilent(void);
void tcpipserver_task(void *pvParameters);
static void keepalive_task(void *pvParameters);
extern void servo_task(void *pvParameters);

extern void ftm_init(void);
extern void BOARD_InitGPIOInterrupts (void);

#define TASK_NFC_STACK_SIZE		1024
#define TASK_NFC_STACK_PRIO		(configMAX_PRIORITIES - 1)
/*******************************************************************************
 * Code
 ******************************************************************************/
extern volatile bool g_ButtonPress;
static struct netif fsl_netif0;
#define MAX_CMD_LENGTH 2
#define TAG_ID_STRING_SIZE 14
QueueHandle_t servo_queue = NULL;
SemaphoreHandle_t servo_mutex = NULL;
EventGroupHandle_t tcpipEvent;
EventBits_t tcpipBits;

char TagUIDstr[TAG_ID_STRING_SIZE]={0};

/*!
 * @brief Prints DHCP status of the interface when it has changed from last status.
 *
 * @param arg pointer to network interface structure
 */
static void print_dhcp_state(void *arg)
{
    struct netif *netif = (struct netif *)arg;
    struct dhcp *dhcp;
    u8_t dhcp_last_state = DHCP_STATE_OFF;

    while (netif_is_up(netif))
    {
        dhcp = netif_dhcp_data(netif);

        if (dhcp == NULL)
        {
            dhcp_last_state = DHCP_STATE_OFF;
        }
        else if (dhcp_last_state != dhcp->state)
        {
            dhcp_last_state = dhcp->state;

            PRINTF(" DHCP state       : ");
            switch (dhcp_last_state)
            {
                case DHCP_STATE_OFF:
                    PRINTF("OFF");
                    break;
                case DHCP_STATE_REQUESTING:
                    PRINTF("REQUESTING");
                    break;
                case DHCP_STATE_INIT:
                    PRINTF("INIT");
                    break;
                case DHCP_STATE_REBOOTING:
                    PRINTF("REBOOTING");
                    break;
                case DHCP_STATE_REBINDING:
                    PRINTF("REBINDING");
                    break;
                case DHCP_STATE_RENEWING:
                    PRINTF("RENEWING");
                    break;
                case DHCP_STATE_SELECTING:
                    PRINTF("SELECTING");
                    break;
                case DHCP_STATE_INFORMING:
                    PRINTF("INFORMING");
                    break;
                case DHCP_STATE_CHECKING:
                    PRINTF("CHECKING");
                    break;
                case DHCP_STATE_BOUND:
                    PRINTF("BOUND");
                    break;
                case DHCP_STATE_BACKING_OFF:
                    PRINTF("BACKING_OFF");
                    break;
                default:
                    PRINTF("%u", dhcp_last_state);
                    assert(0);
                    break;
            }
            PRINTF("\r\n");

            if (dhcp_last_state == DHCP_STATE_BOUND)
            {
                PRINTF("\r\n IPv4 Address     : %s\r\n", ipaddr_ntoa(&netif->ip_addr));
                PRINTF(" IPv4 Subnet mask : %s\r\n", ipaddr_ntoa(&netif->netmask));
                PRINTF(" IPv4 Gateway     : %s\r\n\r\n", ipaddr_ntoa(&netif->gw));

                //Set the event so all the task waiting for TPCIP to start are signal
                tcpipBits = xEventGroupSetBits(tcpipEvent, 0b1);

                //Clear the event bit for TPCIP. All tasks shall be signed by now.
                //tcpipBits = xEventGroupClearBits(tcpipEvent, 0b1);
            }
        }

        sys_msleep(20U);
    }

    vTaskDelete(NULL);
}

/*!
 * @brief Initializes lwIP stack.
 */
static void stack_init(void)
{
    ip4_addr_t fsl_netif0_ipaddr, fsl_netif0_netmask, fsl_netif0_gw;
    ethernetif_config_t fsl_enet_config0 = {
        .phyAddress = EXAMPLE_PHY_ADDRESS,
        .clockName  = EXAMPLE_CLOCK_NAME,
        .macAddress = configMAC_ADDR,
#if defined(FSL_FEATURE_SOC_LPC_ENET_COUNT) && (FSL_FEATURE_SOC_LPC_ENET_COUNT > 0)
        .non_dma_memory = non_dma_memory,
#endif /* FSL_FEATURE_SOC_LPC_ENET_COUNT */
    };

#define WITHDHCP 1
#if WITHDHCP//DHCP INIT
    IP4_ADDR(&fsl_netif0_ipaddr, 0U, 0U, 0U, 0U);
	IP4_ADDR(&fsl_netif0_netmask, 0U, 0U, 0U, 0U);
	IP4_ADDR(&fsl_netif0_gw, 0U, 0U, 0U, 0U);

	tcpip_init(NULL, NULL);

	netifapi_netif_add(&fsl_netif0, &fsl_netif0_ipaddr, &fsl_netif0_netmask, &fsl_netif0_gw, &fsl_enet_config0,
					   ethernetif0_init, tcpip_input);
	netifapi_netif_set_default(&fsl_netif0);
	netifapi_netif_set_up(&fsl_netif0);

	netifapi_dhcp_start(&fsl_netif0);

	PRINTF("\r\n************************************************\r\n");
	PRINTF(" DHCP example\r\n");
	PRINTF("************************************************\r\n");

    if (sys_thread_new("print_dhcp", print_dhcp_state, &fsl_netif0, 1024, 3) == NULL)
    {
        LWIP_ASSERT("stack_init(): Task creation failed.", 0);
    }
#else
    tcpip_init(NULL, NULL);

    IP4_ADDR(&fsl_netif0_ipaddr, configIP_ADDR0, configIP_ADDR1, configIP_ADDR2, configIP_ADDR3);
    IP4_ADDR(&fsl_netif0_netmask, configNET_MASK0, configNET_MASK1, configNET_MASK2, configNET_MASK3);
    IP4_ADDR(&fsl_netif0_gw, configGW_ADDR0, configGW_ADDR1, configGW_ADDR2, configGW_ADDR3);

    netifapi_netif_add(&fsl_netif0, &fsl_netif0_ipaddr, &fsl_netif0_netmask, &fsl_netif0_gw, &fsl_enet_config0,
                       ethernetif0_init, tcpip_input);
    netifapi_netif_set_default(&fsl_netif0);
    netifapi_netif_set_up(&fsl_netif0);


    mdns_resp_init();
    mdns_resp_add_netif(&fsl_netif0, MDNS_HOSTNAME, 60);
    mdns_resp_add_service(&fsl_netif0, MDNS_HOSTNAME, "_http", DNSSD_PROTO_TCP, 80, 300, http_srv_txt, NULL);

    LWIP_PLATFORM_DIAG(("\r\n************************************************"));
    LWIP_PLATFORM_DIAG((" HTTP Server example"));
    LWIP_PLATFORM_DIAG(("************************************************"));
    LWIP_PLATFORM_DIAG((" IPv4 Address     : %u.%u.%u.%u", ((u8_t *)&fsl_netif0_ipaddr)[0],
                        ((u8_t *)&fsl_netif0_ipaddr)[1], ((u8_t *)&fsl_netif0_ipaddr)[2],
                        ((u8_t *)&fsl_netif0_ipaddr)[3]));
    LWIP_PLATFORM_DIAG((" IPv4 Subnet mask : %u.%u.%u.%u", ((u8_t *)&fsl_netif0_netmask)[0],
                        ((u8_t *)&fsl_netif0_netmask)[1], ((u8_t *)&fsl_netif0_netmask)[2],
                        ((u8_t *)&fsl_netif0_netmask)[3]));
    LWIP_PLATFORM_DIAG((" IPv4 Gateway     : %u.%u.%u.%u", ((u8_t *)&fsl_netif0_gw)[0], ((u8_t *)&fsl_netif0_gw)[1],
                        ((u8_t *)&fsl_netif0_gw)[2], ((u8_t *)&fsl_netif0_gw)[3]));
    LWIP_PLATFORM_DIAG((" mDNS hostname    : %s", MDNS_HOSTNAME));
    LWIP_PLATFORM_DIAG(("************************************************"));
#endif
}

/*!
 * @brief The main function containing server thread.
 */
static void main_thread(void *arg)
{
    LWIP_UNUSED_ARG(arg);

    stack_init();
    vTaskDelete(NULL);
}

/*!
 * @brief Main function
 */
int main(void)
{
    SYSMPU_Type *base = SYSMPU;
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    SEGGER_SYSVIEW_Conf();

    /* Disable SYSMPU. */
    base->CESR &= ~SYSMPU_CESR_VLD_MASK;

    /* create server thread in RTOS */
    if (sys_thread_new("main", main_thread, NULL, 1024, 3) == NULL)
        LWIP_ASSERT("main(): Task creation failed.", 0);

	//RTOS objects needed to be started before the scheduler:
    tcpipEvent = xEventGroupCreate();
	servo_queue = xQueueCreate(10, MAX_CMD_LENGTH);
	servo_mutex = xSemaphoreCreateMutex();
	/* Enable queue view in MCUX IDE FreeRTOS TAD plugin. */
	if (servo_queue != NULL)
	{
		vQueueAddToRegistry(servo_queue, "servo");
	}

	//RTOS tasks needed to be started before the scheduler:
	if (xTaskCreate(keepalive_task, "KeepAlive_task", configMINIMAL_STACK_SIZE + 500, NULL, keepalive_task_PRIORITY, NULL) != pdPASS)
	{
		PRINTF("Task creation failed!.\r\n");
		while (1)
			;
	}
	if (xTaskCreate(servo_task, "servo_task", configMINIMAL_STACK_SIZE + 10, NULL, servo_task_PRIORITY, NULL) != pdPASS)
	{
		PRINTF("Task creation failed!.\r\n");
		while (1)
			;
	}

	if (xTaskCreate(tcpipserver_task, "tcpipserver_task", configMINIMAL_STACK_SIZE + 100, NULL, tcpipserver_task_PRIORITY, NULL) != pdPASS)
	{
		PRINTF("Task creation failed!.\r\n");
		while (1)
			;
	}

	PRINTF("\n\rRunning the NXP-NCI task.\n\r");

	/* Create NFC task */
	if (xTaskCreate((TaskFunction_t) task_nfc,
					(const char*) "NFC_task",
					TASK_NFC_STACK_SIZE,
					NULL,
					TASK_NFC_STACK_PRIO,
					NULL) != pdPASS)
	{
		PRINTF("Failed to create NFC task");
	}

    vTaskStartScheduler();

    /* Will not get here unless a task calls vTaskEndScheduler ()*/
    return 0;
}

/*!
 * @brief Task responsible for providing a keepalive signal.
 */
static void keepalive_task(void *pvParameters)
{
	int i=0;

	PRINTF("RTT block address is: 0x%x \r\n", &_SEGGER_RTT);

	tcpipBits = xEventGroupWaitBits(
            tcpipEvent,    /* The event group being tested. */
            0b1,           /* The bit within the event group to wait for: bit 0. */
			pdFALSE,       /* BIT 0 should NOT be cleared before returning. */
            pdFALSE,       /* Don't wait for both bits, either bit will do. */
			portMAX_DELAY);/* Wait a maximum of 100ms for either bit to be set. */

	for (;;)
    {
		PRINTF("KeepAlive: %d.\r\n", i++);
		if (g_ButtonPress)
		{
			/* Reset state of button. */
			g_ButtonPress = false;
			PRINTF("Detected a Push Button Pressed.\r\n");
		}
        vTaskDelay(20000);
		tcp_webclilent();
    }
}

U32 SEGGER_SYSVIEW_X_GetTimestamp(void)
{
    return __get_IPSR() ? xTaskGetTickCountFromISR() : xTaskGetTickCount();
}
#endif

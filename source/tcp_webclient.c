/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

#include "lwip/opt.h"
#include "event_groups.h"

#define TAG_ID_STRING_SIZE 14
extern EventGroupHandle_t tcpipEvent;
extern EventBits_t tcpipBits;
extern char TagUIDstr[TAG_ID_STRING_SIZE];

#if LWIP_NETCONN

#include "lwip/sys.h"
#include "lwip/api.h"
/*-----------------------------------------------------------------------------------*/
void tcp_webclilent(void)
{
  struct netconn *conn;
  //err_t err;
  ip4_addr_t ipaddr;
  char HTTPrequest[200] = {0};

  //Wait until TCPIP stack is up and running
  tcpipBits = xEventGroupWaitBits(
	  tcpipEvent,    /* The event group being tested. */
	  0b1,           /* The bit within the event group to wait for: bit 0. */
	  pdFALSE,       /* BIT 0 should NOT be cleared before returning. */
	  pdFALSE,       /* Don't wait for both bits, either bit will do. */
	  portMAX_DELAY);/* Wait a maximum of 100ms for either bit to be set. */

  /* Create a new connection identifier. */
  /* Bind connection to well known port number 7. */
#if LWIP_IPV6
  conn = netconn_new(NETCONN_TCP_IPV6);
  netconn_bind(conn, IP6_ADDR_ANY, 7);
#else /* LWIP_IPV6 */
  conn = netconn_new(NETCONN_TCP);
  netconn_bind(conn, IP_ADDR_ANY, 7);
#endif /* LWIP_IPV6 */
  LWIP_ERROR("tcpecho: invalid conn", (conn != NULL), return;);


  //http://187.213.214.87:1031/
  IP4_ADDR(&ipaddr, 187,213,214,87);
  netconn_connect(conn, &ipaddr, 1031);

  //Test from the web browser
  //http://10.215.170.119/datalog.php?frdm_id=FRMD-Profe&sensor=acc&data=123123
  //http://192.168.7.2/nfcauth.php?tagid=4474c7a1e4e81
  //http://192.168.7.2/nfcreg.php?tagid=4474c7a1e4e81&name=Luis&lastname=Garabito&access=Mortal
  //Test from the web browser

  sprintf(HTTPrequest, "GET /nfcreg.php?tagid=%s&name=Luis&lastname=Garabito&access=Mortal HTTP/1.0\r\n\r\n", TagUIDstr);

  //sprintf(HTTPrequest, "GET /nfcauth.php?tagid=%s HTTP/1.0\r\n\r\n", TagUIDstr);
 // sprintf(HTTPrequest, "GET /datalog.php?frdm_id=Iteso&sensor=LightSensor&data=%d HTTP/1.0\r\n\r\n", 98765);
  netconn_write(conn, HTTPrequest,strlen(HTTPrequest),NETCONN_COPY);
  PRINTF("Sensor data Logged\n\r");
  PRINTF("%s",TagUIDstr);
  netconn_close(conn);
  netconn_delete(conn);

}
/*-----------------------------------------------------------------------------------*/

#endif /* LWIP_NETCONN */

/*
 * doip_client.h
 *
 *  Created on: 2022Äê4ÔÂ7ÈÕ
 *      Author: Administrator
 */

#ifndef DOIPC_DOIP_CLIENT_H_
#define DOIPC_DOIP_CLIENT_H_

#include "stdint.h"

typedef void (*doip_recv_cb)(const uint8_t *payload, uint16_t len);

int doip_routing_active(void);
int doip_client_send(uint8_t *payload, uint32_t len);
int doip_client_init(void);
int doiphdr_create(void);
void doip_register_cb(doip_recv_cb cb);
void set_ipaddr(char *local, char *remote);

#endif /* DOIPC_DOIP_CLIENT_H_ */

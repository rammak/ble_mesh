/*
 * common.h
 *
 *  Created on: Dec. 20, 2021
 *      Author: rutwij
 */

#ifndef MESH_COMMON_H_
#define MESH_COMMON_H_

#include <stdint.h>
#include "sl_bluetooth.h"

struct __attribute__((__packed__)) hello_packet {
  uint8_t netID[2];
  uint8_t address;
  uint8_t flags;
  uint8_t map[16];
  uint32_t counter;
};

struct __attribute__((__packed__)) data_packet {
  uint8_t netID[2];
  uint8_t src_address;
  uint8_t dst_address;
  uint8_t data[16];
  uint32_t counter;
};



int enable_hello(int interval);
int disable_hello();
void bcn_setup_adv_hello();
void bcn_setup_scan_hello();
void bcn_process_hello(sl_bt_evt_scanner_scan_report_t *hello);
void bcn_handle_expiry(struct data_packet data);

void mesh_send(uint8_t dst_address, bool state);
void mesh_receive(struct data_packet data);
void mesh_message_reset();

#endif /* MESH_COMMON_H_ */

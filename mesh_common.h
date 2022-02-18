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

int enable_hello(int interval);
int disable_hello();
void bcn_setup_adv_hello();
void bcn_setup_scan_hello();
void bcn_process_hello(sl_bt_evt_scanner_scan_report_t *hello);
void bcn_handle_expiry();

#endif /* MESH_COMMON_H_ */

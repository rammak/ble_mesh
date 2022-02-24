/*
 * mesh_message.c
 *
 *  Created on: Feb. 18, 2022
 *      Author: rutwij
 */

#include <math.h>
#include <mesh_common.h>
#include <mesh_config.h>
#include <stdbool.h>
#include <sys/_stdint.h>
#include "sl_bluetooth.h"
#include "app_assert.h"
#include "gatt_db.h"
#include "mesh_routing_table.h"
#include "sl_simple_led_instances.h"

struct data_packet dat;
static uint8_t advertising_set_handle = 0xff;

bool initialized = false;
static uint8_t own_address = 0;

void mesh_send(uint8_t dst_address, bool state) {
  sl_status_t sc;

  if(initialized == false) {
    // Create an advertising set.
    sc = sl_bt_advertiser_create_set(&advertising_set_handle);
    app_assert_status(sc);

    // Set advertising parameters. 20ms advertisement interval.
    sc = sl_bt_advertiser_set_timing(
        advertising_set_handle,
      32,
      32,
      0,       // adv. duration
      0);      // max. num. adv. events
    app_assert_status(sc);

    sc = sl_bt_advertiser_clear_configuration(
        advertising_set_handle,
            1);
    app_assert_status(sc);

    int16_t outpower;
    sc = sl_bt_advertiser_set_tx_power(advertising_set_handle, -30, &outpower);
    app_assert_status(sc);

    bd_addr addrin, addrout;
    addrin.addr[0] = 0x22;
    addrin.addr[1] = 0x22;
    addrin.addr[2] = 0x22;
    addrin.addr[3] = 0x22;
    addrin.addr[4] = 0x22;
    addrin.addr[5] = 0x22;
    sc = sl_bt_advertiser_set_random_address(
        advertising_set_handle,
        3,
        addrin,
        &addrout);
    app_assert_status(sc);

    bd_addr addr;
    sc = sl_bt_system_get_identity_address(&addr, 0);
    app_assert_status(sc);

    own_address = addr.addr[0] % 64;   // statically calculate net address

    dat.netID[0] = 0xab;
    dat.netID[1] = 0xcd;
    dat.src_address = own_address;
    dat.dst_address = dst_address;
    dat.counter = 0;

    if(state == true)
      memset(&dat.data[0], 0x11, 16);
    else
      memset(&dat.data[0], 0x22, 16);


    // Set advertising data
    // First 3 bytes are for the flags
    // 4th byte is the length (1a = 26)
    // 5th byte is the type (ff = manufacturer data)
    // 6th and 7th bytes are the manufacturer ID (0x02ff = Silabs)
    uint8_t data[31] = {0x02, 0x01, 0x06, 0x1b, 0xff, 0x33, 0x33};
  //  data[3] = adv_length + 2;
    memcpy(data + 7, &dat, 24);
    sc = sl_bt_advertiser_set_data(advertising_set_handle,
                                   0,
                                   31,
                                   data);
    app_assert_status(sc);

    // Start advertising in user mode and disable connections.
    sc = sl_bt_advertiser_start(
        advertising_set_handle,
      sl_bt_advertiser_user_data,
      sl_bt_advertiser_non_connectable);
    app_assert_status(sc);

    initialized = true;
  }
  else {
      dat.netID[0] = 0xab;
      dat.netID[1] = 0xcd;
      dat.src_address = own_address;
      dat.dst_address = dst_address;
      dat.counter = 0;

      if(state == true)
        memset(&dat.data[0], 0x11, 16);
      else
        memset(&dat.data[0], 0x22, 16);

      uint8_t data[31] = {0x02, 0x01, 0x06, 0x1b, 0xff, 0x33, 0x33};
      memcpy(data + 7, &dat, 24);
      sc = sl_bt_advertiser_set_data(advertising_set_handle,
                                     0,
                                     31,
                                     data);
      app_assert_status(sc);
  }
}

void mesh_receive(struct data_packet data) {
  if(data.data[0] == 0x11) {
      sl_led_turn_off(SL_SIMPLE_LED_INSTANCE(0));
  }
  if(data.data[0] == 0x22) {
      sl_led_turn_on(SL_SIMPLE_LED_INSTANCE(0));
  }
}

void mesh_message_reset() {
  initialized = false;
}

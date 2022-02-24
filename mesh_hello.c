/*
 *  hello.c
 *
 *  Created on: Feb. 14, 2021
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

struct hello_packet hel;

static uint8_t advertising_set_handle = 0xff;

//struct route route_table[64];
uint8_t route_table[16] = {0};
unsigned int route_update[64] = {0};

static uint8_t own_address = 0;    // own net address
unsigned int current_time = 0;

static void update_table(uint8_t label, uint8_t distance) {
  int shift = label % 4;
  int index = (int)(label / 4);
  uint8_t mask = ~(0b11 << (6 - (shift * 2)));
  uint8_t flag = (distance) << (6 - (shift * 2));
  route_table[index] &= mask;
  route_table[index] |= flag;

  route_update[label] = current_time;
}


static int populate_table(struct hello_packet *h) {
//  int j = 6;
//  for(int i = 0; i < 64; i++) {
//      int q = (int) (i / 4);
////      int r = (int) (i % 8);
//      h->map[q] = (route_table[i].distance & 0b00000011) << j;
//      if(j == 0)
//        j = 6;
//      else
//        j -= 2;
//  }

//  int k = 0;
//  for(int i = 0, j = 62; i < 64; i++, j -= 2){
//
//  }

//  for(int i = 0; i < 4; i++) {
//      uint8_t b;
//      for(int j = 0; j < 4; j++) {
//          int index = (i * 4) + j;
//          b |= (route_table[index].distance & 0b00000011) << (6 - (2 * j));
//      }
//      h->map[i] = b;
//  }
  memcpy(&(h->map[0]), route_table, 16);

}

static void update_hello() {

  sl_status_t sc;
  uint8_t data[31] = {0x02, 0x01, 0x06, 0x1b, 0xff, 0x33, 0x33};
  //  data[3] = adv_length + 2;
  populate_table(&hel);
  memcpy(data + 7, &hel, 24);
  sc = sl_bt_advertiser_set_data(advertising_set_handle,
                                 0,
                                 31,
                                 data);
  app_assert_status(sc);
}

//int enable_hello(int interval) {
////  if(interval < MIN_HELLO || interval > MAX_HELLO)
////    return 1;
//  hello_interval = interval;
//
//  sl_status_t sc;
//
//  // Create an advertising set.
//  sc = sl_bt_advertiser_create_set(&advertising_set_handle);
//  app_assert_status(sc);
//
//  struct hello_packet adv_data;
//  adv_data.netID[0] = 1;
//  adv_data.netID[1] = 2;
//  adv_data.netID[2] = 3;
//  adv_data.netID[3] = 4;
//  adv_data.address = 0;
//  adv_data.next_MAC[0] = 4;
//  adv_data.next_MAC[0] = 5;
//  adv_data.next_MAC[0] = 6;
//  adv_data.next_MAC[0] = 7;
//  adv_data.next_MAC[0] = 8;
//  adv_data.next_MAC[0] = 9;
//  adv_data.next_address = 1;
//  adv_data.flags = 0;
//
//  for(int i = 0; i < 32; i++) {
//      adv_data.map[i] = i;
//  }
//  adv_data.counter = 24;
//
//  uint8_t adv2[25];
//
//  // Set custom advertising data.
//  sc = sl_bt_advertiser_set_data(advertising_set_handle,
//                                 0,
//                                 sizeof(adv2),
//                                 (uint8_t *)(&adv2));
//  app_assert_status(sc);
//
//  // Set advertising parameters. 100ms advertisement interval.
//  sc = sl_bt_advertiser_set_timing(
//      advertising_set_handle,
//      0x20,     // min. adv. interval (milliseconds * 1.6)
//      160,     // max. adv. interval (milliseconds * 1.6)
//      0,       // adv. duration
//      0);      // max. num. adv. events
//  app_assert_status(sc);
//
//  bd_addr addr, addrout;
//  addr.addr[0] = 0x11;
//  addr.addr[1] = 0x11;
//  addr.addr[2] = 0x11;
//  addr.addr[3] = 0x11;
//  addr.addr[4] = 0x11;
//  addr.addr[5] = 0x11;
//  sc = sl_bt_advertiser_set_random_address(
//      advertising_set_handle,
//      3,
//      addr,
//      &addrout);
//
//  // Start advertising in user mode and disable connections.
//  sc = sl_bt_advertiser_start(
//      advertising_set_handle,
//    sl_bt_advertiser_user_data,
//    sl_bt_advertiser_non_connectable);
//  app_assert_status(sc);
//
//  return 0;
//}
//
//int disable_hello() {
//  return 0;
//}

void bcn_setup_adv_hello()
{
  sl_status_t sc;

  // Create an advertising set.
  sc = sl_bt_advertiser_create_set(&advertising_set_handle);
  app_assert_status(sc);

  // Set advertising parameters. 100ms advertisement interval.
  sc = sl_bt_advertiser_set_timing(
      advertising_set_handle,
//    (int)floor(adv_interval / 0.625),     // min. adv. interval (milliseconds * 1.6)
//    (int)floor(adv_interval / 0.625),     // max. adv. interval (milliseconds * 1.6)
    160,
    160,
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

//  bd_addr addr, addrout;
//  addr.addr[0] = 0x33;
//  addr.addr[1] = 0x33;
//  addr.addr[2] = 0x33;
//  addr.addr[3] = 0x33;
//  addr.addr[4] = 0x33;
//  addr.addr[5] = 0x33;


  bd_addr addr;
  sc = sl_bt_system_get_identity_address(&addr, 0);
  app_assert_status(sc);

  own_address = addr.addr[0] % 64;   // statically calculate net address

  hel.netID[0] = 0xab;
  hel.netID[1] = 0xcd;
  hel.address = own_address;
  hel.flags = 0xee;
  hel.counter = 0;
  memset(&hel.map[0], 0x00, 16);
//  sc = sl_bt_advertiser_set_random_address(
//      advertising_set_handle,
//      3,
//      addr,
//      &addrout);

  // Set advertising data
  // First 3 bytes are for the flags
  // 4th byte is the length (1a = 26)
  // 5th byte is the type (ff = manufacturer data)
  // 6th and 7th bytes are the manufacturer ID (0x02ff = Silabs)
  uint8_t data[31] = {0x02, 0x01, 0x06, 0x1b, 0xff, 0x33, 0x33};
//  data[3] = adv_length + 2;
  memcpy(data + 7, &hel, 24);
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
}

void bcn_setup_scan_hello() {
  sl_status_t sc;

  sc = sl_bt_scanner_set_mode(1, 0); // 1M PHY and passive scanning
  app_assert_status(sc);

  sc = sl_bt_scanner_start(1, 1); // 1M PHY and generic discovery
  app_assert_status(sc);
}

void bcn_process_hello(sl_bt_evt_scanner_scan_report_t *hello) {
  sl_status_t sc;

  uint8_t temp[31];
  struct hello_packet newp;
  memcpy(temp, hello->data.data, 31);
  memcpy(&newp, &temp[7], 24);

  // is it our packet? - If flag is 0xee then yes
  if(newp.netID[0] == 0xab && newp.netID[1] == 0xcd) {
      if(newp.flags == 0xee) {            // hello message

        if(abs(hello->rssi) < 0x20) {
            update_table(newp.address, 0b11);
        }
        else if(abs(hello->rssi) < 0x30) {
            update_table(newp.address, 0b10);
        }
        else {
            update_table(newp.address, 0b01);
        }
    //    hel.counter = abs(hello->rssi);
        update_hello();
      }
      else {
          struct data_packet dat;
          memcpy(&dat, &newp, 24);
          mesh_receive(dat);
      }
  }
}

// this function expires old routes
void handle_expiry() {
//  hel.counter++;
  current_time++;
  for(int i = 0; i < 64; i++) {
      if(current_time - route_update[i] > 3) {
          update_table(i, 0);
      }
  }
  update_hello();
}

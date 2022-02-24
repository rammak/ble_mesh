/***************************************************************************//**
 * @file
 * @brief Core application logic.
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

//#include <mesh_common.h>
#include "mesh_common.h"
#include "sl_bluetooth.h"
#include "app_assert.h"
#include "gatt_db.h"
#include "app.h"
#include "math.h"
#include "sl_simple_button_instances.h"
#include "sl_simple_led_instances.h"

// Macros.
#define UINT16_TO_BYTES(n)            ((uint8_t) (n)), ((uint8_t)((n) >> 8))
#define UINT16_TO_BYTE0(n)            ((uint8_t) (n))
#define UINT16_TO_BYTE1(n)            ((uint8_t) ((n) >> 8))

// The advertising set handle allocated from Bluetooth stack.
static uint8_t advertising_set_handle = 0xff;
static uint8_t advertising_set_handle2 = 0xff;
static uint8_t advertising_set_handle3 = 0xff;

bd_addr *remote_address;
bd_addr stored[50];
int cnt = 0;

/**************************************************************************//**
 * Set up a custom advertisement package according to iBeacon specifications.
 * The advertisement package is 30 bytes long.
 * See the iBeacon specification for further details.
 *****************************************************************************/
static void bcn_setup_adv_beaconing(void);
static void bcn_setup_adv_beaconing2(void);
static void bcn_setup_adv_beaconing3(void);

bool report_button_flag = false;

/**************************************************************************//**
 * Application Init.
 *****************************************************************************/
SL_WEAK void app_init(void)
{
  /////////////////////////////////////////////////////////////////////////////
  // Put your additional application init code here!                         //
  // This is called once during start-up.                                    //
  /////////////////////////////////////////////////////////////////////////////
}

/**************************************************************************//**
 * Application Process Action.
 *****************************************************************************/
SL_WEAK void app_process_action(void)
{
  /////////////////////////////////////////////////////////////////////////////
  // Put your additional application code here!                              //
  // This is called infinitely.                                              //
  // Do not call blocking functions from here!                               //
  /////////////////////////////////////////////////////////////////////////////
}

void sl_button_on_change(const sl_button_t *handle)
{
  if (SL_SIMPLE_BUTTON_INSTANCE(0) == handle) {
//      sl_led_toggle(SL_SIMPLE_LED_INSTANCE(0));

      if(report_button_flag == false) {
          report_button_flag = true;
          sl_led_turn_on(SL_SIMPLE_LED_INSTANCE(0));
          mesh_send(54, true);
      }
      else {
          report_button_flag = false;
          sl_led_turn_off(SL_SIMPLE_LED_INSTANCE(0));
          mesh_send(54, false);
      }
//      switch (sl_button_get_state(SL_SIMPLE_BUTTON_INSTANCE(0))) {
////    report_button_flag = true;
////    sl_led_toggle(SL_SIMPLE_LED_INSTANCE(0));
//        case SL_SIMPLE_BUTTON_PRESSED:
////          mesh_send(54, false);
//          report_button_flag = true;
//          sl_led_turn_on(SL_SIMPLE_LED_INSTANCE(0));
//          break;
//
//        case SL_SIMPLE_BUTTON_RELEASED:
////          mesh_send(54, true);
//          report_button_flag = false;
//          sl_led_turn_on(SL_SIMPLE_LED_INSTANCE(0));
//          break;
//      }

  }
}

/**************************************************************************//**
 * Bluetooth stack event handler.
 * This overrides the dummy weak implementation.
 *
 * @param[in] evt Event coming from the Bluetooth stack.
 *****************************************************************************/
void sl_bt_on_event(sl_bt_msg_t *evt)
{
  sl_status_t sc;
  int16_t ret_power_min, ret_power_max;
  switch (SL_BT_MSG_ID(evt->header)) {
    // -------------------------------
    // This event indicates the device has started and the radio is ready.
    // Do not call any stack command before receiving this boot event!
    case sl_bt_evt_system_boot_id:
      // Set 0 dBm maximum Transmit Power.
      sc = sl_bt_system_set_tx_power(SL_BT_CONFIG_MIN_TX_POWER, 0,
                                     &ret_power_min, &ret_power_max);
      app_assert_status(sc);
      (void)ret_power_min;
      (void)ret_power_max;
      // Initialize iBeacon ADV data.
//      bcn_setup_adv_beaconing();
//      bcn_setup_adv_beaconing2();
//      bcn_setup_adv_beaconing3();
//      enable_hello(500);
      bcn_setup_adv_hello();
      bcn_setup_scan_hello();
      sl_bt_system_set_soft_timer(32768, 1, 0);   // timer for route expiry

      sl_button_enable(SL_SIMPLE_BUTTON_INSTANCE(0));

      break;

    ///////////////////////////////////////////////////////////////////////////
    // Add additional event handlers here as your application requires!      //
    ///////////////////////////////////////////////////////////////////////////

    case sl_bt_evt_system_soft_timer_id:
      if(evt->data.evt_system_soft_timer.handle == 1) { // if route expiry timer
          handle_expiry();
      }
      break;

    case sl_bt_evt_scanner_scan_report_id:
      bcn_process_hello(&(evt->data.evt_scanner_scan_report));
      break;

//    case  sl_bt_evt_advertiser_timeout_id:
//      mesh_message_reset();
//      break;
    // -------------------------------
    // Default event handler.
    default:
      break;
  }
}




static void bcn_setup_adv_beaconing(void)
{
  sl_status_t sc;

  struct {
    uint8_t flags_len;     // Length of the Flags field.
    uint8_t flags_type;    // Type of the Flags field.
    uint8_t flags;         // Flags field.
    uint8_t mandata_len;   // Length of the Manufacturer Data field.
    uint8_t mandata_type;  // Type of the Manufacturer Data field.
    uint8_t comp_id[2];    // Company ID field.
    uint8_t beac_type[2];  // Beacon Type field.
    uint8_t uuid[16];      // 128-bit Universally Unique Identifier (UUID). The UUID is an identifier for the company using the beacon.
    uint8_t maj_num[2];    // Beacon major number. Used to group related beacons.
    uint8_t min_num[2];    // Beacon minor number. Used to specify individual beacons within a group.
    uint8_t tx_power;      // The Beacon's measured RSSI at 1 meter distance in dBm. See the iBeacon specification for measurement guidelines.
  }
  bcn_beacon_adv_data
    = {
    // Flag bits - See Bluetooth 4.0 Core Specification , Volume 3, Appendix C, 18.1 for more details on flags.
    2,            // Length of field.
    0x01,         // Type of field.
    0x04 | 0x02,  // Flags: LE General Discoverable Mode, BR/EDR is disabled.

    // Manufacturer specific data.
    26,   // Length of field.
    0xFF, // Type of field.

    // The first two data octets shall contain a company identifier code from
    // the Assigned Numbers - Company Identifiers document.
    // 0x004C = Apple
    { UINT16_TO_BYTES(0x004C) },

    // Beacon type.
    // 0x0215 is iBeacon.
    { UINT16_TO_BYTE1(0x0215), UINT16_TO_BYTE0(0x0215) },

    // 128 bit / 16 byte UUID
    { 0xE2, 0xC5, 0x6D, 0xB5, 0xDF, 0xFB, 0x48, 0xD2, \
      0xB0, 0x60, 0xD0, 0xF5, 0xA7, 0x10, 0x96, 0xE0 },

    // Beacon major number.
    // Set to 34987 and converted to correct format.
    { UINT16_TO_BYTE1(34987), UINT16_TO_BYTE0(34987) },

    // Beacon minor number.
    // Set as 1025 and converted to correct format.
    { UINT16_TO_BYTE1(1025), UINT16_TO_BYTE0(1025) },

    // The Beacon's measured RSSI at 1 meter distance in dBm.
    // 0xD7 is -41dBm
    0xD7
    };

  // Create an advertising set.
  sc = sl_bt_advertiser_create_set(&advertising_set_handle);
  app_assert_status(sc);

  // Set custom advertising data.
  sc = sl_bt_advertiser_set_data(advertising_set_handle,
                                 0,
                                 sizeof(bcn_beacon_adv_data),
                                 (uint8_t *)(&bcn_beacon_adv_data));
  app_assert_status(sc);

  // Set advertising parameters. 100ms advertisement interval.
  sc = sl_bt_advertiser_set_timing(
    advertising_set_handle,
    160,     // min. adv. interval (milliseconds * 1.6)
    160,     // max. adv. interval (milliseconds * 1.6)
    0,       // adv. duration
    0);      // max. num. adv. events
  app_assert_status(sc);

  // Start advertising in user mode and disable connections.
  sc = sl_bt_advertiser_start(
    advertising_set_handle,
    sl_bt_advertiser_user_data,
    sl_bt_advertiser_non_connectable);
  app_assert_status(sc);
}


static void bcn_setup_adv_beaconing2(void)
{
  sl_status_t sc;

  struct {
    uint8_t flags_len;     // Length of the Flags field.
    uint8_t flags_type;    // Type of the Flags field.
    uint8_t flags;         // Flags field.
    uint8_t mandata_len;   // Length of the Manufacturer Data field.
    uint8_t mandata_type;  // Type of the Manufacturer Data field.
    uint8_t comp_id[2];    // Company ID field.
    uint8_t beac_type[2];  // Beacon Type field.
    uint8_t uuid[16];      // 128-bit Universally Unique Identifier (UUID). The UUID is an identifier for the company using the beacon.
    uint8_t maj_num[2];    // Beacon major number. Used to group related beacons.
    uint8_t min_num[2];    // Beacon minor number. Used to specify individual beacons within a group.
    uint8_t tx_power;      // The Beacon's measured RSSI at 1 meter distance in dBm. See the iBeacon specification for measurement guidelines.
  }
  bcn_beacon_adv_data
    = {
    // Flag bits - See Bluetooth 4.0 Core Specification , Volume 3, Appendix C, 18.1 for more details on flags.
    2,            // Length of field.
    0x01,         // Type of field.
    0x04 | 0x02,  // Flags: LE General Discoverable Mode, BR/EDR is disabled.

    // Manufacturer specific data.
    26,   // Length of field.
    0xFF, // Type of field.

    // The first two data octets shall contain a company identifier code from
    // the Assigned Numbers - Company Identifiers document.
    // 0x004C = Apple
    { UINT16_TO_BYTES(0x00E0) },

    // Beacon type.
    // 0x0215 is iBeacon.
    { UINT16_TO_BYTE1(0x0215), UINT16_TO_BYTE0(0x0215) },

    // 128 bit / 16 byte UUID
    { 0xE2, 0xC5, 0x6D, 0xB5, 0xDF, 0xFB, 0x48, 0xD2, \
      0xB0, 0x60, 0xD0, 0xF5, 0xA7, 0x10, 0x96, 0xE0 },

    // Beacon major number.
    // Set to 34987 and converted to correct format.
    { UINT16_TO_BYTE1(34987), UINT16_TO_BYTE0(34987) },

    // Beacon minor number.
    // Set as 1025 and converted to correct format.
    { UINT16_TO_BYTE1(1025), UINT16_TO_BYTE0(1025) },

    // The Beacon's measured RSSI at 1 meter distance in dBm.
    // 0xD7 is -41dBm
    0xD7
    };

  // Create an advertising set.
  sc = sl_bt_advertiser_create_set(&advertising_set_handle2);
  app_assert_status(sc);

  // Set custom advertising data.
  sc = sl_bt_advertiser_set_data(advertising_set_handle2,
                                 0,
                                 sizeof(bcn_beacon_adv_data),
                                 (uint8_t *)(&bcn_beacon_adv_data));
  app_assert_status(sc);

  // Set advertising parameters. 100ms advertisement interval.
  sc = sl_bt_advertiser_set_timing(
    advertising_set_handle2,
    0x20,     // min. adv. interval (milliseconds * 1.6)
    160,     // max. adv. interval (milliseconds * 1.6)
    0,       // adv. duration
    0);      // max. num. adv. events
  app_assert_status(sc);

  bd_addr addr, addrout;
  addr.addr[0] = 0x11;
  addr.addr[1] = 0x11;
  addr.addr[2] = 0x11;
  addr.addr[3] = 0x11;
  addr.addr[4] = 0x11;
  addr.addr[5] = 0x11;
  sc = sl_bt_advertiser_set_random_address(
      advertising_set_handle2,
      3,
      addr,
      &addrout);

  // Start advertising in user mode and disable connections.
  sc = sl_bt_advertiser_start(
    advertising_set_handle2,
    sl_bt_advertiser_user_data,
    sl_bt_advertiser_non_connectable);
  app_assert_status(sc);
}

static void bcn_setup_adv_beaconing3(void)
{
  sl_status_t sc;

  // Create an advertising set.
  sc = sl_bt_advertiser_create_set(&advertising_set_handle3);
  app_assert_status(sc);

  uint8_t adv_data2[180] = {0x68, 0x65, 0x6C, 0x6C, 0x6F, 0x64, 0x61, 0x72, 0x6B, 0x6E, 0x65, 0x73, 0x73, 0x6D, 0x79, 0x6F, 0x6C, 0x64, 0x66, 0x72, 0x68, 0x65, 0x6C, 0x6C, 0x6F, 0x64, 0x61, 0x72, 0x6B, 0x6E, 0x65, 0x73, 0x73, 0x6D, 0x79, 0x6F, 0x6C, 0x64, 0x66, 0x72, 0x68, 0x65, 0x6C, 0x6C, 0x6F, 0x64, 0x61, 0x72, 0x6B, 0x6E, 0x65, 0x73, 0x73, 0x6D, 0x79, 0x6F, 0x6C, 0x64, 0x66, 0x72, 0x68, 0x65, 0x6C, 0x6C, 0x6F, 0x64, 0x61, 0x72, 0x6B, 0x6E, 0x65, 0x73, 0x73, 0x6D, 0x79, 0x6F, 0x6C, 0x64, 0x66, 0x72, 0x68, 0x65, 0x6C, 0x6C, 0x6F, 0x64, 0x61, 0x72, 0x6B, 0x6E, 0x65, 0x73, 0x73, 0x6D, 0x79, 0x6F, 0x6C, 0x64, 0x66, 0x72, 0x68, 0x65, 0x6C, 0x6C, 0x6F, 0x64, 0x61, 0x72, 0x6B, 0x6E, 0x65, 0x73, 0x73, 0x6D, 0x79, 0x6F, 0x6C, 0x64, 0x66, 0x72, 0x68, 0x65, 0x6C, 0x6C, 0x6F, 0x64, 0x61, 0x72, 0x6B, 0x6E, 0x65, 0x73, 0x73, 0x6D, 0x79, 0x6F, 0x6C, 0x64, 0x66, 0x72, 0x68, 0x65, 0x6C, 0x6C, 0x6F, 0x64, 0x61, 0x72, 0x6B, 0x6E, 0x65, 0x73, 0x73, 0x6D, 0x79, 0x6F, 0x6C, 0x64, 0x66, 0x72, 0x68, 0x65, 0x6C, 0x6C, 0x6F, 0x64, 0x61, 0x72, 0x6B, 0x6E, 0x65, 0x73, 0x73, 0x6D, 0x79, 0x6F, 0x6C, 0x64, 0x66, 0x72};

  // Set advertising parameters. 100ms advertisement interval.
  sc = sl_bt_advertiser_set_timing(
    advertising_set_handle3,
    0x20,     // min. adv. interval (milliseconds * 1.6)
    160,     // max. adv. interval (milliseconds * 1.6)
    0,       // adv. duration
    0);      // max. num. adv. events
  app_assert_status(sc);

  sc = sl_bt_advertiser_clear_configuration(
          advertising_set_handle3,
          1);
  app_assert_status(sc);

  bd_addr addr, addrout;
  addr.addr[0] = 0x22;
  addr.addr[1] = 0x22;
  addr.addr[2] = 0x22;
  addr.addr[3] = 0x22;
  addr.addr[4] = 0x22;
  addr.addr[5] = 0x22;
  sc = sl_bt_advertiser_set_random_address(
      advertising_set_handle3,
      3,
      addr,
      &addrout);

  sc = sl_bt_advertiser_set_phy(
      advertising_set_handle3,
      sl_bt_gap_1m_phy,
      sl_bt_gap_2m_phy);
  app_assert_status(sc);

  uint8_t amout_bytes = 0;
  const uint8_t service_uuid[16] = {0};
  uint8_t extended_buf[200];
  /* https://www.bluetooth.com/specifications/assigned-numbers/company-identifiers - To get your company ID*/
  uint16_t company_id = 0x02FF; // 0x02FF - Silicon Labs' company ID

  // Initialize advertising data with Flag and Local name
  uint8_t adv_data[253] = {
      0x02, // Length of flag
      0x01, // Type flag
      0x06, // Flag data
      0x05, // Length of Local name
      0x09, // Type local name
      'A', 'd', 'v', 'C', // Local name
  };
  // Byte amount in advertising data buffer now increased by 9
  amout_bytes += 9;

  // Adding Service UUID to the advertising - len of UUID is 16 bytes
  adv_data[amout_bytes++] = 0x11;// length 17 bytes
  adv_data[amout_bytes++] = 0x06;//more_128_uuids
  memcpy(adv_data+amout_bytes, service_uuid, 16);//adding UUID
  amout_bytes += 16;

  // Prepare manufacturer_specific_data
  memcpy(extended_buf, (uint8_t *)&company_id, 2);
  for (uint8_t i = 2; i < 200; i++) {
    extended_buf[i] = i;
  }

  // Adding manufacturer_specific_data
  adv_data[amout_bytes++] = 0xC9;//length TEST_EXT_ELE_LENGTH + 1
  adv_data[amout_bytes++] = 0xFF;//ad type: manufacturer_specific_data
  memcpy(adv_data + amout_bytes, (uint8_t *)&extended_buf, 200);
  amout_bytes += 200;

  // Set advertising data
  sc = sl_bt_advertiser_set_data(advertising_set_handle3, 0, amout_bytes, adv_data);


  // Set custom advertising data.
//  sc = sl_bt_advertiser_set_data(advertising_set_handle3,
//                                 0,
//                                 40,
//                                 (uint8_t *)(adv_data));
  app_assert_status(sc);

//  // Set custom advertising data.
//  sc = sl_bt_advertiser_set_data(advertising_set_handle3,
//                                 1,
//                                 40,
//                                 (uint8_t *)(adv_data));
//  app_assert_status(sc);

  // Start advertising in user mode and disable connections.
  sc = sl_bt_advertiser_start(
    advertising_set_handle3,
    sl_bt_advertiser_user_data,
    sl_bt_advertiser_non_connectable);
  app_assert_status(sc);
}

//static void bcn_setup_adv_hello(void)
//{
//  sl_status_t sc;
//
//  // Create an advertising set.
//  sc = sl_bt_advertiser_create_set(&advertising_set_handle4);
//  app_assert_status(sc);
//
//  uint8_t adv_data2[180] = {0x68, 0x65, 0x6C, 0x6C, 0x6F, 0x64, 0x61, 0x72, 0x6B, 0x6E, 0x65, 0x73, 0x73, 0x6D, 0x79, 0x6F, 0x6C, 0x64, 0x66, 0x72, 0x68, 0x65, 0x6C, 0x6C, 0x6F, 0x64, 0x61, 0x72, 0x6B, 0x6E, 0x65, 0x73, 0x73, 0x6D, 0x79, 0x6F, 0x6C, 0x64, 0x66, 0x72, 0x68, 0x65, 0x6C, 0x6C, 0x6F, 0x64, 0x61, 0x72, 0x6B, 0x6E, 0x65, 0x73, 0x73, 0x6D, 0x79, 0x6F, 0x6C, 0x64, 0x66, 0x72, 0x68, 0x65, 0x6C, 0x6C, 0x6F, 0x64, 0x61, 0x72, 0x6B, 0x6E, 0x65, 0x73, 0x73, 0x6D, 0x79, 0x6F, 0x6C, 0x64, 0x66, 0x72, 0x68, 0x65, 0x6C, 0x6C, 0x6F, 0x64, 0x61, 0x72, 0x6B, 0x6E, 0x65, 0x73, 0x73, 0x6D, 0x79, 0x6F, 0x6C, 0x64, 0x66, 0x72, 0x68, 0x65, 0x6C, 0x6C, 0x6F, 0x64, 0x61, 0x72, 0x6B, 0x6E, 0x65, 0x73, 0x73, 0x6D, 0x79, 0x6F, 0x6C, 0x64, 0x66, 0x72, 0x68, 0x65, 0x6C, 0x6C, 0x6F, 0x64, 0x61, 0x72, 0x6B, 0x6E, 0x65, 0x73, 0x73, 0x6D, 0x79, 0x6F, 0x6C, 0x64, 0x66, 0x72, 0x68, 0x65, 0x6C, 0x6C, 0x6F, 0x64, 0x61, 0x72, 0x6B, 0x6E, 0x65, 0x73, 0x73, 0x6D, 0x79, 0x6F, 0x6C, 0x64, 0x66, 0x72, 0x68, 0x65, 0x6C, 0x6C, 0x6F, 0x64, 0x61, 0x72, 0x6B, 0x6E, 0x65, 0x73, 0x73, 0x6D, 0x79, 0x6F, 0x6C, 0x64, 0x66, 0x72};
//
//  // Set advertising parameters. 100ms advertisement interval.
//  sc = sl_bt_advertiser_set_timing(
//      advertising_set_handle4,
//    32,     // min. adv. interval (milliseconds * 1.6)
//    32,     // max. adv. interval (milliseconds * 1.6)
//    0,       // adv. duration
//    0);      // max. num. adv. events
//  app_assert_status(sc);
//
//  sc = sl_bt_advertiser_clear_configuration(
//      advertising_set_handle4,
//          1);
//  app_assert_status(sc);
//
//  bd_addr addr, addrout;
//  addr.addr[0] = 0x33;
//  addr.addr[1] = 0x33;
//  addr.addr[2] = 0x33;
//  addr.addr[3] = 0x33;
//  addr.addr[4] = 0x33;
//  addr.addr[5] = 0x33;
//  sc = sl_bt_advertiser_set_random_address(
//      advertising_set_handle4,
//      3,
//      addr,
//      &addrout);
//
////  sc = sl_bt_advertiser_set_phy(
////      advertising_set_handle4,
////      sl_bt_gap_1m_phy,
////      sl_bt_gap_2m_phy);
////  app_assert_status(sc);
//
//  uint8_t amout_bytes = 0;
//  const uint8_t service_uuid[16] = {0};
//  uint8_t extended_buf[200];
//  /* https://www.bluetooth.com/specifications/assigned-numbers/company-identifiers - To get your company ID*/
//  uint16_t company_id = 0x02FF; // 0x02FF - Silicon Labs' company ID
//
//  // Initialize advertising data with Flag and Local name
//  uint8_t adv_data[253] = {
//      0x02, // Length of flag
//      0x01, // Type flag
//      0x06, // Flag data
//      0x05, // Length of Local name
//      0x09, // Type local name
//      'h', 'e', 'l', 'l', // Local name
//  };
//  // Byte amount in advertising data buffer now increased by 9
//  amout_bytes += 9;
//
//  // Adding Service UUID to the advertising - len of UUID is 16 bytes
//  adv_data[amout_bytes++] = 0x11;// length 17 bytes
//  adv_data[amout_bytes++] = 0x06;//more_128_uuids
//  memcpy(adv_data+amout_bytes, service_uuid, 16);//adding UUID
//  amout_bytes += 16;
//
//  // Prepare manufacturer_specific_data
//  memcpy(extended_buf, (uint8_t *)&company_id, 2);
//  for (uint8_t i = 2; i < 200; i++) {
//    extended_buf[i] = i;
//  }
//
//  // Adding manufacturer_specific_data
//  adv_data[amout_bytes++] = 0xC9;//length TEST_EXT_ELE_LENGTH + 1
//  adv_data[amout_bytes++] = 0xFF;//ad type: manufacturer_specific_data
//  memcpy(adv_data + amout_bytes, (uint8_t *)&extended_buf, 200);
//  amout_bytes += 200;
//
//  // Set advertising data
//  const uint8_t data3[31] = {0x02, 0x01, 0x06, 0x1a, 0xff, 0xff, 0x02};
//  size_t cnt = 31;
//  sc = sl_bt_advertiser_set_data(advertising_set_handle4, 0, cnt, data3);
//
//
//  // Set custom advertising data.
////  sc = sl_bt_advertiser_set_data(advertising_set_handle3,
////                                 0,
////                                 40,
////                                 (uint8_t *)(adv_data));
//  app_assert_status(sc);
//
////  // Set custom advertising data.
////  sc = sl_bt_advertiser_set_data(advertising_set_handle3,
////                                 1,
////                                 40,
////                                 (uint8_t *)(adv_data));
////  app_assert_status(sc);
//
//  // Start advertising in user mode and disable connections.
//  sc = sl_bt_advertiser_start(
//      advertising_set_handle4,
//    sl_bt_advertiser_user_data,
//    sl_bt_advertiser_non_connectable);
//  app_assert_status(sc);
//}



///***************************************************************************//**
// * @file
// * @brief Core application logic.
// *******************************************************************************
// * # License
// * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
// *******************************************************************************
// *
// * SPDX-License-Identifier: Zlib
// *
// * The licensor of this software is Silicon Laboratories Inc.
// *
// * This software is provided 'as-is', without any express or implied
// * warranty. In no event will the authors be held liable for any damages
// * arising from the use of this software.
// *
// * Permission is granted to anyone to use this software for any purpose,
// * including commercial applications, and to alter it and redistribute it
// * freely, subject to the following restrictions:
// *
// * 1. The origin of this software must not be misrepresented; you must not
// *    claim that you wrote the original software. If you use this software
// *    in a product, an acknowledgment in the product documentation would be
// *    appreciated but is not required.
// * 2. Altered source versions must be plainly marked as such, and must not be
// *    misrepresented as being the original software.
// * 3. This notice may not be removed or altered from any source distribution.
// *
// ******************************************************************************/
//#include "em_common.h"
//#include "app_assert.h"
//#include "app_log.h"
//#include "sl_bluetooth.h"
//#include "gatt_db.h"
//#include "app.h"
//#include "sl_simple_button_instances.h"
//#include "sl_simple_led_instances.h"
//
//// The advertising set handle allocated from Bluetooth stack.
//static uint8_t advertising_set_handle = 0xff;
//
//static bool report_button_flag = false;
//
//// Updates the Report Button characteristic.
//static sl_status_t update_report_button_characteristic(void);
//// Sends notification of the Report Button characteristic.
//static sl_status_t send_report_button_notification(void);
//
///**************************************************************************//**
// * Application Init.
// *****************************************************************************/
//SL_WEAK void app_init(void)
//{
//  // Make sure there will be no button events before the boot event.
//  sl_button_disable(SL_SIMPLE_BUTTON_INSTANCE(0));
//
//  /////////////////////////////////////////////////////////////////////////////
//  // Put your additional application init code here!                         //
//  // This is called once during start-up.                                    //
//  /////////////////////////////////////////////////////////////////////////////
//}
//
///**************************************************************************//**
// * Application Process Action.
// *****************************************************************************/
//SL_WEAK void app_process_action(void)
//{
//  // Check if there was a report button interaction.
//  if (report_button_flag) {
//    sl_status_t sc;
//
//    report_button_flag = false; // Reset flag
//
//    sc = update_report_button_characteristic();
//    app_log_status_error(sc);
//
//    if (sc == SL_STATUS_OK) {
//      sc = send_report_button_notification();
//      app_log_status_error(sc);
//    }
//  }
//
//  /////////////////////////////////////////////////////////////////////////////
//  // Put your additional application code here!                              //
//  // This is called infinitely.                                              //
//  // Do not call blocking functions from here!                               //
//  /////////////////////////////////////////////////////////////////////////////
//}
//
///**************************************************************************//**
// * Bluetooth stack event handler.
// * This overrides the dummy weak implementation.
// *
// * @param[in] evt Event coming from the Bluetooth stack.
// *****************************************************************************/
//void sl_bt_on_event(sl_bt_msg_t *evt)
//{
//  sl_status_t sc;
//  bd_addr address;
//  uint8_t address_type;
//  uint8_t system_id[8];
//
//  switch (SL_BT_MSG_ID(evt->header)) {
//    // -------------------------------
//    // This event indicates the device has started and the radio is ready.
//    // Do not call any stack command before receiving this boot event!
//    case sl_bt_evt_system_boot_id:
//      // Extract unique ID from BT Address.
//      sc = sl_bt_system_get_identity_address(&address, &address_type);
//      app_assert_status(sc);
//
//      // Pad and reverse unique ID to get System ID.
//      system_id[0] = address.addr[5];
//      system_id[1] = address.addr[4];
//      system_id[2] = address.addr[3];
//      system_id[3] = 0xFF;
//      system_id[4] = 0xFE;
//      system_id[5] = address.addr[2];
//      system_id[6] = address.addr[1];
//      system_id[7] = address.addr[0];
//
//      sc = sl_bt_gatt_server_write_attribute_value(gattdb_system_id,
//                                                   0,
//                                                   sizeof(system_id),
//                                                   system_id);
//      app_assert_status(sc);
//
//      // Create an advertising set.
//      sc = sl_bt_advertiser_create_set(&advertising_set_handle);
//      app_assert_status(sc);
//
//      // Set advertising interval to 100ms.
//      sc = sl_bt_advertiser_set_timing(
//        advertising_set_handle,
//        160, // min. adv. interval (milliseconds * 1.6)
//        160, // max. adv. interval (milliseconds * 1.6)
//        0,   // adv. duration
//        0);  // max. num. adv. events
//      app_assert_status(sc);
//      // Start general advertising and enable connections.
//      sc = sl_bt_advertiser_start(
//        advertising_set_handle,
//        sl_bt_advertiser_general_discoverable,
//        sl_bt_advertiser_connectable_scannable);
//      app_assert_status(sc);
//
//      // Button events can be received from now on.
//      sl_button_enable(SL_SIMPLE_BUTTON_INSTANCE(0));
//
//      // Check the report button state, then update the characteristic and
//      // send notification.
//      sc = update_report_button_characteristic();
//      app_log_status_error(sc);
//
//      if (sc == SL_STATUS_OK) {
//        sc = send_report_button_notification();
//        app_log_status_error(sc);
//      }
//      break;
//
//    // -------------------------------
//    // This event indicates that a new connection was opened.
//    case sl_bt_evt_connection_opened_id:
//      app_log_info("Connection opened.\n");
//      break;
//
//    // -------------------------------
//    // This event indicates that a connection was closed.
//    case sl_bt_evt_connection_closed_id:
//      app_log_info("Connection closed.\n");
//
//      // Restart advertising after client has disconnected.
//      sc = sl_bt_advertiser_start(
//        advertising_set_handle,
//        sl_bt_advertiser_general_discoverable,
//        sl_bt_advertiser_connectable_scannable);
//      app_assert_status(sc);
//      break;
//
//    // -------------------------------
//    // This event indicates that the value of an attribute in the local GATT
//    // database was changed by a remote GATT client.
//    case sl_bt_evt_gatt_server_attribute_value_id:
//      // The value of the gattdb_led_control characteristic was changed.
//      if (gattdb_led_control == evt->data.evt_gatt_server_characteristic_status.characteristic) {
//        uint8_t data_recv;
//        size_t data_recv_len;
//
//        // Read characteristic value.
//        sc = sl_bt_gatt_server_read_attribute_value(gattdb_led_control,
//                                                    0,
//                                                    sizeof(data_recv),
//                                                    &data_recv_len,
//                                                    &data_recv);
//        (void)data_recv_len;
//        app_log_status_error(sc);
//
//        if (sc != SL_STATUS_OK) {
//          break;
//        }
//
//        // Toggle LED.
//        if (data_recv == 0x00) {
//          sl_led_turn_off(SL_SIMPLE_LED_INSTANCE(0));
//          app_log_info("LED off.\n");
//        } else if (data_recv == 0x01) {
//          sl_led_turn_on(SL_SIMPLE_LED_INSTANCE(0));
//          app_log_info("LED on.\n");
//        } else {
//          app_log_error("Invalid attribute value: 0x%02x\n", (int)data_recv);
//        }
//      }
//      break;
//
//    // -------------------------------
//    // This event occurs when the remote device enabled or disabled the
//    // notification.
//    case sl_bt_evt_gatt_server_characteristic_status_id:
//      if (gattdb_report_button == evt->data.evt_gatt_server_characteristic_status.characteristic) {
//        // A local Client Characteristic Configuration descriptor was changed in
//        // the gattdb_report_button characteristic.
//        if (evt->data.evt_gatt_server_characteristic_status.client_config_flags
//            & sl_bt_gatt_notification) {
//          // The client just enabled the notification. Send notification of the
//          // current button state stored in the local GATT table.
//          app_log_info("Notification enabled.");
//
//          sc = send_report_button_notification();
//          app_log_status_error(sc);
//        } else {
//          app_log_info("Notification disabled.\n");
//        }
//      }
//      break;
//
//    ///////////////////////////////////////////////////////////////////////////
//    // Add additional event handlers here as your application requires!      //
//    ///////////////////////////////////////////////////////////////////////////
//
//    // -------------------------------
//    // Default event handler.
//    default:
//      break;
//  }
//}
//
///***************************************************************************//**
// * Simple Button
// * Button state changed callback
// * @param[in] handle Button event handle
// ******************************************************************************/
//void sl_button_on_change(const sl_button_t *handle)
//{
//  if (SL_SIMPLE_BUTTON_INSTANCE(0) == handle) {
//    report_button_flag = true;
//  }
//}
//
///***************************************************************************//**
// * Updates the Report Button characteristic.
// *
// * Checks the current button state and then writes it into the local GATT table.
// ******************************************************************************/
//static sl_status_t update_report_button_characteristic(void)
//{
//  sl_status_t sc;
//  uint8_t data_send;
//
//  switch (sl_button_get_state(SL_SIMPLE_BUTTON_INSTANCE(0))) {
//    case SL_SIMPLE_BUTTON_PRESSED:
//      data_send = (uint8_t)SL_SIMPLE_BUTTON_PRESSED;
//      break;
//
//    case SL_SIMPLE_BUTTON_RELEASED:
//      data_send = (uint8_t)SL_SIMPLE_BUTTON_RELEASED;
//      break;
//
//    default:
//      // Invalid button state
//      return SL_STATUS_FAIL; // Invalid button state
//  }
//
//  // Write attribute in the local GATT database.
//  sc = sl_bt_gatt_server_write_attribute_value(gattdb_report_button,
//                                               0,
//                                               sizeof(data_send),
//                                               &data_send);
//  if (sc == SL_STATUS_OK) {
//    app_log_info("Attribute written: 0x%02x", (int)data_send);
//  }
//
//  return sc;
//}
//
///***************************************************************************//**
// * Sends notification of the Report Button characteristic.
// *
// * Reads the current button state from the local GATT database and sends it as a
// * notification.
// ******************************************************************************/
//static sl_status_t send_report_button_notification(void)
//{
//  sl_status_t sc;
//  uint8_t data_send;
//  size_t data_len;
//
//  // Read report button characteristic stored in local GATT database.
//  sc = sl_bt_gatt_server_read_attribute_value(gattdb_report_button,
//                                              0,
//                                              sizeof(data_send),
//                                              &data_len,
//                                              &data_send);
//  if (sc != SL_STATUS_OK) {
//    return sc;
//  }
//
//  // Send characteristic notification.
//  sc = sl_bt_gatt_server_notify_all(gattdb_report_button,
//                                    sizeof(data_send),
//                                    &data_send);
//  if (sc == SL_STATUS_OK) {
//    app_log_append(" Notification sent: 0x%02x\n", (int)data_send);
//  }
//  return sc;
//}

/**
 * \file    setup.h
 * \author  Robson Costa
 * \date    16/08/2019 
 */
#if !defined(__SETUP_H__)
#define __SETUP_H__

// System information
#define DEV_TYPE            "Arduino M0 Pro"
#define DEV_CONN            "LoRa"
#define DEV_FW_VERSION      "alpha"
#define DEV_SENSOR_LIST     "none"
#define DEV_ACTUATOR_LIST   "none"

// System configurations
#define SERIAL_BAUDRATE     115200
#define SYSTEM_PERIOD       1000
#define DHT_PERIOD          10 * SYSTEM_PERIOD
 
// LoRa configurations
const char* lora_band       = "AU920"; // [EU868|US915|AU920]
const char* lora_druplink   = "DR0";
const char* lora_channel0   = "0,917.2,DR0"; // [|0,904.3,DR0,DR3|0,917.2,DR0]
const char* lora_channel1   = "1,917.9,DR0"; // [|1,905.0,DR0,DR3|1,917.9,DR0]
const char* lora_rxwin2     = "923.3,DR8"; // [869.525,DR3|923.3,DR8|923.3,DR8]
const char* lora_mode       = "LWABP"; // [TEST|LWABP|LWOTAA]
const char* lora_txpower    = "14";
const char* lora_adr        = "OFF"; // [ON|OFF]
const char* lora_class      = "A"; // [A|C]
const char* lora_port_dht   = "1"; // LoRa port used to TX DHT values
const char* lora_devaddr    = "26031C38";
const char* lora_deveui     = "007C151EA69482C4";
const char* lora_appeui     = "70B3D57ED001FB51";
const char* lora_appkey     = "";
const char* lora_nwkskey    = "07371E8DA6A33E4A901F1E8132C8B0D1";
const char* lora_appskey    = "B16CF3E8B1B6D73EE55B517DFC06FD14";


#endif // __SETUP_H__

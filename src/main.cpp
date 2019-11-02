#include <Arduino.h>
#include <setup.h>
#include <util.h>
#include <DHT_U.h>
//#include <ArduinoLowPower.h>
//#include <LowPower.h>
//#include <RTCZero.h>
#include <Adafruit_SleepyDog.h>


#define DEBUGINFO 1

#define PRINTERROR(...) SerialUSB.println(__VA_ARGS__)

#ifdef DEBUGINFO
  #define PRINTLN(...) SerialUSB.println(__VA_ARGS__)
  #define WRITE(...) SerialUSB.write(__VA_ARGS__)
  #define PRINT(...) SerialUSB.print(__VA_ARGS__)
#else
  #define PRINTLN(...)
  #define WRITE(...)
  #define PRINT(...)
#endif

// Global variables
DHT dht(8, DHT22);
String loraReturn;
float temp;
float humid;
uint32_t timer = 0;
bool initialized = false;

// Funtion prototypes
void printInitInfo();
boolean initLoRa();
boolean getDHTValues();
boolean txDHTValues();
//void serialEventRun(void);
//void serialEvent1();

void setup() {
  // Initiate serial terminal
  Serial.begin(SERIAL_BAUDRATE);

  // Print initial information
  printInitInfo();

  // Initiate DHT22
  dht.begin();

  // Initiate LoRa modem
  if ((initialized = initLoRa()) == false) {
    PRINTERROR("\n>>> ERROR initiating LoRa modem!!!");
  }
}

void loop() {

  // Check if is time to get and transmit DHT values
  if (millis() - timer >= DHT_PERIOD) {
    // Get DHT sensor values
    if (getDHTValues() == true) {
      // Create payload and transmitt
      txDHTValues();
    }
    timer = millis();  
  }

  // System period cycle
  //delay(SYSTEM_PERIOD);


  PRINTLN(">>> Entering Sleep in 10 seconds");
  delay(10000);
  PRINTLN(">>> Entering Sleep now, for ? seconds");
  //LowPower.idle(5000);
  //LowPower.standby();
  //LowPower.idle(IDLE_0);
  //LowPower.idle(IDLE_1);
  //LowPower.idle(IDLE_2);
  //int sleepMS = Watchdog.sleep(10000);
  int sleepMS = Watchdog.sleep();

  WRITE("I'm awake now! I slept for ");
  PRINT(sleepMS, DEC);
  WRITE(" milliseconds.\n");

}

/**
 * \fn    printInitInfo
 * \brief Print inital information on serial monitor
 */
void printInitInfo() {
  WRITE("\n########################################\n");
  WRITE("#             LoRa endnode             #\n");
  WRITE("########################################\n");
  WRITE("Staring LoRa endnode...");
  WRITE("\n\tDevice type............: ");
  WRITE(DEV_TYPE);
  WRITE("\n\tDevice communication...: ");
  WRITE(DEV_CONN);
  WRITE("\n\tFirmware version.......: ");
  WRITE(DEV_FW_VERSION);
  WRITE("\n\tDevice sensor list.....: ");
  WRITE(DEV_SENSOR_LIST);
  WRITE("\n\tDevice actuator list...: ");
  WRITE(DEV_ACTUATOR_LIST);
}

/**
 * \fn    initLoRa
 * \brief Initiate LoRa modem
 */
boolean initLoRa() {
  String at_cmd;

  // Configure UART communication between M0-Pro and RHF76-052 LoRa modem 
  
  WRITE("\nInitiating UART port between M0-Pro and LoRa modem...");
  Serial1.begin(9600);
  Serial1.setTimeout(2000); // every single readString takes this time 
  WRITE(" [OK]\n");

  // Reset RHF76-052 LoRa modem
  // After reset, 2s is required to POR cycle before commencing communications over UART
  /* Not needed
  PRINTLN(">>> Reseting LoRa modem...");
  Serial1.println("AT+RESET");
  delay(2000);  
  loraReturn = Serial1.readString();
  WRITE(loraReturn);
  if (strcmp((char*)loraReturn.c_str(), "+RESET: OK\r\n") != 0) {
    return false;
  }
  */

 /* Not needed
  // Check if RHF76-052 LoRa modem is OK
  PRINTLN(">>> Sending AT command...");
  Serial1.println("AT");
  //delay(300);
  loraReturn = Serial1.readString();
  WRITE(loraReturn);
  if (strcmp((char*)loraReturn.c_str(), "+AT: OK\r\n") != 0) {
    return false;
  }
*/

  // Get firmware version of RHF76-052 LoRa modem
  /* Not needed
  PRINTLN(">>> Getting firmware version...");
  Serial1.println("AT+VER");
  //delay(300);
  loraReturn = Serial1.readString();
  WRITE(loraReturn);
  */

  // Set default configuration
  PRINTLN(">>> Setting default configuration...");
  Serial1.println("AT+FDEFAULT=RISINGHF");
  //delay(500);
  loraReturn = Serial1.readString();
  if (strcmp((char*)loraReturn.c_str(), "+FDEFAULT: OK\r\n") != 0) {
    WRITE(">>> Setting default configuration failed! : ");
    WRITE(loraReturn.c_str());
    return false;
  }
  PRINTLN(loraReturn);

  // Set frequency band 
  PRINTLN(">>> Setting frequency band...");
  at_cmd = "AT+DR=";
  at_cmd.concat(lora_band);
  Serial1.println(at_cmd);
  //delay(300);
  loraReturn = Serial1.readString();
  //todo: test return
  PRINTLN(loraReturn);

  // Set channel 0 
  PRINTLN(">>> Setting channel 0...");
  at_cmd = "AT+CH=";
  at_cmd.concat(lora_channel0);
  Serial1.println(at_cmd);
  //delay(300);
  loraReturn = Serial1.readString();
  PRINTLN(loraReturn);

  // Set channel 1 
  PRINTLN(">>> Setting channel 1...");
  at_cmd = "AT+CH=";
  at_cmd.concat(lora_channel1);
  Serial1.println(at_cmd);
  //delay(300);
  loraReturn = Serial1.readString();
  PRINTLN(loraReturn);

  // Set DR
  PRINTLN(">>> Setting uplink DR (Data Rate)...");
  at_cmd = "AT+DR=";
  at_cmd.concat(lora_druplink);
  Serial1.println(at_cmd);
  //delay(300);
  loraReturn = Serial1.readString();
  PRINTLN(loraReturn);
 
  // Set RXWIN2
  PRINTLN(">>> Setting RXWIN2...");
  at_cmd = "AT+RXWIN2=";
  at_cmd.concat(lora_rxwin2);
  Serial1.println(at_cmd);
  //delay(300);
  loraReturn = Serial1.readString();
  PRINTLN(loraReturn);

  /*
  // Get DR scheme
  PRINTLN(">>> Getting DR schemme...");
  Serial1.println("AT+DR=?");
  //delay(300);
  loraReturn = Serial1.readString();
  PRINTLN(loraReturn);
  */

  // Set DevEui
  PRINTLN(">>> Setting DevEui...");
  at_cmd = "AT+ID=DevEui,\"";
  at_cmd.concat(lora_deveui);
  at_cmd.concat("\"");
  Serial1.println(at_cmd);
  //delay(300);
  loraReturn = Serial1.readString();
  PRINTLN(loraReturn);

  // Set AppEui
  PRINTLN(">>> Setting AppEui...");
  at_cmd = "AT+ID=AppEui,\"";
  at_cmd.concat(lora_appeui);
  at_cmd.concat("\"");
  Serial1.println(at_cmd);
  //delay(300);
  loraReturn = Serial1.readString();
  PRINTLN(loraReturn);

  /*
  // Get ID parameters of RHF76-052 LoRa modem
  PRINTLN(">>> Getting DevAddr, DevEui and AppEui...");
  Serial1.println("AT+ID");
  //delay(300);
  loraReturn = Serial1.readString();
  PRINTLN(loraReturn); 
  */

  // Set LoRa transmission power
  PRINTLN(">>> Setting transmission power to 14dBm...");
  at_cmd = "AT+POWER=";
  at_cmd.concat(lora_txpower);
  Serial1.println(at_cmd);
  //delay(300);
  loraReturn = Serial1.readString();
  PRINTLN(loraReturn);

  // Set LoRa ADR (Automatic Data Rate)
  PRINTLN(">>> Setting ADR (Automatic Data Rate) mode...");
  at_cmd = "AT+ADR=";
  at_cmd.concat(lora_adr);
  Serial1.println(at_cmd);
  //delay(300);
  loraReturn = Serial1.readString();
  PRINTLN(loraReturn);

  // Set Authentication mode
  PRINTLN(">>> Setting authentication mode...");
  at_cmd = "AT+MODE=";
  at_cmd.concat(lora_mode);
  Serial1.println(at_cmd);
  //delay(300);
  loraReturn = Serial1.readString();
  PRINTLN(loraReturn);

  // Set CLASS mode
  PRINTLN(">>> Setting LoRa Class...");
  at_cmd = "AT+CLASS=";
  at_cmd.concat(lora_class);
  Serial1.println(at_cmd);
  //delay(300);
  loraReturn = Serial1.readString();
  PRINTLN(loraReturn);
  
  if (strcmp(lora_mode, "LWABP") == 0) {
    // Set DevAddr
    PRINTLN(">>> Setting DevAddr...");
    at_cmd = "AT+ID=DevAddr,\"";
    at_cmd.concat(lora_devaddr);
    at_cmd.concat("\"");
    Serial1.println(at_cmd);
    //delay(300);
    loraReturn = Serial1.readString();
    PRINTLN(loraReturn);

    // Set NwkSKey
    PRINTLN(">>> Setting NwkSKey...");
    at_cmd = "AT+KEY=NwkSKey,\"";
    at_cmd.concat(lora_nwkskey);
    at_cmd.concat("\"");
    Serial1.println(at_cmd);
    //delay(300);
    loraReturn = Serial1.readString();
    PRINTLN(loraReturn);

    // Set AppSKey
    PRINTLN(">>> Setting AppSKey...");
    at_cmd = "AT+KEY=AppSKey,\"";
    at_cmd.concat(lora_appskey);
    at_cmd.concat("\"");
    Serial1.println(at_cmd);
    //delay(300);
    loraReturn = Serial1.readString();
    PRINTLN(loraReturn);

  } else { // Authentication OTAA
    // Set AppKey
    PRINTLN(">>> Setting AppKey...");
    at_cmd = "AT+KEY=AppKey,\"";
    at_cmd.concat(lora_appkey);
    at_cmd.concat("\"");
    Serial1.println(at_cmd);
    //delay(300);
    loraReturn = Serial1.readString();
    PRINTLN(loraReturn);

    // Joining to LoRa network (via OTAA - Over The Air Authentication)
    PRINTLN(">>> Sending AT+Join");
    Serial1.println("AT+Join");
    //delay(2000);
    loraReturn = Serial1.readString();
    PRINTLN(loraReturn);
  }

  return true;
}

/**
 * \fn    getDHTValues
 * \brief Get DHT sensor temperature and humidity
 */
boolean getDHTValues() {
  PRINTLN("Getting temperature and humidity from DHT22...");
  humid = dht.readHumidity();
  temp = dht.readTemperature();
  WRITE("\tHumidity: ");
  PRINT(humid);
  WRITE(" %");
  WRITE("\n\tTemperature: ");
  PRINT(temp);
  WRITE(" oC");
  if ((temp != NAN) && (humid != NAN)) {
    return true;
  } else {
    return false;
  }
}


/**
 * \fn    txDHTValues
 * \brief Create payload and transmit DHT sensor temperature and humidity
 */
boolean txDHTValues() {
  String payload = "";
  String at_cmd;

  // Create payload
  // adjust for the f2sflt16 range (-1 to 1)
  temp = temp / 100;
  humid = humid / 100;
  
  // float -> int
  uint16_t  payloadTemp = f2sflt16(temp);
  uint16_t  payloadHumid = f2sflt16(humid);

  // int -> byte
  byte tempLow = lowByte(payloadTemp);
  byte tempHigh = highByte(payloadTemp);
  byte humidLow = lowByte(payloadHumid);
  byte humidHigh = highByte(payloadHumid);
  
  // Create LoRa payload
  payload.concat(String(tempLow, HEX));
  payload.concat(String(tempHigh, HEX));
  payload.concat(String(humidLow, HEX));
  payload.concat(String(humidHigh, HEX));

  // Set LoRa port communication
  PRINTLN("\nSetting LoRa port...");
  at_cmd = "AT+PORT=";
  at_cmd.concat(lora_port_dht);
  Serial1.println(at_cmd);
  //delay(300);  
  loraReturn = Serial1.readString();
  PRINTLN(loraReturn);

  // Sending data via RHF76-052 LoRa modem
  PRINTLN("\nSending unconfirmed data via LoRa...");
  at_cmd = "AT+MSGHEX=\"";
  at_cmd.concat(payload);  
  at_cmd.concat("\"");
  PRINTLN(at_cmd);
  Serial1.println(at_cmd);
//  delay(5000);  
  loraReturn = Serial1.readString();
  PRINTLN(loraReturn);
  return true;
}

/*
void serialEventRun(void) {
  if (Serial1.available()) serialEvent1();
}

void serialEvent1() {
  while (Serial1.available()) {
    loraReturn = Serial1.readString();
    Serial.print(loraReturn);
  }
}
*/
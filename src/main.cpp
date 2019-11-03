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
unsigned long timer = 0;
bool initialized = false;
const int buffersz = 64;
char strBuffer[buffersz];
const char termChar = '\n'; 

// Funtion prototypes
void printInitInfo();
boolean initLoRa();
boolean getDHTValues();
boolean txDHTValues();
char * readCString();

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
WRITE("*************************\n");
WRITE("* Loop Started          *\n");
WRITE("*************************\n");

//millis do not compute time during sleep
unsigned long now = millis();
timer = now; // loop start time

// Get DHT sensor values
if (getDHTValues() == true) {
  // Create payload and transmit
  txDHTValues();
}

now = millis();
unsigned long McAwakeTime = now - timer;
WRITE("Microcontroller awake time: ");
SerialUSB.print(McAwakeTime); //4014ms
WRITE(" mS\n");
long sleepTime = (unsigned long)DHT_PERIOD - McAwakeTime;
while (sleepTime > 0) {
  WRITE("Will sleep for ");
  PRINT(sleepTime, DEC);
  WRITE(" milliseconds.\n");
  int sleepMS = Watchdog.sleep(sleepTime);
  sleepTime = sleepTime - sleepMS;
  WRITE("I'm awake now! I slept for ");
  PRINT(sleepMS, DEC);
  WRITE(" milliseconds.\n");

}

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

  unsigned long initStart = millis();
  String at_cmd;
  char * b;

  // Configure UART communication between M0-Pro and RHF76-052 LoRa modem 
  
  WRITE("\nInitiating UART port between M0-Pro and LoRa modem...");
  Serial1.begin(9600);
  Serial1.setTimeout(2000); // every single readString takes this time 
  WRITE(" [OK]\n");

  /* Not needed
  // Reset RHF76-052 LoRa modem
  // After reset, 2s is required to POR cycle before commencing communications over UART
  PRINTLN(">>> Reseting LoRa modem...");
  Serial1.println("AT+RESET");
  b = readCString();
  if (strcmp((char*)b, "+RESET: OK\r") != 0) {
    return false;
  }
  */

  /* Not needed
  // Check if RHF76-052 LoRa modem is OK
  PRINTLN(">>> Sending AT command...");
  Serial1.println("AT");
  b = readCString();
  if (strcmp((char*)b, "+AT: OK\r") != 0) {
    return false;
  }
  */

  /* Not needed
  // Get firmware version of RHF76-052 LoRa modem
  PRINTLN(">>> Getting firmware version...");
  Serial1.println("AT+VER");
  readCString();
  */

  // Set default configuration
  PRINTLN(">>> Setting default configuration...");
  Serial1.println("AT+FDEFAULT=RISINGHF");
  b = readCString();
  if (strcmp((char*)b, "+FDEFAULT: OK\r") != 0) {
    WRITE(">>> Setting default configuration failed!");
    return false;
  }

  // Set frequency band 
  PRINTLN(">>> Setting frequency band...");
  at_cmd = "AT+DR=";
  at_cmd.concat(lora_band);
  Serial1.println(at_cmd);
  readCString();

  // Set channel 0 
  PRINTLN(">>> Setting channel 0...");
  at_cmd = "AT+CH=";
  at_cmd.concat(lora_channel0);
  Serial1.println(at_cmd);
  readCString();

  // Set channel 1 
  PRINTLN(">>> Setting channel 1...");
  at_cmd = "AT+CH=";
  at_cmd.concat(lora_channel1);
  Serial1.println(at_cmd);
  readCString();

  // Set DR
  PRINTLN(">>> Setting uplink DR (Data Rate)...");
  at_cmd = "AT+DR=";
  at_cmd.concat(lora_druplink);
  Serial1.println(at_cmd);
  readCString();
  
  // Set RXWIN2
  PRINTLN(">>> Setting RXWIN2...");
  at_cmd = "AT+RXWIN2=";
  at_cmd.concat(lora_rxwin2);
  Serial1.println(at_cmd);
  readCString();
  
  /* Not Needed
  // Get DR scheme
  PRINTLN(">>> Getting DR schemme...");
  Serial1.println("AT+DR=?");
  readCString();
  */

  // Set DevEui
  PRINTLN(">>> Setting DevEui...");
  at_cmd = "AT+ID=DevEui,\"";
  at_cmd.concat(lora_deveui);
  at_cmd.concat("\"");
  Serial1.println(at_cmd);
  readCString();

  // Set AppEui
  PRINTLN(">>> Setting AppEui...");
  at_cmd = "AT+ID=AppEui,\"";
  at_cmd.concat(lora_appeui);
  at_cmd.concat("\"");
  Serial1.println(at_cmd);
  readCString();

  /*
  // Get ID parameters of RHF76-052 LoRa modem
  PRINTLN(">>> Getting DevAddr, DevEui and AppEui...");
  Serial1.println("AT+ID");
  readCString();
  */

  // Set LoRa transmission power
  PRINTLN(">>> Setting transmission power to 14dBm...");
  at_cmd = "AT+POWER=";
  at_cmd.concat(lora_txpower);
  Serial1.println(at_cmd);
  readCString();

  // Set LoRa ADR (Automatic Data Rate)
  PRINTLN(">>> Setting ADR (Automatic Data Rate) mode...");
  at_cmd = "AT+ADR=";
  at_cmd.concat(lora_adr);
  Serial1.println(at_cmd);
  readCString();
  
  // Set Authentication mode
  PRINTLN(">>> Setting authentication mode...");
  at_cmd = "AT+MODE=";
  at_cmd.concat(lora_mode);
  Serial1.println(at_cmd);
  readCString();
  
  // Set CLASS mode
  PRINTLN(">>> Setting LoRa Class...");
  at_cmd = "AT+CLASS=";
  at_cmd.concat(lora_class);
  Serial1.println(at_cmd);
  readCString();
  
  if (strcmp(lora_mode, "LWABP") == 0) {
    // Set DevAddr
    PRINTLN(">>> Setting DevAddr...");
    at_cmd = "AT+ID=DevAddr,\"";
    at_cmd.concat(lora_devaddr);
    at_cmd.concat("\"");
    Serial1.println(at_cmd);
    readCString();
  
    // Set NwkSKey
    PRINTLN(">>> Setting NwkSKey...");
    at_cmd = "AT+KEY=NwkSKey,\"";
    at_cmd.concat(lora_nwkskey);
    at_cmd.concat("\"");
    Serial1.println(at_cmd);
    readCString();
    
    // Set AppSKey
    PRINTLN(">>> Setting AppSKey...");
    at_cmd = "AT+KEY=AppSKey,\"";
    at_cmd.concat(lora_appskey);
    at_cmd.concat("\"");
    Serial1.println(at_cmd);
    readCString();
    
  } else { // Authentication OTAA
    // Set AppKey
    PRINTLN(">>> Setting AppKey...");
    at_cmd = "AT+KEY=AppKey,\"";
    at_cmd.concat(lora_appkey);
    at_cmd.concat("\"");
    Serial1.println(at_cmd);
    readCString();
    
    // Joining to LoRa network (via OTAA - Over The Air Authentication)
    PRINTLN(">>> Sending AT+Join");
    Serial1.println("AT+Join");
    readCString();
  }

  unsigned long initEnd = millis();
  WRITE("Lora initialization took : ");
  PRINTLN(initEnd-initStart);
  WRITE(" mS\n");

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
  readCString();
  
  // Sending data via RHF76-052 LoRa modem
  PRINTLN("\nSending unconfirmed data via LoRa...");
  at_cmd = "AT+MSGHEX=\"";
  at_cmd.concat(payload);  
  at_cmd.concat("\"");
  Serial1.println(at_cmd);
  Serial1.setTimeout(5000);  
  readCString();
  readCString();
  readCString();
  return true;
}

char * readCString() {
  const int buffsz = 64;
  static char buff[buffsz];
  memset(buff, 0, buffsz);
  Serial1.readBytesUntil('\n', buff, buffsz);
  PRINTLN(buff);
  return buff;
}

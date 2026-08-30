
// must have for external includes and some other stuff
// pinout check https://lastminuteengineers.com/esp8266-pinout-reference/
#include "Arduino.h"

#include <ArduinoUniqueID.h>

// include the webserver module / class
#include "config/config_esp_8266.h"

#define ADC_OUTPUT_1 4
#define ADC_OUTPUT_2 12
#define ADC_OUTPUT_3 13
#define ADC_OUTPUT_4 15

#define LED_PIN 14
#define ERROR_PIN 12
#define SWITCH_PIN 5 // attention this is different because 

// switches
String switch1 = "ON";
String switch2 = "ON";
String switch3 = "ON";
String switch4 = "ON";

// ---------------------------------

//
//  global classes for config interaction
//

ConfigParams* configParams = NULL;
WifiGetter* wifiHandler = NULL;
String idStr = "";
String typeStr = "AXSWITCH";
bool refreshProxy = true;
#define SWITCH_OFF_SECRET "switchoffsecret"


// this is specific for each gadget and needs to be called to init the data reader
ConfigParams* GetConfigParameters (String devicetype, String deviceid) {
    if (configParams) {
        delete configParams;
    }
     
    configParams = new ConfigParams();
    // this is common for all boards 
    AddWifiParams(configParams, devicetype, deviceid);
    // special parameters
    configParams->AddParam(SWITCH_OFF_SECRET, "Switch-Off-Secret", "OFF");
    // return
    return configParams;
}

//
//  global call to re-read the connection or configuration data
//

void setGlobals() {

  // we will move this later ... just somewhere else
  configParams = GetConfigParameters (typeStr, idStr);

  printf("setGlobals: init\n");
  // digitalWrite(ERROR_PIN,HIGH);

  if (wifiHandler == NULL) {
    wifiHandler = new WifiGetter(configParams->GetValue(WIFI_SSID),
                            configParams->GetValue(WIFI_PASS),
                            configParams->GetValue(WIFI_REDIRECTURL),
                            configParams->GetValue(WIFI_REDIRECTUSER),
                            configParams->GetValue(WIFI_REDIRECTSECRET),
                            configParams->GetValue(WIFI_URL));
  }

  // auth string is set as
  //  base64 encoded user:password
  //

  String httpRequest;
  // This will send the request to the server
  if (configParams->GetBoolValue(WIFI_POST)) {
      httpRequest = String("device_type ") + configParams->GetValue(WIFI_DEVICE_TYPE) + String("\r\n")
             + String("device_id ") + configParams->GetValue(WIFI_DEVICE_ID) + String("\r\n")
             + String("switch1_") + configParams->GetValue(WIFI_DEVICE_ID) + String(" ") + switch1 + String("\r\n")
             + String("switch2_") + configParams->GetValue(WIFI_DEVICE_ID) + String(" ") + switch2 + String("\r\n")
             + String("switch3_") + configParams->GetValue(WIFI_DEVICE_ID) + String(" ") + switch3 + String("\r\n")
             + String("switch4_") + configParams->GetValue(WIFI_DEVICE_ID) + String(" ") + switch4 + String("\r\n");
  }
  else {
      httpRequest = String("/set?device_type=") + configParams->GetValue(WIFI_DEVICE_TYPE) + String("&device_id=") + configParams->GetValue(WIFI_DEVICE_ID);
             //+ String("&amps4=") + String(amps4,4);
             // do NOT set the switches
  }

  String line;
  if (wifiHandler->sendHttpRequest(httpRequest, line, configParams->GetBoolValue(WIFI_POST), refreshProxy, Base64Encode(configParams->GetValue(WIFI_URLUSER), configParams->GetValue(WIFI_URLSECRET)))) {

    printf("-------------------\n");
    printf("request:\n%s\n", httpRequest.c_str());
    printf("-------------------\n");
    printf("line:\n%s\n", line.c_str());
    printf("-------------------\n");

    if (line.length() > 500) {

      // this is implementation specific
      // extraction of the parameters from the get / set routine

      String newSwitch1 = wifiHandler->parseHtml(line, String("switch1"), switch1);
      String newSwitch2 = wifiHandler->parseHtml(line, String("switch2"), switch2);
      String newSwitch3 = wifiHandler->parseHtml(line, String("switch3"), switch3);
      String newSwitch4 = wifiHandler->parseHtml(line, String("switch4"), switch4);
    
      if (newSwitch1 != switch1) {
        switch1 = newSwitch1;
        printf("change switch setting 1 %s\n", switch1.c_str());
      }
      if (newSwitch2 != switch2) {
        switch2 = newSwitch2;
        printf("change switch setting 2 %s\n", switch2.c_str());
      }
      if (newSwitch3 != switch3) {
        switch3 = newSwitch3;
        printf("change switch setting 3 %s\n", switch3.c_str());
      }
      if (newSwitch4 != switch4) {
        switch4 = newSwitch4;
        printf("change switch setting 4 %s\n", switch4.c_str());
      }

      // end of specific stuff

      printf("reply parsed\n");

      digitalWrite(ERROR_PIN, LOW);
      
    } else {
      
      printf("reply too short, retry\n\n%s", line.c_str());
      digitalWrite(ERROR_PIN, HIGH);
    }
    refreshProxy = false;
    
  } else {
    printf("failed, full refresh initiated\n");
    refreshProxy = true;
    digitalWrite(ERROR_PIN, HIGH);
  }

}


//
//   initial setup called by OS
//
void setup() {

  pinMode(LED_PIN, OUTPUT);
  pinMode(ERROR_PIN, OUTPUT);
  pinMode(SWITCH_PIN, INPUT);
  pinMode(ADC_OUTPUT_1, OUTPUT);
  pinMode(ADC_OUTPUT_2, OUTPUT);
  pinMode(ADC_OUTPUT_3, OUTPUT);
  pinMode(ADC_OUTPUT_4, OUTPUT);
  digitalWrite(ADC_OUTPUT_1, HIGH);
  digitalWrite(ADC_OUTPUT_2, HIGH);
  digitalWrite(ADC_OUTPUT_3, HIGH);
  digitalWrite(ADC_OUTPUT_4, HIGH);
  
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(100);  // we need time to switch the port

  ArduinoUniqueID uniqueId = ArduinoUniqueID();
  for (int i=0; i < UniqueIDbuffer; i++) {
    int buff = uniqueId.id[i];
     idStr += String(buff) + "_"; 
  }

  printf("\n---------------------------------------------------------------\n");
  printf("        AX WIFI Switch, Version 1.0 \n");
  printf("        Id %s\n",idStr.c_str());
  printf("---------------------------------------------------------------\n");

  Serial.println("init");

  delay(100);

  digitalWrite(ERROR_PIN, HIGH);  
  digitalWrite(LED_PIN, LOW);

  Serial.println("init done");
  
}

void loop() {

  bool configMode = digitalRead(SWITCH_PIN);  // open+3.3v = true, gnd = false

  printf("start loop\n");


  // config mode 
  if (configMode) {
    
      printf("enter config mode\n");
      
      digitalWrite(ERROR_PIN, LOW);
      delay(100);
      digitalWrite(ERROR_PIN, HIGH);
      delay(100);
      digitalWrite(ERROR_PIN, LOW);
      delay(100);
      digitalWrite(ERROR_PIN, HIGH);
      delay(100);
      digitalWrite(ERROR_PIN, LOW);

      ConfigParams* confData = GetConfigParameters (typeStr, idStr);
      WifiConfigWebserver* configServer = new WifiConfigWebserver(confData, idStr, typeStr);
      configServer->runAcessPoint();  // this does not return
  
  } else {
    // regular mode

    setGlobals();

    // double factor
    String offSetting = configParams->GetValue(SWITCH_OFF_SECRET);

    digitalWrite(LED_PIN, LOW);
    delay(100);
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
    delay(100);
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
    
    int loops = 0;
    
    while (loops < 2000) {
      loops++;
      digitalWrite(LED_PIN, HIGH);
      delay (1000);
      
      Serial.println("lesen");

      // switch the relais
      // default is on, only off when the sprecial string is set
      if (switch1 == offSetting) {
        printf("switch 1 off\n");
        digitalWrite(ADC_OUTPUT_1, LOW);
      } else {
        printf("switch 1 on\n");
        digitalWrite(ADC_OUTPUT_1, HIGH);
      }
      if (switch2 == offSetting) {
        printf("switch 2 off\n");
        digitalWrite(ADC_OUTPUT_2, LOW);
      } else {
        printf("switch 2 on\n");
        digitalWrite(ADC_OUTPUT_2, HIGH);
      }
      if (switch3 == offSetting) {
        printf("switch 3 off\n");
        digitalWrite(ADC_OUTPUT_3, LOW);
      } else {
        printf("switch 3 on\n");
        digitalWrite(ADC_OUTPUT_3, HIGH);
      }
      if (switch4 == offSetting) {
        printf("switch 4 off\n");
        digitalWrite(ADC_OUTPUT_4, LOW);
      } else {
        printf("switch 4 on\n");
        digitalWrite(ADC_OUTPUT_4, HIGH);
      }

  
      // in theory we would set a timer ...... see first line
      // because sending by WLan takes some time
      setGlobals();
      digitalWrite(LED_PIN, LOW);
      delay(7000);
      printf("waiting loop: %d\n", loops);
      delay(45000);
    } // while
  } // else 

}

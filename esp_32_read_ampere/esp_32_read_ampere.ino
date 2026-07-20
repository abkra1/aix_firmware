
// #include <Arduino.h>
#include "SPI.h"
#include "esp_wifi.h"

#include <WiFi.h>
#include <WiFiClientSecure.h>

#include <driver/adc.h>
#include <ArduinoUniqueID.h>

#include "EmonLib.h"

// include the webserver module / class
#include "config/config.h"


#define ADC_INPUT_1 34
#define ADC_INPUT_2 35
#define ADC_INPUT_3 32
#define ADC_INPUT_4 33

// Force EmonLib to use 10bit ADC resolution
#define ADC_BITS    10
#define ADC_COUNTS  (1<<ADC_BITS)


#define LED_PIN 14
#define ERROR_PIN 12
#define SWITCH_PIN 26
#define MAX_READ 10

// we have 4 channels to read, let us hope it is this simple
EnergyMonitor emon1;
EnergyMonitor emon2;
EnergyMonitor emon3;
EnergyMonitor emon4;

// actuals
double amps1 = 0.0;
double amps2 = 0.0;
double amps3 = 0.0;
double amps4 = 0.0;

// ---------------------------------

//
//  global classes for config interaction
//

ConfigParams* configParams = NULL;
WifiGetter* wifiHandler = NULL;
String idStr = "";
String typeStr = "AXAMP";
bool refreshProxy = true;
#define SENSOR_OFFSET "sensoroffset"
#define SENSOR_FACTOR "sensorfactor"


// this is specific for each gadget and needs to be called to init the data reader
ConfigParams* GetConfigParameters (String devicetype, String deviceid) {
    if (configParams) {
        delete configParams;
    }
     
    configParams = new ConfigParams();
    // this is common for all boards 
    AddWifiParams(configParams, devicetype, deviceid);
    // special parameters
    configParams->AddParam(SENSOR_OFFSET, "Sensor-Offset", "0.0");
    configParams->AddParam(SENSOR_FACTOR, "Sensor-Factor", "1.0");
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
      httpRequest = String("device_type=") + configParams->GetValue(WIFI_DEVICE_TYPE) + String("\r\n")
             + String("device_id=") + configParams->GetValue(WIFI_DEVICE_ID) + String("\r\n")
             + String("amps1=") + String(amps1,4) + String("\r\n")
             + String("amps2=") + String(amps2,4) + String("\r\n")
             + String("amps3=") + String(amps3,4) + String("\r\n")
             + String("amps4=") + String(amps4,4) + String("\r\n");

  }
  else {
      httpRequest = String("/set?device_type=") + configParams->GetValue(WIFI_DEVICE_TYPE) + String("&device_id=") + configParams->GetValue(WIFI_DEVICE_ID)
             + String("&amps1=") + String(amps1,4) 
             + String("&amps2=") + String(amps2,4)
             + String("&amps3=") + String(amps3,4) 
             + String("&amps4=") + String(amps4,4); 
  }
    
  String line;
  if (wifiHandler->sendHttpRequest(httpRequest, line, configParams->GetBoolValue(WIFI_POST), refreshProxy, Base64Encode(configParams->GetValue(WIFI_URLUSER), configParams->GetValue(WIFI_URLSECRET)))) {
    
    printf("-------------------\n");
    printf("request:\n%s\n", httpRequest.c_str());
    printf("-------------------\n");
    printf("line:\n%s\n", line.c_str());
    printf("-------------------\n");

    if (line.length() > 500) {

#if 0

      // this is implementation specific
      // extraction of the parameters from the get / set routine

      String newTimezone = getter->parseHtml(line, String("timezone"), oldTimezone);
      String newClockColor = getter->parseHtml(line, String("clock_color"), oldClockColor);
      String newMsgColor = getter->parseHtml(line, String("msg_color"), oldMsgColor);

      String newBrightness = getter->parseHtml(line, String("brightness"), oldBrightness);
      String newMsgLoops = getter->parseHtml(line, String("msg_loops"), oldMsgLoops);
      //String newMsgTime = getter->parseHtml(line, String("msg_time"), oldMsgTime);

      String newAlertTime = getter->parseHtml(line, String("alert_time"), oldAlertTime);
      String newAlert = getter->parseHtml(line, String("alert"), oldAlert);

      String newMessage = getter->parseHtml(line, String("message"), oldMessage);

      // printf("new: %s %s %s\n",newMode, newStart, newWheels);
    
      // mode is switched my new string
      String newMode = getter->parseHtml(line,String("mode"),oldMode);
      if (newMode != oldMode) {
        oldMode = newMode;
        mode = newMode;
        printf("setting mode to %s\n",mode);
      }

      if (newTimezone != oldTimezone) {
        oldTimezone = newTimezone;
        int offset = oldTimezone.toInt();
        if ((offset > -24) && (offset < 24)) {
          timezone = offset;
          // setting time to now .... to avoid old messages
          //msgTime = getTimesInt(timezone);
        }
        printf("setting summer/wintertime offset to %d\n", timezone);
      }

      if (newClockColor != oldClockColor) {
        oldClockColor = newClockColor;
        int value = newClockColor.toInt();
        if ((value >= 0) && (value <= RANDOM_COLOR)) {
          clockColor = value;
        }
        printf("setting clock color to %d\n", clockColor);
      }

      if (newMsgColor != oldMsgColor) {
        oldMsgColor = newMsgColor;
        int value = newMsgColor.toInt();
        if ((value >= 0) && (value <= RANDOM_COLOR)) {
          msgColor = value;
        }
        printf("setting message color to %d\n", msgColor);        
      }

      if (newBrightness != oldBrightness) {
        oldBrightness = newBrightness;
        int bright = oldBrightness.toInt();
        if ((bright >= 0) && (bright < 255)) {
          brightness = bright;
        }
        printf("setting brightness to %d\n", brightness);
      }

      // how often to display this stuff
      if (newMsgLoops != oldMsgLoops) {
        oldMsgLoops = newMsgLoops;
        int value = newMsgLoops.toInt();
        if ((value >= 0) && (value <= 10000)) {
          msgLoops = value;
        }
        printf("setting message loops to %d\n", msgLoops);
      }

      // printf("old: %s new: %s\n",oldMsgTime.c_str(),newMsgTime.c_str() );
      if (newMsgTime != oldMsgTime) {
        oldMsgTime = newMsgTime;
        // printf("old: %s new: %s\n",oldMsgTime.c_str(),newMsgTime.c_str() );
        if ((oldMsgTime > "220221200911") && (oldMsgTime < "420221200911")) {
          if (msgTime < oldMsgTime) {
            msgTime = oldMsgTime;
            mode = "MESSAGE";
            printf("setting mode to MESSAGE\n");
          }
        }
        printf("setting last message time to %s\n", msgTime.c_str());
      }


      if (newAlert != oldAlert) {
        int value = newAlert.toInt();
        if ((value >= 0) && (value <= 255)) {
          oldAlert = newAlert;
        }
        printf("setting alert to %s\n", oldAlert.c_str());
        //mode = "MESSAGE";  this is done in the clock loop
      }

      if (newAlertTime != oldAlertTime) {
        newAlertTime.replace("%3A", ":");
        if (newAlertTime.length() == 5) {
          oldAlertTime = newAlertTime;
          printf("setting alert time to %s\n", oldAlertTime.c_str());
        } else {
          printf("illegal alert time to %s\n", newAlertTime.c_str());
        }
      }

      // if we want to display this ... we should decide by timestamp (currently commented out)

      if (newMessage != oldMessage) {
        oldMessage = newMessage;
        printf("setting message to %s\n", oldMessage.c_str());
        if (msgCount > 0) {
          mode = "MESSAGE";
          printf("setting mode to MESSAGE\n");
        }
        msgCount++; 
      }
      printf("reply parsed\n  Timezone: %s\n   alert:   %s\n", oldTimezone.c_str(), oldAlert.c_str());

      // end of specific stuff

#endif


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

  
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(100);  // we need time to switch the port

  ArduinoUniqueID uniqueId = ArduinoUniqueID();
  for (int i=0; i < UniqueIDbuffer; i++) {
    int buff = uniqueId.id[i];
     idStr += String(buff) + "_"; 
  }

  printf("\n---------------------------------------------------------------\n");
  printf("        AX WIFI Amperemeter, Version 2.5 \n");
  printf("        Id %s\n",idStr.c_str());
  printf("---------------------------------------------------------------\n");

  Serial.println("init");

  pinMode(LED_PIN, OUTPUT);
  pinMode(ERROR_PIN, OUTPUT);
  pinMode(SWITCH_PIN, INPUT);
  delay(100);

  
/* error  handling of wifi init, left in for reference
  wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));
    //esp_err_t esp_wifi_set_ps(wifi_ps_type_t type)  
  ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
*/

  // this seems not necessary any more even when doc requires it
  // it causes the esp32 to reset, so not use it for now
  // adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_11);
 
  analogReadResolution(10);

  // Initialize emon library (28-30 = calibration number for 1 = 1A)
  emon1.current(ADC_INPUT_1, 185.0);
  emon2.current(ADC_INPUT_2, 185.0);
  emon3.current(ADC_INPUT_3, 185.0);
  emon4.current(ADC_INPUT_4, 185.0);
  // 1600W calibr 68 = 6.8 A = 285.0 cal

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
    double sensorOffset = configParams->GetValue(SENSOR_OFFSET).toDouble();
    double sensorFactor = configParams->GetValue(SENSOR_FACTOR).toDouble();

    digitalWrite(LED_PIN, LOW);
    delay(100);
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
    delay(100);
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);


    unsigned long currentMillis = millis();
  
    // If it's been longer then 1000ms since we took a measurement, take one now!
    // if(currentMillis - lastMeasurement > 1000){
    //   nada nada nada jada
    //   some code for later to make it reall one minute or something
    
    int loops = 0;
    
    while (loops < 2000) {
      loops++;
      digitalWrite(LED_PIN, HIGH);
      delay (1000);
      
      Serial.println("lesen");

      int MAGIC=1480;
        
      // we read several times and use the average value
      amps1 = (emon1.calcIrms(MAGIC) * sensorFactor) - sensorOffset; // Calculate Irms only with magic number
      amps2 = (emon2.calcIrms(MAGIC) * sensorFactor) - sensorOffset; // Calculate Irms only with magic number
      amps3 = (emon3.calcIrms(MAGIC) * sensorFactor) - sensorOffset; // Calculate Irms only with magic number
      amps4 = (emon4.calcIrms(MAGIC) * sensorFactor) - sensorOffset; // Calculate Irms only with magic number
      
      size_t i;
      for (i=1;i<MAX_READ;i++) {
        Serial.print(".");
        delay(20);
        amps1 += (emon1.calcIrms(MAGIC) * sensorFactor) - sensorOffset; 
        amps2 += (emon2.calcIrms(MAGIC) * sensorFactor) - sensorOffset;
        amps3 += (emon3.calcIrms(MAGIC) * sensorFactor) - sensorOffset;
        amps4 += (emon4.calcIrms(MAGIC) * sensorFactor) - sensorOffset;
      }
      amps1 = amps1 / ((double) MAX_READ);
      amps2 = amps2 / ((double) MAX_READ);
      amps3 = amps3 / ((double) MAX_READ);
      amps4 = amps4 / ((double) MAX_READ);
  
      // just make some noise
      Serial.println("+");   
      printf("werte: %f %f %f %f\n", amps1, amps2, amps3, amps4);
  
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

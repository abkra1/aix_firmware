/*
 * 
 *  read the temperature and send it to the server
 *
 */

#include <OneWire.h> 
#include <DallasTemperature.h>

// need this for the config module 
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoUniqueID.h>

// global settings


// ----------------------------

#define SENSOR_PIN 14     // D5

#define SWITCH_PIN 5
#define ERROR_PIN 4
#define MIN_PER_SAMPLE "minpersample"

String actualTemp1 = "99.9";
int    minPerSample = 15;



// include the webserver module / class
#include "config/config.h"

//
//  global classes for config interaction
//

ConfigParams* configParams = NULL;
WifiGetter* wifiHandler = NULL;
String idStr = "";
String typeStr = "AXTEMP";
bool refreshProxy = true;

// this is specific for each gadget and needs to be called to init the data reader
ConfigParams* GetConfigParameters (String devicetype, String deviceid) {
    if (configParams) {
        //delete configParams;
        return configParams;
    }
     
    configParams = new ConfigParams();
    // this is common for all boards 
    AddWifiParams(configParams, devicetype, deviceid);
    // special parameters
    configParams->AddParam(MIN_PER_SAMPLE, "Minutes-per-Sample", "15");

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

  // This will send the request to the server
  String httpRequest = String("/set?device_type=") + configParams->GetValue(WIFI_DEVICE_TYPE) + String("&device_id=") + configParams->GetValue(WIFI_DEVICE_ID)
             + String("&temp1=") + actualTemp1 
             + String (" HTTP/1.1\r\n") + String("Host: ")
             + wifiHandler->GetRealIP() + String("\r\n") + String("Authorization: Basic ")
             + Base64Encode(configParams->GetValue(WIFI_URLUSER), configParams->GetValue(WIFI_URLSECRET)) + String("\r\n\r\n");
    
  String line;
  if (wifiHandler->sendHttpGetRequest(httpRequest, line, refreshProxy)) {
    
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


//  -------------------------------------------------------

class TempGet
{
  public:
    TempGet(int pin) {
      tempSensorBus = (OneWire*) new OneWire(pin);
      tempSensors = (DallasTemperature*) new DallasTemperature(tempSensorBus);
    }
    ~TempGet()
    {
      delete tempSensors;
      delete tempSensorBus;
    }

    // the one and only usefull call
    String getTempString() 
    {
        return String(getTempValue());
    }
    
    // the one and only usefull call
    float getTempValue() 
    {
       if (tempSensors) {
         tempSensors->requestTemperatures();
         return tempSensors->getTempCByIndex(0);
       }
       return -99.99;
    }

    
  private:
    OneWire* tempSensorBus;
    DallasTemperature* tempSensors; 
    
};


// ------------------------------------------------------------------------------

//
//   worker and init loops
//
//
 

 
void setup() {
  //
  //   do not use pin D8 ever !!!!!!! (used by arduino upload
  //   never, as in never ever ... do anything here that may cause an exception / segv ... whatever
  //
  Serial.begin(115200);
  delay(100); // we need time to switch the port

  ArduinoUniqueID uniqueId = ArduinoUniqueID();
  for (int i=0; i < UniqueIDbuffer; i++) {
    int buff = uniqueId.id[i];
     idStr += String(buff) + "_"; 
  }

  printf("\n---------------------------------------------------------------\n");
  printf("        AX WIFI TempReader, Version 2.3 \n");
  printf("        Id %s\n",idStr.c_str());
  printf("---------------------------------------------------------------\n");

  Serial.println("init");

  delay(100);
  pinMode(ERROR_PIN, OUTPUT);
  pinMode(SWITCH_PIN, INPUT);
  delay(100);
  
  //printf("setup done\n");
}
 
//
//   the main loop to execute
//
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

    // init ....
    // read once to male the right call
    TempGet tempGetter(SENSOR_PIN);
    actualTemp1 = tempGetter.getTempString();
  
    printf("initial temp read %s\n", actualTemp1.c_str());
  
    // this will conveniently set the temp as well
    setGlobals();

    int value = configParams->GetValue(MIN_PER_SAMPLE).toInt();
    if ((value > 0) && (value <= 12*60)) {
          minPerSample = value;
    } 
    printf("sample time: %d\n", minPerSample);
  
    // blink to show all is fine
    digitalWrite(ERROR_PIN, LOW);
    delay(1000);
    digitalWrite(ERROR_PIN, HIGH);
    delay(1000);
    digitalWrite(ERROR_PIN, LOW);
    delay(1000);
    digitalWrite(ERROR_PIN, HIGH);
    
    int loops = 0;
    
    while (loops < 2000) {
      loops++;
      
      actualTemp1 = tempGetter.getTempString();
      printf("%d temp read %s\n", loops, actualTemp1.c_str());
#if 0      
      if (activeTempLimit < tempGetter.getTempValue()) {
        digitalWrite(LED_PIN, HIGH);
        digitalWrite(RELAIS_PIN, HIGH);
        printf("switch cooling on\n");
      }
      else {
        digitalWrite(LED_PIN, LOW);
        digitalWrite(RELAIS_PIN, LOW);
        printf("switch cooling off\n");
      }
#endif  
      // and wait about 15 minutes
      //delay(60*1000*15);
      setGlobals(); 
      // printf("actual tempLimit is %f\n",activeTempLimit);
      if (!connected) {
        digitalWrite(ERROR_PIN, LOW);
        delay(1000);
        digitalWrite(ERROR_PIN, HIGH);
        delay(1000);
        digitalWrite(ERROR_PIN, LOW);
        delay(1000);
        digitalWrite(ERROR_PIN, HIGH);
      }
      delay(60*1000*minPerSample);
      loops++;
    }    
    
    //sensors.requestTemperatures();
    //Serial.println(sensors.getTempCByIndex(0));
    delay(10);
    
  } // else
  
}

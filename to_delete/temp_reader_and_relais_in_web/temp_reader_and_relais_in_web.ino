/*
 * 
 *  justrwad the temp and write it somewhere
 *  later switch a relais doing so
 *
 */
//#include "config.h"
#include <FS.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

#include <OneWire.h> 
#include <DallasTemperature.h>

#define SENSOR_PIN 14 // D5
#define PWR_PIN    13   // D7
#define LED_PIN    12   // D6
#define RELAIS_PIN    15   // D8

//OneWire oneWire(ONE_WIRE_BUS); 
//DallasTemperature sensors(&oneWire); 

class ConfigData {
    public:

       ConfigData():
       wifiPassword("empty"),
       wifiSid("empty"),
       wifiIp("empty"),
       wifiTemp("empty"),
       configError(false)
       {
            printf("ConfigData() init\n");
            bool initFlag = SPIFFS.begin();
            if (!initFlag) {
              printf("ConfigData() init failed\n");
              configError = true;
            }
            else {
              configError = readConfig();
            }
       }
       
       ~ConfigData(){
            SPIFFS.end();
            wifiPassword = "";
       }

       bool readConfig() {
           printf("ConfigData()::readConfig()\n");
           wifiSid = readConfigFile("/sid.txt");
           wifiPassword = readConfigFile("/pass.txt");
           wifiIp = readConfigFile("/realip.txt");
           wifiTemp = readConfigFile("/temp.txt");
           return configError;
       }

       String readConfigFile (String fileName) {
          bool hasError = false;
          String content;
          if (SPIFFS.exists(fileName)) {
            File fd = SPIFFS.open(fileName, "r");
            if (!fd) {
              printf("ConfigData()::readConfigFile(): failed to open %s\n",fileName.c_str());
              hasError = true;
            }
            else {
              fd.seek(0, SeekSet);
              // reed as loop since we have no idea how much is done in one loop
              while (fd.available()) {
                content = content + fd.readString();
              }
              fd.close();
            }
          }
          else {
            printf("ConfigData()::readConfigFile(): file does not exist %s\n",fileName.c_str());
            hasError = true;
          }
          configError = hasError; // whatever
          printf("ConfigData()::readConfigFile(): content %s\n",content.c_str());
          return content;
       }

       bool writeConfigFile (String fileName, String content) {
          bool hasError = false;
          if (!SPIFFS.exists(fileName)) {
            printf("ConfigData()::writeConfigFile(): creating %s\n",fileName.c_str());
          }
          File fd = SPIFFS.open(fileName, "w");
          if (!fd) {
            printf("ConfigData()::writeConfigFile(): failed to open %s\n",fileName.c_str());
            hasError = true;
          }
          else {
            fd.seek(0, SeekSet);
            printf("ConfigData()::writeConfigFile(): content %s\n",content.c_str());
            int written = fd.print(content.c_str());
            if (written != content.length()) {
              printf("ConfigData()::writeConfigFile(): failed to write %s\n",fileName.c_str());
              hasError = true;
            }
            fd.close();
          }
          configError = hasError;
          return hasError;
       }


       bool writeConfig(String sid, String password, String realip, String temp) {
           printf("ConfigData()::writeConfig()\n");
           setWifiSid(sid);
           setWifiPassword(password);
           setWifiIp(realip);
           setWifiTemp(temp);
           // and now read it again to check and set internal values
           readConfig();
           return configError;
       }

       
       String getWifiPassword() {
            return wifiPassword;
       }

       String getWifiSid() {
            return wifiSid;
       }
       
       String getWifiIp() {
            return wifiSid;
       }
       
       String getWifiTemp() {
            return wifiTemp;
       }

       bool setWifiPassword(String newVal) {
         if (newVal != wifiPassword) {
           writeConfigFile("/pass.txt",newVal);
         }
         return configError;
       }
       
       bool setWifiSid(String newVal) {
         if (newVal != wifiSid) {
           writeConfigFile("/sid.txt",newVal);
         }
         return configError;
       }
       
       bool setWifiIp(String newVal) {
         if (newVal != wifiIp) {
           writeConfigFile("/realip.txt",newVal);
         }
         return configError;
       }
       
       bool setWifiTemp(String newVal) {
         if (newVal != wifiTemp) {
           writeConfigFile("/temp.txt",newVal);
         }
         return configError;
       }

    private:
       String wifiSid;
       String wifiPassword;
       String wifiIp;
       String wifiTemp;
       bool configError;
};

// -------------------------------------- WIFI stuff -------


String ssid = "noldor"; //replace this with your WiFi network name
String password = "MWisWima8Zh."; //replace this with your WiFi network password
String realIP = "";

// config page
String tempToolVersion = "2.01";

String newTemp = "4.0";
String oldTemp = "4.0";
String newTempLimit = "4.0";
String oldTempLimit = "4.0";
float activeTempLimit = 4.0;
bool connected = false;

ConfigData* wifiData = nullptr;

String parseHtml(String reply, String tag, String def) {
  String tagStart = String("<"+tag+">");
  String tagEnd = String("</"+tag+">");
  String value = def;
  int posStart = reply.indexOf(tagStart);
  int posEnd = reply.indexOf(tagEnd);
  //printf("check %d tag: %s %d-%d len: %d\n",reply.length(), tag, posStart, posEnd, tagStart.length());
  
  if ((posStart > -1) && ((posStart + tagStart.length()) < posEnd) && (posEnd < reply.length())) {
    value = reply.substring(posStart + tagStart.length(), posEnd);
    //printf("Tag: %s Val: #%s#\n",tag,value);
  }
  return value;
  
}

bool connect(String sidName, String pw) {

  printf("Wifi init\n");
  size_t wait = 0;  
  WiFi.begin(sidName.c_str(), pw.c_str());
  while ((WiFi.status() != WL_CONNECTED) && (wait < 30)) {
    wait++;
    //printf("connecting: %d \n", wait);
    delay(500);
  }
  if (WiFi.status() == WL_CONNECTED) {
    IPAddress ip = WiFi.localIP();
    Serial.print("IP Address: ");
    Serial.println(ip);

    return false;
  }
  printf("failed: %d \n", WiFi.status());
  return true;
}


String getIPViaAbkra(String ssid) {
  if (ssid == "noldor") {
    return "192.168.93.13";
  }
  
  WiFiClientSecure abkraClient;
  abkraClient.setInsecure(); // we have no 1:1 root certs here (many domains for one IP)
  const int httpPort = 443;
  String myHost = "www.abkra.de";
  if (!abkraClient.connect(myHost.c_str(), httpPort)) {
    Serial.println("abkra connection failed");
    return "";
  }
  
  // This will send the request to the server
  abkraClient.print(String("GET") +" /num.html HTTP/1.1\r\nHost: "+ myHost +"\r\nConnection: close\r\n\r\n");

  delay(5000);
  String line;
  char nextChar = 0;
  //printf("html read: ");
  //Serial.println(client.status());
  //printf("\n");
  while(abkraClient.available() && (nextChar < 254)){
    nextChar = abkraClient.read();
    if (nextChar < 254) {
      //printf("%c",nextChar); 
      line += String(nextChar);
    }
  }
  //printf("abkra reply: %s\n",line.c_str());
  
  abkraClient.stop();
  
  String newIP;
  //printf("\n");
  if (line.length() > 100) {
    String ref = parseHtml(line,String("td"),"<a href=\"https://88.152.251.180\">Heimat</a>");
    int posStart = ref.indexOf("https");
    int posEnd = ref.indexOf("Heimat");
    if ((posStart > 0) && (posEnd-2 > posStart+8)) {
      newIP = ref.substring(posStart+8,posEnd-2);
      printf("realIP: %s\n",newIP.c_str());
      return newIP;
    }
  }
  Serial.println("abkra getting IP failed");
  return newIP;
}

// this is the only real ..... WIFI connector we need

void setGlobals () {

  // we will move this later towards somewhere else
  if (!wifiData) {
    wifiData = new ConfigData();
  }

  if (wifiData->getWifiSid() != "noldor") {
    wifiData->writeConfig(ssid, password, realIP, newTempLimit);
  }
  
  printf("setGlobals: init\n");
  if (WiFi.status() != WL_CONNECTED) {
     //printf("setGlobals: WIFI (re-)connect: %s\n", getTimestring());
    if (connect(wifiData->getWifiSid(), wifiData->getWifiPassword())) {
      printf("failed to reconnect, using defaults/old values\n");
      return;
    }
  }
  // try to read some page from my local webserver

  // first determine the real IP to connect to
  if (realIP.length() < 2) {
    realIP = getIPViaAbkra(wifiData->getWifiSid());
    if (realIP.length() < 2) {
      return; // all  failed, make globals later
    }
  }
  if (realIP != wifiData->getWifiIp()) {
    wifiData->setWifiIp(realIP);
  }

  WiFiClientSecure client;
  client.setInsecure(); // we have insecure certs here
    
  const int httpPort = 443;
  if (!client.connect(realIP.c_str(), httpPort)) {
    Serial.println("connection failed");
    connected = false;
    return;
  }
  connected = true;
  //Serial.print("Requesting URL: ");
  //Serial.println(url);


  // auth string is set as 
  //  base64 encoded user:password
  // 
 
  // This will send the request to the server
  client.print(String("GET ") + "/kuehlschrank" + "/SETTER.sh?" + newTemp + " HTTP/1.1\r\n" +
               "Host: " + realIP + "\r\n" + 
               "Authorization: Basic YmxpbmRlcjpHaWJIZXI=" + "\r\n\r\n");

  delay(5000);
  String line;
  char nextChar = 0;
  //printf("setGlobals: html read: %s\n", getTimestring());
  while(client.available() && (nextChar < 254)){
    nextChar = client.read();
    if (nextChar < 254) {
      //printf("%c",nextChar); 
      line += String(nextChar);
    }
  }
  //printf("\n");
  
  //printf("setGlobals: html done reading:\n%s\n",line.c_str());
  

  if (line.length() > 100) {
    // String newTemp = parseHtml(line,String("actualtemp"),oldTemp);
    String newTempLimit = parseHtml(line,String("limittemp"),oldTempLimit);
    //printf("new: %s %s %s\n",line, newTemp, newTempLimit);

    // we do not set our own temp    
    //if (newTemp != oldTemp) {
    //  oldTemp = newTemp;
    //  printf("setting temp to %f\n",newTemp);
    //}
    
    if (newTempLimit != oldTempLimit) {
      float value = newTempLimit.toFloat();
      if ((value > 0.0) && (value < 30.0)) {
        oldTempLimit = newTempLimit;
        activeTempLimit = value;
        printf("setting tempLimit to %f\n",activeTempLimit);
      }
      else {
        printf("Temperature limit %f out of bound (0-20)\n",newTempLimit);
      }
    }
  }
  else {
    printf("setGlobals: html error - no reply.\n");
  }
  // printf("setGlobals: html close connection (maybe leave open ?): %s\n", getTimestring());
  client.stop();
  // delete wifiData;
  // printf("setGlobals: finished: %s\n", getTimestring());

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
        return String(tempSensors->getTempCByIndex(0));
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
  // if we have a problem, give me 5 secs to interrupt on startup
  printf("\n---------------------------------------------------------------\n");
  printf("        Version: TempTool %s\n",tempToolVersion.c_str());
  printf("---------------------------------------------------------------\n");
  Serial.flush();
  printf("setup\n");
  delay(1000);
  pinMode(LED_PIN, OUTPUT);
  pinMode(PWR_PIN, OUTPUT);
  pinMode(RELAIS_PIN, OUTPUT);
  //printf("setup done\n");
}
 
//
//   the main loop to execute
//
void loop() {
  printf("start loop\n");

  // init ....
  // read once to male the right call
  TempGet tempGetter(SENSOR_PIN);
  newTemp = tempGetter.getTempString();
  oldTemp = newTemp;

  printf("initial temp read %s\n", newTemp.c_str());

  // this will conveniently set the temp as well
  setGlobals();


  // blink to show all is fine
  digitalWrite(PWR_PIN, LOW);
  delay(1000);
  digitalWrite(PWR_PIN, HIGH);
  delay(1000);
  digitalWrite(PWR_PIN, LOW);
  delay(1000);
  digitalWrite(PWR_PIN, HIGH);

  int counter = 0;
  while (true) {
    
    newTemp = tempGetter.getTempString();
    printf("%d temp read %s\n", counter, newTemp.c_str());
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

    // and wait about 15 minutes
    //delay(60*1000*15);
    setGlobals(); 
    printf("actual tempLimit is %f\n",activeTempLimit);
    if (!connected) {
      digitalWrite(PWR_PIN, LOW);
      delay(1000);
      digitalWrite(PWR_PIN, HIGH);
      delay(1000);
      digitalWrite(PWR_PIN, LOW);
      delay(1000);
      digitalWrite(PWR_PIN, HIGH);
    }
    delay(60*1000);
    counter++;
  }
  
  
  //sensors.requestTemperatures();
  //Serial.println(sensors.getTempCByIndex(0));
  delay(10);
}

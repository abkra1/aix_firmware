#include "config.h"
#if 0
#include <WiFiClient.h>
#include <ESP8266WiFiType.h>
#include <ESP8266WiFiSTA.h>
#include <WiFiServerSecureBearSSL.h>
#include <ESP8266WiFiGeneric.h>
#include <ESP8266WiFiAP.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecureBearSSL.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiUdp.h>
#include <BearSSLHelpers.h>
#include <CertStoreBearSSL.h>
#include <WiFiClientSecure.h>
#include <ESP8266WiFiGratuitous.h>
#include <WiFiServerSecure.h>
#include <ArduinoWiFiServer.h>
#include <WiFiServer.h>
#include <ESP8266WiFiScan.h>
#include <NTPClient.h>
#endif
// really used wifi stuff
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
// pixel stuff
#include <Adafruit_NeoPixel.h>
#include <Adafruit_NeoMatrix.h>
// clock
#include <time.h>
// config stuff
#include <ESP8266WebServer.h>

// configurtion stuff
//  saved on local
#include "FS.h"

// globals
//
//  
//  pinMode(6, OUTPUT); // INPUT
//  digitalWrite(13, HIGH); // sets the digital pin 13 on
//  digitalWrite(13, LOW);  // sets the digital pin 13 off



// Declare our NeoPixel strip object:
// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1:
//#define LED_PIN    13
#define LED_PIN    10
#define SWITCH_PIN  5
// How many NeoPixels are attached to the Arduino?
#define LED_ROWS 8
#define LED_COLS 32
#define LED_COUNT LED_ROWS*LED_COLS
// Argument 1 = Number of pixels in NeoPixel strip
// Argument 2 = Arduino pin number (most are valid)
// Argument 3 = Pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
// Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);



//
//   define some basic colors
//
#define COL_RED 0 
#define COL_GREEN 1 
#define COL_BLUE 2 
#define COL_YELLOW 3
#define COL_PINK 4
#define COL_PURPLE 5
#define COL_ORANGE 6
#define COL_WHITE 7
#define NUM_COLORS 8
#define RANDOM_COLOR 1000

#define COL_R 0
#define COL_G 1
#define COL_B 2

std::vector<std::vector<size_t>> Colors = {{255,0,0}, // red
                                           {0,255,0}, // green
                                           {0,0,255}, // blue
                                           //{155,50,10}, // yellow
                                           {255,255,0}, // yellow
                                           //{255,0,255}, // pink
                                           {255,0,128}, // pink
                                           {128,0,128}, // purple
                                           //{200,25,5}, // orange
                                           {255,128,0}, // orange
                                           {255,255,255}, // white
                                          };



// for debugging return time string

String twoChars(int value) {
  String ret;
  ret = "00" + String(value);
  return ret.substring(ret.length()-2,ret.length());
  
}

String getTimestring(int timeZone) {
  
        time_t rawtime;
        struct tm * timeinfo;
        time (&rawtime);
        timeinfo = localtime (&rawtime);

        int hour = timeinfo->tm_hour + timeZone;
        if (hour > 23) {
          hour = hour - 24;
        }
        else if (hour < 0) {
          hour = hour + 24;
        }


        // we have someting looking like time
        String theTime = twoChars(hour) + ":" + twoChars(timeinfo->tm_min); //+ ":" + twoChars(timeinfo->tm_sec);
        return theTime;
  
}


String getTimesInt(int timeZone) {
  
        time_t rawtime;
        struct tm * timeinfo;
        time (&rawtime);
        timeinfo = localtime (&rawtime);

        int hour = timeinfo->tm_hour + timeZone;
        if (hour > 23) {
          hour = hour - 24;
        }
        else if (hour < 0) {
          hour = hour + 24;
        }

        // we have someting looking like time
        String theTime = twoChars(timeinfo->tm_year) + twoChars(timeinfo->tm_mon +1)+ twoChars(timeinfo->tm_mday) + twoChars(hour) + twoChars(timeinfo->tm_min) + twoChars(timeinfo->tm_sec);

        printf("getTimesInt(): %s\n",theTime.c_str());

        
        return theTime;
  
}

class ConfigData {
    public:

       ConfigData():
       wifiPassword("empty"),
       wifiSid("empty"),
       wifiIp("empty"),
       wifiBrand("empty"),
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
           wifiBrand = readConfigFile("/brand.txt");
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


       bool writeConfig(String sid, String password, String realip, String brand) {
           printf("ConfigData()::writeConfig()\n");
           setWifiSid(sid);
           setWifiPassword(password);
           setWifiIp(realip);
           setWifiBrand(brand);
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
            return wifiIp;
       }
       
       String getWifiBrand() {
            return wifiBrand;
       }

       bool setWifiPassword(String newVal) {
         writeConfigFile("/pass.txt",newVal);
         wifiPassword = newVal;
         return configError;
       }
       
       bool setWifiSid(String newVal) {
         writeConfigFile("/sid.txt",newVal);
         wifiSid = newVal;
         return configError;
       }

       bool setWifiIp(String newVal) {
         writeConfigFile("/realip.txt",newVal);
         wifiIp = newVal;
         return configError;
       }

       bool setWifiBrand(String newVal) {
         writeConfigFile("/brand.txt",newVal);
         wifiBrand = newVal;
         return configError;
       }

    private:
       String wifiSid;
       String wifiPassword;
       String wifiIp;
       String wifiBrand;
       bool configError;
};




// clock
// attention need to get the real time from somewhere ???
//
class LedClock {
  public:
    LedClock(size_t numLedsIn, int timeZoneIn, bool circleIn, size_t colorIn):
    circle(circleIn),
    lightBefore(false),
    numLeds(numLedsIn),
    hour(2),
    minute(10),
    second(0),
    fraction(0.0),
    timeZone(timeZoneIn)
    {
      factor = ((float)numLeds) / 60.0;
      SetTime(false);
      lastTimeStr = "";
      setColor(colorIn);
    };
    ~LedClock() {};
    
    void Init() {};
    
    int GetNext(int in) {
      if (in <= 0) {
        return 59;
      }
      if (in >= 60) {
        return 0;
      }
      return in;
    }
    int GetPrev(int in) {
      if (in <= 0) {
        return 59;
      }
      if (in >= 60) {
        return 0;
      }
      return in;
    }


    void Next(int msWaitIn, Adafruit_NeoMatrix& matrix) {
      UpdTime(msWaitIn);
      // hour in 12 format
      int hour24 = (int)(((float)(hour % 12)) * factor * 5);
      
      // printf("h: %d m: %d s: %d\n",hour12, minute60,second60);
      // String timeStr = String(hour) + ":" + String(minute) + ":" + String(second);
      String timeStr = getTimestring(timeZone);
      
      if (lastTimeStr != timeStr) {
        printf(String("Timestr: "+ timeStr + "\n").c_str());
        lastTimeStr = timeStr;
        matrix.fillScreen(0);
        matrix.setTextColor(matrix.Color(colorR,colorG,colorB));
        matrix.setCursor(1, 0);
        matrix.print(timeStr);
        matrix.show();
      }
    }

    size_t getColor() {
      return color;
    }

    void setColor(size_t newColor) {
      
      color = newColor;
      
      if (color == RANDOM_COLOR) {
        
        colorR = random(100);     
        colorG = random(100);      
        colorB = random(100);
        
        // prefer 1 color     
        int tint = random(3);
        if (tint == 0) {
          colorR = 2*colorR;
        }
        else if (tint == 1) {
          colorG = 2*colorG;
        }
        else if (tint == 2) {
          colorB = 2*colorB;
        }
        
      }  
      else if (color < NUM_COLORS) {
        colorR = Colors[color][COL_R];     
        colorG = Colors[color][COL_G];      
        colorB = Colors[color][COL_B];     
      }  
      else {
        colorR = Colors[COL_WHITE][COL_R];     
        colorG = Colors[COL_WHITE][COL_G];     
        colorB = Colors[COL_WHITE][COL_B];   
      }
      
    }

    void Next(int msWaitIn, Adafruit_NeoPixel& strip) {
      UpdTime(msWaitIn);
      // hour in 12 format
      int hour12 = (int)(((float)(hour % 12)) * factor * 5);
      // minute in 60
      int minute60 = (int) ((float)minute * factor);
      int second60 = (int) ((((float)second)+fraction) * factor);
      //bool lightBefore = false;
      //if ((minute60 % 2) > 0) {
      //  lightBefore = true;
      //}

      // printf("h: %d m: %d s: %d\n",hour12, minute60,second60);


      // all dark
      // strip.clear(); 
      size_t pix;
      for (pix = 0; pix < numLeds; pix++) {
        size_t red = 0;
        size_t green = 0;
        size_t blue = 0;

        // make elseif -> no merge
        if (pix == minute60) {
          blue = 200;
        }
        else if (pix == hour12) {
          green = 200;
        }
        else if (pix == second60) {
          red = 200;
        }
          // now check for seconds "ring"
        else if (circle) {
          if (lightBefore) {
            if (pix < second60) {
              red = 30;
            }
          }
          else {
            if (pix > second60) {
              red = 30;
            }
          }
        }
        else { // no circle
          if ((pix == (GetPrev(pix))) || (pix == (GetNext(pix)))) {
            red = 30;
          }
        }
        strip.setPixelColor(pix,Adafruit_NeoPixel::Color(red, green, blue));
      }  // led count loop
      // now show it
      strip.show();
    }
    
    void SetTime(bool skipSeconds) {
        time_t rawtime;
        struct tm * timeinfo;
        time (&rawtime);
        timeinfo = localtime (&rawtime);

        // we have someting looking like time
        hour = timeinfo->tm_hour;
        hour = hour + timeZone;
        if (hour > 23) {
          hour = hour - 24;
        }
        else if (hour < 0) {
          hour = hour + 24;
        }
        minute = timeinfo->tm_min;
        
        if (!skipSeconds) {
          second = timeinfo->tm_sec;
        }
        else {
          second = 0;
        }
        if ((minute % 2) == 0) {
          lightBefore = true;
        }
        else {
          lightBefore = false;
        }
        // printf("now: %d:%d:%d\n",hour,minute,second);
    }
    
    void UpdTime(int msWaitIn) {
      fraction = fraction + (((float)msWaitIn)/920.0); // ~4% time 
      if (fraction >= 1.0) {
        fraction = 0.0;
        second = second + 1;
      }
      if (second > 59) {
        SetTime(true);
      }
    }
    
    void UpdOfflineTime(int msWaitIn) {
      fraction = fraction + (((float)msWaitIn)/1000.0);
      if (fraction >= 1.0) {
        fraction = 0.0;
        second = second + 1;
      }
      if (second > 59) {
        second = 0;
        minute = minute +1;
        //if (lightBefore) {
        //  lightBefore = false;       
        //}
        //else {
        //  lightBefore = true;
        //}
        lightBefore = !lightBefore;
        ////printf("new minute %d\n",lightBefore);
        SetTime(false);
      }
      if (minute > 59) {
        minute = 0;
        hour = hour+1;
      }
    }
  
  private:
    String lastTimeStr;
    bool circle;
    bool lightBefore;
    size_t numLeds;
    size_t color;
    int colorR;
    int colorG;
    int colorB;

    int hour;
    int minute;
    int second;  
    float fraction;
    float factor;
    int timeZone;
};






// WheelOfFortune
class SingleMessage {
  
  public:
    SingleMessage(String messageIn, size_t colIndexIn, size_t maxLoopsIn)
    {
      Init(messageIn, colIndexIn, maxLoopsIn);
    }
    ~SingleMessage() {}

    void Init(String messageIn, size_t colIndex, size_t maxLoopsIn) {
      
      printf("SingleMessage::init\n");

      message = messageIn;
      maxPos = messageIn.length() * 6;
      // char-length*6 + size of panel = one loop
      numCalls = maxLoopsIn;
      actPos = maxPos;
      actCalls = 0;

      setColor(colIndex);
           
    }


    size_t getColor() {
      return color;
    }

    void setColor(size_t newColor) {
      
      color = newColor;
      
      if (color == RANDOM_COLOR) {
        
        colorR = random(100);     
        colorG = random(100);      
        colorB = random(100);
        
        // prefer 1 color     
        int tint = random(3);
        if (tint == 0) {
          colorR = 2*colorR;
        }
        else if (tint == 1) {
          colorG = 2*colorG;
        }
        else if (tint == 2) {
          colorB = 2*colorB;
        }
        
      }  
      else if (color < NUM_COLORS) {
        colorR = Colors[color][COL_R];     
        colorG = Colors[color][COL_G];      
        colorB = Colors[color][COL_B];     
      }  
      else {
        colorR = Colors[COL_WHITE][COL_R];     
        colorG = Colors[COL_WHITE][COL_G];     
        colorB = Colors[COL_WHITE][COL_B];   
      }
      
    }

    void Next(Adafruit_NeoMatrix& matrix)
    {
      if (actPos > maxPos) {
        actPos = -maxPos;
      }
      if (actPos < -maxPos) {
        actPos = maxPos;
        actCalls = actCalls +1;
        printf("next messge display loop\n");
      }
      else {
        actPos = actPos-1;
      }
      
        matrix.fillScreen(0);
        matrix.setTextColor(matrix.Color(colorR, colorG, colorB));
        matrix.setCursor(actPos, 0);
        matrix.print(message);
        matrix.show();

    }

    bool IsDone() {
      return (actCalls >= numCalls);
    }

    bool IsPixel(size_t pixel) {
       return (pixel == (size_t) actPos);
    }

    uint32_t GetColor() {
      return Adafruit_NeoPixel::Color(colorR, colorG, colorB);
    }
    uint32_t GetDimmedColor(float factor) {
      return Adafruit_NeoPixel::Color(colorR * factor, colorG * factor, colorB * factor);
    }
    
    size_t GetR() {
      return colorR;
    }
    size_t GetG() {
      return colorG;
    }
    size_t GetB() {
      return colorB;
    }

  
  private:
  
    int colorR;
    int colorG;
    int colorB;
    size_t color;
    int actPos;
    int maxPos;
    String message;
    int numCalls;
    int actCalls;
};


//
//     ---------------------  main -------------------------------
//

//
//   WIFI shit ... we make it global
//

String ssid = "unknown"; //replace this with your WiFi network name
String password = "empty."; //replace this with your WiFi network password
String realIP = "";

// config page
String brand = "ax";
String clockVersion = "3.00";


// for time init
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -3600;   //Replace with your GMT offset (seconds)
const int   daylightOffset_sec = 0;  //Replace with your daylight offset 

//
//  globals definition
//
//   what to startup
//
String mode = "CLOCK";
String oldMode = "x";
String oldMessage = "x";
String oldMsgTime = "x";
String oldBrightness = "x";
String oldTimezone = "x";
String oldClockColor = "x";
String oldMsgColor = "x";
String oldMsgLoops = "x";

String msgTime = "4";
size_t brightness = 20;
int timezone = 2;
size_t clockColor = 4;
size_t msgColor = 0; 
size_t msgLoops = 5; 

String parseHtml(String reply, String tag, String def) {
  String tagStart = String("<"+tag+">");
  String tagEnd = String("</"+tag+">");
  String value = def;
  int posStart = reply.indexOf(tagStart);
  int posEnd = reply.indexOf(tagEnd);
  //printf("check %d tag: %s %d-%d len: %d\n",reply.laength(), tag, posStart, posEnd, tagStart.length());
  
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

    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    printf("time: %s \n", getTimestring(0));

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



void setGlobals () {

  // we will move this later towards somewhere else
  ConfigData* wifiData = new ConfigData();

  //if (wifiData->getWifiSid() != "noldor") {
  //  wifiData->writeConfig(ssid, password, realIP, brand);
  //}
  
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
    return;
  }
  
  //Serial.print("Requesting URL: ");
  //Serial.println(url);


  // auth string is set as 
  //  base64 encoded user:password
  // 
 
  // This will send the request to the server
  client.print(String("GET ") + "/clock_" + wifiData->getWifiBrand() + "/params.sh" + " HTTP/1.1\r\n" +
               "Host: " + realIP + "\r\n" + 
               "Authorization: Basic YmxpbmRlcjpHaWJIZXI=" + "\r\n\r\n");

  delay(5000);
  String line;
#if 0  
  client.setTimeout(50);
  while(client.available()){
    //printf("setGlobals: html wait another: %s\n", getTimestring());
    //delay(100);
    //String line = client.readStringUntil('\r');
    //printf("setGlobals: html read: %s\n", getTimestring());
    line += client.readString();
    // Serial.print(line);
  }
#else
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
#endif
  
  //printf("setGlobals: html done reading: %s\n", getTimestring());
  //Serial.print(line);


  if (line.length() > 100) {
    String newTimezone = parseHtml(line,String("timezone"),oldTimezone);
    String newClockColor = parseHtml(line,String("clock_color"),oldClockColor);
    String newMsgColor = parseHtml(line,String("msg_color"),oldMsgColor);

    String newBrightness = parseHtml(line,String("brightness"),oldBrightness);
    String newMsgLoops = parseHtml(line,String("msg_loops"),oldMsgLoops);
    String newMsgTime = parseHtml(line,String("msg_time"),oldMsgTime);
    String newMessage = parseHtml(line,String("message"),oldMessage);

    // printf("new: %s %s %s\n",newMode, newStart, newWheels);
#if 0    
    // mode is switched my new string
    String newMode = parseHtml(line,String("mode"),oldMode);
    if (newMode != oldMode) {
      oldMode = newMode;
      mode = newMode;
      printf("setting mode to %s\n",mode);
    }
#endif
    
    if (newTimezone != oldTimezone) {
      oldTimezone = newTimezone;
      int offset = oldTimezone.toInt();
      if ((offset > -24) && (offset < 24)) {
        timezone = offset;
        // setting time to now .... to avoid old messages
        msgTime = getTimesInt(timezone);        
      }
      printf("setting summer/wintertime offset to %d, msgtime to %s\n",timezone, msgTime.c_str());
    }
    
    if (newClockColor != oldClockColor) {
      oldClockColor = newClockColor;
      int value = newClockColor.toInt();
      if ((value >= 0) && (value <= RANDOM_COLOR)) {
        clockColor = value;
      }
      printf("setting clock color to %d\n",clockColor);
    }
    
    if (newMsgColor != oldMsgColor) {
      oldMsgColor = newMsgColor;
      int value = newMsgColor.toInt();
      if ((value >= 0) && (value <= RANDOM_COLOR)) {
        msgColor = value;
      }
      printf("setting message color to %d\n",msgColor);
    }

    if (newBrightness != oldBrightness) {
      oldBrightness = newBrightness;
      int bright = oldBrightness.toInt();
      if ((bright > 0) && (bright < 255)) {
        brightness = bright;
      }
      printf("setting brightness to %d\n",brightness);
    }

    // how often to display this stuff
    if (newMsgLoops != oldMsgLoops) {
      oldMsgLoops = newMsgLoops;
      int value = newMsgLoops.toInt();
      if ((value >= 0) && (value <= 10000)) {
        msgLoops = value;
      }
      printf("setting message loops to %d\n",msgLoops);
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
      printf("setting last message time to %s\n",msgTime.c_str());
    }

    // if we want to display this ... decided by timestamp
    if (newMessage != oldMessage) {
      oldMessage = newMessage;
      printf("setting message to %s\n",oldMessage.c_str());
      //mode = "MESSAGE";
    }    

  }

  
  // printf("setGlobals: html close connection (maybe leave open ?): %s\n", getTimestring());
  client.stop();
  delete wifiData;
  // printf("setGlobals: finished: %s\n", getTimestring());

}

//
//   another part of spaghetti code 
//    wifi config mode
//

IPAddress local_IP(192,168,0,1);
IPAddress gateway(192,168,0,1);
IPAddress subnet(255,255,255,0);


// global wep page for credentials
ConfigData* wifiData;



ESP8266WebServer* server;


String makeHTMLPage(bool reply)
{
    String theValue;
    if (!reply) {
      theValue = String("<!DOCTYPE html><html><body><h2>AX-Clock<h2><h3> Wifi Credentials</h3>")
                    + String("<form action='/action_page'>")
                    + String("SSID:<br><input type='text' name='SSID' value='") + wifiData->getWifiSid() + String("'><br>")
                    + String("PassPhrase:<br><input type='text' name='PassPhrase' value='") + wifiData->getWifiPassword() + String("'><br><br>")
                    + String("ClockID:<br><input type='text' name='ClockID' value='") + wifiData->getWifiBrand() + String("'><br><br>")
                    + String("<input type='submit' value='Submit'></form>")
                    + String("</body></html>"); 
    }
    else {
      theValue = String("<!DOCTYPE html><html><body><h2>AX-Clock<h2><h3> Wifi Credentials</h3>")
                    + String("<b>Saved values<b><br>")
                    + String("SSID: ") + wifiData->getWifiSid() + String("<br>")
                    + String("PassPhrase: ") + wifiData->getWifiPassword() + String("<br>")
                    + String("ClockID: ") + wifiData->getWifiBrand() + String("<br>")
                    + String("</body></html>"); 
    }
    return theValue;
}



void openAP ()
{
  Serial.print("WIFI name: ");
  Serial.println("AXclock passw: 12345678");
  WiFi.mode(WIFI_AP);
  // WiFi.softAP(APssid, APpassword);
  // need at least8 chars in PW !!!
  boolean result = WiFi.softAP("AXclock", "12345678");

  // Serial.println(WiFi.softAP("axel","axel") ? "AP Ready" : "AP Failed!");
  Serial.print("Soft-AP IP address = ");
  Serial.println(WiFi.softAPIP());
#if 0
  while (true) {
    delay(5000); // just avoid busy wait
    Serial.printf("Stations connected = %d\n", WiFi.softAPgetStationNum());
  }
#endif
  
  wifiData = new ConfigData();
  
}


void handleConfig()
{
#if 1  
  server = new ESP8266WebServer(80);

  server->begin();
  
  server->on("/", handleRoot);      //Which routine to handle at root location. This is display page
  //server->on("/config", handleNewConf); //as Per  <a href="ledOn">, Subroutine to be called
  server->on("/action_page", handleForm);

  while (true) {
    delay(100); // just avoid busy wait
    server->handleClient();
    
    // Serial.println("http read loop");
  }
#else
  //WiFiServer webserver(80);
  webserver.begin();
  Serial.println("Server started");  
  while (true) {
    String totalRequest;
    WiFiClient client = webserver.available();
  
    if (client)
    {
      Serial.println("New client connected");
      while (client.connected())
      {
        
        if (client.available())
        {
          String request = client.readStringUntil('\r');
          if (request.length() > 1) {
            Serial.print("chunk: ");
            Serial.println(request);
            
            totalRequest = totalRequest + request;
            Serial.print("total: ");
            Serial.println(totalRequest);
            
          }
          else if (totalRequest.indexOf("GET") > 0) {  
            Serial.print("Client request: ");
            Serial.println(totalRequest);
            Serial.println("providing page");
            String s = makeHTMLPage(false);
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println("");
            client.println(s);
            totalRequest = "";
          }
        }
        delay(20);
      }
    }
    else {
      delay (100);
    }
 
  }
#endif
  
}
#if 1
void handleRoot() {
 Serial.println("You called root page");
 String s = makeHTMLPage(false);
 server->send(200, "text/html", s); //Send web page 
}


void handleForm() {
 Serial.println("You posted a config");
 String mySSID = server->arg("SSID"); 
 String myPass = server->arg("PassPhrase"); 
 String myBrand = server->arg("ClockID"); 

 Serial.print("mySSID:");
 Serial.println(mySSID);

 Serial.print("myPass:");
 Serial.println(myPass);

 wifiData->setWifiSid(mySSID);
 wifiData->setWifiPassword(myPass);
 wifiData->setWifiBrand(myBrand);

 String s = makeHTMLPage(true);
 server->send(200, "text/html", s); //Send web page
}
#endif
#if 0
void handleNewConf() { 
 Serial.println("LED on page");
 digitalWrite(LED,LOW); //LED is connected in reverse
 server->send(200, "text/html", "LED is ON"); //Send ADC value only to client ajax request
}
#endif

//
//   the init / setup boot loop (called only once)
//

void setup() {
  // put your setup code here, to run once:
  // https://www.instructables.com/ESP8266-controlling-Neopixel-LEDs-using-Arduino-ID/
  //  do NOT use the IO-pin (6) from the example !!
  //   never, as in never ever ... do anything here that may cause an exception / segv ... whatever
  //
  Serial.begin(115200);
  delay(100); // we need time to switch the port
  // if we have a problem, give me 5 secs to interrupt on startup
  printf("\n---------------------------------------------------------------\n");
  printf("        Version %s\n",clockVersion.c_str());
  printf("---------------------------------------------------------------\n");
  Serial.flush();
  printf("setup\n");
  
  pinMode(LED_PIN, OUTPUT);
  pinMode(SWITCH_PIN, INPUT);
  delay(1000);
  //printf("setup done\n");

}


//
//   the main loop to execute
//
void loop() {

  bool configMode = digitalRead(SWITCH_PIN);
  
  printf("start loop\n");

  // put your main code here, to run repeatedly:
  //connect();

  // init, call this only once
  Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(LED_COLS, LED_ROWS, LED_PIN,
                            NEO_MATRIX_TOP     + NEO_MATRIX_LEFT +
                            NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG,
                            NEO_GRB            + NEO_KHZ800);
  

    //printf("strip begin\n");
  matrix.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  //printf("strip brightness\n");
  matrix.setBrightness(brightness); // Set BRIGHTNESS (max = 255)

  // wasn dasn ?? scrolling .... could be nice we will see
  matrix.setTextWrap(false);


  //   15 sec intro
  //printf("intro run\n");
  int i = 0;
  {  // as init show all colors and pixels for 2 secs

     printf("intro \n");
     matrix.fillScreen(0);
     matrix.setCursor(0, 0);
     // for configuration give a different text
     if (configMode) {
       matrix.setTextColor(matrix.Color(Colors[COL_WHITE][COL_R], Colors[COL_WHITE][COL_G], Colors[COL_WHITE][COL_B]));
       matrix.print("SETUP");
     }
     else {
       matrix.setTextColor(matrix.Color(Colors[COL_WHITE][COL_R], Colors[COL_WHITE][COL_G], Colors[COL_WHITE][COL_B]));
       matrix.print("init ..");
     }
     matrix.show(); 
     printf("done show\n");
     delay (2000);            // wait 5 secs for next loop
  } // 3 times

  size_t loops = 0;
  int delayMs = 100; // wait 1/10 sec

  if (configMode)
  {
     printf("enter config mode\n");
     // open access point
     openAP();
     // send config page
     handleConfig();
  }
  else {
    setGlobals();

    while (true) {
    
      if (mode == "MESSAGE") {
          printf("start message %s\n", oldMessage.c_str());
          
          SingleMessage ledMessage(oldMessage, msgColor, msgLoops);
          //SingleMessage ledMessage("1234567890", COL_RED);
          
          while ((loops < 1000000) && (mode == "MESSAGE")) {
            // wait 1/10 sec
            delay(delayMs);
            ledMessage.Next(matrix);
            loops++;
            if (ledMessage.IsDone()) {
              mode = "CLOCK";
              setGlobals();
              if (matrix.getBrightness() != brightness) {              
                matrix.setBrightness(brightness); 
              }
            }
          }
      }
      else if (mode == "CLOCK") {
          loops = 0;
          printf("start ledClock\n");
          // clock
          //  1: number LEDs to use
          //  2: seconds leave trace
          LedClock ledClock(LED_COUNT, timezone, true, clockColor);
          
          while (mode == "CLOCK") {
            // wait 1/10 sec
            delay(delayMs);
            ledClock.Next(delayMs, matrix);
            loops++;
            if ((loops % 3000) == 0) {
              setGlobals();
              if (matrix.getBrightness() != brightness) {              
                matrix.setBrightness(brightness); 
              }
              if (ledClock.getColor() != clockColor) {
                ledClock.setColor(clockColor);
              }
              // ledClock.setTime();
              //if ((mode != "CLOCK" ) || (timezone != oldTimezone.toInt())) {
              //  return;
              //}
            }
          }
      }     
      else {
        printf("unknown mode: %s, using CLOCK\n",mode);
        oldMode = mode;
        mode = "CLOCK";
        delay(20000);
      }
    }
  }
  printf("ending + restart\n");
  // strip.end();

}

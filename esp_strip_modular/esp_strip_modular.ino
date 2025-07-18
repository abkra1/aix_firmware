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
// clock
#include <time.h>

#include <ArduinoUniqueID.h>
// include the webserver module / class
#include "config_filesystem/config_filesystem.h"
#include "config_webserver/config_webserver.h"
#include "config_httpget/config_httpget.h"
#include "FS.h"


// globals
//
//  
//  pinMode(6, OUTPUT); // INPUT
//  digitalWrite(13, HIGH); // sets the digital pin 13 on
//  digitalWrite(13, LOW);  // sets the digital pin 13 off
String hwDeviceType = "AXLEDSTRIP";
String firmwareVersion = "4.0";


// Declare our NeoPixel strip object:
// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1:
#define SWITCH_PIN  5
#define ERROR_PIN   4

// How many NeoPixels are attached to the Arduino?
#define LED_PIN    10
#define LED_COUNT 155
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
#define WARM_WHITE 8
#define NUM_COLORS 9
#define RANDOM_COLOR 1000

#define COL_R 0
#define COL_G 1
#define COL_B 2

std::vector<std::vector<size_t>> Colors = {{255,0,0}, // red
                                           {0,255,0}, // green
                                           {0,0,255}, // blue
                                           {155,50,10}, // yellow
                                           // real {255,255,0}, // yellow
                                           //{255,0,255}, // pink
                                           {255,0,128}, // pink
                                           {128,0,128}, // purple
                                           //{200,25,5}, // orange
                                           {255,128,0}, // orange
                                           {255,255,255}, // white
                                           {255,155,40}, // warm white
                                          };

 // define the modes
 
#define WHEEL 0
#define LEDCLOCK 1
#define TWINCLE 2
#define RUNNER 3


// clock
// attention need to get the real time from somewhere ???
//
class LedClock {
  public:
    LedClock(size_t numLedsIn, int timeZoneIn, int circleIn):
    circleMode(circleIn),
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
    };
    ~LedClock() {};
    
    void Init() {};
    
    int GetNext(int in) {
      if (in >= numLeds) {
        return 0;
      }
      return in+1;
    }
    int GetPrev(int in) {
      if (in <= 0) {
        return numLeds-1;
      }
      return in-1;
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

      //printf("rh: %d rm: %d rs: %d\n",hour, minute, second);
      //printf("h: %d m: %d s: %d\n",hour12, minute60, second60);

      // all dark
      // strip.clear(); 
      size_t pix;
      for (pix = 0; pix < numLeds; pix++) {
        size_t red = 0;
        size_t green = 0;
        size_t blue = 0;

        // make elseif -> no merge
        if ((pix == hour12) || (pix == GetPrev(hour12)) || (pix == GetNext(hour12))) {
          green = 200;
        } 
        else if ((pix == minute60) || (pix == GetPrev(minute60)) || (pix == GetNext(minute60))) {
          blue = 200;
        }
        // the one bright dot, draw when mode != 0
        else if ((circleMode != 0) && (pix == second60)) {
          red = 200;
        }
        // now check for seconds "ring/line" draw only in mode 2
        else if (circleMode == 2) {
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
    int circleMode;
    bool lightBefore;
    size_t numLeds;
    int hour;
    int minute;
    int second;  
    float fraction;
    float factor;
    int timeZone;
};


class Twincle {
  public:
    Twincle(size_t numLedsIn, size_t percentOnIn, size_t percentChangeIn):
    numLeds(numLedsIn),
    percentOn(percentOnIn),
    percentChange(percentChangeIn),
    color_index(WARM_WHITE)
    {
      
      size_t pix;
      for (pix=0; (pix < numLeds); pix++) {
        if (percentOnIn > random(100)) {  
          stars.push_back(GetBrightness());
        }
        else {
          stars.push_back(0);
        }
      }
      
    }
    ~Twincle() {};
                    
    int GetBrightness() {
      return random(2) +1;
    }
                    
    uint32_t GetDimmedPixel(int factor) { 
      int realFactor = factor;
      return Adafruit_NeoPixel::Color(Colors[color_index][COL_R] / realFactor, Colors[color_index][COL_G] / realFactor, Colors[color_index][COL_B] / realFactor);
    }
                    
    
                    
    void Next(Adafruit_NeoPixel& strip) {
      // loop all pix any change when needed
      size_t pix;
      for (pix=0; (pix < stars.size()); pix++) {
        // only change a small part
        if (percentChange > random(100)) {
          if (percentOn > random(100)) {  
            stars[pix] = GetBrightness();
          }
          else {
            stars[pix] = 0;
          }
        }
        // now set the light
        if (stars[pix] > 0) {
          strip.setPixelColor(pix,GetDimmedPixel(stars[pix]));
        }
        else {
          strip.setPixelColor(pix,0);
        }
      }
      strip.show();
    }
    
  private:
    size_t numLeds;
    size_t percentOn;
    size_t percentChange;
    size_t color_index;
    std::vector<size_t> stars;
  
};


// WheelOfFortune
class SingleWheel {
  public:
    SingleWheel(size_t numLedsIn, size_t colIndex, size_t numLoops, float inWidth, bool bounceIn) :
    numLeds((float) numLedsIn),
    actPos(0.0),
    right(true),
    loops(numLoops),
    width(inWidth),
    bounce(bounceIn)
    {
      Init(colIndex, numLoops, inWidth);
    }
    ~SingleWheel() {}

    void Init(size_t colIndex, size_t numLoops, float inWidth) {
      //printf("SingleWheel::init\n");
      if (numLoops < 50) {
        maxLoops = numLoops;
      }
      else {
        maxLoops = 10; 
      }
      if (colIndex == RANDOM_COLOR) {
        float tint = random(3);
        colorR = random(100);
        colorG = random(100);
        colorB = random(100);        
        if (tint > 2.0) {
          colorR = colorR*2;
        }
        else if (tint > 2.0) {
          colorG = colorG*2;
        }
        else  {
          colorB = colorB*2;          
        }
      }
      else if (colIndex < NUM_COLORS) {
        colorR = Colors[colIndex][COL_R];     
        colorG = Colors[colIndex][COL_G];      
        colorB = Colors[colIndex][COL_B];     
      }  
      else {
        colorR = Colors[COL_WHITE][COL_R];     
        colorG = Colors[COL_WHITE][COL_G];     
        colorB = Colors[COL_WHITE][COL_B];   
      }
      if ((inWidth > 0.0) && (inWidth < 20.0)) {
        width = inWidth;
      }
      else {
        width =0.1;
      }
      if (random(10) > 5) {
        right = false;
        actPos = numLeds;

      }
      else {
        right = true;
        actPos = 0.0;
      }

      // calc max calls 
      // maxCalls = (int) (random(10) * ((float) maxLoops) * inWidth) + 1;
      maxCalls = (int) (random(2*numLeds) + numLeds);
      
      loops = 0;
      numCalls = 0;

      //printf("init:\nmaxCalls: %d\nwidth: %f\nmaxLoops: %d \n",maxCalls,width,maxLoops);

      
    }

    void Next()
    {
      if (!IsDone()) {
        numCalls++;
        if (right) {
          //actPos = actPos + (width / ((float)loops) + 1.0);
          actPos = actPos + width;
          if (actPos > numLeds-1) {
            if (bounce) {
             right = false;
             actPos = (float)numLeds -width;
            }
            else {
             actPos = actPos - (float)numLeds;
            }
            loops++;
          }
        }
        else {
          // actPos = actPos - (width / ((float)loops) + 1.0);
          actPos = actPos - width;
          if (actPos < 0.0) {
            if (bounce) {
             right = true;
             actPos = width;
            }
            else {
              actPos = actPos + (float)numLeds;
            }
            loops++;
          }
        }
        width = width * (1.0-1.0/maxCalls);
        //printf("SingleWheel::Next %f width %f  calls: %d of %d\n", actPos, width, numCalls, maxCalls);
      }
      else {
        //printf("SingleWheel::Next is Done\n");
      }
    }

    bool IsDone() {
      return (numCalls >= maxCalls);
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
    float numLeds;
    float actPos;
    float width;
    int loops;
    int maxLoops;
    int numCalls;
    int maxCalls;
    bool right;
    bool bounce;
 
};


class WheelOfFortune {
  public:
    WheelOfFortune(size_t ledCount, size_t maxWheels, bool rndColorIn):
    numLeds(ledCount),
    numWheels(maxWheels),
    rndColor(rndColorIn)
    {
      Init();
    }
    ~WheelOfFortune() {}
  
    void Init() 
    {
      int i;
      for (i=0; (i < numWheels) && (i < NUM_COLORS); i++) {
        if (i >= wheels.size()) {
          if (rndColor) {
            wheels.push_back(SingleWheel(numLeds, RANDOM_COLOR, (int) (random(5)+1), 2.0 + ((float)random(20.0)) / 5.0, true));
          }
          else {
            wheels.push_back(SingleWheel(numLeds, i, (int) (random(5)+1), 2.0 + ((float)random(20.0)) / 5.0, true));
          }
        }
        else {
          wheels[i].Init(i, (int) (random(5)+1), 2.0 + ((float)random(20.0)) / 5.0);
        }
      }
      //printf("size: %d\n",wheels.size());
    }
  
    void Next(Adafruit_NeoPixel& strip) {
      size_t i;
      for (i=0; (i < wheels.size()); i++) {
        wheels[i].Next();
      }
      SetStrip(strip);
    }
    
    bool AllDone() {
      size_t i;
      for (i=0; (i < wheels.size()); i++) {
        if (!wheels[i].IsDone()) {
          return false; 
        }
      }
      return true;
    }

    void SetStrip(Adafruit_NeoPixel& strip) {
      size_t pixel;
      size_t wheel;
      // clear the strip (all off)
      strip.clear(); 
      for (pixel = 0; pixel < numLeds; pixel++) {
        // clear the pixel (off)
        // done by clear() -> strip.setPixelColor(pixel, Adafruit_NeoPixel::Color(0,0,0));
        
        // set if position matches
        for (wheel = 0; wheel < wheels.size(); wheel++) {
          if (wheels[wheel].IsPixel(pixel)) { // position of the fortune wheel
            // extract all wheels
            // we need to do something about "overwriting here"
            strip.setPixelColor(pixel,wheels[wheel].GetColor());
            //("%d color\n", pixel);
            // before pixel
            if (pixel == 0) {
              strip.setPixelColor(numLeds-1,wheels[wheel].GetDimmedColor(0.3));
            }
            else {
              strip.setPixelColor(pixel-1,wheels[wheel].GetDimmedColor(0.3));
            }
            // after pixel
            if (pixel == numLeds-1) {
              strip.setPixelColor(0,wheels[wheel].GetDimmedColor(0.3));
            }
            else {
              strip.setPixelColor(pixel+1,wheels[wheel].GetDimmedColor(0.3));
            }
          }
        }
      }
      // make it visible
      strip.show();
    }

  private:
    size_t numWheels;
    size_t numLeds;
    bool rndColor;
    std::vector<SingleWheel> wheels; 

    
};



//
//     ---------------------  main -------------------------------
//

//
//   WIFI shit ... we make it global
//

bool refreshProxy = true;
WifiGetter* getter = NULL;
String deviceID = "empty";
String hardwareDdeviceID ="empty";

//
//  globals definition
//
//   whatever defaults we have ....
//
String mode = "CLOCK";
String oldMode = "x";
String oldStart = "x";
String oldWheels = "x";
String oldBrightness = "x";
String oldTimezone = "x";
String oldShowSecs = "x";
// set something save
bool startNow = true;
size_t numberWheels = 4;
size_t brightness = 200;
int timezone = 2;
int numLeds = LED_COUNT;
int showSecs = 1;


/*
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
*/

// for debugging return time string

String getTimestring() {
  
        time_t rawtime;
        struct tm * timeinfo;
        time (&rawtime);
        timeinfo = localtime (&rawtime);

        // we have someting looking like time
        String theTime = String(timeinfo->tm_hour) + ":" + String (timeinfo->tm_min)+ ":" + String (timeinfo->tm_sec);
        return theTime;
  
}

/*
bool connect() {

  printf("Wifi init\n");
  size_t wait = 0;  
  WiFi.begin(ssid, password);
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
    printf("time: %s \n", getTimestring());
    return false;
  }
  printf("failed: %d \n", WiFi.status());
  return true;
}
*/

// 
//   this si the init call 
//

void setGlobals () {
  
  // we will move this later ... just somewhere else
  ConfigData* wifiData = new ConfigData();
  
  printf("setGlobals: init\n");
  // digitalWrite(ERROR_PIN,HIGH);

  if (getter == NULL) {
      getter = new WifiGetter(wifiData->getWifiSid(), 
                              wifiData->getWifiPassword(), 
                              wifiData->getValue("redirectwebserver"), 
                              wifiData->getValue("redirectwebserverport").toInt(), 
                              wifiData->getValue("redirectwebserverpage"),
                              wifiData->getValue("redirectwebserversecret"));
  }
  
  //printf("setGlobals: html done reading: %s\n", getTimestring());
  //Serial.print(line);
  
  String  httpRequest = String("GET /params_") + hwDeviceType + String("_") + wifiData->getWifiDeviceId() + String(".html HTTP/1.1\r\n") +
               String("Host: ") + getter->GetRealIP() + String("\r\n") + 
               String("Authorization: Basic ") + wifiData->getValue("redirectwebserversecret") + String("\r\n\r\n");

  String line;
  
  if (getter->sendHttpRequest(httpRequest, line, refreshProxy)) {
    
    printf("-------------------\n");
    printf("request:\n%s\n", httpRequest.c_str());
    printf("-------------------\n");
    printf("line:\n%s\n", line.c_str());
    printf("-------------------\n");

    if (line.length() > 300) {
      String newMode = getter->parseHtml(line,String("mode"),oldMode);
      String newStart = getter->parseHtml(line,String("start"),oldStart);
      String newWheels = getter->parseHtml(line,String("wheels"),oldWheels);
      String newBrightness = getter->parseHtml(line,String("brightness"),oldBrightness);
      String newTimezone = getter->parseHtml(line,String("timezone"),oldTimezone);
      String newShowSecs = getter->parseHtml(line,String("showsecs"),oldShowSecs);
      // printf("new: %s %s %s\n",newMode, newStart, newWheels);
      
      if (newMode != oldMode) {
        oldMode = newMode;
        mode = newMode;
        printf("setting mode to %s\n",mode);
      }
      if (newStart != oldStart) {
        oldStart = newStart;
        startNow = true;
        printf("setting start to %d\n",startNow);
      }   
      if (newWheels != oldWheels) {
        oldWheels = newWheels;
        int wheels = oldWheels.toInt();
        if ((wheels > 0) && (wheels < NUM_COLORS)) {
          numberWheels = wheels;
        }
        printf("setting wheels to %d\n",numberWheels);
      }    
      if (newBrightness != oldBrightness) {
        oldBrightness = newBrightness;
        int bright = oldBrightness.toInt();
        if ((bright > 0) && (bright < 255)) {
          brightness = bright;
        }
        printf("setting brightness to %d\n",brightness);
      }
      if (newShowSecs != oldShowSecs) {
        oldShowSecs = newShowSecs;
        int sec = oldShowSecs.toInt();
        if ((sec >= 0) && (sec < 3)) {
          showSecs = sec;
        }
        printf("setting show seconds to %d\n",showSecs);
      }
      if (newTimezone != oldTimezone) {
        oldTimezone = newTimezone;
        int offset = oldTimezone.toInt();
        if ((offset > -24) && (offset < 24)) {
          timezone = offset;
        }
        printf("setting summer/wintertime offset to %d\n",timezone);
      }
      digitalWrite(ERROR_PIN,LOW);
    }    
    else {
      printf("reply too short, retry\n");
      digitalWrite(ERROR_PIN,HIGH);
      // initial call failed ... start blinking
      if (oldMode == "x") {
        for (int loops = 0; loops < 10; loops++) {
          delay(200);
          digitalWrite(ERROR_PIN,LOW);
          delay(200);
          digitalWrite(ERROR_PIN,HIGH);
        }
      }
    }
    refreshProxy = false;
  }
  else {
    printf("failed, full refresh initiated\n");
    refreshProxy = true;
    digitalWrite(ERROR_PIN,HIGH);
  }
}

//
//   first entry point
//
void setup() {
  // put your setup code here, to run once:
  // https://www.instructables.com/ESP8266-controlling-Neopixel-LEDs-using-Arduino-ID/
  //  do NOT use the IO-pin (6) from the example !!
  //   never, as in never ever ... do anything here that may cause an exception / segv ... whatever
  //
  Serial.begin(115200);
  delay(100); // we need time to switch the port

  ArduinoUniqueID uniqueId = ArduinoUniqueID();
  String idStr = "";
  for (int i=0; i < UniqueIDbuffer; i++) {
    int buff = uniqueId.id[i];
     idStr += String(buff) + " "; 
     hardwareDdeviceID = String(buff); // let us hope this is "sufficiently unique"
  }

  
  // if we have a problem, give me 5 secs to interrupt on startup
  printf("\n---------------------------------------------------------------\n");
  printf("        Version %s \n", firmwareVersion);
  printf("---------------------------------------------------------------\n");
  Serial.flush();
  printf("setup\n");

  pinMode(LED_PIN, OUTPUT);
  pinMode(ERROR_PIN, OUTPUT);
  pinMode(SWITCH_PIN, INPUT);
  delay(1000);
  digitalWrite(ERROR_PIN,HIGH);
  //printf("setup done\n");
  
  ConfigData numLedData = ConfigData();

  // digitalWrite(ERROR_PIN,HIGH);

  // only set if reasonably configured
  int numLedsTemp = numLedData.getValue("numleds").toInt();
  if ((numLedsTemp > 12) && (numLedsTemp < 320)) {
    numLeds = numLedsTemp;
    printf("leds now:%d\n", numLeds);
  }
 
}

//
//   the main loop to execute
//
void loop() {
  printf("start loop\n");
  bool configMode = digitalRead(SWITCH_PIN); // open+3.3v = true, gnd = false

  // put your main code here, to run repeatedly:
  //connect();
  // we will move this later ... just somewhere else
 
  // init, call this only once
  Adafruit_NeoPixel strip(numLeds, LED_PIN, NEO_GRB + NEO_KHZ800);
  //printf("strip begin\n");
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  //printf("strip brightness\n");
  strip.setBrightness(20); // Set BRIGHTNESS (max = 255)


  //printf("strip begin\n");
  //printf("strip brightness\n");

  //   15 sec intro
  //printf("intro run\n");
  int i = 0;
  {  // as init show all colors and pixels for 2 secs
      size_t pixel;
      size_t index;
      for (pixel = 0; pixel < numLeds; pixel++) {
        if (index >= NUM_COLORS) {
          index = 0;
        }
        uint32_t nowCol = Adafruit_NeoPixel::Color(Colors[index][COL_R], Colors[index][COL_G], Colors[index][COL_B]);
        strip.setPixelColor(pixel,nowCol);  
        index++;      
      }
      // off we go
      //printf("intro \n");
      strip.show();            // Turn OFF all pixels ASAP
      //printf("done show\n");
      delay (2000);            // wait 5 secs for next loop
  } // 3 times

  size_t loops = 0;
  int delayMs = 100; // wait 1/10 sec

  if (configMode)
  {
     printf("enter config mode\n");
     // create nec config instance and
     // open access point
     ConfigData* confData = new ConfigData();
     WifiConfigWebserver* configServer = new WifiConfigWebserver(confData, hardwareDdeviceID, hwDeviceType);
     configServer->runAcessPoint(); // this does not return
  }
  else {
    setGlobals();
    // take whatever the server said
    strip.setBrightness(brightness); 

    if (mode == "RUNNER") {
        //  intro done
        printf("start wheels\n");
        startNow = false;
        // wheel with 3 weels 
        // and there we go
        WheelOfFortune wheels(numLeds,numberWheels,true);
        while (loops < 100000) {
          wheels.Next(strip);
      
          // wait 1/10 sec
          delay(delayMs);
          ////printf("100 wait done\n"); 
          
          if (wheels.AllDone()) {
            //printf("all done, restart wheels\n"); 
            setGlobals();
            if (strip.getBrightness() != brightness) {              
              strip.setBrightness(brightness); 
            }
            if ((mode != "RUNNER" ) || (numberWheels != oldWheels.toInt())) {
              return;
            }
            // next run
            wheels.Init();
            delay(5000);
            //printf("and ... go\n"); 
          }
          loops++;
        }
    }
    else if (mode == "WHEEL") {
        //  intro done
        printf("start wheels\n");
        startNow = false;
        // wheel with 3 weels 
        // and there we go
        WheelOfFortune wheels(numLeds,numberWheels,false);
        while (loops < 100000) {
          wheels.Next(strip);
      
          // wait 1/10 sec
          delay(delayMs);
          ////printf("100 wait done\n"); 
          
          if (wheels.AllDone()) {
            //printf("all done, restart wheels\n"); 
            setGlobals();
            if (strip.getBrightness() != brightness) {              
              strip.setBrightness(brightness); 
            }
            if ((mode != "WHEEL" ) || (numberWheels != oldWheels.toInt())) {
              return;
            }
            
            // next run
            if (!startNow) {
              delay(20000);
            }
            //printf("and ... go\n"); 
          }
          loops++;
        }
    }
    else if (mode == "CLOCK") {
        printf("start ledClock\n");
        // clock
        //  1: number LEDs to use
        //  2: timezone offset
        //  3: seconds leave trace
        LedClock ledClock(numLeds, timezone, showSecs);
        startNow = false;
        while (loops < 1000000) {
          // wait 1/10 sec
          delay(delayMs);
          ledClock.Next(delayMs, strip);
          loops++;
          if ((loops % 6000) == 0) {
            setGlobals();
            if (strip.getBrightness() != brightness) {      
              strip.setBrightness(brightness); 
            }           
            if ((mode != "CLOCK" ) || (timezone != oldTimezone.toInt())) {
              return;
            }
          }
        
        }
    }     
    else if (mode == "TWINCLE") {
        printf("start twincle\n");
        // twincle action
        //  1: number LEDS to use
        //  2: percent lights on
        //  3: percent change per 1/10 sec
        Twincle twincle(numLeds, 80, 1);
        startNow = false;
        while (loops < 100000) {
          // wait 1/10 sec
          delay(2*delayMs);
          twincle.Next(strip);
          loops++;      
          if ((loops % 6000) == 0) {
            setGlobals();
            if (strip.getBrightness() != brightness) {              
              strip.setBrightness(brightness); 
            }
            if (mode != "TWINCLE" ) {
              return;
            }
          }
        }     
    }
    else {
      printf("unknown mode: %s, using WHEEL\n",mode);
      oldMode = mode;
      mode = "WHEEL";
      delay(20000);
    }
  }
  
  printf("ending + restart\n");
  // strip.end();

}

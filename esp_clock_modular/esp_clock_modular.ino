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

#include <ArduinoUniqueID.h>

// include the webserver module / class
#include "config_filesystem/config_filesystem.h"
#include "config_webserver/config_webserver.h"
#include "config_httpget/config_httpget.h"

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
//#define LED_PIN    13
#define SWITCH_PIN 5
#define ERROR_PIN 4
// How many NeoPixels are attached to the Arduino?
#define LED_PIN 10
#define LED_ROWS 8
#define LED_COLS 32
#define LED_COUNT LED_ROWS* LED_COLS
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

std::vector<std::vector<size_t>> Colors = {
  { 255, 0, 0 },  // red
  { 0, 255, 0 },  // green
  { 0, 0, 255 },  // blue
  //{155,50,10}, // yellow
  { 255, 255, 0 },  // yellow
  //{255,0,255}, // pink
  { 255, 0, 128 },  // pink
  { 128, 0, 128 },  // purple
  //{200,25,5}, // orange
  { 255, 128, 0 },    // orange
  { 255, 255, 255 },  // white
};



// for debugging return time string
//     ----------------   global time helper


String twoChars(int value) {
  String ret;
  ret = "00" + String(value);
  return ret.substring(ret.length() - 2, ret.length());
}

String getTimestring(int timeZone) {

  time_t rawtime;
  struct tm* timeinfo;
  time(&rawtime);
  timeinfo = localtime(&rawtime);

  int hour = timeinfo->tm_hour + timeZone;
  if (hour > 23) {
    hour = hour - 24;
  } else if (hour < 0) {
    hour = hour + 24;
  }


  // we have someting looking like time
  String theTime = twoChars(hour) + ":" + twoChars(timeinfo->tm_min);  //+ ":" + twoChars(timeinfo->tm_sec);
  return theTime;
}


String getTimesInt(int timeZone) {

  time_t rawtime;
  struct tm* timeinfo;
  time(&rawtime);
  timeinfo = localtime(&rawtime);

  int hour = timeinfo->tm_hour + timeZone;
  if (hour > 23) {
    hour = hour - 24;
  } else if (hour < 0) {
    hour = hour + 24;
  }

  // we have someting looking like time
  String theTime = twoChars(timeinfo->tm_year) + twoChars(timeinfo->tm_mon + 1) + twoChars(timeinfo->tm_mday) + twoChars(hour) + twoChars(timeinfo->tm_min) + twoChars(timeinfo->tm_sec);

  printf("getTimesInt(): %s\n", theTime.c_str());


  return theTime;
}


//    ------------   end global time helper




// clock
// get time from ntp.org
//
class LedClock {
public:
  LedClock(size_t numLedsIn, int timeZoneIn, bool circleIn, size_t colorIn)
    : circle(circleIn),
      lightBefore(false),
      numLeds(numLedsIn),
      hour(2),
      minute(10),
      second(0),
      fraction(0.0),
      timeZone(timeZoneIn) {
    factor = ((float)numLeds) / 60.0;
    SetTime(false);
    lastTimeStr = "";
    setColor(colorIn);
  };
  ~LedClock(){};

  void Init(){};

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
    // int hour24 = (int)(((float)(hour % 12)) * factor * 5);

    // printf("h: %d m: %d s: %d\n",hour12, minute60,second60);
    // String timeStr = String(hour) + ":" + String(minute) + ":" + String(second);
    String timeStr = getTimestring(timeZone);

    if (lastTimeStr != timeStr) {
      printf(String("Timestr: " + timeStr + "\n").c_str());
      lastTimeStr = timeStr;
      matrix.fillScreen(0);
      matrix.setTextColor(matrix.Color(colorR, colorG, colorB));
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
        colorR = 2 * colorR;
      } else if (tint == 1) {
        colorG = 2 * colorG;
      } else if (tint == 2) {
        colorB = 2 * colorB;
      }

    } else if (color < NUM_COLORS) {
      colorR = Colors[color][COL_R];
      colorG = Colors[color][COL_G];
      colorB = Colors[color][COL_B];
    } else {
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
    int minute60 = (int)((float)minute * factor);
    int second60 = (int)((((float)second) + fraction) * factor);
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
      } else if (pix == hour12) {
        green = 200;
      } else if (pix == second60) {
        red = 200;
      }
      // now check for seconds if the led "ring" is used
      else if (circle) {
        if (lightBefore) {
          if (pix < second60) {
            red = 30;
          }
        } else {
          if (pix > second60) {
            red = 30;
          }
        }
      } else {  // no circle
        if ((pix == (GetPrev(pix))) || (pix == (GetNext(pix)))) {
          red = 30;
        }
      }
      strip.setPixelColor(pix, Adafruit_NeoPixel::Color(red, green, blue));
    }  // led count loop
    // now show it
    strip.show();
  }

  void SetTime(bool skipSeconds) {
    time_t rawtime;
    struct tm* timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);

    // we have someting looking like time
    hour = timeinfo->tm_hour;
    hour = hour + timeZone;
    if (hour > 23) {
      hour = hour - 24;
    } else if (hour < 0) {
      hour = hour + 24;
    }
    minute = timeinfo->tm_min;

    if (!skipSeconds) {
      second = timeinfo->tm_sec;
    } else {
      second = 0;
    }
    if ((minute % 2) == 0) {
      lightBefore = true;
    } else {
      lightBefore = false;
    }
    // printf("now: %d:%d:%d\n",hour,minute,second);
  }

  void UpdTime(int msWaitIn) {
    fraction = fraction + (((float)msWaitIn) / 920.0);  // ~4% time
    if (fraction >= 1.0) {
      fraction = 0.0;
      second = second + 1;
    }
    if (second > 59) {
      SetTime(true);
    }
  }

  void UpdOfflineTime(int msWaitIn) {
    fraction = fraction + (((float)msWaitIn) / 1000.0);
    if (fraction >= 1.0) {
      fraction = 0.0;
      second = second + 1;
    }
    if (second > 59) {
      second = 0;
      minute = minute + 1;
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
      hour = hour + 1;
    }
  }

  // calcualte the alert minute here and return true exacltly once per day
  bool IsAlert(bool& hadAlert, String alertString, String alertMarker) {
    if ((alertMarker != "") && (alertMarker != "0")) {
      if (lastTimeStr == alertString) {
        if (hadAlert) {
          return false;
        }
        printf("signaling ALERT %s\n", alertString);
        hadAlert = true;
        return true;
      }
    }
    hadAlert = false;
    return false;
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

  // bool hadAlert;
};



class SingleMessage {

public:
  SingleMessage(String messageIn, size_t colIndexIn, size_t maxLoopsIn) {
    Init(messageIn, colIndexIn, maxLoopsIn);
  }
  ~SingleMessage() {}

  void Init(String messageIn, size_t colIndex, size_t maxLoopsIn) {

    printf("SingleMessage::init\n");

    message = messageIn;
    maxPos = message.length() * 6;
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
        colorR = 2 * colorR;
      } else if (tint == 1) {
        colorG = 2 * colorG;
      } else if (tint == 2) {
        colorB = 2 * colorB;
      }

    } else if (color < NUM_COLORS) {
      colorR = Colors[color][COL_R];
      colorG = Colors[color][COL_G];
      colorB = Colors[color][COL_B];
    } else {
      colorR = Colors[COL_WHITE][COL_R];
      colorG = Colors[COL_WHITE][COL_G];
      colorB = Colors[COL_WHITE][COL_B];
    }
  }

  void Next(Adafruit_NeoMatrix& matrix) {
    if (actPos > LED_COLS) {
      actPos = LED_COLS;
    }
    if (actPos < -maxPos) {
      actPos = LED_COLS;
      actCalls = actCalls + 1;
      printf("next message display loop\n");
    } else {
      actPos = actPos - 1;
      printf("message pos %d\n", actPos);
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
    return (pixel == (size_t)actPos);
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


class FlashAlert {

public:
  FlashAlert(size_t colIndexIn, size_t maxLoopsIn, int brightnessIn) {
    Init(colIndexIn, maxLoopsIn, brightnessIn);
  }
  ~FlashAlert() {}

  void Init(size_t colIndex, size_t maxLoopsIn, int brightnessIn) {

    printf("FlashAlarm::init\n");

    numCalls = maxLoopsIn;
    actCalls = 0;
    setColor(colIndex);
    isBlack = false;
    brightness = brightnessIn;
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
        colorR = 2 * colorR;
      } else if (tint == 1) {
        colorG = 2 * colorG;
      } else if (tint == 2) {
        colorB = 2 * colorB;
      }

    } else if (color < NUM_COLORS) {
      colorR = Colors[color][COL_R];
      colorG = Colors[color][COL_G];
      colorB = Colors[color][COL_B];
    } else {
      colorR = Colors[COL_WHITE][COL_R];
      colorG = Colors[COL_WHITE][COL_G];
      colorB = Colors[COL_WHITE][COL_B];
    }
  }

  void Next(Adafruit_NeoMatrix& matrix) {
    actCalls = actCalls + 1;

    matrix.fillScreen(0);
    if (isBlack) {
      isBlack = false;
      matrix.fillScreen(matrix.Color(0, 0, 0));
      printf("next alert display loop (black) \n");
    } else {
      isBlack = true;
      matrix.setBrightness(brightness);  // make it really "loud"
      matrix.fillScreen(matrix.Color(colorR, colorG, colorB));
      printf("next alert display loop (color) \n");
    }
    matrix.show();
  }

  bool IsDone() {
    return (actCalls >= numCalls);
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

  int brightness;
  int colorR;
  int colorG;
  int colorB;
  size_t color;
  int numCalls;
  int actCalls;
  bool isBlack;
};


//
//     ---------------------  main -------------------------------
//

// config page
String hwDeviceType = "AXCLOCK";
String firmwareVersion = "5.2";

//
//   WIFI stuff ... we make it global
//
//
//   another part of spaghetti code
//    wifi config mode
//

bool refreshProxy = true;
WifiGetter* getter = NULL;
String hardwareDeviceID = "empty";


//
//  globals definition
//
//   what to startup
//
String mode = "CLOCK";
String oldMode = "x";
String oldMessage = "x";
//String oldmessge = "x";
String oldBrightness = "x";
String oldTimezone = "x";
String oldClockColor = "x";
String oldMsgColor = "x";
String oldMsgLoops = "x";
String oldAlert = "";
String oldAlertTime = "";

//String msgTime = "4";
size_t brightness = 20;
int timezone = 2;
size_t clockColor = 4;
size_t msgColor = 0;
size_t msgLoops = 5;
size_t msgCount = 0;
bool hadAlert = false;


void setGlobals() {

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


  // auth string is set as
  //  base64 encoded user:password
  //

  // This will send the request to the server
  String httpGetRequest = String("/params_") + hwDeviceType + String("_") + wifiData->getWifiDeviceId() + String(".html HTTP/1.1\r\n") + String("Host: ") + getter->GetRealIP() + String("\r\n") + String("Authorization: Basic ") + wifiData->getValue("redirectwebserversecret") + String("\r\n\r\n");

  String line;
  if (getter->sendHttpGetRequest(httpGetRequest, line, refreshProxy)) {
    /*
    printf("-------------------\n");
    printf("request:\n%s\n", httpRequest.c_str());
    printf("-------------------\n");
    printf("line:\n%s\n", line.c_str());
    printf("-------------------\n");
*/
    if (line.length() > 500) {
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
#if 0    
      // mode is switched my new string
      String newMode = getter->parseHtml(line,String("mode"),oldMode);
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
#if 0
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
#endif

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
      digitalWrite(ERROR_PIN, LOW);
    } else {
      printf("reply too short, retry\n\n%s", line.c_str());
      digitalWrite(ERROR_PIN, HIGH);
      // initial call failed ... start blinking
      if (oldMode == "x") {
        for (int loops = 0; loops < 10; loops++) {
          delay(200);
          digitalWrite(ERROR_PIN, LOW);
          delay(200);
          digitalWrite(ERROR_PIN, HIGH);
        }
      }
    }
    refreshProxy = false;
  } else {
    printf("failed, full refresh initiated\n");
    refreshProxy = true;
    digitalWrite(ERROR_PIN, HIGH);
  }
  // printf("setGlobals: finished: %s\n", getTimestring());
}





//
//   the init / setup boot loop (called only once)
//

void setup() {

  //  do NOT use the IO-pin (6) from some examples !!
  //   never, as in never ever ... do anything here that may cause an exception / segv ... whatever
  //
  Serial.begin(115200);
  delay(100);  // we need time to switch the port
  // if we have a problem, give me 1-2 secs to interrupt on startup

  // read the unique string, we should use this as setter for the default ID
  // maybe even the ID is non-mutable in next release
  ArduinoUniqueID uniqueId = ArduinoUniqueID();
  String idStr = "";
  for (int i = 0; i < UniqueIDbuffer; i++) {
    int buff = uniqueId.id[i];
    idStr += String(buff) + " ";
    hardwareDeviceID = String(buff);  // let us hope this is "sufficiently unique"
  }

  printf("\n---------------------------------------------------------------\n");
  printf("        Version %s\n", firmwareVersion.c_str());
  printf("        Id %s\n", idStr.c_str());
  printf("        HardwareId %s\n", hardwareDeviceID.c_str());
  printf("---------------------------------------------------------------\n");
  Serial.flush();
  printf("setup\n");

  pinMode(LED_PIN, OUTPUT);
  pinMode(ERROR_PIN, OUTPUT);
  pinMode(SWITCH_PIN, INPUT);
  delay(1000);
  digitalWrite(ERROR_PIN, HIGH);
  //printf("setup done\n");
}


//
//   the main loop to execute
//
void loop() {

  bool configMode = digitalRead(SWITCH_PIN);  // open+3.3v = true, gnd = false

  printf("start loop\n");

  // put your main code here, to run repeatedly:
  //connect();

  // init, call this only once
  Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(LED_COLS, LED_ROWS, LED_PIN,
                                                 NEO_MATRIX_TOP + NEO_MATRIX_LEFT + NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG,
                                                 NEO_GRB + NEO_KHZ800);


  //printf("strip begin\n");
  matrix.begin();  // INITIALIZE NeoPixel strip object (REQUIRED)
  //printf("strip brightness\n");
  matrix.setBrightness(brightness);  // Set BRIGHTNESS (max = 255)

  // wasn dasn ?? scrolling .... could be nice we will see
  matrix.setTextWrap(false);


  //   15 sec intro
  //printf("intro run\n");
  int i = 0;
  {  // as init show all colors and pixels for 2 secs

    printf("intro \n");

    ConfigData* wifiData = new ConfigData();

    matrix.fillScreen(0);
    matrix.setCursor(0, 0);
    // for configuration give a different text
    if (configMode) {
      matrix.setTextColor(matrix.Color(Colors[COL_WHITE][COL_R], Colors[COL_WHITE][COL_G], Colors[COL_WHITE][COL_B]));
      matrix.print("SETUP");
    } else {
      matrix.setTextColor(matrix.Color(Colors[COL_WHITE][COL_R], Colors[COL_WHITE][COL_G], Colors[COL_WHITE][COL_B]));
      String initStr = ":" + wifiData->getWifiDeviceId();
      matrix.print(initStr.c_str());
    }
    matrix.show();

    delete wifiData;

    printf("done show\n");
    delay(2000);  // wait 5 secs for next loop
  }               // 3 times

  size_t loops = 0;
  int delayMs = 100;  // wait 1/10 sec

  if (configMode) {
    printf("enter config mode\n");
    // create nec config instance and
    // open access point
    ConfigData* confData = new ConfigData();
    WifiConfigWebserver* configServer = new WifiConfigWebserver(confData, hardwareDeviceID, hwDeviceType);
    configServer->runAcessPoint();  // this does not return
  } else {
    setGlobals();
    matrix.setBrightness(brightness);  // Set BRIGHTNESS (max = 255)

    while (true) {

      if (mode == "ALERT") {
        printf("start alarm %s\n", oldMessage.c_str());

        FlashAlert alert(msgColor, 2 * msgLoops, oldAlert.toInt());
        //SingleMessage ledMessage("1234567890", COL_RED);

        while ((loops < 1000000) && (mode == "ALERT")) {
          // wait 1/10 sec
          delay(delayMs * 5);
          alert.Next(matrix);
          loops++;
          if (alert.IsDone()) {
            mode = "CLOCK";
            setGlobals();
            //if (matrix.getBrightness() != brightness) {
            matrix.setBrightness(brightness);
            //}
          }
        }
      } else if (mode == "MESSAGE") {
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
      } else if (mode == "CLOCK") {
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
            //if (ledClock.getColor() != clockColor) {
            //  using RANDOM_COLOR allow to to change every 5-6 minutes
            ledClock.setColor(clockColor);
            //}
          }
          if ((loops % 300) == 0) {
            // special alert check .... has to be here since we do not have a global time
            if (ledClock.IsAlert(hadAlert, oldAlertTime, oldAlert)) {
              mode = "ALERT";
            }
          }
        }
      } else {
        printf("unknown mode: %s, using CLOCK\n", mode);
        oldMode = mode;
        mode = "CLOCK";
        delay(20000);
      }
    }
  }
  printf("ending + restart\n");
  // strip.end();
}

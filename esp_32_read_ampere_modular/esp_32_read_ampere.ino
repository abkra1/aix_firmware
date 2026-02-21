
// #include <Arduino.h>
#include "SPI.h"
#include "esp_wifi.h"

#include <WiFi.h>
#include <WiFiClientSecure.h>

#include <driver/adc.h>
#include <config.h>
#include <ArduinoUniqueID.h>


#include "EmonLib.h"

#define ADC_INPUT_1 34
#define ADC_INPUT_2 35
#define ADC_INPUT_3 32
#define ADC_INPUT_4 33

// Force EmonLib to use 10bit ADC resolution
#define ADC_BITS    10
#define ADC_COUNTS  (1<<ADC_BITS)


#define LED_BLUE 14
#define LED_YELLOW 12
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
// config page
String hwDeviceType = "AXAMPERE";
String firmwareVersion = "2.0";


// const char* ssid = "galadriel"; //replace this with your WiFi network name
// const char* password = "3Kinderund2Erwachsene"; //replace this with your WiFi network password
//const char* ssid = "noldor"; //replace this with your WiFi network name
//const char* password = "MWisWima8Zh."; //replace this with your WiFi network password
// const char* host = "192.168.93.13";
const int httpPort = 443;
String realHost =  "192.168.93.13";
bool initDone = false;
int failCount = 0;

/*
void listNetworks() {
  // scan for nearby networks:
  Serial.println("** Scan Networks **");
  int numSsid = WiFi.scanNetworks();
  if (numSsid == -1) {
    Serial.println("Couldn't get any wifi connection");
    return;
  }

  // print the list of networks seen:
  Serial.print("number of available networks:");
  Serial.println(numSsid);

  // print the network number and name for each network found:
  for (int thisNet = 0; thisNet < numSsid; thisNet++) {
    Serial.print(thisNet);
    Serial.print(") <");
    Serial.print(WiFi.SSID(thisNet));
    Serial.print("> Signal: ");
    Serial.print(WiFi.RSSI(thisNet));
    Serial.println(" dBm");
  }
}
*/
/*
void IRAM_ATTR WiFiEvent(WiFiEvent_t event)
{
  switch (event) {
    case SYSTEM_EVENT_STA_CONNECTED:
      printf("connected\n");
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      printf("Disconnected from WiFi access point\n");
      break;
    case SYSTEM_EVENT_AP_STADISCONNECTED:
      printf("WiFi client disconnected\n");
      break;
    case WIFI_REASON_AUTH_EXPIRE:
      printf("WiFi client auth expire\n");
      break;
    case WIFI_REASON_UNSPECIFIED:
      printf("WiFi client unspecified\n");
      break;
    default: 
      printf("WiFi event: %d\n",event);
      break;
  }
}
*/
/*
WL_IDLE_STATUS   0
WL_NO_SSID_AVAIL  1
WL_SCAN_COMPLETED   2
WL_CONNECTED  3
WL_CONNECT_FAILED   4
WL_CONNECTION_LOST  5
WL_DISCONNECTED   6
*/
/*
bool connect() {

  printf("Wifi init\n");
  size_t wait = 0;  
  //WiFi.disconnect();
  //WiFi.onEvent(WiFiEvent);
  WiFi.mode(WIFI_STA);// we are a client !
  delay(100);
  printf("WifiData: <%s> <%s>\n", ssid, password);
  WiFi.begin(ssid, password);
  initDone = false;
  //uint8_t status = WiFi.waitForConnectResult();
  uint8_t status = WiFi.status();
  while ((status != WL_CONNECTED) && (wait < 30)) {
    wait++;

    //Serial.println(connectionStatusMessage(status));
    
    printf("connecting: %d, %d \n", wait, status);
    
    delay(300);

    if ((wait == 20) || (wait == 20)) { 

      digitalWrite(LED_YELLOW, LOW);
      delay(100);
      digitalWrite(LED_YELLOW, HIGH);
      delay(100);
      digitalWrite(LED_YELLOW, LOW);
      delay(100);
      digitalWrite(LED_YELLOW, HIGH);
      delay(100);
      digitalWrite(LED_YELLOW, LOW);
      
      int blink=0;
      for (blink=0;blink<status;blink++) {
        delay(250);
        digitalWrite(LED_YELLOW, HIGH);
        delay(250);
        digitalWrite(LED_YELLOW, LOW);
      }
      delay(3000);
      digitalWrite(LED_YELLOW, HIGH);
      
      printf("reconnect\n");
      //WiFi.disconnect();
      delay(100);
      //WiFi.begin(ssid, password);
      //WiFi.reconnect();
    }
    
//    status = WiFi.waitForConnectResult();
    status = WiFi.status();
  }
  if (WiFi.status() == WL_CONNECTED) {
    IPAddress ip = WiFi.localIP();
    Serial.print("IP Address: ");
    Serial.println(ip);
    
    digitalWrite(LED_YELLOW, LOW);
    failCount = 0;

    return false;
  }
  printf("failed: %d \n", WiFi.status());
  failCount++;
  return true;
}

*/


void sendData () {

  //  network stuff
  if (WiFi.status() != WL_CONNECTED) {
     //printf("setGlobals: WIFI (re-)connect: %s\n", getTimestring());
    if (connect()) {
      if (failCount > 5) {
        ESP.restart();
      }
      printf("failed to connect\n");
      return;
    }
  }
  
  realHost = getIPViaAbkra();
   
  WiFiClientSecure client;
  client.setInsecure(); // we have insecure certs here
    
  if (!client.connect(realHost.c_str(), httpPort)) {
    printf("connection to %s failed\n", realHost.c_str());
    
    return;
  }

  // build the URL (passing on the values
  String url = "/amperemeter/SETTER.sh?amps1=" + String(amps1,4) +"&amps2=" + String(amps2,4) +"&amps3=" + String(amps3,4) +"&amps4=" + String(amps4,4);


  String getterCall = "GET " + url + " HTTP/1.1\r\n" +
               "Host: " + realHost + "\r\n" + 
               "Authorization: Basic YmxpbmRlcjpHaWJIZXI=" + "\r\n\r\n";

  printf("calling\n%s\n", getterCall.c_str());

  // This will send the request to the server
  client.print(getterCall);

  delay(50);
  String line;

  // read the reply .... an do not care at all :-)
  char nextChar = 0;
  //printf("setGlobals: html read: %s\n", getTimestring());
  while(client.available() && (nextChar < 254)){
    nextChar = client.read();
    if (nextChar < 254) {
      //printf("%c",nextChar); 
      line += String(nextChar);
    }
  }
  
  printf("getter returned: %s\n", line.c_str());
  
  // we could ..... parse something here
#if 0
    if (line.length() > 100) {
       yickety yack
    }
#endif
  client.stop();
   
}




void setup() {

  
  // put your setup code here, to run once:
  delay(5000);
  Serial.begin(115200);
  
  pinMode(LED_BLUE, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  
  digitalWrite(LED_BLUE, HIGH);
  digitalWrite(LED_YELLOW, HIGH);

  ArduinoUniqueID uniqueId = ArduinoUniqueID();
  String idStr = "";
  for (int i=0; i < UniqueIDbuffer; i++) {
    int buff = uniqueId.id[i];
     idStr += String(buff) + " "; 
  }

  printf("\n---------------------------------------------------------------\n");
  printf("        WIFI Amperemeter\n");
  printf("        Version %s\n", firmwareVersion.c_str());
  printf("        Id %s\n", idStr.c_str());
  printf("        HardwareId %s\n", hardwareDeviceID.c_str());
  printf("---------------------------------------------------------------\n");

  Serial.println("init");

  wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));
    //esp_err_t esp_wifi_set_ps(wifi_ps_type_t type)  
  ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));

  
  adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_11);
  analogReadResolution(10);

  // Initialize emon library (28-30 = calibration number for 1 = 1A)
  emon1.current(ADC_INPUT_1, 185.0);
  emon2.current(ADC_INPUT_2, 185.0);
  emon3.current(ADC_INPUT_3, 185.0);
  emon4.current(ADC_INPUT_4, 185.0);
  // 1600W calibr 68 = 6.8 A = 285.0 cal

  listNetworks();

  delay(1000);  

  connect();
  
  digitalWrite(LED_BLUE, LOW);
  Serial.println("init done");


  // If we still couldn't connect to the WiFi, go to deep sleep for a
  // minute and try again.
  // if(WiFi.status() != WL_CONNECTED){
  //    esp_sleep_enable_timer_wakeup(1 * 60L * 1000000L);
  //  esp_deep_sleep_start();
  // }


  
}

void loop() {


  unsigned long currentMillis = millis();

  // If it's been longer then 1000ms since we took a measurement, take one now!
  // if(currentMillis - lastMeasurement > 1000){
  //   nada nada nada jada
  
  int loops = 0;
  
  while (loops < 1000) {
    loops++;
    digitalWrite(LED_BLUE, HIGH);
    delay (1000);
    
    Serial.println("lesen");

    // we read several times and use average value
    amps1 = emon1.calcIrms(1480); // Calculate Irms only with magic number
    amps2 = emon2.calcIrms(1480); // Calculate Irms only with magic number
    amps3 = emon3.calcIrms(1480); // Calculate Irms only with magic number
    amps4 = emon4.calcIrms(1480); // Calculate Irms only with magic number
    
    size_t i;
    for (i=1;i<MAX_READ;i++) {
      Serial.print(".");
      delay(20);
      amps1 += emon1.calcIrms(1480); 
      amps2 += emon2.calcIrms(1480); 
      amps3 += emon3.calcIrms(1480); 
      amps4 += emon4.calcIrms(1480); 
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
    sendData();
    digitalWrite(LED_BLUE, LOW);
    delay(55000);
  }
  
}

#ifndef H_CONFIG_WEBSERVER
#define H_CONFIG_WEBSERVER

//
//   configuration web module
//     to be included and called by setup + loop
//

// #include "config.h"
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>


#include <ESP8266WebServer.h>
// attention this needs to be inlcluded in main
//#include "../config_fileserver/config_fileserver.h"




// the one global instance of the webserver
// has to be global and static for callback handling
static ESP8266WebServer* server;
static ConfigData*       wifiData;
static String            hardwareDevice;
static String            deviceType;

// -----------------------------


class WifiConfigWebserver {


  public: 
    WifiConfigWebserver (ConfigData* configData, String newHwDevice, String newDeviceType)
    {
      wifiData = configData;
      hardwareDevice = newHwDevice;
      deviceType = newDeviceType;
    }
        
    // really start the acess point and give over the handling
    // this will not return
    void runAcessPoint ()
    {
      Serial.print("WIFI name: ");
      Serial.println("AIX-Gadged passw: 12345678");
      WiFi.mode(WIFI_AP);
      // WiFi.softAP(APssid, APpassword);
      // need at least8 chars in PW !!!
      boolean result = WiFi.softAP("AIX-Gadged (PW:12345678)", "12345678");
    
      // Serial.println(WiFi.softAP("aix","aix") ? "AP Ready" : "AP Failed!");
      Serial.print("Soft-AP IP address = ");
      Serial.println(WiFi.softAPIP());
    #if 0
      // some debug BS ..... 
      while (true) {
        delay(5000); // just avoid busy wait
        Serial.printf("Stations connected = %d\n", WiFi.softAPgetStationNum());
      }
    #endif
      
      handleConfig();
      
    }
     

  private:

    // just really create and start the ap
    void handleConfig()
    {
      server = new ESP8266WebServer(80);
    
      server->begin();
      
      server->on("/", handleRoot);      //Which routine to handle at root location  (fallback for all)
      //server->on("/config", handleNewConf); //as Per  <a href="ledOn">, Subroutine to be called
      server->on("/action_page", this->handleForm);
      //server->on("/restart", this->handleRestart);
    
      while (true) {
        delay(100); // just avoid busy wait
        server->handleClient();
        
        // Serial.println("http read loop");
      }
    }



    //
    //  this is the one page returned to the browser
    //   
    //
    static String makeHTMLPage(bool reply)
    {
    
    
        // there are nicer ways to define this ....
        // I wanted to avoid having to embed a template, well this is not better but it gets the job done
        String theValue;
        if (!reply) {
          theValue = String("<!DOCTYPE html><html><head><title>AIX-Gadged config page</title></head><body><h2>AIX-Gadged:<br>Type: AXLEDSTRIP, HW-ID-") + hardwareDevice +String("<h2><h3>Settings and Wifi Credentials</h3>")
                        + String("<form action='/action_page'>")
                        + String("SSID:<br><input type='text' name='SSID' value='") + wifiData->getWifiSid() + String("'><br>")
                        + String("PassPhrase:<br><input type='text' name='PassPhrase' value='") + wifiData->getWifiPassword() + String("'><br><br>")
                        + String("DeviceID:<br><input type='text' name='DeviceID' value='") + wifiData->getWifiDeviceId() + String("'><br><br>")
                        + String("Redirect Webserver:<br><input type='text' name='RedirectWebserver' value='") + wifiData->getValue("redirectwebserver") + String("'><br><br>")
                        + String("Redirect Webserver Port:<br><input type='text' name='RedirectWebserverPort' value='") + wifiData->getValue("redirectwebserverport") + String("'><br><br>")
                        + String("Redirect Webserver Page:<br><input type='text' name='RedirectWebserverPage' value='") + wifiData->getValue("redirectwebserverpage") + String("'><br><br>")
                        + String("Redirect Webserver Secret:<br><input type='text' name='RedirectWebserverSecret' value='") + wifiData->getValue("redirectwebserversecret") + String("'><br><br>");
                        
                        // special value for this gadged
                        // none so far ....  
          //theValue = theValue              
          //              + String("NumLeds:<br><input type='text' name='NumLeds' value='") + wifiData->getValue("numleds") + String("'><br><br>");
                        
          theValue = theValue               
                        + String("<input type='submit' value='Submit'></form>")
                        + String("</body></html>"); 
        }
        else {
          theValue = String("<!DOCTYPE html><html><head><title>AIX-Gadged config page</title></head><body><h2>AIX-Gadged:<br>Type: AXLEDSTRIP, HW-ID-") + hardwareDevice +String("<h2><h3>Settings and  Wifi Credentials</h3>")
                        + String("<b>Saved values<b><br>")
                        + String("SSID: ") + wifiData->getWifiSid() + String("<br>")
                        + String("PassPhrase: ") + wifiData->getWifiPassword() + String("<br>")
                        + String("DeviceID: ") + wifiData->getWifiDeviceId() + String("<br><br>")
                        + String("Redirect Webserver: ") + wifiData->getValue("redirectwebserver") + String("<br><br>")
                        + String("Redirect Webserver Port: ") + wifiData->getValue("redirectwebserverport") + String("<br><br>")
                        + String("Redirect Webserver Page: ") + wifiData->getValue("redirectwebserverpage") + String("<br><br>")
                        + String("Redirect Webserver Secret: ") + wifiData->getValue("redirectwebserversecret") + String("<br><br>");
                        
                        
          // adding the special setting for this page
          // none so far ....              
          //theValue = theValue              
          //              + String("NumLeds: ") + wifiData->getValue("numleds") + String("<br><br>");
            
          theValue = theValue 
			            + String("<a href='/'>Edit Mode</a><br><br>")
                        + String("</body></html>"); 
        }
        return theValue;
    }
    

    static void handleRoot() {
     Serial.println("You called root page");
     String s = makeHTMLPage(false);
     server->send(200, "text/html", s); //Send web page 
    }
    
    static void handleRestart() {
     Serial.println("Restarting by web request");
     ESP.restart();     
    }
    
    
    static void handleForm() {
     Serial.println("You posted a config");
     String mySSID = server->arg("SSID"); 
     String myPass = server->arg("PassPhrase"); 
     String myDeviceId = server->arg("DeviceID"); 
     
     String myRedirectWebserver = server->arg("RedirectWebserver"); 
     String myRedirectWebserverPort = server->arg("RedirectWebserverPort"); 
     String myRedirectWebserverPage = server->arg("RedirectWebserverPage"); 
     String myRedirectWebserverSecret = server->arg("RedirectWebserverSecret"); 
         
     Serial.print("mySSID:");
     Serial.println(mySSID);
    
     Serial.print("myPass:");
     Serial.println(myPass);
    
     // directly write to flash memory
     wifiData->setWifiSid(mySSID);
     wifiData->setWifiPassword(myPass);
     wifiData->setWifiDeviceId(myDeviceId);
     
     wifiData->setValue("redirectwebserver", myRedirectWebserver);
     wifiData->setValue("redirectwebserverport", myRedirectWebserverPort);
     wifiData->setValue("redirectwebserverpage", myRedirectWebserverPage);
     wifiData->setValue("redirectwebserversecret", myRedirectWebserverSecret);
     
     // special definition for this gadged
     // none so far ....
     // String myNumLeds = server->arg("NumLeds"); 
     // wifiData->setValue("numleds", myNumLeds);
        
        
     String s = makeHTMLPage(true);
     server->send(200, "text/html", s); //Send web page
    }



  


};


#endif

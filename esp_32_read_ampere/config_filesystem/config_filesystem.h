#ifndef H_CONFIG_FILESYSTEM
#define H_CONFIG_FILESYSTEM
//
//   configuration local filesystem module
//     to be included and called by setup + loop
//


// configurtion stuff
//  saved on local
#include <SPIFFS.h>
#include "FS.h"


//
//
//   some defines used by all modules 
//
//
#define WIFI



//
//   the real class
//
class ConfigData {
    public:

       ConfigData():
       wifiPassword("empty"),
       wifiSid("empty"),
       wifiDeviceId("empty"),
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
           wifiDeviceId = readConfigFile("/deviceid.txt");
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


       bool writeConfig(String sid, String password, String realip, String deviceid) {
           printf("ConfigData()::writeConfig()\n");
           setWifiSid(sid);
           setWifiPassword(password);
           setWifiDeviceId(wifiDeviceId);
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
       
       String getWifiDeviceId() {
            return wifiDeviceId;
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

       bool setWifiDeviceId(String newVal) {
         writeConfigFile("/deviceid.txt",newVal);
         wifiDeviceId = newVal;
         return configError;
       }
       
       // in addition there are custom setters and getters by "name"
       

       // returns false if the value is not existing
       bool setValue(String valueName, String& valueContent) {
         String fileName = "/" + valueName + ".txt"; 
         writeConfigFile(fileName, valueContent);
         return configError;
       }
       // returns empty the value is not existing
       String getValue(String valueName) {
         String fileName = "/" + valueName + ".txt"; 
         String valueContent = readConfigFile(fileName);
         return valueContent;
       }

       bool getValue(String valueName, String& valueContent) {
         String fileName = "/" + valueName + ".txt"; 
         valueContent = readConfigFile(fileName);
         return configError;
       }
       

    private:
       String wifiSid;
       String wifiPassword;
       String wifiDeviceId;
       bool configError;
       
};

#endif

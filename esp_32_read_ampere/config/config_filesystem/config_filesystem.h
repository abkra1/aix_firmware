#ifndef H_CONFIG_FILESYSTEM
#define H_CONFIG_FILESYSTEM
//
//   configuration local filesystem module
//     to be included and called by setup + loop
//


// configurtion stuff
//  saved on local
#include "FS.h"
#include <SPIFFS.h>


//
//
//   some defines used by all modules 
//
//
#define WIFI



//
//   the real class
//
class FileSystemData {
    public:

       FileSystemData():
       wifiPassword("empty"),
       wifiSid("empty"),
       wifiDeviceId("empty"),
       configError(false)
       {
            printf("FileSystemData: v 2.0\n");
            printf("ConfigData() init\n");
	    //fileDataparams = Params();
            bool initFlag = SPIFFS.begin(true);  // true = format + create fs if failed
            if (!initFlag) {
              printf("ConfigData() init failed, formating fs\n");
	      initFlag = SPIFFS.begin(true);
	      if (!initFlag) {
	          printf("ConfigData() format failed\n");
                  configError = true;
	      }
            }
            else {
              configError = readConfig();
            }
       }
       
       ~FileSystemData(){
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

       String readConfigFile (const String& fileName) {
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

       bool writeConfigFile (const String& fileName, const String& content) {
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

       
       // keep this only for compatibility
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
       // this is the real interface 

       // returns true if the value is not existing
       bool setValue(const String& valueName, const String& valueContent) {
         String fileName = "/" + valueName + ".txt"; 
         writeConfigFile(fileName, valueContent);
         return configError;
       }
       // returns true the value is not existing
       //String getValue(const String& valueName) {
       //  String fileName = "/" + valueName + ".txt"; 
       //  String valueContent = readConfigFile(fileName);
       //  return valueContent;
       //}

       bool getValue(const String& valueName, String& valueContent) {
         String fileName = "/" + valueName + ".txt"; 
         valueContent = readConfigFile(fileName);
         return configError;
       }
       
       // returns true if the value could not be set
       bool addParam(const String& inName, const String& inDefault) {
	   String dummy;
	   if (getValue(inName, dummy)) {
	       printf("ConfigData()::addParam() not found creating default value file content\n");
	       configError = false;
	       return setValue(inName, inDefault);
	   }
	   return false;   
       }
       

    private:
       String wifiSid;
       String wifiPassword;
       String wifiDeviceId;
       bool configError;
       
};

#endif

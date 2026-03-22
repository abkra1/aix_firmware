
#ifndef H_CONFIG_PARAMS
#define H_CONFIG_PARAMS
//
//   configuration local http getter module
//     to be included and called by setup + loop
//


#include <map>
// attention config filesystem needs to be included before


class ParamSet
{
  public:
    
  ParamSet(String inName, String inDisplay, String inDefault, String inValue):
  name(inName),
  display(inDisplay),
  default_value(inDefault),
  value(inValue)
  {}
  
  ParamSet(String inName, String inDisplay, String inDefault):
  name(inName),
  display(inDisplay),
  default_value(inDefault),
  value("")
  {}
  
  ParamSet() {}
  
  void SetValue_unised(String inValue) { 
    value = inValue;
  }
  
  void SetDisplay(String inDisplay) { 
    display = inDisplay;
  }
    
  void SetDefaultValue(String inDefault) { 
    default_value = inDefault;
  }
    
  String GetValue_unused() { 
    return value;
  }
  
  String GetName() { 
    return name;
  }
  
  String GetDefaultValue() { 
    return default_value;
  }  
  
  String GetDisplay() { 
    return display;
  }  
  
    
  private: 
    String name;
    String display;
    String default_value;
    String value;

};


//
// real access class
//

class ConfigParams
{
  public:

  ConfigParams() {
     printf("ConfigParams: v 2.0\n");
     fileData = new FileSystemData();
  }

  ~ConfigParams() {
     delete fileData;
  }

  
  bool AddParam(const String& inName, const String& inDisplay, const String& inDefault) {
     // we assime there is no param so far .... or better the last one wins
     params[inName] = ParamSet(inName, inDisplay, inDefault);
     return fileData->addParam(inName, inDefault);
  }
  
  bool SetValue(const String& inName, const String& inValue) {
     ParamSet set = FindByName(inName);
     if (set.GetName() != "") {
        // set.SetValue(inValue); 
        params[inName]=set; 
        return fileData->setValue(inName, inValue);
     }
     return true;
  }
  

  // true on err
  bool GetValue(const String& inName, String& outValue) {
     ParamSet set = FindByName(inName);
     if (set.GetName() != "") {
        return fileData->getValue(inName, outValue); 
     }
     return true;
  }
    
  bool GetDefault(const String& inName, String& outValue) {
     ParamSet set = FindByName(inName);
     if (set.GetName() != "") {
        outValue = set.GetDefaultValue(); 
        return false; 
     }
     return true;
  }
  
  bool GetDisplay(const String& inName, String& outValue) {
     ParamSet set = FindByName(inName);
     if (set.GetName() != "") {
        outValue = set.GetDisplay(); 
        return false; 
     }
     return true;
  }
  
  // deliberatly return a copy here
  String GetNextParamValue(String inName) {  
      // first call    
      if (inName == "") {
          auto paramset = params.begin();
	  if (paramset != params.end()) {
	      return paramset->second.GetName();
	  }
	  return String();
      }
      // subsequent calls
      auto paramset = params.find(inName);
      if (paramset == params.end()) {
          return String();
      }
      paramset++;
      if (paramset == params.end()) {
          return String();
      }
      return paramset->second.GetName();      
  }



  private:
  
    ParamSet FindByName(const String& name) {
        auto paramset = params.find(name);
        if (paramset != params.end()) {
            return paramset->second;
        }
        return ParamSet();
    }
  
    std::map<String,ParamSet> params;
    FileSystemData* fileData;

};






#endif

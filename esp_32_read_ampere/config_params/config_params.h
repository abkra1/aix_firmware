#ifndef H_CONFIG_PARAMS
#define H_CONFIG_PARAMS
//
//   configuration params module
//     to be included and called by setup + loop
//


class ParamSet
{
  public:
    String name;
    String display;
    String default;
    String value;
    
  ParamSet(String inName, String inDisplay, String inDefault, String inValue):
  name(inName),
  dsplay(inDisplay),
  default(inDefault),
  value(inValue),
  valid(true)
  {}
  
  ParamSet():
  valid(false) {
  }
  
  SetValue(String inValue) { 
    value = inValue;
  }
  
  SetDisplay(String inDisplay) { 
    display = inDisplay;
  }
    
  SetDefault(String inDefault) { 
    default = inDefaut;
  }
    
  String GetValue() { 
    return value;
  }
  
  String GetName() { 
    return name;
  }
  
  String GetDefault() { 
    return default;
  }  
  
  String GetDisplay() { 
    return display;
  }  
  
  bool IsValid() {
      return valid;
  }
    
  private: 
      String inName;
      String inDisplay;
      String inDefault;
      String inValue;
      bool valid;

}

class Params
{

  Params() {}
  
  NewParam(String inName, String inDisplay, String inDefault, String inValue) {
     set = FindByName(InName);
     if set.IsValid() {
         set.SetValue(inValue);
         set.SetDisplay(inDisplay);
         set.SetDefault(inDefault);
     } else {
       set = ParamSet(inName, inDisplay, inDefault, inValue);
     }
     params[name] = set
  }
  
  SetValue(String inName, String inValue) {
     set = FindByName(InName);
     if set.IsValid() {
         set.SetValue(inValue);
     }
     params[name] = set
  }
  

  String GetValue(String inName) {
     set = FindByName(InName);
     if set.IsValid() {
         return set.GetValue(inValue);
     }
     return String();  
  }



  private:
  
    ParamSet FindByName(const String& name) {
        auto paramset = params.find(name);
        if (paramset != paramset.end()) {
            return *paramset;
        }
        return ParamSet()
    }
  
    map<string,ParamSet> params;


}


#endif

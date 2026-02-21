
#ifndef H_CONFIG_PARAMS
#define H_CONFIG_PARAMS
//
//   configuration local http getter module
//     to be included and called by setup + loop
//


class ParamSet
{
  public:
    String name;
    String display;
    String default_value;
    String value;
    
  ParamSet(String inName, String inDisplay, String inDefault, String inValue):
  name(inName),
  dsplay(inDisplay),
  default_value(inDefault),
  value(inValue)
  {}
  
  ParamSet& SetValue(String inValue) { 
    value = inValue;
  }
  
  void SetDisplay(String inDisplay) { 
    display = inDisplay;
  }
    
  void SetDefaultValue(String inDefault) { 
    default_value = inDefaut;
  }
    
  void String GetValue() { 
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


#
#  
#

class Params
{

  Params() {}
  
  AddParam(const String& inName, const String& inDisplay, const String& inDefault) {
     # we assime there is no param so far .... or better the last one wins
     params[name] = ParamSet(inName, inDisplay, inDefault, "")
     # we should read the config here if possible ... TODO
     
  }
  
  void SetValue(String inName, String inValue) {
     set = params.FindByName(inName)
     if (set.GetName() != "") {
        set.SetValue(inValue); 
        params[inName]=set; 
     }
  }
  

  String GetValue(String inName,) {
     set = params.FindByName(inName)
     if (set.GetName() != "") {
        return set.GetValue(); 
     }
     return "";
  }
  
  String GetDefault(String inName) {
     set = params.FindByName(inName)
     if (set.GetName() != "") {
        return set.GetDefaultValue(); 
     }
     return "";
  }
  
  String GetDisplay(String inName) {
     set = params.FindByName(inName)
     if (set.GetName() != "") {
        return set.GetDisplay(); 
     }
     return "";
  }

  // deliberatly return a copy here
  ParamSet GetNextParamValue(String inName) {
      if (paramset.begin == paramset.end()) {
          return ParamSet();
      }
      if name == "" {
          return paramset.begin();
      }
      # iterate, change this to something indexed / sorted
      auto paramset = params.begin();
      while (paramset.begin != paramset.end()) {
          # we deliberatly never check .begin()
          paramset++;
          if (paramset.begin != paramset.end()) && (*paramset.first == inName) {
	     return *paramset;
	  }
	  
      }
      return ParamSet();
      
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


};






#endif

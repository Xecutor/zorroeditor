#ifndef __GLIDER_UI_UICONIG_HPP__
#define __GLIDER_UI_UICONIG_HPP__
#include "Font.hpp"
#include <kst/Throw.hpp>
#include <map>

namespace glider{
namespace ui{

class UIConfig{
public:
  void init();
  void shutdown();
  FontRef getButtonFont()
  {
    return buttonFont;
  }
  FontRef getLabelFont()
  {
    return labelFont;
  }

  FontRef getWindowTitleFont()
  {
    return windowTitleFont;
  }

  FontRef getEditLineFont()
  {
    return editLineFont;
  }

  Color getClearColor()
  {
    return clearColor;
  }

  void setClearColor(Color clr);


  Color getEditLineFontColor()
  {
    return editLineFontColor;
  }

  float getConst(const std::string& name)
  {
    ConstsMap::iterator it=constsMap.find(name);
    if(it==constsMap.end())
    {
      KSTHROW("Unknown constant %{}",name);
    }
    return it->second;
  }

  void setConst(const std::string& name,float value)
  {
    constsMap.insert(ConstsMap::value_type(name,value));
  }

  Color getColor(const std::string& name)
  {
    ColorsMap::iterator it=colorsMap.find(name);
    if(it==colorsMap.end())
    {
      KSTHROW("Unknown color %{}",name);
    }
    return it->second;
  }

  void setColor(const std::string& name,Color value)
  {
    colorsMap.insert(ColorsMap::value_type(name,value));
  }

protected:
  FontRef buttonFont;
  FontRef labelFont;
  FontRef windowTitleFont;
  FontRef editLineFont;

  typedef std::map<std::string,float> ConstsMap;
  ConstsMap constsMap;

  typedef std::map<std::string,Color> ColorsMap;
  ColorsMap colorsMap;

  Color clearColor;
  Color editLineFontColor;
};

extern UIConfig uiConfig;

}
}

#endif

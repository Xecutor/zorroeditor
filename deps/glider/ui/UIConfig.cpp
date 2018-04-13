#include "UIConfig.hpp"
#include "ResourcesManager.hpp"
#include "Engine.hpp"
#include "UIRoot.hpp"
#include "kst/File.hpp"

namespace glider{
namespace ui{

UIConfig uiConfig;

void UIConfig::init()
{
  clearColor=Color(0.1f,0.1f,0.1f,1.0f);
  UIRoot::init();
  /*const char* fntPath="../data/FSEX300.ttf";
  if(!kst::File::Exists(fntPath))
  {
    fntPath="data/cour.ttf";
  }*/
  //const char* fntPath="data/cour.ttf";
  //const char* fntPath="../data/FreeMono.ttf";
  //const char* fntPath="../data/Vera.ttf";
  //int sz=16;

  buttonFont=engine.getDefaultFont();//manager.getFont(fntPath,sz);
  labelFont=engine.getDefaultFont();//manager.getFont(fntPath,sz);
  windowTitleFont=engine.getDefaultFont();//manager.getFont(fntPath,sz);
  editLineFont=engine.getDefaultFont();//manager.getFont(fntPath,sz);
  root->setSize(Pos((float)engine.getWidth(),(float)engine.getHeight()));
  editLineFontColor=Color(1.0f,1.0f,1.0f,1.0f);

  setConst("scrollBarWidth",10);
  setConst("scrollBarMinSize",16);

  setConst("splitterWidth", 10);
  setConst("splitterMinWidth", 200);
  setColor("splitterColor", Color::red);

  setColor("scrollBarBgColor",Color::gray);
  setColor("scrollBarScrollerColor",Color(0.8f,1.0f,0.8f));
  setColor("listBoxBgColor",Color::gray);
  setColor("listBoxSelColor",Color::white);

  setColor("labelTextColor",Color::white);
  setColor("labelShadowColor",Color::black);
}

void UIConfig::shutdown()
{
  buttonFont=0;
  labelFont=0;
  windowTitleFont=0;
  editLineFont=0;
  UIRoot::shutdown();
  engine.setDefaultFont(0);
}

void UIConfig::setClearColor(Color clr)
{
  clearColor=clr;
  engine.setClearColor(clearColor);
}

}
}

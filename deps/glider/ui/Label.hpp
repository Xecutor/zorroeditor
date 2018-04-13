#ifndef __GLIDER_UI_LABEL_HPP__
#define __GLIDER_UI_LABEL_HPP__

#include "UIObject.hpp"
#include "Font.hpp"
#include "Text.hpp"

namespace glider{
namespace ui{

class Label:public UIObject{
public:
  Label(const char* argCaption=0,const char* argName=0);

  void assignFont(FontRef argFont)
  {
    caption.assignFont(argFont);
  }
  void setColor(Color clr)
  {
    caption.setColor(clr);
  }
  void setShadowColor(Color clr)
  {
    captionShadow.setColor(clr);
  }
  void setCaption(const char* argCaption,int argMaxWidth=0,bool argRawText=false,bool agrWordWrap=true)
  {
    captionText=argCaption;
    caption.setText(argCaption,argRawText,argMaxWidth,agrWordWrap);
    if(drawShadow)
    {
      captionShadow.setText(argCaption);
    }
    size=Pos((float)caption.getWidth(),(float)caption.getHeight());
  }
  const std::string& getCaption()const
  {
    return captionText;
  }
  void setShadowOffset(Pos off)
  {
    captionShadow.setPosition(off);
  }
  void enableShadow()
  {
    drawShadow=true;
    captionShadow.setText(captionText.c_str());
  }
  void disableShadow()
  {
    drawShadow=false;
  }
  void draw();
protected:
  std::string captionText;
  Text caption;
  Text captionShadow;
  bool drawShadow;
};

typedef ReferenceTmpl<Label> LabelRef;


}
}

#endif

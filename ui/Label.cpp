#include "Label.hpp"
#include "GLState.hpp"
#include "UIConfig.hpp"

namespace glider{
namespace ui{

Label::Label(const char* argCaption, const char* argName)
{
  drawShadow=false;
  caption.assignFont(uiConfig.getLabelFont());
  caption.setColor(uiConfig.getColor("labelTextColor"));
  captionShadow.assignFont(uiConfig.getLabelFont());
  captionShadow.setPosition(Pos(1,1));
  captionShadow.setColor(uiConfig.getColor("labelShadowColor"));
  if(argCaption)
  {
    setCaption(argCaption);
  }
  if(argName)
  {
    setName(argName);
  }
}

void Label::draw()
{
  RelOffsetGuard rog(pos);
  if(drawShadow)
  {
    captionShadow.draw();
  }
  caption.draw();
}

}
}

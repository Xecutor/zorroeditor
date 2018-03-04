#include "ScrollBar.hpp"
#include "UIRoot.hpp"
#include "UIConfig.hpp"
#include "GLState.hpp"

namespace glider{
namespace ui{

ScrollBar::ScrollBar(float argTotal,float argFrame,float argValue):total(argTotal),frame(argFrame),value(argValue),dragValue(0),dragging(false)
{
  bgRect.setColor(uiConfig.getColor("scrollBarBgColor"));
  scrollerRect.setColor(uiConfig.getColor("scrollBarScrollerColor"));
}


void ScrollBar::onAssignParent()
{
  updatePosFromParent();
}

void ScrollBar::draw()
{
  RelOffsetGuard rg(pos);
  bgRect.draw();
  scrollerRect.draw();
}


void ScrollBar::onMouseButtonDown(const MouseEvent& evt)
{
  static kst::Logger* log=kst::Logger::getLogger("sb.mousedown");
  root->lockMouse(this);
  Pos mpos=Pos((float)evt.x,(float)evt.y)-getAbsPos();
  LOGDEBUG(log,"mpos=%{},pos=%{},size=%{}",mpos,scrollerRect.getPosition(),scrollerRect.getSize());

  if(scrollerRect.getRect().isInside(mpos))
  {
    dragging=true;
    dragPos=Pos(evt);
    dragValue=value;
  }else
  {
    setValue(total*mpos.y/getSize().y);
  }
}
void ScrollBar::onMouseButtonUp(const MouseEvent& evt)
{
  dragging=false;
  root->unlockMouse();
}

void ScrollBar::onParentResizeEnd()
{
  onAssignParent();
}

void ScrollBar::updatePosFromParent()
{
  setSize(Pos(uiConfig.getConst("scrollBarWidth"),parent->getSize().y));
  bgRect.setSize(size);
  //bgRect.setPosition(Pos(parent->getSize().x-bgRect.getSize().x,0));
  setPos(Pos(parent->getSize().x-size.x,0));
  if(frame)
  {
    float h=size.y*frame/total;
    float mh=uiConfig.getConst("scrollBarMinSize");
    if(h<mh)h=mh;
    scrollerRect.setSize(Pos(size.x,h));
  }else
  {
    scrollerRect.setSize(Pos(size.x,size.x));
  }
  scrollerRect.setPosition(Pos(0,(size.y-scrollerRect.getSize().y)*value/(total-frame)));
}

void ScrollBar::onMouseMove(const MouseEvent& evt)
{
  if(dragging)
  {
    float m=size.y-size.y*frame/total;
    float v=dragValue+(Pos(evt)-dragPos).y*(total-frame)/m;
    setValue(v);
  }
}


}
}

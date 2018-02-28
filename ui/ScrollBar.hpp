#ifndef __GLIDER_UI_SCROLLBAR_HPP__
#define __GLIDER_UI_SCROLLBAR_HPP__

#include "UIObject.hpp"
#include "Rectangle.hpp"

namespace glider{
namespace ui{

class ScrollBar:public UIObject{
public:
  ScrollBar(float argTotal,float argFrame=0,float argValue=0);
  void draw();
  void setValueChangeHandler(UICallBack argCb)
  {
    cb=argCb;
  }
  void setValue(float argValue)
  {
    value=argValue;
    if(value<0)value=0;
    if(value>total-frame)value=total-frame;
    scrollerRect.setPosition(Pos(0,(getSize().y-scrollerRect.getSize().y)*value/(total-frame)));
    if(cb)
    {
      cb(UIEvent(this,0));
    }
  }
  void updatePosFromParent();
  float getValue()const
  {
    return value;
  }
  void setTotal(float argTotal)
  {
    total=argTotal;
    updatePosFromParent();
  }
  float getTotal()const
  {
    return total;
  }
  void setFrame(float argFrame)
  {
    frame=argFrame;
    updatePosFromParent();
  }
  float getFrame()const
  {
    return frame;
  }
  void setColors(Color argBgColor,Color argScrollerColor)
  {
    bgRect.setColor(argBgColor);
    scrollerRect.setColor(argScrollerColor);
  }
protected:

  void onAssignParent();
  void onParentResizeEnd();
  void onMouseButtonDown(const MouseEvent& evt);
  void onMouseButtonUp(const MouseEvent& evt);
  void onMouseMove(const MouseEvent& evt);

  float total,frame,value,dragValue;
  Pos dragPos;
  Rectangle bgRect,scrollerRect;
  UICallBack cb;
  bool dragging;
};

typedef ReferenceTmpl<ScrollBar> ScrollBarRef;


}
}

#endif

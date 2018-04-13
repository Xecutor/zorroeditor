#include "Button.hpp"
#include "UIConfig.hpp"
#include "GLState.hpp"
#include "UIRoot.hpp"

namespace glider{
namespace ui{

Button::Button(const char* argCaption,const char* argName,UICallBack cb)
{
  ani.btn=this;
  aniCnt=aniMaxCnt=200000;
  newClr=Color(1,1,1,1);
  caption.assignFont(uiConfig.getButtonFont());
  captionShade.assignFont(uiConfig.getButtonFont());
  caption.setColor(Color(0,0,0,1));
  setNormalState();


  int w=80;
  if(argCaption)
  {
    caption.setText(argCaption);
    w=caption.getWidth()+8;
  }
  size=Pos((float)w,(float)(uiConfig.getButtonFont()->getHeight()+6));
  tmpRect.setSize(size);
  capSciss.setRect(Rect(Pos(),size));
  buttonDown=false;
  mouseOver=false;
  if(argCaption)
  {
    setCaption(argCaption);
  }
  if(argName)
  {
    setName(argName);
  }
  if(cb)
  {
    setEventHandler(betOnClick,cb);
  }
}

Button::~Button()
{
  root->removeAnimation(&ani);
}

void Button::setCaption(const char* argCaption)
{
  caption.setText(argCaption);
  captionShade.setColor(Color(0.7f,0.7f,0.7f));
  captionShade.setText(argCaption);
  Pos capPos=size;
  capPos-=Pos((float)caption.getWidth(),(float)caption.getHeight());
  capPos/=2;
  capPos.x=floor(capPos.x);
  capPos.y=floor(capPos.y);
  caption.setPosition(capPos);
  captionShade.setPosition(capPos+Pos(1,1));
}

void Button::draw()
{
  RelOffsetGuard rog(pos);
  tmpRect.draw();
  capSciss.draw(&captionShade);
  capSciss.draw(&caption);
  //caption.draw();
}

void Button::onMouseEnter(const MouseEvent& me)
{
  if(buttonDown)
  {
    setDownState();
  }else
  {
    setHoverState();
  }
  mouseOver=true;
  UIObject::onMouseEnter(me);
}

void Button::onMouseLeave(const MouseEvent& me)
{
  mouseOver=false;
  setNormalState();
  UIObject::onMouseLeave(me);
}

void Button::onMouseMove(const MouseEvent& me)
{
  if(isInside(Pos((float)me.x,(float)me.y)))
  {
    mouseOver=true;
    if(buttonDown)
    {
      setDownState();
    }else
    {
      setHoverState();
    }
  }else
  {
    mouseOver=false;
    setNormalState();
  }
}


void Button::onMouseButtonDown(const MouseEvent& me)
{
  if(me.eventButton==1)
  {
    setDownState();
    root->lockMouse(this);
    buttonDown=true;
  }
  UIObject::onMouseButtonDown(me);
}

void Button::onMouseButtonUp(const MouseEvent& me)
{
  if(me.eventButton==1 && buttonDown)
  {
    if(mouseOver)
    {
      setHoverState();
    }else
    {
      setNormalState();
    }
    root->unlockMouse();
    if(isInside(Pos((float)me.x,(float)me.y)))
    {
      onMouseClick(me);
    }
    buttonDown=false;
    UIObject::onMouseButtonUp(me);
  }
}

void Button::onMouseClick(const MouseEvent& me)
{
  if(me.eventButton==1)
  {
    if(btnCb[betOnClick])
    {
      btnCb[betOnClick](UIEvent(uietMouseClick,me));
    }
  }
  UIObject::onMouseClick(me);
}

Color Button::getCurrentClr()
{
  if(aniCnt>=aniMaxCnt)
  {
    return newClr;
  }
  Color rv=oldClr;
  rv.r+=(newClr.r-oldClr.r)*aniCnt/aniMaxCnt;
  rv.g+=(newClr.g-oldClr.g)*aniCnt/aniMaxCnt;
  rv.b+=(newClr.b-oldClr.b)*aniCnt/aniMaxCnt;
  rv.a+=(newClr.a-oldClr.a)*aniCnt/aniMaxCnt;
  return rv;
}


void Button::changeClr(Color argNewClr)
{
  oldClr=getCurrentClr();
  newClr=argNewClr;
  startAnimation();
}

void Button::setHoverState()
{
  changeClr(Color(0.8f,0.7f,0.8f));
}

void Button::setDownState()
{
  //changeClr(Color(0.3,0.2,0.2));
  newClr=Color(0.7f,0.8f,0.8f);
  updateAnimation(0);
}

void Button::setNormalState()
{
  changeClr(Color(0.8f,0.8f,0.8f));
}

void Button::startAnimation()
{
  if(aniCnt>=aniMaxCnt)
  {
    root->addAnimation(&ani);
  }
  aniCnt=0;
}

bool Button::updateAnimation(int mcs)
{
  aniCnt+=mcs;
  if(aniCnt>aniMaxCnt)
  {
    aniCnt=aniMaxCnt;
  }
  tmpRect.setColor(getCurrentClr());
  return aniCnt<aniMaxCnt;
}


}
}

#include "UIObject.hpp"
#include <stdio.h>
#include "UIRoot.hpp"

namespace glider{
namespace ui{

static int UIObjectUidSeq=0;

UIObject::UIObject(const char* argName):parent(0),intTag(0)
{
  uid=UIObjectUidSeq++;
  if(!argName)
  {
    char buf[32];
    sprintf(buf,"object%d",uid);
    name=buf;
  }else
  {
    name=argName;
  }
  focused=false;
  tabStop=false;
  visible=true;
  tabOrder=0;
  log=kst::Logger::getLogger("obj");
}

void UIObject::onMouseEnter(const MouseEvent& me)
{
  const UIEventType et=uietMouseEnter;
  if(cbArray[et])
  {
    UIEvent evt(et,me);
    cbArray[et](evt);
  }
}

void UIObject::onMouseLeave(const MouseEvent& me)
{
  const UIEventType et=uietMouseLeave;
  if(cbArray[et])
  {
    UIEvent evt(et,me);
    cbArray[et](evt);
  }
}

void UIObject::onMouseButtonDown(const MouseEvent& me)
{
  const UIEventType et=uietMouseButtonDown;
  if(cbArray[et])
  {
    UIEvent evt(et,me);
    cbArray[et](evt);
  }
}

void UIObject::onMouseButtonUp(const MouseEvent& me)
{
  const UIEventType et=uietMouseButtonUp;
  if(cbArray[et])
  {
    UIEvent evt(et,me);
    cbArray[et](evt);
  }
}

void UIObject::onMouseClick(const MouseEvent& me)
{
  const UIEventType et=uietMouseClick;
  if(cbArray[et])
  {
    UIEvent evt(et,me);
    cbArray[et](evt);
  }
}

void UIObject::onMouseMove(const MouseEvent& me)
{
  const UIEventType et=uietMouseMove;
  if(cbArray[et])
  {
    UIEvent evt(et,me);
    cbArray[et](evt);
  }
}

void UIObject::onMouseScroll(const MouseEvent& me)
{
  const UIEventType et=uietMouseScroll;
  if(cbArray[et])
  {
    UIEvent evt(et,me);
    cbArray[et](evt);
  }
}


void UIObject::onKeyDown(const KeyboardEvent& ke)
{
  const UIEventType et=uietKeyDown;
  if(cbArray[et])
  {
    UIEvent evt(et,ke);
    cbArray[et](evt);
  }
}

void UIObject::onKeyUp(const KeyboardEvent& ke)
{
  const UIEventType et=uietKeyUp;
  if(cbArray[et])
  {
    UIEvent evt(et,ke);
    cbArray[et](evt);
  }
}

void UIObject::onKeyPress(const KeyboardEvent& ke)
{
  const UIEventType et=uietKeyPress;
  if(cbArray[et])
  {
    UIEvent evt(et,ke);
    cbArray[et](evt);
  }
}


void UIObject::setFocus()
{
  if(!focused)
  {
    focused=true;
    if(isTabStop())root->setKeyboardFocus(this);
    onFocusGain();
  }
}

void UIObject::removeFocus()
{
  if(focused)
  {
    focused=false;
    root->setKeyboardFocus(0);
    onFocusLost();
    if(parent)parent->removeFocus();
  }
}

}
}

#ifndef __GLIDER_UI_UIOBJECT_HPP__
#define __GLIDER_UI_UIOBJECT_HPP__

#include "Drawable.hpp"
#include "Utility.hpp"
#include <string>
#include "EventCallback.hpp"
#include "EventHandler.hpp"
#include <kst/Logger.hpp>

namespace glider{
namespace ui{

enum HAlignment{
  haNone,haLeft,haCenter,haRight
};

enum VAlignment{
  vaNone,vaLeft,vaCenter,vaRight
};

class UIRoot;
class UIContainer;

class UIObject;
typedef ReferenceTmpl<UIObject> UIObjectRef;

enum UIEventType{
  uietMouseMove,uietMouseButtonDown,
  uietMouseButtonUp,uietMouseClick,
  uietMouseEnter,uietMouseLeave,uietMouseScroll,
  uietKeyDown,uietKeyUp,
  uietKeyPress,uietOther,
  uietCount
};

struct UIEvent{
  UIEvent(UIEventType argEt,const MouseEvent& argMe):et(argEt),etEx(0),me(argMe){}
  UIEvent(UIEventType argEt,const KeyboardEvent& argKe):et(argEt),etEx(0),ke(argKe){}
  UIEvent(UIObject* obj,uint8_t argEtEx):et(uietOther),etEx(argEtEx),sender(obj){}
  UIEventType et;
  uint8_t etEx;
  union{
    MouseEvent me;
    KeyboardEvent ke;
    UIObject* sender;
  };
};

typedef EventCallback<UIEvent> UICallBack;

#define MKUICALLBACK(name) MKCALLBACK(name)

class UIObject:public Drawable{
public:
  UIObject(const char* argName=0);

  void assignParent(UIObject* argParent)
  {
    parent=argParent;
    onAssignParent();
  }

  UIObject* getParent()const
  {
    return parent;
  }

  void setEventHandler(UIEventType et,UICallBack cb)
  {
    cbArray[et]=cb;
  }

  Rect getAbsRect()const
  {
    return Rect(getAbsPos(),size);
  }

  Rect getRect()const
  {
    return Rect(pos,size);
  }

  const Pos& getPos()const
  {
    return pos;
  }

  void setPos(const Pos& argPos)
  {
    pos=argPos;
    onObjectMove();
  }

  const Pos& getSize()const
  {
    return size;
  }

  void setSize(const Pos& argSize)
  {
    size=argSize;
    onObjectResize();
    onObjectResizeEnd();
  }

  int getUid()const
  {
    return uid;
  }

  const std::string getName()const
  {
    return name;
  }

  void setName(const std::string& argName)
  {
    name=argName;
  }

  std::string getFullName()const
  {
    std::string rv;
    if(parent)
    {
      rv=parent->getFullName();
      rv+='.';
    }
    rv+=name;
    return rv;
  }

  Pos getAbsPos()const
  {
    Pos rv;
    if(parent)
    {
      rv=parent->getAbsPos();
    }
    rv+=pos;
    return rv;
  }

  bool isInside(const Pos& argPos)const
  {
    return Rect(getAbsPos(),size).isInside(argPos);
  }

  virtual bool isContainer()const
  {
    return false;
  }

  bool isTabStop()const
  {
    return tabStop;
  }

  int getTabOrder()const
  {
    return tabOrder;
  }

  bool isFocused()const
  {
    return focused;
  }

  void setFocus();
  void removeFocus();

  void setVisible(bool argValue)
  {
    visible=argValue;
    if(!visible && focused)
    {
      removeFocus();
    }
  }

  bool isVisible()const
  {
    return visible;
  }

  void setIntTag(int newValue)
  {
    intTag=newValue;
  }

  int getIntTag()const
  {
    return intTag;
  }

  void attachPtr(void* argPtr)
  {
    attachedPtr=argPtr;
  }

  void* getAttachedPtr()const
  {
    return attachedPtr;
  }

protected:
  Pos pos;
  Pos size;
  UIObject* parent;
  void* attachedPtr = nullptr;
  int uid;
  int tabOrder;
  int intTag;
  bool focused;
  bool tabStop;
  bool visible;


  UICallBack cbArray[uietCount];

  std::string name;

  kst::Logger* log;

  virtual void onAssignParent(){}
  virtual void onObjectMove(){}
  virtual void onObjectResize(){}
  virtual void onObjectResizeEnd(){}
  virtual void onParentResize(){}
  virtual void onParentResizeEnd(){}
  virtual void onFocusGain(){}
  virtual void onFocusLost(){}

  friend class UIRoot;
  friend class UIContainer;

  virtual void onMouseEnter(const MouseEvent& me);
  virtual void onMouseLeave(const MouseEvent& me);
  virtual void onMouseButtonDown(const MouseEvent& me);
  virtual void onMouseButtonUp(const MouseEvent& me);
  virtual void onMouseClick(const MouseEvent& me);
  virtual void onMouseMove(const MouseEvent& me);
  virtual void onMouseScroll(const MouseEvent& me);
  virtual void onKeyDown(const KeyboardEvent& ke);
  virtual void onKeyUp(const KeyboardEvent& ke);
  virtual void onKeyPress(const KeyboardEvent& ke);


};

typedef std::list<UIObjectRef> UIObjectsList;

}
}

#endif

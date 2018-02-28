#ifndef __GLIDER_UI_UIROOT_HPP__
#define __GLIDER_UI_UIROOT_HPP__

#include "UIContainer.hpp"
#include "UIAnimation.hpp"

namespace glider{
namespace ui{

typedef EventCallback0 UserCallback;

class UIRoot:public UIContainer,public EventHandler{
public:
  UIRoot();
  void onActiveChange(bool active);
  void onMouseEvent(MouseEvent& argEvent);
  void onKeyboardEvent(KeyboardEvent& argEvent);
  static void postMouseEvent(const MouseEvent& argEvent,UIObject* obj);
  static void postKeyboardEvent(const KeyboardEvent& argEvent,UIObject* obj);
  void onResize();
  void onQuit();
  void onFrameUpdate(int mcsec);
  void onUserEvent(void* data1,void* data2);
  void lockMouse(UIObject* obj)
  {
    mouseLock.push_back(obj);
  }
  void unlockMouse()
  {
    if(mouseLock.empty())
    {
      KSTHROW("unbalanced mouse unlock!");
    }
    mouseLock.pop_back();
  }
  bool isMouseLocked()const
  {
    return !mouseLock.empty();
  }
  void setKeyboardFocus(UIObject* obj)
  {
    if(keyFocus.get() && keyFocus.get()!=obj)
    {
      keyFocus->removeFocus();
    }
    keyFocus=obj;
  }
  static void init();
  static void shutdown();
  void addAnimation(UIAnimation* argAni);
  void removeAnimation(UIAnimation* argAni);

  void replaceLoop(UIObject& obj);
  void modalLoop(UIObject& obj);

  void exitModal();

protected:
  UIObjectRef keyFocus;
  UIObjectsList mouseLock;
  typedef std::list<UIAnimation*> AniList;
  AniList animations;
  Pos mouseButtonPressPos;
};

extern UIRoot* root;

}
}

#endif

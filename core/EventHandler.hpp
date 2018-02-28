#ifndef __GLIDER_EVENTHANDLER_HPP__
#define __GLIDER_EVENTHANDLER_HPP__

#include "Keyboard.hpp"
#include "Utility.hpp"

namespace glider{

enum MouseEventType{
  metMove,
  metButtonPress,
  metButtonRelease,
  metScroll
};

struct MouseEvent{
  char deviceIndex;
  MouseEventType eventType;
  int buttonsState;
  int eventButton;
  int x,y;
  int xRel,yRel;
  Posi<int> getPos()const
  {
    return {x,y};
  }
};

enum KeyboardEventType{
  ketPress,
  ketRelease,
  ketInput
};

struct KeyboardEvent{
  KeyboardEventType eventType;
  keyboard::KeyModifier keyMod;
  keyboard::KeySymbol keySym;
  ushort unicodeSymbol;
  bool repeat;
};

class EventHandler{
public:
  virtual ~EventHandler(){}
  virtual void onActiveChange(bool active){}
  virtual void onMouseEvent(MouseEvent& argEvent){}
  virtual void onKeyboardEvent(KeyboardEvent& argEvent){}
  virtual void onResize(){};
  virtual void onQuit()=0;
  virtual void onFrameUpdate(int mcsec)=0;
  virtual void onUserEvent(void* data1,void* data2){}
};

}

#endif

#ifndef __GLIDER_UI_EDITLINE_HPP__
#define __GLIDER_UI_EDITLINE_HPP__
#include "UIObject.hpp"
#include <string>
#include "Line.hpp"
#include "Text.hpp"
#include "Rectangle.hpp"
#include "UIAnimation.hpp"

namespace glider{
namespace ui{

enum EditInputEventType{
  eiOnAccept,
  eiOnCancel,
  eiOnModify,
  eiCount
};

class EditInput:public UIObject{
public:
  EditInput(const char* name=0);
  void draw();
  void setValue(const std::string& argValue);
  const std::string& getValue()const
  {
    return value;
  }
  int getIntValue()const
  {
    int rv=0;
    sscanf(value.c_str(),"%d",&rv);
    return rv;
  }
  int getCurPos()const
  {
    return curPos;
  }
  void setCurPos(int argCurPos,bool extendSelection=false);
  void insertText(const char* txt);
  void resetSelection();

  using UIObject::setEventHandler;
  EditInput& setEventHandler(EditInputEventType et,UICallBack cb)
  {
    eiCb[et]=cb;
    return *this;
  }

protected:
  std::string value;
  Text text;
  Line cursor;
  Rectangle rect;
  Rectangle selection;
  int hShift;
  bool selecting;

  UICallBack eiCb[eiCount];

  void deleteSymbol(int dir);

  class CursorBlinkAnimation:public UIAnimation{
  public:
    CursorBlinkAnimation():active(false){}
    EditInput* ei = nullptr;
    bool active;
    void onStart()
    {
      active=true;
    }
    void onEnd()
    {
      active=false;
    }
    bool update(int mcsec)
    {
      return active && ei->cursorBlinkAnimation(mcsec);
    }
    bool deleteOnFinish()
    {
      return false;
    }
  };
  CursorBlinkAnimation curBlinkAni;
  int lastCurBlink;
  int curPos;
  int selStart,selEnd;
  bool haveSelection()const
  {
    return selStart!=-1 && selStart!=selEnd;
  }
  bool isCurVisible;
  bool cursorBlinkAnimation(int mcsec);
  int mouseXToCurPos(int x);
  struct ScrollAnimation:public UIAnimation{
  public:
    ScrollAnimation():ei(0),isCancelled(false)
    {
    }
    EditInput* ei;
    bool isCancelled;
    bool update(int mcsec)
    {
      if(!isCancelled)
      {
        ei->scroll(mcsec);
      }
      return !isCancelled;
    }
    bool deleteOnFinish()
    {
      return false;
    }
    void cancel()
    {
      isCancelled=true;
    }
  };
  ScrollAnimation scrollAni;
  int scrollTimer;
  int scrollDir;
  void scroll(int mcsec);

  void onFocusGain();
  void onFocusLost();
  void onKeyDown(const KeyboardEvent& ke);
  void onKeyPress(const KeyboardEvent& ke);
  void onMouseButtonDown(const MouseEvent& me);
  void onMouseButtonUp(const MouseEvent& me);
  void onMouseMove(const MouseEvent& me);

  void onObjectResize();

};

}
}

#endif

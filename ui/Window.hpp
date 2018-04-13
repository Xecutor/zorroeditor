#ifndef __GLIDER_UI_WINDOW_HPP__
#define __GLIDER_UI_WINDOW_HPP__

#include "UIContainer.hpp"
#include "Rectangle.hpp"
#include "Text.hpp"
#include "Label.hpp"

namespace glider{
namespace ui{

class Window:public UIContainer{
public:
  Window(const Pos& argPos=Pos(0,0),const Pos& argSize=Pos(500,300),const char* argTitle=0);
  void draw();
  void setTitle(const char* argTitle)
  {
    titleText->setCaption(argTitle);
  }
  void addObject(UIObject* obj)
  {
    client->addObject(obj);
  }
  void setLayout(LayoutRef argLayout)
  {
    client->setLayout(argLayout);
  }
  Layout& getLayout()
  {
    return client->getLayout();
  }
  int getTitleHeight()const
  {
    return (int)title->rect.getSize().y;
  }
  Pos getClientSize()const
  {
    return size-Pos(0.0f,(float)getTitleHeight());
  }
  void setClientSize(const Pos& argSize)
  {
    setSize(argSize+Pos(0.0f,(float)getTitleHeight()));
  }
  void setResizable(bool argValue);

  virtual UIObjectRef findByName(const std::string& argName)
  {
    return client->findByName(argName);
  }
  void setFocus()
  {
    client->setFocus();
  }
protected:
  class WindowTitle:public UIContainer{
  public:
    WindowTitle():dragging(false){}
    void draw();
    void onObjectResize();
    Rectangle rect;
    bool dragging;
    void onMouseButtonDown(const MouseEvent& me);
    void onMouseButtonUp(const MouseEvent& me);
    void onMouseMove(const MouseEvent& me);
  };
  typedef ReferenceTmpl<WindowTitle> WindowTitleRef;
  class WindowClient:public UIContainer{
  public:
    void draw();
    void onObjectResize();
    Rectangle rect;
  };
  typedef ReferenceTmpl<WindowClient> WindowClientRef;
  WindowTitleRef title;
  WindowClientRef client;
  LabelRef titleText;
  RectangleRef frame;
  RectangleRef resizer;
  bool resizing;
  bool resizable;
  void onMouseButtonDown(const MouseEvent& me);
  void onMouseButtonUp(const MouseEvent& me);
  void onMouseMove(const MouseEvent& me);
  void onObjectResize();
  void onFocusGain();
  void onFocusLost();

};

}
}

#endif

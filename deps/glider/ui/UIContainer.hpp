#ifndef __GLIDER_UI_UICONTAINER_HPP__
#define __GLIDER_UI_UICONTAINER_HPP__

#include "UIObject.hpp"
#include <map>
#include "Scene.hpp"
#include "Layout.hpp"

namespace glider{
namespace ui{

class UIContainerEnumerator{
public:
  virtual ~UIContainerEnumerator(){}
  virtual bool nextItem(UIObject* obj)=0;
};

class UIContainer:public UIObject{
public:
  UIContainer(){}
  void draw();
  void addObject(UIObject* obj);
  void removeObject(UIObject* obj);
  void moveObjectToFront(UIObject* obj);
  void clear();

  virtual bool isContainer()const
  {
    return true;
  }

  void setLayout(LayoutRef argLayout)
  {
    layout=argLayout;
    //layout->fillObjects(this);
    layout->update(Pos(0,0),size);
  }

  Layout& getLayout()
  {
    return *layout.get();
  }

  void updateLayout()
  {
    if(layout.get())
    {
      layout->update(Pos(0,0), getSize());
    }
  }

  virtual UIObjectRef findByName(const std::string& argName);

  void moveFocusToNext();

  void enumerate(UIContainerEnumerator* enumerator);

  size_t getCount()const
  {
    return objLst.size();
  }

  const UIObjectsList& getObjList()const
  {
    return objLst;
  }

protected:

  UIObjectsList objLst;
  LayoutRef layout;

  typedef std::map<int,UIObjectsList::iterator> IdMap;
  IdMap idMap;
  UIObjectsList mouseEnterStack;
  typedef std::map<std::string,UIObjectRef> NameMap;
  NameMap nameMap;

  void endOfResize();

  virtual void onAddObject(){}
  virtual void onRemoveObject(UIObjectsList::iterator it){}
  virtual void onClear(){}

  void onMouseLeave(const MouseEvent& me);
  void onMouseMove(const MouseEvent& me);
  void onMouseButtonDown(const MouseEvent& me);
  void onMouseButtonUp(const MouseEvent& me);
  void onMouseClick(const MouseEvent& me);
  void onMouseScroll(const MouseEvent& me);

  void onObjectResize();
  void onObjectResizeEnd();
  void onFocusGain();
  void onFocusLost();

};

typedef ReferenceTmpl<UIContainer> UIContainerRef;

}
}

#endif

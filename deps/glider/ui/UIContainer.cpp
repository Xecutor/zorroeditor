#include "UIContainer.hpp"
#include "Scissors.hpp"
#include <stdio.h>
#include "GLState.hpp"
#include "UIRoot.hpp"

namespace glider{
namespace ui{


void UIContainer::draw()
{
  RelOffsetGuard rog(pos);
  ScissorsGuard sgrd(Rect(Pos(),size));
  for(UIObjectsList::iterator it=objLst.begin(),end=objLst.end();it!=end;++it)
  {
    if((*it)->isVisible())
    {
      (*it)->draw();
    }
  }
}

void UIContainer::enumerate(UIContainerEnumerator* enumerator)
{
  for(UIObjectsList::iterator it=objLst.begin(),end=objLst.end();it!=end;++it)
  {
    if(!enumerator->nextItem(it->get()))
    {
      break;
    }
  }
}


void UIContainer::addObject(UIObject* obj)
{
  nameMap.insert(NameMap::value_type(obj->getName(),obj));
  UIObjectsList::iterator id=objLst.insert(objLst.end(),obj);
  idMap.insert(IdMap::value_type(obj->getUid(),id));
  obj->assignParent(this);
  onAddObject();
}

void UIContainer::removeObject(UIObject* obj)
{
  if(obj->isFocused())
  {
    obj->removeFocus();
    root->setKeyboardFocus(0);
  }
  IdMap::iterator it=idMap.find(obj->getUid());
  if(it==idMap.end())
  {
    //!!TODO!! throw exception?
    return;
  }
  nameMap.erase(obj->getName());
  onRemoveObject(it->second);
  objLst.erase(it->second);
  idMap.erase(it);
  if(!mouseEnterStack.empty() && mouseEnterStack.back().get()==obj)
  {
    mouseEnterStack.pop_back();
  }
}

UIObjectRef UIContainer::findByName(const std::string& argName)
{
  std::string::size_type dotPos = argName.find('.');
  if(dotPos==std::string::npos) {
    NameMap::iterator it=nameMap.find(argName);
    if(it==nameMap.end())
    {
      return 0;
    }else
    {
      return it->second;
    }
  }
  std::string baseName=argName.substr(0, dotPos);
  NameMap::iterator it=nameMap.find(baseName);
  if(it==nameMap.end() || !it->second->isContainer())
  {
    return 0;
  }
  UIContainer* uic=it->second.as<UIContainer>();
  return uic->findByName(argName.substr(dotPos+1));
}


void UIContainer::moveObjectToFront(UIObject* obj)
{
  IdMap::iterator it=idMap.find(obj->getUid());
  if(it==idMap.end())
  {
    //!!TODO!! throw exception?
    return;
  }
  objLst.splice(objLst.end(),objLst,it->second);
}

void UIContainer::clear()
{
  onClear();
  objLst.clear();
  idMap.clear();
  nameMap.clear();
  mouseEnterStack.clear();
}

void UIContainer::onMouseLeave(const MouseEvent& me)
{
  for(UIObjectsList::iterator it=mouseEnterStack.begin(),end=mouseEnterStack.end();it!=end;++it)
  {
    (*it)->onMouseLeave(me);
  }
  mouseEnterStack.clear();
  UIObject::onMouseLeave(me);
}

void UIContainer::onMouseMove(const MouseEvent& me)
{
  Pos mPos((float)me.x,(float)me.y);
  while(!mouseEnterStack.empty() && !mouseEnterStack.back()->isInside(mPos))
  {
    mouseEnterStack.back()->onMouseLeave(me);
    mouseEnterStack.pop_back();
  }
  for(UIObjectsList::reverse_iterator it=objLst.rbegin(),end=objLst.rend();it!=end;++it)
  {
    if((*it)->isInside(mPos))
    {
      if((mouseEnterStack.empty() || mouseEnterStack.back()!=*it))
      {
        (*it)->onMouseEnter(me);
        if((*it)->isContainer())
        {
          while(!mouseEnterStack.empty())// && !mouseEnterStack.back()->isContainer())
          {
            mouseEnterStack.back()->onMouseLeave(me);
            mouseEnterStack.pop_back();
          }
        }
        mouseEnterStack.push_back(*it);
      }
      break;
    }
  }
  if(!mouseEnterStack.empty())
  {
    mouseEnterStack.back()->onMouseMove(me);
  }else
  {
    UIObject::onMouseMove(me);
  }
}

void UIContainer::onMouseButtonDown(const MouseEvent& me)
{
  Pos mPos((float)me.x,(float)me.y);
  //printf("mouse button down:%d,%d\n",me.x,me.y);
  for(UIObjectsList::reverse_iterator it=objLst.rbegin(),end=objLst.rend();it!=end;++it)
  {
    if((*it)->isInside(mPos))
    {
      //if((*it)->isTabStop() && !(*it)->isFocused())
      {
        (*it)->setFocus();
      }
      (*it)->onMouseButtonDown(me);
      return;
    }
  }
  UIObject::onMouseButtonDown(me);
}

void UIContainer::onMouseButtonUp(const MouseEvent& me)
{
  Pos mPos((float)me.x,(float)me.y);
  for(UIObjectsList::reverse_iterator it=objLst.rbegin(),end=objLst.rend();it!=end;++it)
  {
    if((*it)->isInside(mPos))
    {
      (*it)->onMouseButtonUp(me);
      return;
    }
  }
  UIObject::onMouseButtonUp(me);
}

void UIContainer::onMouseClick(const MouseEvent& me)
{
  Pos mPos((float)me.x,(float)me.y);
  for(UIObjectsList::reverse_iterator it=objLst.rbegin(),end=objLst.rend();it!=end;++it)
  {
    if((*it)->isInside(mPos))
    {
      (*it)->onMouseClick(me);
      return;
    }
  }
  UIObject::onMouseClick(me);
}

void UIContainer::onMouseScroll(const MouseEvent& me)
{
  Pos mPos((float)me.x,(float)me.y);
  for(UIObjectsList::reverse_iterator it=objLst.rbegin(),end=objLst.rend();it!=end;++it)
  {
    if((*it)->isInside(mPos))
    {
      (*it)->onMouseScroll(me);
      return;
    }
  }
  UIObject::onMouseClick(me);
}

void UIContainer::onObjectResize()
{
  if(layout.get())
  {
    layout->update(Pos(),size);
  }
  for(UIObjectsList::iterator it=objLst.begin(),end=objLst.end();it!=end;++it)
  {
    (*it)->onParentResize();
  }
}

void UIContainer::endOfResize()
{
  for(UIObjectsList::iterator it=objLst.begin(),end=objLst.end();it!=end;++it)
  {
    (*it)->onParentResizeEnd();
  }
}

void UIContainer::onFocusGain()
{
  for(UIObjectsList::iterator it=objLst.begin(),end=objLst.end();it!=end;++it)
  {
    if((*it)->isTabStop())
    {
      (*it)->setFocus();
      break;
    }
  }
}

void UIContainer::onFocusLost()
{
  for(UIObjectsList::iterator it=objLst.begin(),end=objLst.end();it!=end;++it)
  {
    if((*it)->isFocused())
    {
      (*it)->removeFocus();
      break;
    }
  }
}


void UIContainer::moveFocusToNext()
{
  std::vector<UIObject*> ts;
  int fIdx=0;
  for(UIObjectsList::iterator it=objLst.begin(),end=objLst.end();it!=end;++it)
  {
    if((*it)->isTabStop())
    {
      if((*it)->isFocused())
      {
        fIdx=ts.size();
      }
      ts.push_back(it->get());
    }
  }
  if(ts.size()>1)
  {
    ts[fIdx]->removeFocus();
    fIdx=(fIdx+1)%ts.size();
    ts[fIdx]->setFocus();
  }else if(ts.size()==1 && !ts[0]->isFocused())
  {
    ts[0]->setFocus();
  }
}

void UIContainer::onObjectResizeEnd()
{
  if(layout.get())
  {
    layout->update(Pos(),size);
  }
  for(UIObjectsList::iterator it=objLst.begin(),end=objLst.end();it!=end;++it)
  {
    (*it)->onParentResizeEnd();
  }
}


}
}


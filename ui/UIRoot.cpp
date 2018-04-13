#include "UIRoot.hpp"
#include "Engine.hpp"
#include "UIConfig.hpp"

namespace glider{
namespace ui{

UIRoot* root;

UIRoot::UIRoot()
{
  setName("root");
  refCount++;
  engine.setClearColor(uiConfig.getClearColor());
  engine.enableKeyboardRepeat();
}

void UIRoot::init()
{
  root=new UIRoot;
}

void UIRoot::shutdown()
{
  root->animations.clear();
  root->clear();
  delete root;
}

void UIRoot::onActiveChange(bool active)
{

}

void UIRoot::postMouseEvent(const MouseEvent& argEvent,UIObject* obj)
{
  switch(argEvent.eventType)
  {
    case metMove:
    {
      obj->onMouseMove(argEvent);
    }break;
    case metButtonPress:
    {
      obj->onMouseButtonDown(argEvent);
    }break;
    case metButtonRelease:
    {
      obj->onMouseButtonUp(argEvent);
    }break;
  }
}
void UIRoot::postKeyboardEvent(const KeyboardEvent& argEvent,UIObject* obj)
{
  switch(argEvent.eventType)
  {
    case ketInput:
    {
      obj->onKeyPress(argEvent);
    }break;
    case ketPress:
    {
      obj->onKeyDown(argEvent);
    }break;
    case ketRelease:
    {
      obj->onKeyUp(argEvent);
    }break;
  }

}

void UIRoot::onMouseEvent(MouseEvent& argEvent)
{
  switch(argEvent.eventType)
  {
    case metMove:
    {
      if(!mouseLock.empty())
      {
        mouseLock.back()->onMouseMove(argEvent);
      }else
      {
        onMouseMove(argEvent);
      }
    }break;
    case metButtonPress:
    {
      mouseButtonPressPos=Pos(argEvent);
      if(!mouseLock.empty())
      {
        mouseLock.back()->onMouseButtonDown(argEvent);
      }else
      {
        onMouseButtonDown(argEvent);
      }
    }break;
    case metButtonRelease:
    {
      if(!mouseLock.empty())
      {
        mouseLock.back()->onMouseButtonUp(argEvent);
        if(!mouseLock.empty() && argEvent.eventButton==1 && (mouseButtonPressPos-Pos(argEvent)).sum()<1)
        {
          mouseLock.back()->onMouseClick(argEvent);
        }
      }else
      {
        onMouseButtonUp(argEvent);
        if(argEvent.eventButton==1 && (mouseButtonPressPos-Pos(argEvent)).sum()<1)
        {
          onMouseClick(argEvent);
        }
      }
    }break;
    case metScroll:
    {
      if(!mouseLock.empty())
      {
        mouseLock.back()->onMouseScroll(argEvent);
      }else
      {
        onMouseScroll(argEvent);
      }
    }break;
  }
}

void UIRoot::onKeyboardEvent(KeyboardEvent& argEvent)
{
  switch(argEvent.eventType)
  {
    case ketInput:
    {
      if(keyFocus.get())
      {
        keyFocus->onKeyPress(argEvent);
      }else
      {
        onKeyPress(argEvent);
      }
    }break;
    case ketPress:
    {
      if(keyFocus.get())
      {
        keyFocus->onKeyDown(argEvent);
      }else
      {
        onKeyDown(argEvent);
      }
    }break;
    case ketRelease:
    {
      if(keyFocus.get())
      {
        keyFocus->onKeyUp(argEvent);
      }else
      {
        onKeyUp(argEvent);
      }
    }break;
  }
}

void UIRoot::onResize()
{
  setSize(Pos((float)engine.getWidth(),(float)engine.getHeight()));
}

void UIRoot::onQuit()
{
  engine.exitApp();
}

void UIRoot::addAnimation(UIAnimation* argAni)
{
  argAni->onStart();
  animations.push_back(argAni);
}
void UIRoot::removeAnimation(UIAnimation* argAni)
{
  for(AniList::iterator it=animations.begin(),end=animations.end();it!=end;++it)
  {
    if(*it==argAni)
    {
      (*it)->onEnd();
      animations.erase(it);
      break;
    }
  }
}


void UIRoot::onFrameUpdate(int mcsec)
{
  std::vector<AniList::iterator> toKill;
  for(AniList::iterator it=animations.begin(),end=animations.end();it!=end;++it)
  {
    if(!(*it)->update(mcsec))
    {
      (*it)->onEnd();
      toKill.push_back(it);
    }
  }
  for(std::vector<AniList::iterator>::iterator it=toKill.begin(),end=toKill.end();it!=end;++it)
  {
    if((**it)->deleteOnFinish())
    {
      delete **it;
    }
    animations.erase(*it);
  }
}

void UIRoot::onUserEvent(void* data1,void* data2)
{
  UserCallback* ucb=(UserCallback*)data1;
  (*ucb)();
}

void UIRoot::replaceLoop(UIObject& obj)
{
  UIObjectRef oldFocus=keyFocus;
  setKeyboardFocus(0);
  UIObjectsList swp;
  objLst.swap(swp);
  //objLst.push_back(&obj);
  Reference(&obj).forceRef();
  addObject(&obj);
  /*if(obj.isTabStop())
  {
    setKeyboardFocus(&obj);
  }*/
  obj.setFocus();
  try{
    engine.loop(this);
  }catch(...)
  {
    objLst.swap(swp);
    throw;
  }
  //if(keyFocus.get())keyFocus->removeFocus();
  //setKeyboardFocus(oldFocus.get());
  removeObject(&obj);
  setKeyboardFocus(oldFocus.get());
  while(isMouseLocked())unlockMouse();
  objLst.swap(swp);
}

void UIRoot::modalLoop(UIObject& obj)
{
  UIObjectRef oldFocus=keyFocus;
  Reference(&obj).forceRef();
  keyFocus=0;
  lockMouse(&obj);
  addObject(&obj);
  obj.setFocus();
  engine.loop(this);
  removeObject(&obj);
  while(!mouseLock.empty() && mouseLock.back().get()!=&obj)
  {
    mouseLock.pop_back();
  }
  unlockMouse();
  keyFocus=oldFocus;
}

void UIRoot::exitModal()
{
  engine.exitLoop();
}

}
}

#include "Window.hpp"
#include "UIConfig.hpp"
#include "GLState.hpp"
#include "UIRoot.hpp"
#include "Scissors.hpp"

namespace glider{
namespace ui{

void Window::WindowTitle::draw()
{
  rect.draw();
  UIContainer::draw();
}

void Window::WindowTitle::onObjectResize()
{
  rect.setSize(size);
}

void Window::WindowClient::draw()
{
  rect.draw();
  UIContainer::draw();
}

void Window::WindowClient::onObjectResize()
{
  rect.setSize(size);
  UIContainer::onObjectResize();
}

Window::Window(const Pos& argPos,const Pos& argSize,const char* argTitle)
{
  frame=new Rectangle(Rect(-Pos(1,1),argSize+Pos(1,1)),Color(1,1,1,0.5),false);
  resizer=new Rectangle(Rect(argSize-Pos(7,7),Pos(7,7)),Color(0.7,0.7,0.7,0.7));
  resizable=true;
  resizing=false;
  pos=argPos;
  size=argSize;
  title=new WindowTitle;
  client=new WindowClient;
  titleText=new Label;
  titleText->assignFont(uiConfig.getWindowTitleFont());
  titleText->setPos(Pos(3,3));
  titleText->setColor(Color(0,0,0));
  titleText->enableShadow();
  titleText->setShadowColor(Color(0.9,0.9,0.9));
  titleText->setShadowOffset(Pos(0,1));

  title->addObject(titleText.get());
  title->setSize(Pos(argSize.x,uiConfig.getWindowTitleFont()->getHeight()+6));
  title->rect.setColor(Color(0.8,0.8,0.8));
  if(argTitle)
  {
    titleText->setCaption(argTitle);
  }

  client->setPos(Pos(0,title->getSize().y));
  client->rect.setPosition(client->getPos());
  client->setSize(Pos(size.x,size.y-title->getSize().y));
  client->rect.setColor(Color(0.1,0.1,0.1));
  UIContainer::addObject(title.get());
  UIContainer::addObject(client.get());
}

void Window::WindowTitle::onMouseButtonDown(const MouseEvent& me)
{
  if(me.eventButton==1)
  {
    UIContainer::onMouseButtonDown(me);
    //if(!root->isMouseLocked())
    {
      root->lockMouse(this);
      dragging=true;
    }
  }
}

void Window::setResizable(bool argValue)
{
  resizable=argValue;
}


void Window::WindowTitle::onMouseMove(const MouseEvent& me)
{
  if(dragging)
  {
    parent->setPos(parent->getPos()+Pos(me.xRel,me.yRel));
  }else
  {
    UIContainer::onMouseMove(me);
  }
}

void Window::WindowTitle::onMouseButtonUp(const MouseEvent& me)
{
  if(me.eventButton==1)
  {
    dragging=false;
    root->unlockMouse();
  }else
  {
    UIContainer::onMouseButtonUp(me);
  }
}

void Window::onMouseButtonDown(const MouseEvent& me)
{
  if(parent && parent->isContainer())
  {
    UIContainer* con=(UIContainer*)parent;
    con->moveObjectToFront(this);
    client->setFocus();
  }
  if(me.eventButton==1 && resizable)
  {
    Pos mpos(me.x,me.y);
    mpos-=getAbsPos();
    if(Rect(resizer->getPosition(),resizer->getSize()).isInside(mpos))
    {
      root->lockMouse(this);
      resizing=true;
      return;
    }
  }
  UIContainer::onMouseButtonDown(me);
}

void Window::onMouseButtonUp(const MouseEvent& me)
{
  if(resizing && me.eventButton==1)
  {
    resizing=false;
    root->unlockMouse();
    endOfResize();
    return;
  }
  UIContainer::onMouseButtonUp(me);
}

void Window::onMouseMove(const MouseEvent& me)
{
  if(resizing)
  {
    Pos mrpos(me.xRel,me.yRel);
    setSize(size+mrpos);
  }else
  {
    UIContainer::onMouseMove(me);
  }
}


void Window::draw()
{
  RelOffsetGuard rog(pos);
  frame->draw();
  title->draw();
  client->draw();
  if(resizable)
  {
    resizer->draw();
  }
}

void Window::onObjectResize()
{
  if(size.x<64)size.x=64;
  if(size.y<getTitleHeight()+32)size.y=getTitleHeight()+32;
  title->setSize(Pos(size.x,uiConfig.getWindowTitleFont()->getHeight()+6));
  client->setSize(Pos(size.x,size.y-title->getSize().y));
  //frame->setPosition(pos-Pos(1,1));
  frame->setSize(size+Pos(1,1));
  resizer->setPosition(size-resizer->getSize());
}

void Window::onFocusGain()
{
  client->setFocus();
}

void Window::onFocusLost()
{
  if(title->dragging)
  {
    root->unlockMouse();
    title->dragging=false;
  }
  if(resizing)
  {
    resizing=false;
    root->unlockMouse();
  }
  client->removeFocus();
}

}
}

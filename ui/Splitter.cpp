/*
 * Splitter.cpp
 *
 *  Created on: 11 ñåíò. 2016 ã.
 *      Author: konst
 */

#include "Splitter.hpp"
#include "UIRoot.hpp"
#include "UIConfig.hpp"

namespace glider{
namespace ui{

Splitter::Splitter(SplitterOrientation argOrientation, int argPosition):
    orientation(argOrientation), position(0)
{
  sRect=new SplitterRect();
  sRect->rect.setColor(uiConfig.getColor("splitterColor"));
  splitterWidth=(int)uiConfig.getConst("splitterWidth");
  minFirstWidth=(int)uiConfig.getConst("splitterMinWidth");
  minSecondWidth=minFirstWidth;
  addObject(sRect.get());
  setPosition(argPosition);
}

void Splitter::setPosition(int argPosition)
{
  position=argPosition;
  if(getSize().isZero()) {
    return;
  }
  if(position<minFirstWidth)
  {
    position=minFirstWidth;
  }
  if(orientation==soVertical)
  {
    if(position>getSize().x-minSecondWidth-splitterWidth)
    {
      position=(int)getSize().x-minSecondWidth-splitterWidth;
    }
  }else
  {
    if(position>getSize().y-minSecondWidth-splitterWidth)
    {
      position=(int)getSize().y-minSecondWidth-splitterWidth;
    }
  }
  Rectangle& r=sRect->rect;
  if(orientation==soHorizontal)
  {
    r.setPosition(Pos{0, (float)position});
    r.setSize(Pos{getSize().x, (float)splitterWidth});
  }else
  {
    r.setPosition(Pos{(float)position, (float)0});
    r.setSize(Pos{(float)splitterWidth, getSize().y});
  }
  sRect->setPos(r.getPosition());
  sRect->setSize(r.getSize());
  updateFirstPosAndSize();
  updateSecondPosAndSize();
  LOGDEBUG("ui.splitter", "%{}, position=%{}, pos=%{}, size=%{}", getName(), position, r.getPosition(), r.getSize());
}

void Splitter::setFirst(UIObjectRef obj)
{
  if(first.get())
  {
    removeObject(first.get());
  }
  first=obj;
  addObject(obj.get());
  updateFirstPosAndSize();
}

void Splitter::setSecond(UIObjectRef obj)
{
  if(second.get())
  {
    removeObject(second.get());
  }
  second=obj;
  addObject(obj.get());
  updateSecondPosAndSize();
}

void Splitter::updateFirstPosAndSize()
{
  if(!first.get())
  {
    return;
  }
  first->setPos(Pos(0,0));
  if(orientation==soHorizontal)
  {
    first->setSize(Pos(getSize().x, sRect->rect.getPosition().y));
  }else
  {
    first->setSize(Pos(sRect->rect.getPosition().x, getSize().y));
  }
  if(first->isContainer()) {
    first.as<UIContainer>()->updateLayout();
  }
  LOGDEBUG("ui.splitter", "first(%{}):%{} size=%{}", getName(), first->getName(), first->getSize());
}

void Splitter::updateSecondPosAndSize()
{
  if(!second.get())
  {
    return;
  }
  if(orientation==soHorizontal)
  {
    second->setPos(Pos(0, sRect->rect.getPosition().y+splitterWidth));
    second->setSize(Pos(getSize().x, getSize().y-sRect->rect.getPosition().y-splitterWidth));
  }else
  {
    second->setPos(Pos(sRect->rect.getPosition().x+splitterWidth, 0));
    second->setSize(Pos(getSize().x-sRect->rect.getPosition().x-splitterWidth, getSize().y));
  }
  if(second->isContainer()) {
    second.as<UIContainer>()->updateLayout();
  }
  LOGDEBUG("ui.splitter", "second(%{}):%{} pos=%{} size=%{}", getName(), second->getName(), second->getPos(), second->getSize());
}


void Splitter::SplitterRect::onMouseButtonDown(const MouseEvent& me)
{
  root->lockMouse(this);
  resizing=true;
  resStart=me.getPos();
  Splitter* parent=(Splitter*)getParent();
  startPos=parent->getPosition();
}

void Splitter::SplitterRect::onMouseButtonUp(const MouseEvent& me)
{
  root->unlockMouse();
  resizing=false;
}

void Splitter::SplitterRect::onMouseMove(const MouseEvent& me)
{
  if(resizing) {
    Splitter* parent=(Splitter*)getParent();
    int diff = parent->getOrientation()==soVertical?me.x-resStart.x:me.y-resStart.y;
    int newPos = startPos + diff;
    parent->setPosition(newPos);
  }
}

void Splitter::recalcPosition()
{
  if(!second.get())
  {
    setPosition(position);
    return;
  }
  Pos oldSize=second->getPos()+second->getSize();
  int newPosition=position;
  if(resizePolicy==rpProportional)
  {
    if(orientation==soHorizontal)
    {
      if(oldSize.y)
      {
        newPosition=(int)(position*size.y/oldSize.y);
      }
    }else
    {
      if(oldSize.x)
      {
        newPosition=(int)(position*size.x/oldSize.x);
      }
    }
  }
  else if(resizePolicy==rpKeepSecond)
  {
    if(orientation==soHorizontal)
    {
      newPosition=(int)(size.y-(oldSize.y-position));
    }else
    {
      newPosition=(int)(size.x-(oldSize.x-position));
    }
  }
  setPosition(newPosition);
}

void Splitter::onObjectResize()
{
  recalcPosition();
}

void Splitter::onObjectResizeEnd()
{
  recalcPosition();
}

void Splitter::onParentResize()
{
  setSize(parent->getSize());
  recalcPosition();
}

void Splitter::onParentResizeEnd()
{
  setSize(parent->getSize());
  recalcPosition();
}


}
}

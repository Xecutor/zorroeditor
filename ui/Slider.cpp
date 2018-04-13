/*
 * Slider.cpp
 *
 *  Created on: 26.08.2012
 *      Author: Xecutor
 */

#include "Slider.hpp"
#include "UIRoot.hpp"
#include "../core/GLState.hpp"

namespace glider{
namespace ui{

Slider::Slider(float argCurValue,float argMinValue,float argMaxValue):
    curValue(argCurValue),minValue(argMinValue),maxValue(argMaxValue),mouseDown(false)
{
  size=Pos(200,10);
  rect.setColor(Color(0.3f,0.3f,0.3f,1.0f));
  rect.setSize(size);
  rect.setFill(true);
  updateValue((int)(getAbsPos().x+(curValue-minValue)/(maxValue-minValue)*size.x));
}

void Slider::onMouseButtonDown(const MouseEvent& me)
{
  mouseDown=true;
  updateValue(me.x);
  root->lockMouse(this);
}

void Slider::onMouseButtonUp(const MouseEvent& me)
{
  mouseDown=false;
  root->unlockMouse();
}
void Slider::onMouseMove(const MouseEvent& me)
{
  if(mouseDown)
  {
    updateValue(me.x);
  }
}

void Slider::updateValue(int mx)
{
  Pos apos=getAbsPos();
  curValue=(mx-apos.x)/size.x;
  if(curValue<minValue)curValue=minValue;
  if(curValue>maxValue)curValue=maxValue;
  int x=(int)((curValue-minValue)*size.x/(maxValue-minValue));
  line.setSource(Pos((float)x,0.0f));
  line.setDestination(Pos((float)x,size.y));
}

void Slider::draw()
{
  RelOffsetGuard rog(pos);
  rect.draw();
  line.draw();
}

}
}

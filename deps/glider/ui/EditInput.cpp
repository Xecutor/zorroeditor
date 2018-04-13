#include "EditInput.hpp"
#include "UIConfig.hpp"
#include "UIRoot.hpp"
#include "GLState.hpp"
#include "UString.hpp"

namespace glider{
namespace ui{

EditInput::EditInput(const char* argName)
{
  if(argName)
  {
    setName(argName);
  }
  curBlinkAni.ei=this;
  scrollAni.ei=this;
  text.assignFont(uiConfig.getEditLineFont());
  text.setPosition(Pos(3,3));
  size=Pos(100.0f,(float)uiConfig.getEditLineFont()->getHeight()+6);
  rect.setColor(Color(0.3f,0.3f,0.3f,1.0f));
  rect.setSize(size);
  tabStop=true;
  cursor.setSource(Pos(text.getPosition().x,0));
  cursor.setDestination(Pos(text.getPosition().x,size.y));
  lastCurBlink=0;
  isCurVisible=false;
  curPos=0;
  selection.setColor(Color(0.4f,0.4f,0.4f,1.0f));
  hShift=0;
  selecting=false;
  scrollTimer=0;
  scrollDir=0;
  selStart=selEnd=-1;
}

void EditInput::onFocusGain()
{
  LOGDEBUG(log,"gain:name=%{} focus=%{}, ani.active=%{}",name,focused?"true":"false",curBlinkAni.active?"true":"false");
  if(!curBlinkAni.active)
  {
    root->addAnimation(&curBlinkAni);
    isCurVisible=true;
    lastCurBlink=0;
  }
}

void EditInput::onFocusLost()
{
  isCurVisible=false;
  if(curBlinkAni.active)
  {
    root->removeAnimation(&curBlinkAni);
  }
  LOGDEBUG(log,"lost:name=%{} focus=%{}, ani.active=%{}",name,focused?"true":"false",curBlinkAni.active?"true":"false");
}


void EditInput::onKeyDown(const KeyboardEvent& ke)
{
  bool shift=(ke.keyMod&keyboard::GK_MOD_SHIFT)!=0;
  if(ke.keySym==keyboard::GK_LEFT)
  {
    setCurPos(curPos-1,shift);
  }
  if(ke.keySym==keyboard::GK_RIGHT)
  {
    setCurPos(curPos+1,shift);
  }
  if(ke.keySym==keyboard::GK_HOME)
  {
    setCurPos(0,shift);
  }
  if(ke.keySym==keyboard::GK_END)
  {
    setCurPos(text.getTextLength(),shift);
  }
  if(ke.keySym==keyboard::GK_BACKSPACE || ke.keySym==keyboard::GK_DELETE)
  {
    if(haveSelection())
    {
      insertText("");
    }else
    {
      deleteSymbol(ke.keySym==keyboard::GK_DELETE?0:-1);
    }
  }
  if(ke.keySym==keyboard::GK_RETURN && eiCb[eiOnAccept])
  {
    eiCb[eiOnAccept](UIEvent(this,eiOnAccept));
  }
  if(ke.keySym==keyboard::GK_ESCAPE && eiCb[eiOnCancel])
  {
    eiCb[eiOnCancel](UIEvent(this,eiOnCancel));
  }
  UIObject::onKeyDown(ke);
}

void EditInput::onKeyPress(const KeyboardEvent& ke)
{
  char val[5];
  UString::ucs2ToUtf8(ke.unicodeSymbol,val);
  insertText(val);
  setCurPos(curPos+1);
  UIObject::onKeyDown(ke);
}

void EditInput::deleteSymbol(int dir)
{
  if(curPos+dir>=0 && curPos+dir<text.getTextLength())
  {
    int idx=text.getUString().getLetterOffset(curPos+dir);
    int start=idx;
    text.getUString().getNext(idx);
    value.erase(start,idx-start);
    text.setText(value.c_str(),true);
    setCurPos(curPos+dir,false);
    if(eiCb[eiOnModify])eiCb[eiOnModify](UIEvent(this,eiOnModify));
  }
}

void EditInput::insertText(const char* txt)
{
  if(haveSelection())
  {
    int idx1=text.getUString().getLetterOffset(selStart);
    int idx2=text.getUString().getLetterOffset(selEnd);
    value.erase(idx1,idx2-idx1);
    setCurPos(selStart) ;
    resetSelection();
  }
  int idx=curPos;
  if(idx>0)
  {
    idx=text.getUString().getLetterOffset(curPos);
  }

  value.insert(idx,txt);
  text.setText(value.c_str(),true);
  if(eiCb[eiOnModify])eiCb[eiOnModify](UIEvent(this,eiOnModify));
}

void EditInput::setCurPos(int argCurPos,bool extendSelection)
{
  int oldCurPos=curPos;
  if(!extendSelection)
  {
    selStart=selEnd=-1;
  }
  curPos=argCurPos;
  if(curPos<0)
  {
    curPos=0;
  }
  int tl=text.getTextLength();
  if(curPos>tl)
  {
    curPos=tl;
  }
  if(tl==0)
  {
    hShift=0;
    cursor.setSource(Pos(text.getPosition().x,0));
    cursor.setDestination(Pos(text.getPosition().x,size.y));
    if(focused)
    {
      isCurVisible=true;
      lastCurBlink=0;
    }
    return;
  }
  int idx=curPos;
  bool atEnd=false;
  if(tl>0 && curPos==tl)
  {
    idx=curPos-1;
    atEnd=true;
  }
  Pos lPos,lSize;
  text.getLetterExtent(idx,lPos,lSize);
  lPos.y=1;
  lPos.x+=text.getPosition().x-1;
  if(atEnd)
  {
    lPos.x+=lSize.x+2;
  }
  cursor.setSource(lPos);
  lPos.y+=size.y-2;
  cursor.setDestination(lPos);
  if(focused)
  {
    isCurVisible=true;
    lastCurBlink=0;
  }

  if(extendSelection && oldCurPos!=curPos)
  {
    if(selStart==-1)
    {
      if(oldCurPos<curPos)
      {
        selStart=oldCurPos;
        selEnd=curPos;
      }else
      {
        selStart=curPos;
        selEnd=oldCurPos;
      }
    }else
    if(oldCurPos==selStart)
    {
      selStart=curPos;
    }else
    if(oldCurPos==selEnd)
    {
      selEnd=curPos;
    }
    if(selEnd<selStart)
    {
      int tmp=selEnd;
      selEnd=selStart;
      selStart=tmp;
    }
    if(selStart!=selEnd)
    {
      Pos sPos,sSize;
      text.getLetterExtent(selStart,sPos,sSize);
      sPos.y=1;
      Pos ePos,eSize;
      text.getLetterExtent(selEnd==tl?selEnd-1:selEnd,ePos,eSize);
      Pos sz(ePos.x-sPos.x,size.y-2);
      if(selEnd==tl)
      {
        sz.x+=eSize.x;
      }
      sz.x+=2;
      sPos.x+=text.getPosition().x-1;
      selection.setPosition(sPos);
      selection.setSize(sz);
    }
  }
  if(lPos.x-hShift>size.x)
  {
    hShift=(int)(lPos.x-size.x+2);
  }
  if(hShift+2>lPos.x)
  {
    hShift=(int)(lPos.x-2);
    if(hShift<0)
    {
      hShift=0;
    }
  }
}


void EditInput::setValue(const std::string& argValue)
{
  value=argValue;
  text.setText(argValue.c_str(),true);
  resetSelection();
  setCurPos(value.length());
  if(eiCb[eiOnModify])eiCb[eiOnModify](UIEvent(this,eiOnModify));
}

void EditInput::resetSelection()
{
  selStart=selEnd=-1;
}

int EditInput::mouseXToCurPos(int x)
{
  int tl=text.getTextLength();
  x=x-(int)getAbsPos().x-(int)text.getPosition().x+hShift;
  //printf("x=%d\n",x);
  int x0=0;
  for(int i=0;i<tl;i++)
  {
    Pos pos,sz;
    text.getLetterExtent(i,pos,sz);
    if(i==0 && x<pos.x)
    {
      return 0;
    }
    int x1;
    if(i<tl-1)
    {
      Pos pos2,sz2;
      text.getLetterExtent(i+1,pos2,sz2);
      x1=(int)pos2.x;
    }else
    {
      x1=(int)(pos.x+sz.x);
      if(x>x1)
      {
        return tl;
      }
    }
    //printf("x0=%d, x1=%d\n",x0,x1);
    if(x>=x0 && x<=x1)
    {
      return i+(x-pos.x>sz.x/2?1:0);
    }
    x0=x1;
  }
  return tl;
}

bool EditInput::cursorBlinkAnimation(int mcsec)
{
  if(!focused)
  {
    isCurVisible=false;
    return false;
  }
  lastCurBlink+=mcsec;
  if(lastCurBlink>500000)
  {
    isCurVisible=!isCurVisible;
    lastCurBlink-=500000;
  }
  return true;
}


void EditInput::scroll(int mcsec)
{
  scrollTimer+=mcsec;
  if(scrollTimer>500000)
  {
    scrollTimer=0;
    setCurPos(curPos+scrollDir,true);
  }
}

void EditInput::onMouseButtonDown(const MouseEvent& me)
{
  if(me.eventButton==1)
  {
    resetSelection();
    root->lockMouse(this);
    setCurPos(mouseXToCurPos(me.x),false);
    selecting=true;
  }
}
void EditInput::onMouseButtonUp(const MouseEvent& me)
{
  if(me.eventButton==1)
  {
    scrollAni.cancel();
    root->unlockMouse();
    selecting=false;
  }
}
void EditInput::onMouseMove(const MouseEvent& me)
{
  if(selecting)
  {
    setCurPos(mouseXToCurPos(me.x),true);
    Pos aPos=getAbsPos();
    if(me.x>aPos.x+size.x || me.x<aPos.x)
    {
      scrollDir=me.x<aPos.x?-1:1;
      scrollAni.isCancelled=false;
      root->addAnimation(&scrollAni);
    }else
    {
      scrollAni.cancel();
    }
  }
}


void EditInput::draw()
{
  ScissorsGuard sg(Rect(pos,size));
  RelOffsetGuard rog(pos);
  rect.draw();
  RelOffsetGuard rog2(Pos((float)-hShift,0.0f));
  if(selStart!=-1 && selStart!=selEnd)
  {
    selection.draw();
  }
  text.draw();
  if(isCurVisible)
  {
    cursor.draw();
  }
}

void EditInput::onObjectResize()
{
  size.y=(float)(uiConfig.getEditLineFont()->getHeight()+6);
  rect.setSize(size);
}

}
}

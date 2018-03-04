#include "ListBox.hpp"
#include "GLState.hpp"
#include "UIConfig.hpp"
#include "UIRoot.hpp"

namespace glider{
namespace ui{

ListBox::ListBox()
{
  tabStop=true;
  totalHeight=0;
  sb=new ScrollBar(0,0);
  sb->assignParent(this);
  sb->setVisible(false);
  typedef ListBox ThisClass;
  sb->setValueChangeHandler(MKUICALLBACK(onScroll));

  multiSel=false;
  topItem=objLst.end();
  selItem=objLst.end();
  bgRect.setColor(uiConfig.getColor("listBoxBgColor"));
  selRect.setColor(uiConfig.getColor("listBoxSelColor"));
}


void ListBox::onObjectResizeEnd()
{
  UIContainer::onObjectResizeEnd();
  sb->setFrame(size.y);
  sb->updatePosFromParent();
  sb->setTotal(totalHeight);
  if(totalHeight>size.y)
  {
    sb->setVisible(true);
    bool ch=false;
    if(!objLst.empty() && topItem!=objLst.end() && topItem!=objLst.begin())
    {
      float vh=calcHeight(topItem,objLst.end());
      UIObjectsList::iterator it=topItem;
      --it;
      vh+=(*it)->getSize().y;
      while(vh<size.y)
      {
        --topItem;
        if(topItem==objLst.begin())break;
        it=topItem;
        --it;

        vh+=(*it)->getSize().y;
        ch=true;
      }
    }
    if(ch)
    {
      updateScroll();
    }
  }else
  {
    topItem=objLst.begin();
    sb->setValue(0);
    sb->setVisible(false);
  }
  bgRect.setSize(size);
}

void ListBox::onClear()
{
  setSelItem(topItem=objLst.end());
}

void ListBox::onAddObject()
{
  if(topItem==objLst.end())
  {
    topItem=objLst.begin();
  }
  int h=0;
  int th=0;
  for(UIObjectsList::iterator it=objLst.begin(),end=objLst.end();it!=end;++it)
  {
    if(it==topItem)
    {
      th=h;
    }
    if(!(*it)->isVisible())continue;
    h+=(int)(*it)->getSize().y;
  }
  if(h>size.y)
  {
    sb->setVisible(true);
  }
  totalHeight=(float)h;
//  printf("h=%d,size.y=%f,drawScroll=%s\n",h,size.y,drawScrollBar?"true":"false");
  if(sb->isVisible())
  {
    sb->setTotal(totalHeight);
    /*if(sb->getValue()==0)*/sb->setValue((float)th);
//    printFmt("sb->pos=%{}, sb->size=%{} sb->total=%d\n",sb->getPos(),sb->getSize(),(int)sb->getTotal());
  }
}
void ListBox::onRemoveObject(UIObjectsList::iterator it)
{
  totalHeight-=(**it).getSize().y;
  sb->setTotal(totalHeight);
  if(totalHeight<size.y)
  {
    sb->setVisible(false);
  }
  if(it==topItem)
  {
    if(++topItem==objLst.end())
    {
      --topItem;
      --topItem;
    }
  }
  if(it==selItem)
  {
    setSelItem(objLst.end());
  }
  updateScroll();
}


void ListBox::draw()
{
  RelOffsetGuard rog(pos);
  ScissorsGuard sgrd(Rect(Pos(),size));
  bgRect.draw();
  float yOff=0;
  for(UIObjectsList::iterator it=topItem,end=objLst.end();it!=end;++it)
  {
    UIObjectRef& obj=*it;
    if(obj->isVisible())
    {
      if((multiSel && selSet.find(it)!=selSet.end()) || it==selItem)
      {
        selRect.setPosition(Pos(0,yOff));
        selRect.setSize(Pos(bgRect.getSize().x,obj->getSize().y));
        selRect.draw();
      }
      obj->setPos(Pos(0,yOff));
      obj->draw();
      yOff+=obj->getSize().y;
      if(yOff>size.y)break;
    }
  }
  if(sb->isVisible())
  {
    sb->draw();
  }
}

template <class T>
T inc(T a)
{
  T rv=a;
  return ++rv;
}

template <class T>
T dec(T a)
{
  T rv=a;
  return --rv;
}


void ListBox::onKeyDown(const KeyboardEvent& key)
{
  if(key.keySym==keyboard::GK_UP)
  {
    if(multiSel)selSet.clear();
    if(selItem==objLst.end())
    {
      selItem=objLst.begin();
    }else if(selItem!=objLst.begin())
    {
      if(selItem==topItem)
      {
        --topItem;
      }
      setSelItem(dec(selItem));
      int h=(int)(calcHeight(objLst.begin(),selItem)-calcHeight(objLst.begin(),topItem));
      if(h<0)
      {
        topItem=selItem;
      }else
      if(h>size.y)
      {
        topItem=selItem;
        float h=0;
        while(h+(*topItem)->getSize().y<size.y)
        {
          h+=(*topItem)->getSize().y;
          --topItem;
        }
      }
      updateScroll();
    }
  }else if(key.keySym==keyboard::GK_DOWN)
  {
    if(multiSel)selSet.clear();
    if(selItem==objLst.end())
    {
      if(!objLst.empty())
      {
        setSelItem(--objLst.end());
      }
    }else
    {
      if(selItem!=--objLst.end())
      {
        setSelItem(inc(selItem));

        int h=(int)(calcHeight(objLst.begin(),selItem)-calcHeight(objLst.begin(),topItem));
        if(h<0)
        {
          topItem=selItem;
        }else
        if(h+(*selItem)->getSize().y>size.y)
        {
          ++topItem;
        }
        updateScroll();
      }
    }
  }else if(key.keySym==keyboard::GK_PAGEUP)
  {
    if(sb->isVisible())
    {
      int h=0;
      while(topItem!=objLst.begin() && h<size.y)
      {
        h+=(int)(*topItem)->getSize().y;
        --topItem;
      }
      updateScroll();
    }
  }else if(key.keySym==keyboard::GK_PAGEDOWN)
  {
    if(sb->isVisible())
    {
      int h=0;
      while(calcHeight(topItem,objLst.end())>size.y && h<size.y)
      {
        h+=(int)(*topItem)->getSize().y;
        ++topItem;
      }
      updateScroll();
    }
  }else if(key.keySym==keyboard::GK_HOME)
  {
    if(sb->isVisible())
    {
      topItem=objLst.begin();
      updateScroll();
    }
  }else if(key.keySym==keyboard::GK_END)
  {
    scrollToEnd();
  }else
  {
    UIObject::onKeyPress(key);
  }
}

void ListBox::setScrollPos(int idx)
{
  if(sb->isVisible())
  {
    topItem=objLst.begin();
    for(int i=0;i<idx;++i)
    {
      ++topItem;
      if(topItem==objLst.end())
      {
        --topItem;
        break;
      }
    }
    updateScroll();
  }
}

void ListBox::scrollToEnd()
{
  if(sb->isVisible())
  {
    topItem=--objLst.end();
    int h=0;
    while(h+(*topItem)->getSize().y<size.y)
    {
      h+=(int)(*topItem)->getSize().y;
      --topItem;
    }
    ++topItem;
    updateScroll();
  }
}


void ListBox::onScroll(const UIEvent& evt)
{
  float h=0;
  float v=sb->getValue();
  for(UIObjectsList::iterator it=objLst.begin(),end=objLst.end();it!=end;++it)
  {
    UIObjectRef& obj=*it;
    if(obj->isVisible())
    {
      if(h>=v)
      {
        topItem=it;
        break;
      }
      h+=obj->getSize().y;
    }
  }
}

void ListBox::updateScroll()
{
  if(sb->isVisible())
  {
    sb->setValue(calcHeight(objLst.begin(),topItem));
  }
}

void ListBox::onMouseButtonDown(const MouseEvent& me)
{
  if(sb->isVisible() && me.eventButton==1)
  {
    if(sb->getAbsRect().isInside(Pos(me)))
    {
      root->postMouseEvent(me,sb.get());
      return;
    }
  }
  if(me.eventButton==1)
  {
    for(UIObjectsList::iterator it=topItem,end=objLst.end();it!=end;++it)
    {
      UIObjectRef& obj=*it;
      if(obj->isVisible())
      {
        Rect r=obj->getAbsRect();
        r.size.x=size.x;
        if(r.isInside(Pos(me)))
        {
          if(selItem!=it)
          {
            setSelItem(it);
            updateScroll();
          }
          break;
        }
      }
    }
  }else if(me.eventButton==4)
  {
    if(sb->isVisible() && topItem!=objLst.begin())
    {
      --topItem;
      updateScroll();
    }
  }else if(me.eventButton==5)
  {
    if(sb->isVisible() && calcHeight(topItem,objLst.end())>size.y)
    {
      ++topItem;
      updateScroll();
    }
  }else
  {
    UIContainer::onMouseButtonDown(me);
  }
  //root->setKeyboardFocus(this);
  //setFocus();
  //UIContainer::onMouseButtonDown(me);
}

float ListBox::calcHeight(UIObjectsList::iterator from,UIObjectsList::iterator to)
{
  float h=0;
  for(UIObjectsList::iterator it=from;it!=to;++it)
  {
    UIObjectRef& obj=*it;
    if(obj->isVisible())
    {
      h+=obj->getSize().y;
    }
  }
  return h;
}

}
}

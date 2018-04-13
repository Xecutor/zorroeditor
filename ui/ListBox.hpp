#ifndef __GLIDER_UI_LISTBOX_HPP__
#define __GLIDER_UI_LISTBOX_HPP__
#include "UIContainer.hpp"
#include "ScrollBar.hpp"
#include <set>

namespace glider{
namespace ui{

enum ListBoxEventType{
  lbetSelectionChanged,
  lbetCount
};

class ListBox:public UIContainer{
public:
  ListBox();
  void draw();
  UIObject* getSelection()
  {
    if(selItem!=objLst.end())
    {
      return selItem->get();
    }else
    {
      return 0;
    }
  }
  using UIObject::setEventHandler;
  void setEventHandler(ListBoxEventType et,UICallBack cb)
  {
    eventHandlers[et]=cb;
  }
  UIObjectsList getMultiSelection()
  {
    UIObjectsList rv;
    for(SelSet::iterator it=selSet.begin(),end=selSet.end();it!=end;++it)
    {
      rv.push_back(**it);
    }
    return rv;
  }
  void setSelection(int idx)
  {
    for(UIObjectsList::iterator it=objLst.begin(),end=objLst.end();it!=end;++it)
    {
      if(idx--==0)
      {
        setSelItem(it);
        break;
      }
    }
  }
  void setBgColor(Color clr)
  {
    bgRect.setColor(clr);
  }
  void setSelColor(Color clr)
  {
    selRect.setColor(clr);
  }
  void setScrollPos(int idx);
  void scrollToEnd();
protected:
  UIObjectsList::iterator topItem;
  UIObjectsList::iterator selItem;
  struct IteratorComparator{
    bool operator()(const UIObjectsList::iterator& a,const UIObjectsList::iterator& b)const
    {
      return a->get()<b->get();
    }
  };
  typedef std::set<UIObjectsList::iterator,IteratorComparator> SelSet;
  SelSet selSet;
  ScrollBarRef sb;
  Rectangle bgRect,selRect;
  float totalHeight;
  bool multiSel;
  UICallBack eventHandlers[lbetCount];
  void onAddObject();
  void onRemoveObject(UIObjectsList::iterator it);
  void onClear();
  void onObjectResizeEnd();
  void onKeyDown(const KeyboardEvent& key);
  void onMouseButtonDown(const MouseEvent& me);

  void setSelItem(UIObjectsList::iterator newSelItem)
  {
    if(newSelItem!=selItem)
    {
      selItem=newSelItem;
      if(selItem!=objLst.end())
      {
        int h=(int)(calcHeight(objLst.begin(),selItem)-calcHeight(objLst.begin(),topItem));
        if(h<0)
        {
          topItem=selItem;
        }else
        {
          while(h>=size.y)
          {
            h-=(int)(*topItem)->getSize().y;
            ++topItem;
          }
        }
      }
      updateScroll();
      if(eventHandlers[lbetSelectionChanged])
      {
        eventHandlers[lbetSelectionChanged](UIEvent(this,lbetSelectionChanged));
      }
    }
  }

  void onScroll(const UIEvent& evt);

  float calcHeight(UIObjectsList::iterator from,UIObjectsList::iterator to);
  void updateScroll();
};

typedef ReferenceTmpl<ListBox> ListBoxRef;

class MultiColumnListItem:public UIContainer{
public:
  MultiColumnListItem(int argSpacing=4):spacing(argSpacing)
  {

  }
  void setSpacing(int argSpacing)
  {
    spacing=argSpacing;
  }
protected:
  int spacing;
  void onAssignParent()
  {
    updateSizes();
  }
  void onAddObject()
  {
    size.x+=objLst.back()->getSize().x;
    if(objLst.back()->getSize().y>size.y)
    {
      size.y=objLst.back()->getSize().y;
    }
  }
  void updateSizes()
  {
    std::vector<int> sizes;
    ListBox* lb=dynamic_cast<ListBox*>(parent);
    if(!lb)return;
    ListEnumerator le(sizes);
    lb->enumerate(&le);
    int x=0;
    for(size_t i=0;i<sizes.size();++i)
    {
      int w=sizes[i];
      sizes[i]=x;
      x+=w+spacing;
    }
    PosUpdater pu(sizes);
    lb->enumerate(&pu);
  }
  struct ListEnumerator:UIContainerEnumerator{
    std::vector<int>& sizes;

    ListEnumerator(std::vector<int>& argSizes):sizes(argSizes)
    {

    }

    bool nextItem(UIObject* obj)
    {
      MultiColumnListItem* mci=dynamic_cast<MultiColumnListItem*>(obj);
      if(!mci)return true;
      int idx=0;
      for(UIObjectsList::iterator it=mci->objLst.begin(),end=mci->objLst.end();it!=end;++it,++idx)
      {
        if((int)sizes.size()<idx+1)
        {
          sizes.resize(idx+1);
        }
        if(sizes[idx]<(*it)->getSize().x)
        {
          sizes[idx]=(int)(*it)->getSize().x;
        }
      }
      return true;
    }
  };

  struct PosUpdater:UIContainerEnumerator{
    std::vector<int>& positions;

    PosUpdater(std::vector<int>& argPositions):positions(argPositions)
    {

    }

    bool nextItem(UIObject* obj)
    {
      MultiColumnListItem* mci=dynamic_cast<MultiColumnListItem*>(obj);
      if(!mci)return true;
      if(mci->objLst.empty())return true;
      int idx=0;
      for(UIObjectsList::iterator it=mci->objLst.begin(),end=mci->objLst.end();it!=end;++it,++idx)
      {
        (*it)->setPos(Pos((float)positions[idx],(*it)->getPos().y));
      }
      mci->setSize(Pos(mci->objLst.back()->getPos().x+mci->objLst.back()->getSize().x,mci->getSize().y));
      return true;
    }
  };
};

}
}

#endif

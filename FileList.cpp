/*
 * FileList.cpp
 *
 *  Created on: May 2, 2015
 *      Author: Konstantin
 */

#include "FileList.hpp"
#include "UIRoot.hpp"
#include "EventCallback.hpp"
#include "Project.hpp"

FileList::FileList(Project* argPrj):prj(argPrj)
{
  setName("filelist");
  typedef FileList ThisClass;
  lb=new ListBox;
  lb->setName("list");
  lb->setEventHandler(lbetSelectionChanged,MKUICALLBACK(onListClick));
  edt=new EditInput("filter");
  edt->setEventHandler(eiOnModify,MKUICALLBACK(onFilterMod));
  edt->setEventHandler(uietKeyDown,MKUICALLBACK(onFilterKey));
  edt->setEventHandler(eiOnAccept,MKUICALLBACK(onFilterAcc));
  edt->setEventHandler(eiOnCancel,MKUICALLBACK(onFilterAcc));
  //mkEventCallback<decltype(*this),UIEvent,&FileList::onFilterKey>(this);
  addObject(edt);
  addObject(lb);
  Layout* l=new Layout("XTCT[0,0]:filter-,list*!");
  setPos(Pos(0,0));
  setSize(Pos(300,root->getSize().y));
  l->fillObjects(this);
  setLayout(l);
  lb->setSize(Pos(300,root->getSize().y-lb->getPos().y));
  edt->setSize(Pos(300,edt->getSize().y));
}


void FileList::onListClick(const UIEvent& evt)
{
  Label* l=(Label*)lb->getSelection();
  if(l)
  {
    prj->openOrSwitchFile(l->getCaption());
  }
}

static int calcWeight(const std::string& fn,const std::string& flt)
{
  size_t fni=0,flti=0;
  int rv=0;
  bool lastMatch=false;
  while(flti<flt.size() && fni<fn.size())
  {
    if(fn[fni]==flt[flti])
    {
      rv+=lastMatch?2:1;
      lastMatch=true;
      ++flti;
      ++fni;
      continue;
    }
    ++fni;
    lastMatch=false;
  }
  return rv;
}

void FileList::onFilterMod(const UIEvent& evt)
{
  if(edt->getValue().empty())return;
  typedef std::pair<std::string,int> StrInt;
  std::vector<StrInt> v;
  for(auto& i:lb->getObjList())
  {
    const std::string& fn=i.as<Label>()->getCaption();
    v.push_back(std::make_pair(fn,calcWeight(fn,edt->getValue())));
  }
  std::sort(v.begin(),v.end(),[](const StrInt& a,const StrInt& b)
    {
      return a.second>b.second?true:a.second<b.second?false:a.first<b.first;
    });
  lb->clear();
  for(auto& i:v)
  {
    addFile(i.first);
  }
}

void FileList::onFilterKey(const UIEvent& evt)
{
  if(evt.ke.keySym==keyboard::GK_DOWN)
  {
    int idx=-1;
    if(lb->getSelection())
    {
      idx=lb->getSelection()->getIntTag();
    }
    lb->setSelection(idx+1);
    edt->setFocus();
  }else if(evt.ke.keySym==keyboard::GK_UP)
  {
    if(lb->getSelection())
    {
      lb->setSelection(lb->getSelection()->getIntTag()-1);
      edt->setFocus();
    }
  }
}

void FileList::onFilterAcc(const UIEvent& evt)
{
  edt->setValue("");
  UIObjectRef lastEditor=root->findByName("splitter1.splitter2.editor");
  if(lastEditor.get())
  {
    lastEditor->setFocus();
  }
}

class FindByNameEnumerator:public UIContainerEnumerator{
public:
  FindByNameEnumerator(const std::string argFileName):fileName(argFileName){}

  virtual bool nextItem(UIObject* obj) override
  {
    Label* lb=(Label*)obj;
    if(lb->getCaption()==fileName)
    {
      idx=lb->getIntTag();
      return false;
    }
    return true;
  }

  std::string fileName;
  int idx=-1;
};

void FileList::setSelection(const std::string& fileName)
{
  FindByNameEnumerator fbne(fileName);
  lb->enumerate(&fbne);
  if(fbne.idx!=-1)
  {
    lb->setSelection(fbne.idx);
  }
}


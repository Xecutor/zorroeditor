/*
 * FileList.hpp
 *
 *  Created on: May 2, 2015
 *      Author: Konstantin
 */

#ifndef __ZE_FILELIST_HPP__
#define __ZE_FILELIST_HPP__


#include "ListBox.hpp"
#include "EditInput.hpp"
#include "Label.hpp"

using namespace glider;
using namespace glider::ui;

class Project;

class FileList:public UIContainer{
public:
  FileList(Project* argPrj);
  void addFile(const std::string& fileName)
  {
    Label* l=new Label(fileName.c_str(),fileName.c_str());
    l->setColor(Color::black);
    l->setIntTag(lb->getCount());
    lb->addObject(l);
  }
  void setSelection(const std::string& fileName);
protected:
  ListBox* lb;
  EditInput* edt;
  Project* prj;

  void onListClick(const UIEvent& evt);
  void onFilterMod(const UIEvent& evt);
  void onFilterKey(const UIEvent& evt);
  void onFilterAcc(const UIEvent& evt);
};


#endif /* FILELIST_HPP_ */

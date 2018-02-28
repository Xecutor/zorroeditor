/*
 * FindPanel.cpp
 *
 *  Created on: 29 џэт. 2017 у.
 *      Author: konst
 */

#include "FindPanel.hpp"
#include "Editor.hpp"

FindPanel::FindPanel(Editor* argEditor):editor(argEditor)
{
  addObject(input.get());
  updateSizeAndPos();
  input->setEventHandler(eiOnAccept, MKUICALLBACK(onInputAccept));
  input->setEventHandler(eiOnCancel, MKUICALLBACK(onInputCancel));
}

void FindPanel::onInputAccept(const UIEvent& evt)
{
  editor->findNext(input->getValue());
}

void FindPanel::onInputCancel(const UIEvent& evt)
{
  editor->hideFind();
}

void FindPanel::onFocusGain()
{
  updateSizeAndPos();
  input->setFocus();
}

void FindPanel::onFocusLost()
{
  input->removeFocus();
  editor->hideFind();
}


void FindPanel::updateSizeAndPos()
{
  input->setSize(Pos(editor->getSize().x,input->getSize().y));
  setSize(input->getSize());
  input->setPos(Pos(0, editor->getSize().y - input->getSize().y));
}

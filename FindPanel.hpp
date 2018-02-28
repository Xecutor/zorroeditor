/*
 * FindPanel.hpp
 *
 *  Created on: 29 џэт. 2017 у.
 *      Author: konst
 */

#ifndef SRC_ZE_FINDPANEL_HPP_
#define SRC_ZE_FINDPANEL_HPP_

#include "ui/EditInput.hpp"
#include "UIContainer.hpp"

using namespace glider;
using namespace glider::ui;

class Editor;

class FindPanel: public UIContainer{
public:
  FindPanel(Editor* argEditor);
  void draw()
  {
    input->draw();
  }
protected:
  Editor* editor;
  ReferenceTmpl<EditInput> input = new EditInput;
  void onInputAccept(const UIEvent& evt);
  void onInputCancel(const UIEvent& evt);
  void onFocusGain();
  void onFocusLost();
  void updateSizeAndPos();
};


#endif /* SRC_ZE_FINDPANEL_HPP_ */

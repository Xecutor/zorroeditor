//
// Created by Konstantin on 11-May-15.
//

#include "ZUI.hpp"

#include "ZBuilder.hpp"

#include "ui/Window.hpp"
#include "ui/Button.hpp"
#include "ui/EditInput.hpp"
#include "ui/UIRoot.hpp"

using namespace zorro;
using namespace glider;
using namespace glider::ui;

static ClassInfo* uiobjcls=0;

static void zwindowctor(ZorroVM* vm,Value* cls)
{
  Window* w=new Window;
  root->addObject(w);
  Value rv=NObjValue(vm,cls->classInfo,new UIObjectRef(new Window));
  vm->setResult(rv);
}

static void zwindowaddobj(ZorroVM* vm,Value* obj)
{
  if(vm->getArgsCount()!=1)KSTHROW("invalid args count for window.addobj");
  Value arg=vm->getLocalValue(0);
  if(arg.vt!=vtNativeObject || !arg.nobj->classInfo->isInParents(uiobjcls))
  {
    KSTHROW("Invalid arg type for window.addobj");
  }
  Window* wnd=obj->nobj->as<UIObjectRef>().as<Window>();
  UIObject* v=arg.nobj->as<UIObjectRef>().get();
  v->setPos(Pos(10,10));
  wnd->addObject(v);
}

static void zobjrefdtor(ZorroVM* vm,Value* obj)
{
  delete &obj->nobj->as<UIObjectRef>();
}

static void zbuttonctor(ZorroVM* vm,Value* cls)
{
  Value rv=NObjValue(vm,cls->classInfo,new UIObjectRef(new Button("hello")));
  vm->setResult(rv);
}

void registerZUI(ZorroVM* vm)
{
  ZBuilder zb(vm);
  uiobjcls=zb.enterNClass("UIObject",0,0);
  zb.leaveClass();
  zb.enterNClass("Window",zwindowctor,zobjrefdtor);
  zb.registerCMethod("addObj",zwindowaddobj);
  zb.setNClassParent(uiobjcls);
  zb.leaveClass();
  zb.enterNClass("Button",zbuttonctor,zobjrefdtor);
  zb.setNClassParent(uiobjcls);
  zb.leaveClass();
}
#include "Console.hpp"
#include "ZVMHelpers.hpp"
#include "ZBuilder.hpp"

Console::Console()
{
  log=kst::Logger::getLogger("con");
  lb=new ListBox;
  lb->setName("lst");
  addObject(lb);
  Layout* l=new Layout("XBCC[0,0]:lst*");
  l->fillObjects(this);
  setLayout(l);
  addMsg("start");
  setName("console");
  //setTitle("Console");
}


void Console::addMsg(const char* msg)
{
  LOGDEBUG(log,"msg:%{}",msg);
  Label* l=new Label;
  l->setColor(Color::black);
  l->setCaption(msg);
  lb->addObject(l);
  lb->setSelection(lb->getCount()-1);
  lb->scrollToEnd();
}

using namespace zorro;

static void con_addMsg(ZorroVM* vm,Value* obj)
{
  Console* console=&obj->nobj->as<Console>();
  expectArgs(vm,"addMsg",1);
  Value& val=vm->getLocalValue(0);
  console->addMsg(ValueToString(vm,val).c_str());
}


void Console::registerInZVM(zorro::ZorroVM& vm)
{
  ZBuilder zb(&vm);
  ClassInfo* ci=zb.enterNClass("Console",0,0);
  zb.registerCMethod("addMsg",con_addMsg);
  zb.leaveClass();
  Value ev;
  ev.vt=vtNativeObject;
  ev.nobj=vm.allocNObj();
  ev.nobj->classInfo=ci;
  ev.nobj->userData=this;
  zb.registerGlobal("console",ev);
}

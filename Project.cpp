#include "Project.hpp"
#include <kst/File.hpp>
#include "Window.hpp"
#include "UIRoot.hpp"
#include "Console.hpp"
#include "ZBuilder.hpp"
#include "ZEditor.hpp"
#include "ZVMHelpers.hpp"
#include "FileList.hpp"
#include "Splitter.hpp"
#include "ZVMOps.hpp"

using namespace glider::ui;
using namespace zorro;

Project::Project(Console* con):natFr(&freg), scrFr(&freg)
{
  fl=new FileList(this);
  registerInZVM();
  Editor::registerInZVM(vm);
  ZEditor::registerInZVMEx(vm);
  con->registerInZVM(vm);
  FileRegistry::Entry* scrEntry=freg.openFile("zeditor.zs");
  loadScript(scrEntry);
}

void Project::load(const std::string& argFileName)
{
  kst::File f;
  f.ROpen(argFileName.c_str());
  std::string line;
  enum{
    secNone,
    secOptions,
    secFiles
  }sec=secNone;
  int lineNum=0;
  while(f.ReadLine(line))
  {
    ++lineNum;
    if(line.empty() || line[0]=='#')continue;
    if(line[0]=='[' && *line.rbegin()==']')
    {
      line.erase(0,1);
      line.erase(--line.end());
      if(line=="options")
      {
        sec=secOptions;
      }else if(line=="files")
      {
        sec=secFiles;
      }else
      {
        KSTHROW("Unknown section %{}",line);
      }
      continue;
    }
    if(sec==secNone)
    {
      KSTHROW("Expected section name in %{} at line %{}",argFileName,lineNum);
    }
    if(sec==secOptions)
    {
      std::string::size_type pos=line.find('=');
      if(pos==std::string::npos)
      {
        KSTHROW("Unexpected line %{} in %{}",lineNum,argFileName);
      }
      std::string name=line.substr(0,pos);
      std::string val=line.substr(pos+1);
      if(name=="zapi")
      {
        getIndexer().importApi(val.c_str());
        continue;
      }else if(name=="basePath")
      {
        basePath=val;
        if(*basePath.rbegin()!='/')
        {
          basePath+='/';
        }
      }else
      {
        KSTHROW("Unknown option %{}",name);
      }
    }else if(sec==secFiles)
    {
      addFile(line);
    }
  }
}

void Project::addFile(const std::string& argFileName)
{
  std::string fullPath=basePath+argFileName;
  fl->addFile(argFileName);
  auto res=files.insert(FilesMap::value_type(argFileName,new ProjectEntry(fullPath)));
  auto& pe=*res.first->second;
  FileRegistry::Entry* e=freg.openFile(fullPath);
  if(!e)
  {
    e=freg.addEntry(argFileName,"\n",1);
    pe.fr=new FileReader(&freg,e);
  }else
  {
    pe.fr=new FileReader(&freg,e);
    idx.indexate(pe.fr,true);
  }
}

Editor* Project::openOrSwitchFile(const std::string& argFileName)
{
  std::string shortFileName=argFileName;
  if(shortFileName.length()>basePath.length())
  {
    shortFileName=argFileName.substr(basePath.length());
  }
  auto it=files.find(shortFileName);
  if(it==files.end())
  {
    return 0;
  }
  /*UIObjectRef lastEditor=root->findByName("splitter1.splitter2.editor");
  if(lastEditor.get())
  {
    root->removeObject(lastEditor.get());
  }*/
  UIObjectRef s2obj=root->findByName("splitter1.splitter2");
  Splitter* s2=s2obj.as<Splitter>();
  if(it->second->editor.get())
  {
    Editor* edt=it->second->editor.as<Editor>();
    s2->setFirst(edt);
    s2->moveObjectToFront(edt);
    root->setKeyboardFocus(edt);
    edt->setFocus();
    fl->setSelection(shortFileName);
    return edt;
  }else
  {
    ProjectEntry& pe=*it->second;
    ZEditor* edt=new ZEditor(&idx);
    pe.editor=edt;

    Value nobj;
    nobj.vt=vtNativeObject;
    nobj.flags=0;
    nobj.nobj=new NativeObject;
    nobj.nobj->classInfo=Editor::getClassInfo();
    nobj.nobj->userData=edt;

    vm.pushValue(nobj);

    SymInfo* cls=vm.symbols.getSymbol(Symbol(vm.mkZString("ZEditor")));
    Value cobj;
    cobj.vt=vtClass;
    cobj.flags=0;
    cobj.classInfo=(ClassInfo*)cls;

    OpCall op(0, OpArg(atGlobal,0), atStack);
    vm.ctx.lastOp=&op;

    vm.callSomething(&cobj, 1);
    vm.resume();

    edt->setProject(this, *vm.ctx.dataStack.stackTop);
    vm.ctx.dataStack.pop();

    pe.fr->reset();
    edt->open(pe.fr);
    LOGDEBUG("prj","edt.size=%{}",edt->getSize());
    s2->setFirst(edt);
    root->setKeyboardFocus(edt);
    edt->setFocus();
    fl->setSelection(shortFileName);
    return edt;
  }
}


static void prj_openFile(ZorroVM* vm,Value* obj)
{
  expectArgs(vm,"openFile",1,3);
  std::string fn=getStrValue(vm,"filename",0);
  int line=0;
  int col=0;
  if(vm->getArgsCount()>1)
  {
    line=getIntValue(vm,"line",1);
  }
  if(vm->getArgsCount()>2)
  {
    col=getIntValue(vm,"col",2);
  }
  Project& prj=obj->nobj->as<Project>();
  Editor* edt=prj.openOrSwitchFile(fn);
  if(edt)
  {
    edt->setCursorPos(col,line);
  }
}

void Project::registerInZVM()
{
  ZBuilder zb(&vm);
  ClassInfo* ci=zb.enterNClass("Project",0,0);
  zb.registerCMethod("openFile",prj_openFile);
  zb.leaveClass();
  Value ev;
  ev.vt=vtNativeObject;
  ev.nobj=vm.allocNObj();
  ev.nobj->classInfo=ci;
  ev.nobj->userData=this;
  zb.registerGlobal("project",ev);

}


void Project::loadScript(FileRegistry::Entry* fe)
{
  natEntry=freg.addEntry(__FILE__,0,0);
  natFr.assignEntry(natEntry);
  scrFr.setOwner(&freg);
  scrFr.assignEntry(fe);
  ZParser p(&vm);
  ZMacroExpander mex;
  mex.init(&vm,&p);
  p.l.macroExpander=&mex;
  p.pushReader(&scrFr);
  p.parse();
  CodeGenerator cg(&vm);
  cg.generate(p.getResult());
  vm.init();
  vm.run();
}

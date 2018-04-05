#include "Editor.hpp"
#include "kst/File.hpp"
#include "ui/UIConfig.hpp"
#include "ui/Window.hpp"
#include "ui/UIRoot.hpp"
#include "Line.hpp"
#include "GLState.hpp"
#include <cctype>
#include "ZorroLexer.hpp"
#include "ColorType.hpp"
#include "ZVMOps.hpp"
#include "ZorroParser.hpp"
#include "Console.hpp"
#include "ZVMHelpers.hpp"
#include "ZBuilder.hpp"
#include "Project.hpp"
#include "ResourcesManager.hpp"
#include <algorithm>
#include "Clipboard.hpp"
#include "FileList.hpp"
#include "ZUI.hpp"

ClassInfo* Editor::classInfo = nullptr;

/*void VirtualFileReader::onEof()
{
  if(loc.line<editor->lines.size()-1 || (loc.line==editor->lines.size()-1 && loc.col<editor->lines.back()->text.length()))
  {
    EditLine& el=*editor->lines[loc.line];
    curLine=el.text;
    if(el.eoln)
    {
      curLine+=(char)(el.eoln&0xff);
    }
    buf=(char*)curLine.c_str();
    loc.offset=0;
    size=curLine.size();
  }
  if(loc.line<editor->lines.size()-1 || (loc.line==editor->lines.size()-1 && loc.col<editor->lines.back()->text.length()))
  {
    EditLine& el=*editor->lines[loc.line];
    curText+=el.text;
    if(el.eoln)
    {
      curText+=(char)(el.eoln&0xff);
    }
    buf=(char*)curText.c_str();
    size=curText.size();
  }
}*/

EditLine::EditLine(const std::string& argText,uint16_t argEoln):text(argText),eoln(argEoln),updated(true)
{
  charsLen=UString::calcLength(text.c_str(),(int)text.length());
}

void EditLine::unpackColors(Text& txt)
{
  ClrVector clr;
  int col=0,oldCol=0,colStep;
  const UString& us=txt.getUString();

  for(CIVector::iterator it=colors.begin(),end=colors.end();it!=end;++it)
  {
    while(col<it->col && col<us.getLength())
    {
      clr.push4(uiConfig.getEditLineFontColor());
      us.getNext(col);
    }
    oldCol=col;
    while(col-oldCol<it->cnt)
    {
      clr.push4(it->clr);
      colStep=col;
      us.getNext(col);
      if(colStep==col)
      {
        fprintf(stderr,"col out of bounds of line?\n");
        break;
      }
    }
  }
  txt.updateColors(clr);
}


Editor::Editor():fr(0),prj(0),editorObject(NilValue)
{
  setName("editor");
  log=kst::Logger::getLogger("editor");
  cmpl=new ListBox;
  cmpl->setEventHandler(lbetSelectionChanged,MKUICALLBACK(cmplSelChanged));
  cmplDisplayed=false;
  skipCmpl=false;
  vSpacing=0;
  topLine=0;
  lineWrap=1;
  selColor=Color::gray;
  cursorVisible=true;
  needColorize=true;
  pageSize=0;
  defaultEol=0x0a;
  curAni.edt=this;
//  root->setKeyboardFocus(this);
  //root->addAnimation(&curAni);
  curPos=CursorPos(0,0);
  forceCursorUpdate=true;
  tabStop=true;
  undoVec.resize(10000);
}

void Editor::open(FileReader* argFR)
{
  fr=argFR;
  std::string line;
  int eoln=defaultEol;
  while(fr && fr->readLine(line,eoln))
  {
    lines.push_back(new EditLine(line,eoln));
  }
  if(lines.empty() || lines.back()->eoln)
  {
    lines.push_back(new EditLine("",eoln));
  }
  //fr=new VirtualFileReader(this,argFR->getEntry());
  colorize();
}

Editor::~Editor()
{
  prj->getZVM().assign(editorObject, NilValue);
  //printf("good bye, editor\n");
}

void Editor::onFocusGain()
{
  root->addAnimation(&curAni);
}

void Editor::onFocusLost()
{
  root->removeAnimation(&curAni);
}


void Editor::updateCursor()
{
  curAni.blinkCount=0;
  if(curPos.x<0)curPos.x=0;
  if(curPos.y<0)curPos.y=0;
  if(curPos.y>=(int)lines.size())curPos.y=lines.size()-1;
  if(curPos.x>lines[curPos.y]->charsLen)curPos.x=lines[curPos.y]->charsLen;
  if(pageSize==0)
  {
    forceCursorUpdate=true;
    return;
  }
  if(curPos.y<topLine)
  {
    topLine=curPos.y;
    forceCursorUpdate=true;
    return;
  }
  if(curPos.y>=topLine+pageSize-1)
  {
    topLine=curPos.y-pageSize+2;
    forceCursorUpdate=true;
    return;
  }
  if(curPos.y-topLine>=(int)dlines.size())
  {
    forceCursorUpdate=true;
    return;
  }
  if(dlines.empty())
  {
    forceCursorUpdate=true;
    return;
  }
  DisplayLine* dl=dlines[curPos.y-topLine];
  if(dl->line!=lines[curPos.y] || dl->line->updated)
  {
    forceCursorUpdate=true;
    return;
  }
  const UString& us=dl->txt.getUString();
  //if(curPos.x>us.getLength())curPos.x=us.getLength();
  Pos pp1,pp2,xt1,xt2;
  FontRef fnt=uiConfig.getEditLineFont();
  int h=fnt->getHeight();
  bool atEol=false;
  //Console* con=root->findByName("console").as<Console>();
  if(curPos.x==us.getLength())
  {
    atEol=true;
    if(curPos.x==0)
    {
      pp1.x=0;
    }else
    {
      dl->txt.getLetterExtent(curPos.x-1,pp1,xt1);
      //if(con)con->addMsg(FORMAT("pp1=%{}, xt1=%{}",pp1,xt1));
      pp1.x+=xt1.x;
    }
  }else
  {
    if(curPos.x>0)
    {
      dl->txt.getLetterExtent(curPos.x,pp2,xt2);
      dl->txt.getLetterExtent(curPos.x-1,pp1,xt1);
      //if(con)con->addMsg(FORMAT("pp1=%{}, xt1=%{}, pp2=%{}, xt2=%{}",pp1,xt1,pp2,xt2));
      if(dl->txt.getLinesCount()==1) // || pp2.y+xt2.y-(pp1.y+xt1.y)<h/2
      {
        int x=int((pp1.x+xt1.x+pp2.x)/2+0.5);
        pp1.x=x;
      }else
      {
        bool atStart=false;
        for(std::vector<int>::const_iterator it=dl->txt.getLinesStart().begin(),end=dl->txt.getLinesStart().end();it!=end;++it)
        {
          if(curPos.x==*it)
          {
            atStart=true;
            break;
          }
        }
        if(atStart)
        {
          pp1.x=0;
        }else
        {
          int x=int((pp1.x+xt1.x+pp2.x)/2);
          pp1.x=x;
        }
      }
    }else{
      pp1.x=0;
    }
  }
  pp1.y=dl->txt.getPosition().y+(atEol?pp1.y:pp2.y);
  pp1.y-=((int)pp1.y)%h;
  cursor.setSource(pp1);
  pp1.y+=h;
  cursor.setDestination(pp1);
}

void Editor::draw()
{
  ScissorsGuard sg(Rect(pos,size));
  RelOffsetGuard rog(pos);
  if(needColorize)
  {
    colorize();
    needColorize=false;
  }
  if(cmplDisplayed)
  {
    cmpl->setPos(getAbsPos()+cursor.getDestination());
    root->moveObjectToFront(cmpl.get());
  }
  int y=0;
  FontRef fnt=uiConfig.getEditLineFont();
  FontRef glowFnt=manager.getFont(fnt->getFileName().c_str(),fnt->getSize(),fnt->getFlags()|ffGlow);

  int lh=fnt->getHeight();
  DLVector::iterator dlit=dlines.begin();
  int lidx=topLine;
  for(LinesVector::iterator it=lines.begin()+topLine,end=lines.end();it!=end;++it,++dlit,++lidx)
  {
    EditLine& el=**it;
    if(dlit==dlines.end())
    {
      dlit=dlines.insert(dlit,new DisplayLine);
      (*dlit)->txt.assignFont(fnt);
      (*dlit)->glow.assignFont(glowFnt);
    }

    DisplayLine& dl=**dlit;
    if(dl.line!=&el || el.updated)
    {
      dl.displayGlow=false;
      dl.line=&el;
      dl.txt.setText(el.text.c_str(),true,lineWrap==0?0:getSize().x,lineWrap==2);
      //dl.glow.setText(el.text.c_str(),true,lineWrap==0?0:getSize().x,lineWrap==2);
      el.unpackColors(dl.txt);
      dl.txt.setPosition(Pos(0,y));
      //dl.glow.setPosition(Pos(0,y));
      el.updated=false;
    }
    for(Highlighting::iterator sit=hl.begin(),send=hl.end();sit!=send;++sit)
    {
      if(lidx>=sit->start.y && lidx<=sit->end.y)
      {
        Pos ps,pe;
        Pos xs,xe;
        if(lidx==sit->start.y)
        {
          dl.txt.getLetterExtent(sit->start.x,ps,xs);
        }else
        {
          ps=Pos(0,0);
        }
        if(dl.txt.getLinesCount()==1)
        {
          if(sit->end.y==lidx )
          {
            if(sit->end.x<dl.txt.getTextLength())
            {
              dl.txt.getLetterExtent(sit->end.x,pe,xe);
            }else
            {
              dl.txt.getLetterExtent(dl.txt.getTextLength()-1,pe,xe);
              pe.x+=xe.x;
            }
          }else
          {
            pe.x=getSize().x;
          }
          Rectangle(Rect(Pos(ps.x,y),Pos(pe.x-ps.x,lh)),selColor,true).draw();
        }else
        {
          if(sit->end.y==lidx )
          {
            std::vector<int> ls=dl.txt.getLinesStart();
            if(sit->start.x<ls.front())
            {
              if(sit->end.x<ls.front())
              {
                dl.txt.getLetterExtent(sit->end.x,pe,xe);
              }else
              {
                pe.x=getSize().x;
              }
              Rectangle(Rect(Pos(ps.x,y),Pos(pe.x-ps.x,lh)),selColor,true).draw();
            }
            ls.push_back(dl.txt.getTextLength());
            int ty=y+lh;
            for(size_t i=0;i<ls.size()-1;++i)
            {
              if(sit->start.x>ls[i+1])
              {
                continue;
              }
              if(sit->end.x<ls[i])
              {
                break;
              }
              if(sit->start.x>=ls[i] && sit->start.x<ls[i+1])
              {
                dl.txt.getLetterExtent(sit->start.x,ps,xs);
              }else
              {
                ps.x=0;
              }
              if(sit->end.x>=ls[i] && sit->end.x<=ls[i+1])
              {
                if(sit->end.x==dl.txt.getTextLength())
                {
                  dl.txt.getLetterExtent(dl.txt.getTextLength()-1,pe,xe);
                  pe.x+=xe.x;
                }else
                {
                  dl.txt.getLetterExtent(sit->end.x,pe,xe);
                }
              }else
              {
                pe.x=getSize().x;
              }
              Rectangle(Rect(Pos(ps.x,ty),Pos(pe.x-ps.x,lh)),selColor,true).draw();
              ps.x=0;
              ty+=lh;
            }
          }else
          {
            if(sit->start.y==lidx)
            {
              const std::vector<int>& ls=dl.txt.getLinesStart();
              int ty=y;
              for(size_t i=0;i<ls.size();++i)
              {
                if(sit->start.x<ls[i])break;
                ty+=lh;
              }
              pe.x=getSize().x;
              Rectangle(Rect(Pos(ps.x,ty),Pos(pe.x-ps.x,lh)),selColor,true).draw();
              ty+=lh;
              Rectangle(Rect(Pos(0,ty),Pos(getSize().x,dl.txt.getHeight()-(ty-y))),selColor,true).draw();
            }else
            {
              Rectangle(Rect(dl.txt.getPosition(),Pos(getSize().x,dl.txt.getHeight())),selColor,true).draw();
            }
          }
        }
      }
    }
    if(dl.displayGlow)dl.glow.draw();
    dl.txt.draw();
    //el.txt.setPosition(Pos(0,y));
    //el.txt.draw();
    y+=dl.txt.getHeight()?dl.txt.getHeight():lh;
    y+=vSpacing;
    if(y>size.y)break;
  }
  if(cursorVisible && isFocused())
  {
    if(forceCursorUpdate)
    {
      forceCursorUpdate=false;
      updateCursor();
    }
    cursor.draw();
  }
  if(showFindPanel)
  {
//    findPanel->draw();
  }
}

void Editor::moveCursor(int dx,int dy)
{
  curPos.x+=dx;
  curPos.y+=dy;
  updateCursor();
}

void Editor::pageDown()
{
  if(topLine+pageSize<(int)lines.size())
  {
    topLine+=pageSize;
    moveCursor(0,pageSize);
  }else
  {
    topLine=lines.size()-pageSize;
    if(topLine<0)
    {
      topLine=0;
    }
    curPos.y=lines.size()-1;
    updateCursor();
  }
}
void Editor::pageUp()
{
  topLine-=pageSize;
  if(topLine<0)topLine=0;
  moveCursor(0,-pageSize);
}

void Editor::onParentResize()
{
  setSize(((Window*)parent)->getClientSize());
}
void Editor::onParentResizeEnd()
{
  setSize(((Window*)parent)->getClientSize());
}

void Editor::onObjectResize()
{
  pageSize=1+size.y/(uiConfig.getEditLineFont()->getHeight()+vSpacing);
  for(DLVector::iterator it=dlines.begin(),end=dlines.end();it!=end;++it)
  {
    (*it)->line->updated=true;
  }
  forceCursorUpdate=true;
}


void Editor::onKeyDown(const KeyboardEvent& ke)
{
  using namespace keyboard;
  ZorroVM& vm=prj->getZVM();
  ZMap& b=*vm.getGlobal("bindings").map;
  std::string kn;
  KeyModifier km[]={GK_MOD_CTRL,GK_MOD_ALT,GK_MOD_SHIFT,GK_MOD_META};
  for(size_t i=0;i<sizeof(km)/sizeof(km[0]);++i)
  {
    if(ke.keyMod&km[i])
    {
      if(!kn.empty())kn+="-";
      kn+=getModName((KeyModifier)(ke.keyMod&km[i]));
    }
  }
  if(!kn.empty())kn+="-";
  kn+=getKeyName(ke.keySym);
  LOGDEBUG(log,"key=%{}, unicode=%{}",kn,ke.unicodeSymbol);
  skipKeyPress=false;
  if(!kn.empty())
  {
    ZStringRef knStr(&vm,kn.c_str());
    ZMap::iterator it=b.find(StringValue(knStr));
    SymInfo* si=nullptr;
    ClassInfo* ci=editorObject.obj->classInfo;
    ZString* mname=nullptr;
    if(it!=b.end())
    {
      if(it->m_value.vt==vtString){
        si=ci->symMap.findSymbol(it->m_value.str);
        mname=it->m_value.str;
      }
    }else
    {
      si=ci->symMap.findSymbol(knStr);
      mname=knStr.get();
    }
    if(si && si->st!=sytMethod)si=nullptr;
    if(si)
    {
      skipKeyPress=true;
      try{
        callMethod(ZStringRef(&vm, mname));
      }catch(std::exception& e)
      {
        LOGWARN(log,"exception:%s",e.what());
        addConsoleMsg(FORMAT("cmd exec exception:%s",e.what()));
      }
    }else
    {
      if(it==b.end())
      {
        LOGWARN(log,"command/binding %{} not found",knStr.c_str());
      }else
      {
        LOGWARN(log,"command %{} not found",ValueToString(&vm,it->m_value));
        addConsoleMsg(FORMAT("cmd not found:%s",ValueToString(&vm,it->m_value)));
      }
    }
  }
}

void Editor::callMethod(ZStringRef name,int args)
{
  ZorroVM& vm=prj->getZVM();
  int cl=vm.ctx.callStack.size();
  int dl=vm.ctx.dataStack.size()-args;
  zorro::OpCall oc(0,OpArg(),OpArg());
  oc.pos=FileLocation(__LINE__,0);
  oc.pos.fileRd=&prj->getNativeFileReader();
  vm.ctx.lastOp=&oc;
  vm.ctx.nextOp=0;
  //vm.pushFrame(&oc,0,args);
  SymInfo* si=editorObject.obj->classInfo->symMap.findSymbol(name);
  if(!si || si->st!=sytMethod)return;
  vm.callMethod(&editorObject, (MethodInfo*)si, args);
  LOGDEBUG(log,"before cl=%{}, dl=%{}",cl,dl);
  try{
    vm.resume();
  }catch(...)
  {
    vm.ctx.callStack.setSize(cl);
    vm.cleanStack(dl);
    throw;
  }
  cl=vm.ctx.callStack.size();
  dl=vm.ctx.dataStack.size()-args;
  LOGDEBUG(log,"after cl=%{}, dl=%{}",cl,dl);
  //vm.ctx.callStack.setSize(cl);
  //vm.cleanStack(dl);
}

void Editor::onKeyPress(const KeyboardEvent& ke)
{
  if(skipKeyPress)
  {
    skipKeyPress=false;
    return;
  }
  ZorroVM& vm=prj->getZVM();


  uint16_t str[1]={ke.unicodeSymbol};
  ZStringRef zs=vm.mkZString(str,1);
  vm.pushValue(StringValue(zs));
  try{
    callMethod(vm.mkZString("input"),1);
  }catch(std::exception& e)
  {
    LOGWARN(log,"exception:%s",e.what());
    Console* con=(Console*)root->findByName("console").get();
    if(con)
    {
      con->addMsg(FORMAT("exception:%s",e.what()));
    }
  }

    /*EditLine& el=*lines[curPos.y];
    el.text.insert(el.text.begin()+curPos.x,(char)ke.unicodeSymbol);
    el.updated=true;
    curPos.x++;
    forceCursorUpdate=true;
    updateCursor();
    colorize();*/
}

bool Editor::CursorAnimation::update(int mcsec)
{
  blinkCount+=mcsec;
  double r=abs(500000-blinkCount%1000000)/500000.0;
  Color c=Color(1,1,1,r);
  edt->cursor.setColors(c,c);
  return true;
}


void Editor::extendSelection(const CursorPos& from,const CursorPos& to)
{
  bool found=false;
  for(Highlighting::iterator it=hl.begin(),end=hl.end();it!=end;++it)
  {
    if(it->ht!=htSelection)continue;
    if(it->start==from)
    {
      it->start=to;
      found=true;
      break;
    }
    if(it->end==from)
    {
      it->end=to;
      found=true;
      break;
    }

  }
  if(!found)
  {
    hl.push_back(HighlightItem(from,to,htSelection));
  }
}

void Editor::save()
{
  if(fr->getEntry()->name.empty())
  {
    return;
  }
  std::string bak=fr->getEntry()->name+".bak";
  if(kst::File::Exists(bak.c_str()))
  {
    kst::File::Unlink(bak.c_str());
  }
  if(kst::File::Exists(fr->getEntry()->name.c_str()))kst::File::Rename(fr->getEntry()->name.c_str(),bak.c_str());
  kst::File f;
  f.WOpen(fr->getEntry()->name.c_str());
  for(LinesVector::iterator it=lines.begin(),end=lines.end();it!=end;++it)
  {
    EditLine& el=**it;
    f.WriteLine(el.text);
    if(el.eoln&0xff00)
    {
      f.WriteByte(el.eoln>>8);
    }
    if(el.eoln&0xff)
    {
      f.WriteByte(el.eoln);
    }
  }
  f.Flush();
  f.Close();
}

void Editor::deleteSelection()
{
  for(Highlighting::reverse_iterator it=hl.rbegin(),end=hl.rend();it!=end;++it)
  {
    if(it->ht!=htSelection)continue;
    curPos=it->start;
    if(it->start.y==it->end.y)
    {
      int startPos=getU8Offset(it->start.y,it->start.x);
      int endPos=getU8Offset(it->start.y,it->end.x);
      pushUndo(UndoItem(utChange,lines[it->start.y]->text,it->start.y));
      lines[it->start.y]->text.erase(startPos,endPos-startPos);
      lines[it->start.y]->updateLen();
      lines[it->start.y]->updated=true;
    }else
    {
      startUndoGroup();
      int startPos=getU8Offset(it->start.y,it->start.x);
      pushUndo(UndoItem(utChange,lines[it->start.y]->text,it->start.y));
      lines[it->start.y]->text.erase(startPos);
      lines[it->start.y]->updateLen();
      lines[it->start.y]->updated=true;
      startPos=getU8Offset(it->end.y,it->end.x);
      pushUndo(UndoItem(utChange,lines[it->end.y]->text,it->end.y));
      lines[it->end.y]->text.erase(0,startPos);
      lines[it->end.y]->updateLen();
      lines[it->end.y]->updated=true;
      lines[it->start.y]->text+=lines[it->end.y]->text;
      lines[it->start.y]->updateLen();
      lines[it->start.y]->updated=true;
      if(it->end.y-it->start.y>0)
      {
        int startLine=it->start.y+1;
        int endLine=it->end.y;
        //int totalLines=lines.size();
        for(int idx=startLine;idx<=endLine;++idx)
        {
          pushUndo(UndoItem(utDeleteLine,lines[idx]->text,startLine));
          delete lines[idx];
        }
        lines.erase(lines.begin()+startLine,lines.begin()+endLine+1);
      }
      endUndoGroup();
    }
  }
  needColorize=true;
  updateCursor();
  clearHighlighting(htSelection);
}

int Editor::getU8Offset(int linesIdx,int symIdx)
{
  std::string& s=lines[linesIdx]->text;
  int rv=0;
  while(symIdx--)
  {
    rv=UString::getNextPos(s.c_str(),rv);
  }
  return rv;
}



static void ze_moveCursor(ZorroVM* vm,Value* obj)
{
  Editor* editor=&obj->nobj->as<Editor>();
  expectArgs(vm,"moveCursor",2);
  int dx=getIntValue(vm,"x",0);
  int dy=getIntValue(vm,"y",1);
  editor->moveCursor(dx,dy);
}

static void ze_extendSelection(ZorroVM* vm,Value* obj)
{
  Editor* editor=&obj->nobj->as<Editor>();
  expectArgs(vm,"extendSelection",4);
  int fx=getIntValue(vm,"fx",0);
  int fy=getIntValue(vm,"fy",1);
  int tx=getIntValue(vm,"tx",2);
  int ty=getIntValue(vm,"ty",3);
  editor->extendSelection(CursorPos(fx,fy),CursorPos(tx,ty));
}

static void ze_clearSelection(ZorroVM* vm,Value* obj)
{
  Editor* editor=&obj->nobj->as<Editor>();
  expectArgs(vm,"clearSelection",0);
  editor->clearHighlighting(Editor::htSelection);
}

static void ze_haveSelection(ZorroVM* vm,Value* obj)
{
  Editor* editor=&obj->nobj->as<Editor>();
  expectArgs(vm,"haveSelection",0);
  bool rv=false;
  for(auto& i:editor->getHighlighting())
  {
    if(i.ht==Editor::htSelection)
    {
      rv=true;
      break;
    }
  }
  vm->setResult(BoolValue(rv));
}

static void ze_delSelection(ZorroVM* vm,Value* obj)
{
  Editor* editor=&obj->nobj->as<Editor>();
  expectArgs(vm,"delSelection",0);
  editor->deleteSelection();
}

static void ze_getSelection(ZorroVM* vm,Value* obj)
{
  Editor* editor=&obj->nobj->as<Editor>();
  expectArgs(vm,"getSelection",0);
  const Editor::Highlighting& sel=editor->getHighlighting();
  if(sel.empty())
  {
    vm->setResult(NilValue);
  }else
  {
    Value rv=ArrayValue(vm->allocZArray());
    ZArray& za=*rv.arr;
    for(Editor::Highlighting::const_iterator it=sel.begin(),end=sel.end();it!=end;++it)
    {
      if(it->ht!=Editor::htSelection)continue;
      Value item=MapValue(vm->allocZMap());
      ZMap& zm=*item.map;
      zm.insert(StringValue(vm->mkZString("startY")),IntValue(it->start.y));
      zm.insert(StringValue(vm->mkZString("startX")),IntValue(it->start.x));
      zm.insert(StringValue(vm->mkZString("endY")),IntValue(it->end.y));
      zm.insert(StringValue(vm->mkZString("endX")),IntValue(it->end.x));
      za.pushAndRef(item);
    }
    if(za.getCount()>0)
    {
      vm->setResult(rv);
    }else
    {
      vm->setResult(NilValue);
      vm->freeZArray(rv.arr);
    }
  }
}

static void ze_getLineWrapCount(ZorroVM* vm,Value* obj)
{
  Editor* editor=&obj->nobj->as<Editor>();
  expectArgs(vm,"getLineWrapCount",1);
  int idx=getIntValue(vm,"idx",0);
  vm->setResult(IntValue(editor->getLineWrapCount(idx)));
}

static void ze_getLineWrapOffsets(ZorroVM* vm,Value* obj)
{
  Editor* editor=&obj->nobj->as<Editor>();
  expectArgs(vm,"getLineWrapOffset",1);
  int idx=getIntValue(vm,"idx",0);
  ZArray* za=vm->allocZArray();
  Value rv=ArrayValue(za);
  int lwc=editor->getLineWrapCount(idx);
  for(int i=0;i<lwc-1;++i)
  {
    za->push(IntValue(editor->getLineWrapOffset(idx,i)));
  }
  vm->setResult(rv);
}


static void ze_pageUp(ZorroVM* vm,Value* obj)
{
  obj->nobj->as<Editor>().pageUp();
}

static void ze_pageDown(ZorroVM* vm,Value* obj)
{
  obj->nobj->as<Editor>().pageDown();
}

static void ze_getTopLine(ZorroVM* vm,Value* obj)
{
  vm->setResult(IntValue(obj->nobj->as<Editor>().getTopLine()));
}


static void ze_setTopLine(ZorroVM* vm,Value* obj)
{
  expectArgs(vm,"setTopLine",1);
  obj->nobj->as<Editor>().setTopLine(getIntValue(vm,"topLine",0));
}

static void ze_getCurLine(ZorroVM* vm,Value* obj)
{
  EditLine& l=obj->nobj->as<Editor>().getLine(obj->nobj->as<Editor>().getCursorPos().y);
  ZStringRef str=vm->mkZString(l.text.c_str(),l.text.length());
  vm->setResult(StringValue(str));
}

static void ze_setCurLine(ZorroVM* vm,Value* obj)
{
  Value v=vm->getLocalValue(0);
  if(v.vt!=vtString)
  {
    throw std::runtime_error("invalid type for curLine property");
  }
  obj->nobj->as<Editor>().setLine(obj->nobj->as<Editor>().getCursorPos().y,ZStringRef(vm,v.str).c_str());
}


static void ze_getCursorX(ZorroVM* vm,Value* obj)
{
  vm->setResult(IntValue(obj->nobj->as<Editor>().getCursorPos().x));
}

static void ze_getCursorY(ZorroVM* vm,Value* obj)
{
  vm->setResult(IntValue(obj->nobj->as<Editor>().getCursorPos().y));
}

static void ze_getLinesCount(ZorroVM* vm,Value* obj)
{
  vm->setResult(IntValue(obj->nobj->as<Editor>().getLines().size()));
}

static void ze_getPageSize(ZorroVM* vm,Value* obj)
{
  vm->setResult(IntValue(obj->nobj->as<Editor>().getPageSize()));
}

static void ze_setCursorX(ZorroVM* vm,Value* obj)
{
  expectArgs(vm,"setCursorX",1);
  int x=getIntValue(vm,"cursorX",0);
  obj->nobj->as<Editor>().setCursorPos(x,obj->nobj->as<Editor>().getCursorPos().y);
}

static void ze_setCursorY(ZorroVM* vm,Value* obj)
{
  expectArgs(vm,"setCursorY",1);
  int y=getIntValue(vm,"cursorY",0);
  obj->nobj->as<Editor>().setCursorPos(obj->nobj->as<Editor>().getCursorPos().x,y);
}

static void ze_deleteLine(ZorroVM* vm,Value* obj)
{
  if(vm->getArgsCount()!=1)
  {
    throw std::runtime_error("deleteLine expected 1 argument of type int");
  }
  obj->nobj->as<Editor>().deleteLine(getIntValue(vm,"idx",0));
}

static void ze_insertLine(ZorroVM* vm,Value* obj)
{
  Value v;
  if(vm->getArgsCount()!=2 || (v=vm->getLocalValue(1)).vt!=vtString)
  {
    throw std::runtime_error("insertLine expected 2 arguments of type int and string");
  }

  obj->nobj->as<Editor>().insertLine(getIntValue(vm,"idx",0),v.str->c_str(vm));
}

static void ze_addGhostText(ZorroVM* vm,Value* obj)
{
  Value v;
  if(vm->getArgsCount()!=3 || (v=vm->getLocalValue(2)).vt!=vtString)
  {
    throw std::runtime_error("insertLine expected 3 arguments of type int,int and string");
  }

  obj->nobj->as<Editor>().addGhostText(getIntValue(vm,"line",0),getIntValue(vm,"col",1),v.str->c_str(vm));
}

static void ze_removeGhostText(ZorroVM* vm,Value* obj)
{
  obj->nobj->as<Editor>().removeGhostText();
}


static void ze_getLine(ZorroVM* vm,Value* obj)
{
  int idx=getIntValue(vm,"idx",0);
  EditLine& l=obj->nobj->as<Editor>().getLine(idx);
  ZStringRef str=vm->mkZString(l.text.c_str(),l.text.length());
  vm->setResult(StringValue(str));
}

static void ze_setLine(ZorroVM* vm,Value* obj)
{
  expectArgs(vm,"setLine",2);
  int idx=getIntValue(vm,"idx",0);
  std::string val=getStrValue(vm,"val",1);
  obj->nobj->as<Editor>().setLine(idx,val);
}


static void ze_getLineEoln(ZorroVM* vm,Value* obj)
{
  int idx=getIntValue(vm,"idx",0);
  EditLine& l=obj->nobj->as<Editor>().getLine(idx);
  char eoln[3]={0,};
  if(l.eoln>0xff)
  {
    eoln[0]=l.eoln>>8;
    eoln[1]=l.eoln&0xff;
  }else
  {
    eoln[0]=l.eoln;
  }
  ZStringRef str=vm->mkZString(eoln);
  vm->setResult(StringValue(str));
}


static void ze_save(ZorroVM* vm,Value* obj)
{
  expectArgs(vm,"save",0);
  obj->nobj->as<Editor>().save();
}

static void ze_showCmpl(ZorroVM* vm,Value* obj)
{
  expectArgs(vm,"showCmpl",0);
  obj->nobj->as<Editor>().showCompletions();
}

static void ze_hideCmpl(ZorroVM* vm,Value* obj)
{
  expectArgs(vm,"hideCmpl",0);
  obj->nobj->as<Editor>().hideCompletions();
}

static void ze_clearCmpl(ZorroVM* vm,Value* obj)
{
  expectArgs(vm,"clearCmpl",0);
  obj->nobj->as<Editor>().clearCompletions();
}

static void ze_addCmpl(ZorroVM* vm,Value* obj)
{
  expectArgs(vm,"addCmpl",1);
  Label* lb=new Label(getStrValue(vm,"cmpl",0).c_str());
  lb->setColor(Color::black);
  obj->nobj->as<Editor>().addToCompletions(lb);
}

static void ze_nextCmpl(ZorroVM* vm,Value* obj)
{
  expectArgs(vm,"nextCmpl",0);
  vm->setResult(BoolValue(obj->nobj->as<Editor>().nextCompletion()));
}

static void ze_prevCmpl(ZorroVM* vm,Value* obj)
{
  expectArgs(vm,"prevCmpl",0);
  vm->setResult(BoolValue(obj->nobj->as<Editor>().prevCompletion()));
}

static void ze_setClipboard(ZorroVM* vm,Value* obj)
{
  expectArgs(vm,"setClipboard",1);
  copyToClipboard(getStrValue(vm,"txt",0).c_str());
}

static void ze_getClipboard(ZorroVM* vm,Value* obj)
{
  std::string txt=pasteFromClipboard();
  vm->setResult(StringValue(vm->mkZString(txt.c_str(),txt.length())));
}

static void ze_undo(ZorroVM* vm,Value* obj)
{
  expectArgs(vm,"undo",0,1);
  bool soft=false;
  if(vm->getArgsCount()==1)
  {
    Value& v=vm->getLocalValue(0);
    soft=vm->boolOps[v.vt](vm,&v);
  }
  obj->nobj->as<Editor>().undo(soft);
}

static void ze_startUndoGroup(ZorroVM* vm,Value* obj)
{
  obj->nobj->as<Editor>().startUndoGroup();
}

static void ze_endUndoGroup(ZorroVM* vm,Value* obj)
{
  obj->nobj->as<Editor>().endUndoGroup();
}

static void ze_focusFilter(ZorroVM* vm,Value* obj)
{
  obj->nobj->as<Editor>().getProject().getFileList()->setFocus();
}

static void ze_getFileName(ZorroVM* vm,Value* obj)
{
  vm->setResult(StringValue(vm->mkZString(obj->nobj->as<Editor>().getFilename().c_str())));
}

static void ze_showFind(ZorroVM* vm,Value* obj)
{
  obj->nobj->as<Editor>().showFind();
}

static void ze_findNext(ZorroVM* vm,Value* obj)
{
  obj->nobj->as<Editor>().findNext();
}

static void ze_findBack(ZorroVM* vm,Value* obj)
{
  obj->nobj->as<Editor>().findBack();
}

void Editor::registerInZVM(ZorroVM& vm)
{
  registerZUI(&vm);
  ZBuilder zb(&vm);
  ClassInfo* ci=zb.enterNClass("NativeEditor",0,0);
  classInfo=ci;
  zb.registerCMethod("moveCursor",ze_moveCursor);
  zb.registerCMethod("extendSelection",ze_extendSelection);
  zb.registerCMethod("clearSelection",ze_clearSelection);
  zb.registerCMethod("haveSelection",ze_haveSelection);
  zb.registerCMethod("delSelection",ze_delSelection);
  zb.registerCMethod("getSelection",ze_getSelection);
  zb.registerCMethod("getLineWrapCount",ze_getLineWrapCount);
  zb.registerCMethod("getLineWrapOffsets",ze_getLineWrapOffsets);
  zb.registerCMethod("pageUp",ze_pageUp);
  zb.registerCMethod("pageDown",ze_pageDown);
  zb.registerCMethod("deleteLine",ze_deleteLine);
  zb.registerCMethod("insertLine",ze_insertLine);
  zb.registerCMethod("addGhostText",ze_addGhostText);
  zb.registerCMethod("removeGhostText",ze_removeGhostText);
  zb.registerCMethod("getLine",ze_getLine);
  zb.registerCMethod("setLine",ze_setLine);
  zb.registerCMethod("getLineEoln",ze_getLineEoln);
  zb.registerCProperty("topLine",ze_getTopLine,ze_setTopLine);
  zb.registerCProperty("curLine",ze_getCurLine,ze_setCurLine);
  zb.registerCProperty("linesCount",ze_getLinesCount,0);
  zb.registerCProperty("pageSize",ze_getPageSize,0);
  zb.registerCProperty("cursorX",ze_getCursorX,ze_setCursorX);
  zb.registerCProperty("cursorY",ze_getCursorY,ze_setCursorY);
  zb.registerCMethod("save",ze_save);
  zb.registerCMethod("showCmpl",ze_showCmpl);
  zb.registerCMethod("hideCmpl",ze_hideCmpl);
  zb.registerCMethod("clearCmpl",ze_clearCmpl);
  zb.registerCMethod("addCmpl",ze_addCmpl);
  zb.registerCMethod("nextCmpl",ze_nextCmpl);
  zb.registerCMethod("prevCmpl",ze_prevCmpl);
  zb.registerCMethod("setClipboard",ze_setClipboard);
  zb.registerCMethod("getClipboard",ze_getClipboard);
  zb.registerCMethod("undo",ze_undo);
  zb.registerCMethod("startUndoGroup",ze_startUndoGroup);
  zb.registerCMethod("endUndoGroup",ze_endUndoGroup);
  zb.registerCMethod("focusFilter",ze_focusFilter);
  zb.registerCProperty("fileName", ze_getFileName, nullptr);
  zb.registerCMethod("showFind",ze_showFind);
  zb.registerCMethod("findNext",ze_findNext);
  zb.registerCMethod("findBack",ze_findBack);
  zb.leaveClass();
  zb.registerGlobal("osType",StringValue(vm.mkZString(
#if defined( _WINDOWS_)
  "windows"
#elif defined(__APPLE__)
  "macos"
#elif defined(linux)
  "linux"
#else
  "unknown"
#endif
  )));
}

/*
  Value ev;
  ev.vt=vtNativeObject;
  ev.nobj=vm.allocNObj();
  ev.nobj->classInfo=ci;
  ev.nobj->userData=this;
  zb.registerGlobal("editor",ev);

 */

void Editor::cmplSelChanged(const UIEvent& evt)
{
  if(skipCmpl)
  {
    skipCmpl=false;
    return;
  }
  if(!cmpl->getSelection())
  {
    return;
  }
  ZorroVM& vm=prj->getZVM();
  vm.pushValue(IntValue(cmpl->getSelection()->getIntTag()));
  try{
    callMethod(vm.mkZString("complete"),1);
  }catch(std::exception& e)
  {
    LOGWARN(log,"exception:%s",e.what());
    Console* con=(Console*)root->findByName("console").get();
    if(con)
    {
      con->addMsg(FORMAT("exception:%s",e.what()));
    }
  }
  setFocus();
}

void Editor::setProject(Project* argProject, const Value& argEditorObj)
{
  prj=argProject;
  prj->getZVM().assign(editorObject, argEditorObj);
}


class MaxWidthCalc:public UIContainerEnumerator{
public:
  int maxWidth=0;
  bool nextItem(UIObject* obj)
  {
    if(obj->getSize().x>maxWidth)
    {
      maxWidth=obj->getSize().x;
    }
    return true;
  }
};

void Editor::showCompletions()
{
  MaxWidthCalc mwc;
  cmpl->enumerate(&mwc);
  cmpl->setSize(Pos(mwc.maxWidth+uiConfig.getConst("scrollBarWidth"),uiConfig.getLabelFont()->getHeight()*std::min((size_t)8,cmpl->getCount())));
  cmpl->setPos(getAbsPos()+cursor.getDestination());
  skipCmpl=true;
  cmpl->setSelection(0);
  if(!cmplDisplayed)root->addObject(cmpl.get());
  root->moveObjectToFront(cmpl.get());
  cmplDisplayed=true;
}

void Editor::clearCompletions()
{
  cmpl->clear();
}

void Editor::addToCompletions(UIObjectRef obj)
{
  obj->setIntTag(cmpl->getCount());
  cmpl->addObject(obj.get());
}

void Editor::hideCompletions()
{
  root->removeObject(cmpl.get());
  cmplDisplayed=false;
}


bool Editor::nextCompletion()
{
  if(cmpl->getCount()==0 || !cmpl->getSelection() || cmpl->getCount()==cmpl->getSelection()->getIntTag()+1)
  {
    return false;
  }
  skipCmpl=true;
  cmpl->setSelection(cmpl->getSelection()->getIntTag()+1);
  return true;
}

bool Editor::prevCompletion()
{
  if(cmpl->getCount()==0 || !cmpl->getSelection() || cmpl->getSelection()->getIntTag()==0)
  {
    return false;
  }
  skipCmpl=true;
  cmpl->setSelection(cmpl->getSelection()->getIntTag()-1);
  return true;
}

/*void Editor::copy()
{
  if(selection.empty())
  {
    return;
  }
  std::string value;
  for(auto sel:selection)
  {
    if(sel.start.y==sel.end.y)
    {
      value+=lines[sel.start.y]->text.substr(sel.start.x,sel.end.x-sel.start.x);
    }else
    {
      value+=lines[sel.start.y]->text.substr(sel.start.x);
      for(int y=sel.start.y+1;y<sel.end.y;++y)
      {
        value+=lines[y]->text;
      }
      value+=lines[sel.end.y]->text.substr(0,sel.end.y);
    }
  }
  copyToClipboard(value.c_str());
}

void Editor::paste()
{
  std::string value=pasteFromClipboard();
  std::string::size_type pos=0;

}*/

void Editor::addConsoleMsg(const std::string& msg)
{
  Console* con=root->findByName("console").as<Console>();
  if(con)
  {
    con->addMsg(msg.c_str());
  }
}

void Editor::pushUndo(const UndoItem& argUi)
{
  UndoItem& last=lastUndo();
  if(last.ut==utChange && argUi.ut==utChange && last.line==argUi.line && last.text==argUi.text)
  {
    return;
  }
  UndoItem* ui;
  undoVec[undoIdx]=argUi;
  ui=&undoVec[undoIdx];
  undoIdx=(undoIdx+1)%undoVec.size();
  undoVec[undoIdx].ut=utNone;
  ui->pos=curPos;
  if(ui->ut==utChange)
  {
    if(last.ut==utChange && last.line==ui->line && last.pos.x<ui->pos.x)
    {
      ui->seq=undoSeq;
    }else
    {
      ui->seq=undoGroup?undoSeq:++undoSeq;
    }
  }else
  {
    ui->seq=undoGroup?undoSeq:++undoSeq;
  }
}


void Editor::undo(bool soft)
{
  if(undoVec.empty())return;
  UndoItem* ui;
  bool haveSeq=false;
  size_t seq;
  for(;;)
  {
    size_t saveUndoIdx=undoIdx;
    undoIdx=(undoIdx-1+undoVec.size())%undoVec.size();
    ui=&undoVec[undoIdx];
    if(ui->ut==utNone)
    {
      undoIdx=saveUndoIdx;
      return;
    }
    if(haveSeq)
    {
      if(ui->seq!=seq)
      {
        undoIdx=saveUndoIdx;
        return;
      }
    }else
    {
      seq=ui->seq;
      haveSeq=true;
    }
    switch(ui->ut)
    {
      case utNone:break;
      case utChange:
      {
        addConsoleMsg("undo change");
        curPos=ui->pos;
        lines[ui->line]->text=ui->text;
        lines[ui->line]->updated=true;
        lines[ui->line]->updateLen();
      }break;
      case utDeleteLine:
      {
        addConsoleMsg("undo del line");
        curPos=ui->pos;
        lines.insert(lines.begin()+ui->line,new EditLine(ui->text,defaultEol));
      }break;
      case utInsertLine:
      {
        addConsoleMsg("undo ins line");
        curPos=ui->pos;
        delete lines[ui->line];
        lines.erase(lines.begin()+ui->line);
      }break;
    }
    updateCursor();
    needColorize=true;
    if(soft)break;
  }
}

CursorPos Editor::convertMouseToCurPos(int mx,int my)
{
  Pos apos=getAbsPos();
  mx-=apos.x;
  my-=apos.y;
  int y=0;
  CursorPos rv(0,topLine);
  DisplayLine* dl=0;
  FontRef fnt=uiConfig.getEditLineFont();
  int dlh=fnt->getHeight();


  for(auto& l:dlines)
  {
    ++rv.y;
    int lh=l->txt.getHeight();
    lh=lh?lh:dlh;
    if(my>=y && my<y+lh)
    {
      --rv.y;
      dl=l;
      break;
    }
    y+=lh;
  }
  if(!dl)
  {
    rv.y=lines.size()-1;
    dl=dlines.back();
  }
  rv.x=dl->txt.getTextLength();
  for(int i=0;i<rv.x;++i)
  {
    Pos lpos,lsize;
    dl->txt.getLetterExtent(i,lpos,lsize);
    if(mx>=lpos.x && mx<=lpos.x+lsize.x)
    {
      rv.x=i+(mx>lpos.x+lsize.x/2?1:0);
      break;
    }
    if(mx<lpos.x)
    {
      rv.x=i-1;
      break;
    }
  }
  return rv;
}

void Editor::onMouseButtonDown(const MouseEvent& me)
{
  clearHighlighting(htSelection);
  selectionStart=convertMouseToCurPos(me.x,me.y);
  setCursorPos(selectionStart.x,selectionStart.y);
  UIObject::onMouseButtonDown(me);
  selecting=true;
}

void Editor::onMouseButtonUp(const MouseEvent& me)
{
  selecting=false;
  UIObject::onMouseButtonUp(me);
}

void Editor::onMouseMove(const MouseEvent& me)
{
  if(selecting)
  {
    clearHighlighting(htSelection);
    CursorPos to=convertMouseToCurPos(me.x,me.y);
    setCursorPos(to.x,to.y);
    CursorPos from=selectionStart;
    if(to<from)
    {
      std::swap(to,from);
    }
    hl.push_back(HighlightItem(from,to,htSelection));
  }
  UIObject::onMouseMove(me);
}

void Editor::onMouseScroll(const MouseEvent& me)
{
  setTopLine(topLine+(me.yRel<0?1:-1));
  if(curPos.y<topLine)++curPos.y;
  if(curPos.y>=topLine+pageSize-1)
  {
    --curPos.y;
  }
  updateCursor();
  UIObject::onMouseScroll(me);
}


void Editor::addGhostText(int line,int col,const std::string& txt)
{
  LOGDEBUG(log,"add ghost text at %{}, %{}:%{}",line,col,txt);
  line-=topLine;
  if(line<0 || line>=dlines.size())
  {
    return;
  }
  DisplayLine& dl=*dlines[line];
  ClrVector clrs=dl.txt.getColors();
  std::string tmpStr=dl.line->text;
  tmpStr.insert(col,txt);
  for(size_t i=0;i<txt.length();++i)
  {
   clrs.insert(clrs.begin()+col*4,4,uiConfig.getEditLineFontColor());
  }
  dl.txt.setText(tmpStr.c_str(),true);
  dl.txt.updateColors(clrs);
  dl.glow.setPosition(dl.txt.getPosition());
  std::string glow(col,' ');
  glow+=txt;
  glow.append(txt.length()+col,' ');
  dl.displayGlow=true;
  dl.glow.setText(glow.c_str(),true);
}


void Editor::findNext(const std::string& str)
{
  if(str.empty())
  {
    return;
  }
  else 
  {
    lastFindString=str;
  }
  int findLine = this->curPos.y;
  int findPos = this->curPos.x + 1;
  while(findLine<lines.size())
  {
    std::string::size_type pos = lines[findLine]->text.find(lastFindString, findPos);
    if(pos!=std::string::npos)
    {
      setCursorPos(pos, findLine);
      hideFind();
      return;
    }
    ++findLine;
    findPos = 0;
  }
}

void Editor::findBack()
{
  int findLine = this->curPos.y;
  int findPos = this->curPos.x - 1;
  if(findPos<0)findPos=0;
  while (findLine >= 0)
  {
    std::string::size_type pos = lines[findLine]->text.rfind(lastFindString, findPos);
    if (pos != std::string::npos)
    {
      setCursorPos(pos, findLine);
      hideFind();
      return;
    }
    --findLine;
    if(findLine>=0)
    {
      findPos = lines[findLine]->text.length();
    }
  }
}

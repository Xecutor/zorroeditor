#ifndef __ZE_EDITOR_HPP__
#define __ZE_EDITOR_HPP__
#define _DLGS_H

#include "UIObject.hpp"
#include "Text.hpp"
#include <vector>
#include <string>
#include "FileReader.hpp"
#include "Line.hpp"
#include "UIAnimation.hpp"
#include "ZorroVM.hpp"
#include <kst/Logger.hpp>
#include "ZIndexer.hpp"
#include "ListBox.hpp"
#include "FindPanel.hpp"

using namespace glider;
using namespace glider::ui;
using namespace zorro;

struct ColorInfo{
  enum{
    flUnderline=1,
    flGlow=2
  };
  Color clr;
  int col;
  int cnt;
  int flags;
  ColorInfo(const Color& argClr,int argCol,int argCnt,int argFlags=0):clr(argClr),col(argCol),cnt(argCnt),flags(argFlags){}
};

typedef std::vector<ColorInfo> CIVector;

struct EditLine{
  std::string text;
  size_t charsLen;
  CIVector colors;
  uint16_t eoln;
  bool updated;
  void updateLen()
  {
    charsLen=UString::calcLength(text.c_str(),(int)text.length());
  }
  EditLine(const std::string& argText,uint16_t argEoln);
  void unpackColors(Text& txt);
};
typedef std::vector<EditLine*> LinesVector;

class Editor;

/*class VirtualFileReader:public FileReader{
public:
  VirtualFileReader(Editor* argEditor,FileRegistry::Entry* argEntry):FileReader(0,argEntry),editor(argEditor)
  {
    size=0;
    buf=0;
    loc.fileRd=this;
    loc.offset=0;
    loc.line=0;
    loc.col=0;
  }
  virtual ~VirtualFileReader(){}
  virtual void reset()
  {
    FileReader::reset();
    curText.clear();
    size=0;
  }
protected:
  Editor* editor;
  std::string curText;
  void onEof();
};*/

struct CursorPos:Posi<int>{
  CursorPos(){}
  CursorPos(int argX,int argY):Posi<int>(argX,argY){}
  bool operator<(const CursorPos& other)const
  {
    return y<other.y || (y==other.y && x<other.x);
  }
  bool operator>(const CursorPos& other)const
  {
    return y>other.y || (y==other.y && x>other.x);
  }
  bool operator==(const CursorPos& other)const
  {
    return x==other.x && y==other.y;
  }
};

class Project;

class Editor:public UIObject{
public:
  Editor();
  ~Editor();

  void loadScript(FileRegistry::Entry* se);

  void open(FileReader* argFR);

  void setProject(Project* argProject, const Value& argEditorObj);

  void onFocusGain();

  void onFocusLost();


  void draw();
  void moveCursor(int dx,int dy);
  void pageDown();
  void pageUp();
  int getTopLine()const
  {
    return topLine;
  }
  void setTopLine(int argTopLine)
  {
    topLine=argTopLine;
    if(topLine<0)topLine=0;
    if(topLine>=(int)lines.size())topLine=lines.empty()?0:lines.size()-1;
  }
  EditLine& getLine(int idx)
  {
    if(idx<0 || idx>=(int)lines.size())
    {
      throw std::runtime_error("getLine:line num is out of bounds");
    }
    return *lines[idx];
  }
  int getLineWrapCount(int idx)
  {
    if(idx<topLine || idx>=topLine+(int)dlines.size() || idx>(int)lines.size())
    {
      return 0;
    }
    EditLine* el=lines[idx];
    idx-=topLine;
    if(dlines[idx]->line!=el)
    {
      return 0;
    }
    return dlines[idx]->txt.getLinesCount();
  }
  int getLineWrapOffset(int lidx,int oidx)
  {
    if(lidx<topLine || lidx>=topLine+dlines.size() || lidx>(int)lines.size())
    {
      return 0;
    }
    EditLine* el=lines[lidx];
    lidx-=topLine;
    if(dlines[lidx]->line!=el)
    {
      return 0;
    }
    if(oidx<0 || oidx>=dlines[lidx]->txt.getLinesCount()-1)
    {
      return 0;
    }
    return dlines[lidx]->txt.getLinesStart()[oidx];
  }
  void setLine(int idx,const std::string& str)
  {
    if(idx<0 || idx>=(int)lines.size())
    {
      throw std::runtime_error("setLine:line num is out of bounds");
    }
    pushUndo(UndoItem(utChange,lines[idx]->text,idx));
    lines[idx]->text=str;
    lines[idx]->updateLen();
    lines[idx]->updated=true;
    needColorize=true;
    forceCursorUpdate=true;
  }
  void deleteLine(int idx)
  {
    if(idx<0 || idx>=(int)lines.size())
    {
      throw std::runtime_error("deleteLine:line num is out of bounds");
    }
    LinesVector::iterator it=lines.begin()+idx;
    pushUndo(UndoItem(utDeleteLine,(*it)->text,idx));
    delete *it;
    lines.erase(it);
    if(lines.empty())
    {
      lines.push_back(new EditLine("",0));
    }
    needColorize=true;
    forceCursorUpdate=true;
  }
  void insertLine(int idx,const std::string& str)
  {
    if(idx<0 || idx>(int)lines.size())
    {
      throw std::runtime_error("insertLine:line num is out of bounds");
    }
    pushUndo(UndoItem(utInsertLine,idx));
    if(idx==(int)lines.size() && lines.back()->eoln==0)
    {
      lines.back()->eoln=defaultEol;
    }
    lines.insert(lines.begin()+idx,new EditLine(str,defaultEol));
    needColorize=true;
    forceCursorUpdate=true;
  }
  CursorPos getCursorPos()const
  {
    return curPos;
  }
  void setCursorPos(int x,int y)
  {
    if(x<0)x=0;
    if(y<0)y=0;
    if(y>=(int)lines.size())y=lines.size()-1;
    curPos.x=x;
    curPos.y=y;
    updateCursor();
  }
  LinesVector& getLines()
  {
    return lines;
  }
  int getPageSize()
  {
    return pageSize;
  }
  enum HighlightType{
    htSelection,
    htPlaceholder
  };
  struct HighlightItem{
    HighlightItem():ht(htSelection){}
    HighlightItem(const CursorPos& argStart,const CursorPos& argEnd,HighlightType argHt):ht(argHt),start(argStart),end(argEnd)
    {
      if(start>end)
      {
        CursorPos tmp=start;
        start=end;
        end=tmp;
      }
    }
    HighlightType ht;
    CursorPos start,end;
    bool isInside(const CursorPos& pos)
    {
      return pos>start && pos<end;
    }
  };
  typedef std::vector<HighlightItem> Highlighting;
  const Highlighting& getHighlighting()const
  {
    return hl;
  }
  void clearHighlighting(HighlightType ht)
  {
    auto it=std::remove_if(hl.begin(),hl.end(),[=](const HighlightItem& item){return item.ht==ht;});
    hl.erase(it,hl.end());
  }
  void deleteSelection();
  void extendSelection(const CursorPos& from,const CursorPos& to);
  void save();

  const std::string& getFilename()const
  {
    return fr->getEntry()->name;
  }

  Project& getProject()
  {
    return *prj;
  }

  static void registerInZVM(ZorroVM& vm);
  static ClassInfo* getClassInfo()
  {
    return classInfo;
  }

  void showCompletions();
  void clearCompletions();
  void addToCompletions(UIObjectRef obj);
  void hideCompletions();
  bool nextCompletion();
  bool prevCompletion();


  void addConsoleMsg(const std::string& msg);

  void undo(bool soft=false);
  void startUndoGroup()
  {
    ++undoGroup;
    ++undoSeq;
  }
  void endUndoGroup()
  {
    --undoGroup;
  }

  void addGhostText(int line,int col,const std::string& txt);
  void removeGhostText()
  {
    for(auto dl:dlines)
    {
      dl->line->updated=true;
    }
  }

  void showFind()
  {
    if(!findPanel.get())
    {
      findPanel=new FindPanel(this);
    }
    findPanel->setFocus();
    ((UIContainer*)parent)->addObject(findPanel.get());
    showFindPanel=true;
  }
  void findNext(const std::string& str=std::string());
  void findBack();
  void hideFind()
  {
    ((UIContainer*)parent)->removeObject(findPanel.get());
    findPanel->removeFocus();
    showFindPanel=false;
    setFocus();
  }

protected:
  LinesVector lines;
  int topLine;
  int pageSize;
  int vSpacing;
  FileReader* fr;
  CursorPos curPos;
  Line cursor;
  bool cursorVisible;
  bool forceCursorUpdate;
  bool needColorize;
  kst::Logger* log;
  int defaultEol;
  bool skipKeyPress=false;

  static ClassInfo* classInfo;

  void callMethod(ZStringRef name,int args=0);

  Highlighting hl;

  int getU8Offset(int linesIdx,int symIdx);

  //0 = none
  //1 = linewrap
  //2 = word wrap
  int lineWrap;
  Color selColor;

  Project* prj;
  Value editorObject;

  void onParentResize();
  void onParentResizeEnd();
  void onObjectResize();
  virtual void onMouseButtonDown(const MouseEvent& me) override;
  virtual void onMouseMove(const MouseEvent& me) override;
  virtual void onMouseButtonUp(const MouseEvent& me) override;
  virtual void onMouseScroll(const MouseEvent& me) override;

  virtual void colorize(){}

  struct CursorAnimation:public UIAnimation{
    int blinkCount;
    Editor* edt;
    CursorAnimation():blinkCount(0),edt(0){}
    bool update(int mcsec);
    virtual bool deleteOnFinish()
    {
      return false;
    }
  };

  CursorAnimation curAni;

  struct DisplayLine{
    Text txt;
    Text glow;
    EditLine* line;
    bool displayGlow;
    DisplayLine():line(0),displayGlow(false){}
  };
  typedef std::vector<DisplayLine*> DLVector;
  DLVector dlines;


  ListBoxRef cmpl;
  bool cmplDisplayed;
  bool skipCmpl;
  void cmplSelChanged(const UIEvent& evt);

  void updateCursor();
  void onKeyDown(const KeyboardEvent& ke);
  void onKeyPress(const KeyboardEvent& ke);
  //friend class VirtualFileReader;

  CursorPos convertMouseToCurPos(int mx,int my);
  bool selecting=false;
  CursorPos selectionStart;

  ReferenceTmpl<FindPanel> findPanel = nullptr;
  std::string lastFindString;
  bool showFindPanel = false;

  enum UndoType{
    utNone,
    utChange,
    utDeleteLine,
    utInsertLine
  };
  struct UndoItem{
    UndoType ut;
    std::string text;
    int line;
    CursorPos pos;
    size_t seq;
    UndoItem():ut(utNone),line(0),seq(0){}
    UndoItem(UndoType argUt,int argLine):ut(argUt),line(argLine),seq(0){}
    UndoItem(UndoType argUt,const std::string& argText,int argLine):ut(argUt),text(argText),line(argLine),seq(0){}
  };
  typedef std::vector<UndoItem> UndoVector;
  UndoVector undoVec;
  size_t undoIdx=0;
  size_t redoUdx=0;
  size_t undoSeq=0;
  size_t undoGroup=0;
  void pushUndo(const UndoItem& ui);
  UndoItem& lastUndo()
  {
    return undoVec[(undoIdx-1+undoVec.size())%undoVec.size()];
  }
};

#endif

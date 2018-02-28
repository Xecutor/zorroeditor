#ifndef __ZE_ZEDITOR_HPP__
#define __ZE_ZEDITOR_HPP__

#include "Editor.hpp"


class ZEditor:public Editor{
public:
  ZEditor(ZIndexer* argIDx);
  void colorize();
  void updateReader();
  ScopeSym* getCurContext()
  {
    return idx->getContext(fr,curPos.y,curPos.x);
  }

  ZIndexer& getIndex()
  {
    return *idx;
  }

  Term getLastToken(const FileLocation& startPos,int line,int col,bool& lastEol);

  void indexate()
  {
    updateReader();
    idx->indexate(fr);
  }

  static void registerInZVMEx(ZorroVM& vm);

protected:


  ZIndexer* idx;


};

#endif

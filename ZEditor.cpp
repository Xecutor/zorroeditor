#include "ZEditor.hpp"
#include "ZVMHelpers.hpp"
#include "ColorType.hpp"
#include "ZBuilder.hpp"

ColorType getTermColorType(Term t)
{
  switch(t.tt)
  {
    case tInt:return ctInt;
    case tDouble:return ctDouble;
    case tString:return ctString;
    case tRawString:return ctString;
    case tNil:return ctKeyword;
    case tTrue:return ctBool;
    case tFalse:return ctBool;
    case tIdent:return ctIdent;
    case tPlus:return ctSymbol;
    case tPlusPlus:return ctSymbol;
    case tPlusEq:return ctSymbol;
    case tMinus:return ctSymbol;
    case tMinusMinus:return ctSymbol;
    case tMinusEq:return ctSymbol;
    case tMul:return ctSymbol;
    case tMulEq:return ctSymbol;
    case tDiv:return ctSymbol;
    case tDivEq:return ctSymbol;
    case tAmp:return ctSymbol;
    case tDoubleAmp:return ctSymbol;
    case tMod:return ctSymbol;
    case tModEq:return ctSymbol;
    case tLess:return ctSymbol;
    case tGreater:return ctSymbol;
    case tLessEq:return ctSymbol;
    case tGreaterEq:return ctSymbol;
    case tEqual:return ctSymbol;
    case tNotEqual:return ctSymbol;
    case tMatch:return ctSymbol;
    case tAnd:return ctKeyword;
    case tOr:return ctKeyword;
    case tNot:return ctKeyword;
    case tORBr:return ctSymbol;
    case tCRBr:return ctSymbol;
    case tOSBr:return ctSymbol;
    case tCSBr:return ctSymbol;
    case tOCBr:return ctSymbol;
    case tCCBr:return ctSymbol;
    case tDot:return ctSymbol;
    case tDotStar:return ctSymbol;
    case tDoubleDot:return ctSymbol;
    case tDoubleDotLess:return ctSymbol;
    case tColon:return ctSymbol;
    case tDoubleColon:return ctSymbol;
    case tStarColon:return ctSymbol;
    case tComma:return ctSymbol;
    case tArrow:return ctSymbol;
    case tThinArrow:return ctSymbol;
    case tEq:return ctSymbol;
    case tAt:return ctSymbol;
    case tFormat:return ctSymbol;
    case tExclSign:return ctSymbol;
    case tQuestMark:return ctSymbol;
    case tHash:return ctSymbol;
    case tPipe:return ctSymbol;
    case tEol:return ctSymbol;
    case tEof:return ctSymbol;
    case tFunc:return ctKeyword;
    case tEnd:return ctKeyword;
    case tReturn:return ctKeyword;
    case tYield:return ctKeyword;
    case tIf:return ctKeyword;
    case tElse:return ctKeyword;
    case tElsif:return ctKeyword;
    case tWhile:return ctKeyword;
    case tFor:return ctKeyword;
    case tIn:return ctKeyword;
    case tIs:return ctKeyword;
    case tOn:return ctKeyword;
    case tClass:return ctKeyword;
    case tNamespace:return ctKeyword;
    case tTry:return ctKeyword;
    case tCatch:return ctKeyword;
    case tThrow:return ctKeyword;
    case tNext:return ctKeyword;
    case tBreak:return ctKeyword;
    case tRedo:return ctKeyword;
    case tSwitch:return ctKeyword;
    case tEnum:return ctKeyword;
    case tBitEnum:return ctKeyword;
    case tInclude:return ctKeyword;
    case tProp:return ctKeyword;
    case tAttr:return ctKeyword;
    case tAttrGet:return ctKeyword;
    case tUse:return ctKeyword;
    case tNumArg:return ctIdent;
    case tCarret:return ctSymbol;
    case tMapOp:return ctKeyword;
    case tGrepOp:return ctKeyword;
    //case tSumOp:return ctKeyword;
    case tRegExp:return ctRegExp;
    case tMacro:return ctKeyword;
    case tMacroExpand:return ctIdent;
    case tComment:return ctComment;
  }
  return ctIdent;
}

static Color getSymColor(SymInfo* si,const Color& def)
{
  if(si)
  {
    switch(si->st)
    {
      case sytClosedVar:return Color(0.8,0.3,0.4);
      case sytLocalVar:return Color(0.8,0.6,0.4);
      case sytGlobalVar:return Color(0.6,0.8,0.4);
      case sytFunction:return Color(0.4,0.6,0.8);
      case sytClass:return Color(0.9,0.4,0.7);
      case sytClassMember:return Color(0.9,0.7,0.4);
      case sytMethod:return Color(0.4,0.9,0.7);
      default:return def;
    }
  }else
  {
    return Color::red;
  }
}

ZEditor::ZEditor(ZIndexer* argIdx):idx(argIdx)
{

}

void ZEditor::updateReader()
{
  if(!needColorize)return;
  fr->reset();
  FileRegistry::Entry* e=fr->getEntry();
  e->data.clear();
  for(LinesVector::iterator it=lines.begin(),end=lines.end();it!=end;++it)
  {
    EditLine& el=**it;
    el.colors.clear();
    el.updated=true;
    //LOGDEBUG(log,"line:'%s'",el.text.c_str());
    e->data.insert(e->data.end(),el.text.begin(),el.text.end());
    if(el.eoln)
    {
      if(el.eoln&0xff00)
      {
        e->data.push_back(el.eoln>>8);
      }
      e->data.push_back(el.eoln&0xff);
    }
  }
  fr->restoreSize();
}

void ZEditor::colorize()
{
  updateReader();
  ZLexer zl;
  zl.reg("//",tComment,&ZLexer::matchLineComment);
  zl.reg("/*",tComment,&ZLexer::matchMLComment);
  zl.fr=fr;
  Term t;
  //ClrVector clrs;
  Color clr[]={
      Color(1,1,1),//ctKeyword,
      Color(0,1,1),//ctIdent,
      Color(1,1,1),//ctSymbol,
      Color(1,1,0.5),//ctString,
      Color(0.5,0.5,1),//ctInt,
      Color(0,1,0),//ctDouble,
      Color(1,0.5,0.5),//ctBool,
      Color(1,0.5,1),//ctRegExp
      Color(0.7,0.7,0.7),//ctComment
      Color(0.5,1,1),//ctLocalVar
      Color(0.7,0.7,1)//ctFunction
  };
  zl.termOnUnknown=true;
  while((t=zl.getNext()).tt!=tEof)
  {
    if(t.tt==tEol)
    {
      if(zl.errorDetected)
      {
        FileLocation loc=fr->getLoc();
        loc.offset=fr->getSize();
        loc.line++;
        loc.col=0;//loc.offset;
        fr->setLoc(loc);
        zl.errorDetected=false;
      }
      if(fr->isEof())
      {
        break;
      }else
      {
        continue;
      }
    }
    int pp=t.pos.col;
    int len=t.length;
    //fprintf(stderr,"tt=%d:line=%d, col=%d, len=%d\n",t.tt,t.pos.line,t.pos.col,t.length);
    int l=t.pos.line;
    if(lines[l]->colors.size()>100)
    {
      fprintf(stderr,"loop?\n");
      break;
    }
    if(pp+len>lines[l]->text.length())
    {
      while(pp+len>lines[l]->text.length())
      {
        int rl=len>lines[l]->text.length()-pp?lines[l]->text.length()-pp:len;
        //fprintf(stderr,"line=%d, length=%d, len=%d, col=%d, rl=%d\n",l,(int)lines[l]->text.length(),len,pp,rl);
        lines[l]->colors.push_back(ColorInfo(clr[getTermColorType(t)],pp,rl));
        len-=rl;
        if(lines[l]->eoln)
        {
          --len;
        }
        pp=0;
        ++l;
        if(l==(int)lines.size())break;
      }
      if(len && l!=(int)lines.size())
      {
        //fprintf(stderr,"line=%d, len=%d, col=%d, rl=%d\n",l,len,pp,len);
        lines[l]->colors.push_back(ColorInfo(clr[getTermColorType(t)],pp,len));
      }
    }else
    {
      //fprintf(stderr,"line=%d, len=%d, col=%d, rl=%d\n",l,len,pp,len);
      lines[l]->colors.push_back(ColorInfo(clr[getTermColorType(t)],pp,len));
    }
    /*clrs.clear();
    clrs.insert(clrs.begin(),4*len,clr[getTermColorType(t)]);
    lines[t.pos.line-1]->txt.updateColors(clrs,pp*4,pp*4+len*4);*/
  }
  try{
    //idx->clear(fr);
    idx->indexate(fr);

    const SymbolRefList& sl=idx->getList();
    for(SymbolRefList::const_iterator it=sl.begin(),end=sl.end();it!=end;++it)
    {
      if(it->name.pos.fileRd!=fr || it->name.pos.line>=lines.size())continue;
      CIVector& clrs=lines[it->name.pos.line]->colors;
      for(CIVector::iterator cit=clrs.begin(),cend=clrs.end();cit!=cend;++cit)
      {
        if(cit->col==it->name.pos.col)
        {
          cit->clr=getSymColor(it->si,cit->clr);
          break;
        }
        if(cit->col+cit->cnt>it->name.pos.col)
        {
          int ocnt=cit->cnt;
          int ocol=cit->col;
          Color clr=cit->clr;
          int col=it->name.pos.col;
          int sz=it->name.val->getLength();
          cit->cnt=col-ocol;
          ++cit;
          cit=clrs.insert(cit,ColorInfo(getSymColor(it->si,clr),col,sz));
          if(ocol+ocnt>col+sz)
          {
            ++cit;
            cit=clrs.insert(cit,ColorInfo(clr,col+sz,ocol+ocnt-(col+sz)));
          }
          break;
        }
      }
    }

  }catch(SyntaxErrorException& e)
  {
    fprintf(stderr,"ex:%s\n",e.what());
  }
}

static void ze_getContext(ZorroVM* vm,Value* obj)
{
  ZEditor* editor=&obj->nobj->as<ZEditor>();
  std::string ctxName;
  SymInfo* si=editor->getCurContext();
  if(si)
  {
    ctxName=si->name.val.c_str();
  }else
  {
    ctxName="<unknown>";
  }

  ZStringRef str=vm->mkZString(ctxName.c_str(),ctxName.length());
  Value rv=StringValue(str);
  vm->setResult(rv);
}


static void ze_getDefLoc(ZorroVM* vm,Value* obj)
{
  ZEditor* editor=&obj->nobj->as<ZEditor>();
  expectArgs(vm,"getDefLoc",0);
  /*Value arg=vm->getLocalValue(0);
  if(arg.vt!=vtString)
  {
    throw std::runtime_error("invalid arg type for symbol name in jumpToDef");
  }
  ZString* name=arg.str;
  ScopeSym* ctx=editor->getCurContext();
  SymInfo* si=editor->getIndex().findSymbol(ctx,name);
  */
  vm->setResult(NilValue);
  ScopeSym* ctx=editor->getCurContext();
  if(ctx)
  {
    editor->getIndex().fillTypesAll();
    SymInfo* si=editor->getIndex().findSymbolAt(editor->getFilename(),ctx,editor->getCursorPos().y,editor->getCursorPos().x);
    if(si && si->name.pos.fileRd)
    {
      ZArray* arr=vm->allocZArray();
      arr->pushAndRef(StringValue(vm->mkZString(si->name.pos.fileRd->getEntry()->name.c_str())));
      arr->push(IntValue(si->name.pos.line));
      arr->push(IntValue(si->name.pos.col));
      vm->setResult(ArrayValue(arr));
    }
  }
}

static void ze_getCompletions(ZorroVM* vm,Value* obj)
{
  ZEditor* editor=&obj->nobj->as<ZEditor>();
  expectArgs(vm,"getCompletions",1);
  ScopeSym* ctx=editor->getCurContext();
  std::string start=getStrValue(vm,"start",0);
  std::list<SymInfo*> lst;
  editor->updateReader();
  bool lastEol=false;
  Term lt=editor->getLastToken(ctx->name.pos,editor->getCursorPos().y,editor->getCursorPos().x-start.length(),lastEol);
  if(lt.pos.fileRd)
  {
    switch(lt.tt)
    {
      case tInt:
      case tDouble:
      case tString:
      case tRawString:
      case tNil:
      case tTrue:
      case tFalse:
      case tCRBr:
      case tCSBr:
      case tCCBr:
      //case tThinArrow:
      case tFunc:
      case tFor:
      case tOn:
      case tClass:
      case tNamespace:
      case tCatch:
      case tNext:
      case tBreak:
      case tRedo:
      case tEnum:
      case tBitEnum:
      case tInclude:
      case tProp:
      case tAttr:
      case tUse:
      case tNumArg:
      case tRegExp:
      case tLiter:
      case tLiteral:
      case tMacro:
      {
        if(!lastEol)
        {
          vm->setResult(NilValue);
          return;
        }
      }break;
    }
  }
  LOGDEBUG("ze.cmpl","lastToken=%{}",lt.getValue());
  //int len;
  bool memberOrNs=false;
  if(lt.pos.fileRd && (lt.tt==tDot || lt.tt==tDoubleColon))
  {
    editor->getIndex().fillTypesAll();
    memberOrNs=true;
    editor->getIndex().getCompletions(editor->getFilename(),lt.tt==tDoubleColon,ctx,lt.pos.line,lt.pos.col,start,lst);
  }else
  {
    editor->getIndex().getCompletions(ctx,false,start,lst);
  }
  ZArray* arr=vm->allocZArray();
  std::set<SymInfo*> added;

  for(auto v:lst)
  {
    if(!v)continue;
    LOGDEBUG("ze.cmpl","name=%{}",v->name.val);
    SymInfo* p=v->getParent();
    if(!memberOrNs && ctx->st==sytGlobalScope && p && p->st!=sytGlobalScope && p->st!=sytNamespace)continue;
    ZString* nm=v->name.val.get();
    int nl=nm->getLength();
    bool badName=false;
    for(int i=0;i<nl;++i)
    {
      uint16_t c=nm->getCharAt(i);
      if(c=='.' || c==' ' || c=='-')
      {
        badName=true;
        break;
      }
    }
    if(badName)continue;
    //if(->find(*sub)!=-1 || v->name.val->find(*space)!=-1)continue;
    if(added.find(v)!=added.end())continue;
    added.insert(v);
    arr->pushAndRef(StringValue(v->name.val.get()));
  }
  vm->setResult(ArrayValue(arr));
}

static void ze_getDepthLevel(ZorroVM* vm,Value* obj)
{
  expectArgs(vm,"getDepthLevel",0);
  ZEditor* editor=&obj->nobj->as<ZEditor>();
  vm->setResult(IntValue(editor->getIndex().getDepthLevel(editor->getFilename(),editor->getCursorPos().y,editor->getCursorPos().x)));
}

static void ze_indexate(ZorroVM* vm,Value* obj)
{
  expectArgs(vm,"indexate",0);
  ZEditor* editor=&obj->nobj->as<ZEditor>();
  editor->indexate();
}

static void ze_isInsideString(ZorroVM* vm,Value* obj)
{
  expectArgs(vm,"isInsideString",0);
  ZEditor* editor=&obj->nobj->as<ZEditor>();
  ScopeSym* ctx=editor->getCurContext();
  bool lastEol;
  Term lt=editor->getLastToken(ctx->name.pos,editor->getCursorPos().y,editor->getCursorPos().x,lastEol);
  vm->setResult(BoolValue(lt.tt==tString || lt.tt==tRawString));
}

static void ze_isInsideComment(ZorroVM* vm,Value* obj)
{
  expectArgs(vm,"isInsideComment",0);
  ZEditor* editor=&obj->nobj->as<ZEditor>();
  ScopeSym* ctx=editor->getCurContext();
  bool lastEol;
  Term lt=editor->getLastToken(ctx->name.pos,editor->getCursorPos().y,editor->getCursorPos().x,lastEol);
  vm->setResult(BoolValue(lt.tt==tComment));
}


void ZEditor::registerInZVMEx(ZorroVM& vm)
{
  ZBuilder zb(&vm);
  zb.enterNClass("NativeEditor");
  zb.registerCMethod("getDefLoc",ze_getDefLoc);
  zb.registerCMethod("getContext",ze_getContext);
  zb.registerCMethod("getCompletions",ze_getCompletions);
  zb.registerCMethod("getDepthLevel",ze_getDepthLevel);
  zb.registerCMethod("indexate",ze_indexate);
  zb.registerCMethod("isInsideString",ze_isInsideString);
  zb.registerCMethod("isInsideComment",ze_isInsideComment);
  zb.leaveClass();
}


Term ZEditor::getLastToken(const FileLocation& startPos,int line,int col,bool& lastEol)
{
  ZLexer zl;
  zl.reg("//",tComment,&ZLexer::matchLineComment);
  zl.reg("/*",tComment,&ZLexer::matchMLComment);
  zl.termOnUnknown=true;
  fr->reset();
  fr->setLoc(startPos);
  zl.fr=fr;
  Term t,last;
  FileLocation endLoc(line,col);
  while((t=zl.peekNext()).tt!=tEof)
  {
    if(t.tt==tEol)
    {
      if(zl.errorDetected)
      {
        return Term();
      }
      lastEol=true;
      zl.getNext();
      continue;
    }
    if(t.pos>=endLoc)
    {
      break;
    }
    last=zl.getNext();
    lastEol=false;
  }
  return last;
}

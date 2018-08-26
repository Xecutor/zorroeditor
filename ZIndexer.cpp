#include "ZIndexer.hpp"
#include <kst/Logger.hpp>
#include "InputBuffer.hpp"

namespace{
bool isInside(int line,int col,const FileLocation& start,const FileLocation& end)
{
  return ((line>start.line) || (line==start.line && col>=start.col)) &&
      ((line<end.line)||(line==end.line && col<=end.col));
}

bool isInsideEx(int line,int col,const FileLocation& start,const FileLocation& end)
{
  return ((line>start.line) || (line==start.line && col>start.col)) &&
      ((line<end.line)||(line==end.line && col<end.col));
}


bool isInside(int line,int col,ScopeSym* scope)
{
  return scope->name.pos.fileRd && isInside(line,col,scope->name.pos,scope->end);
}

template <class T>
bool isInside(int line,int col,T* st)
{
  return isInside(line,col,st->pos,st->end);
}

template <class T>
bool isInsideEx(int line,int col,T* st)
{
  return isInsideEx(line,col,st->pos,st->end);
}


template <class T>
bool isLess(int line,int col,T* st)
{
  return line<st->pos.line || (line==st->pos.line && col<st->pos.col);
}

template <class T>
bool isGreater(int line,int col,T* st)
{
  return line>st->end.line || (line==st->end.line && col>st->end.col);
}
}

ZIndexer::ZIndexer():cg(&vm)
{
}

struct SymInfoRec{
  SymInfo* ptr;
  ZStringRef fullName;
};

static std::string getFullSymName(SymInfo* sym)
{
  std::string rv=sym->name.val.c_str();
  if(sym->getParent() && sym->getParent()->st==sytGlobalScope)return rv;
  while(sym->getParent())
  {
    sym=sym->getParent();
    rv.insert(0,".");
    rv.insert(0,sym->name.val.c_str());
  }
  return rv;
}

void ZIndexer::indexate(FileReader* fr,bool initial)
{
  auto it=idxMap.find(fr->getEntry()->name);
  if(it==idxMap.end())
  {
    it=idxMap.insert(IndexMap::value_type(fr->getEntry()->name,new IndexEntry)).first;
  }
  kst::Logger* log=kst::Logger::getLogger("idx");

  std::vector<SymInfoRec> clearedSymbols;
  if(!initial)
  {
    SymMap::Iterator sit(vm.symbols.global.symMap);
    ZString* nm;
    SymInfo** sym;
    while(sit.getNext(nm,sym))
    {
      if(!sym || !*sym)continue;
      SymInfo& si=**sym;
      if(si.st==sytNamespace)continue;
      if(si.name.pos.fileRd==fr)
      {
        LOGDEBUG(log,"clear %{}",getFullSymName(*sym));
        if(si.index!=-1)
        {
          vm.symbols.freeGlobal(si.index);
        }
        if(si.st==sytClass || si.st==sytFunction || si.st==sytMethod)
        {
          clearedSymbols.push_back({*sym,vm.mkZString(getFullSymName(*sym).c_str())});
        }else
        {
          if(si.unref())
          {
            delete *sym;
          }
        }
        *sym=0;
      }
    }
    vm.symbols.global.usedNs.clear();
    sl.clear();
  }
  fillSymList=!initial;

  ZParser& p=it->second->zp;
  zmx.init(&vm,&p);
  p.l.macroExpander=&zmx;
  p.setMem(&vm);
  p.pushReader(fr);
  fr->reset();
  StmtList* res=0;
  bool parseOk=false;
  try{
    p.parse();
    res=p.getResult();
    parseOk=true;
  }catch(std::exception& e)
  {
    fprintf(stderr,"exc:%s\n",e.what());
  }
  if(res)
  {
    it->second->lastParseResult.clearAndDelete();
    it->second->lastParseResult.splice(it->second->lastParseResult.end(),*res);
  }else if(parseOk)
  {
    it->second->lastParseResult.clearAndDelete();
  }
  res=&it->second->lastParseResult;
  cg.si->init();
  cg.si->global.name.pos.fileRd=fr;
  fillNames(res);
  cg.si->init();
  indexate(res);
  cg.fillTypes(res);
  //cg.fillTypes(res);

  if(!initial)
  {
    for(auto& srec:clearedSymbols)
    {
      SymInfo* nsym=vm.symbols.global.symMap.findSymbol(srec.fullName);
      LOGDEBUG(log,"nsym for %{}=%{}",srec.fullName,(void*)nsym);
      if(nsym && nsym->st==srec.ptr->st)
      {
        LOGDEBUG(log,"remapping symbol ref for %{}",srec.fullName);
        ScopeSym* ss=(ScopeSym*)srec.ptr;
        for(auto& ref:ss->refs)
        {
          ref->reset((ScopeSym*)nsym);
        }
        ss->refs.clear();
      }
      if(srec.ptr->unref())
      {
        delete srec.ptr;
      }
    }
  }
}

ScopeSym* ZIndexer::getContext(FileReader* fr,int line,int col)
{
  ScopeSym* ptr=&vm.symbols.global;
  bool found=true;
  while(found)
  {
    found=false;
    SymMap::Iterator it(ptr->symMap);
    ZString* name;
    SymInfo** valptr;
    while(it.getNext(name,valptr))
    {
      if(!valptr)
      {
        continue;
      }
      ScopeSym* val=dynamic_cast<ScopeSym*>(*valptr);
      if(val && val->name.pos.fileRd==fr)
      {
        if(isInside(line,col,val))
        {
          ptr=val;
          found=true;
          break;
        }
      }
    }
  }
  return ptr;
}


static bool startsWith(const std::string& str,const char* str2)
{
  size_t len=str.length();
  for(size_t i=0;i<len;++i)
  {
    if(str[i]!=str2[i])return false;
  }
  return true;
}

void ZIndexer::getCompletions(ScopeSym* scope,bool skipParent,const std::string& start,std::list<SymInfo*>& lst)
{
  SymMap::Iterator it(scope->symMap);
  ZString* nm;
  SymInfo** val;
  while(it.getNext(nm,val))
  {
    if(!val)
    {
      continue;
    }
    if(startsWith(start,nm->c_str(&vm)))
    {
      lst.push_back(*val);
    }
  }
  for(auto ns:scope->usedNs)
  {
    getCompletions(ns,true,start,lst);
  }
  if(!skipParent && scope->parent)
  {
    getCompletions(scope->parent,false,start,lst);
  }
}

static Expr* findNearestSubExpr(Expr* e,int line,int col)
{
  if(e->et==etCall && isGreater(line,col,e))
  {
    return e;
  }
  if(e->et==etProp && isInside(line,col,e))
  {
    if(isInside(line,col,e->e1))
    {
      return findNearestSubExpr(e->e1,line,col);
    }
    return e->end.line==line && e->end.col==col?e:e->e1;
  }
  Expr* rv=0;
  if(e->lst)
  {
    for(ExprList::reverse_iterator it=e->lst->rbegin(),end=e->lst->rend();it!=end;++it)
    {
      Expr* ee=*it;
      if(ee && (rv=findNearestSubExpr(ee,line,col)))return rv;
    }
  }
  if(e->e3 && isInside(line,col,e->e3) && (rv=findNearestSubExpr(e->e3,line,col)))return rv;
  if(e->e2 && isInside(line,col,e->e2) && (rv=findNearestSubExpr(e->e2,line,col)))return rv;
  if(e->e1 && isInside(line,col,e->e1) && (rv=findNearestSubExpr(e->e1,line,col)))return rv;
  return e;
}

void ZIndexer::getCompletions(const std::string& fn,bool ns,ScopeSym* scope,int line,int col,const std::string& start,std::list<SymInfo*>& lst)
{
  if(start.length() && ns)
  {
    col-=start.length();
  }
  cg.si->currentScope=scope;
  StmtList* stmtLst=0;
  if(scope->st==sytFunction || scope->st==sytMethod)
  {
    FuncInfo* fi=(FuncInfo*)scope;
    if(!fi->def)
    {
      return;
    }
    stmtLst=fi->def->as<FuncDeclStatement>().body;
  }else if(scope->st==sytGlobalScope)
  {
    IndexEntry* ie=idxMap[fn];
    if(ie)
    {
      stmtLst=&ie->lastParseResult;
    }
  }
  if(stmtLst)
  {
    getCompletions(stmtLst,ns,line,col,start,lst);
  }
}
void ZIndexer::getCompletions(StmtList* stmtLst,bool ns,int line,int col,const std::string& start,std::list<SymInfo*>& lst)
{
  if(!stmtLst)return;
  std::vector<Expr*> subEx;
  std::vector<StmtList*> subStmt;
  Expr* ex=0;
  Statement* prev=0;
  bool found=false;
  for(auto st:*stmtLst)
  {
    if(isInside(line,col,st))
    {
      prev=st;
      found=true;
      break;
    }
    if(prev && isLess(line,col,st))
    {
      found=true;
      break;
    }
    prev=st;
  }
  if(prev)
  {
    prev->getChildData(subEx,subStmt);
    for(auto e:subEx)
    {
      if(isInside(line,col,e))
      {
        ex=e;
        break;
      }
    }
    if(!ex)
    {
      for(auto s:subStmt)
      {
        if(s && !s->empty() && isInside(line,col,s->front()->pos,s->back()->end))
        {
          getCompletions(s,ns,line,col,start,lst);
          return;
        }
      }
    }
  }

  if(!ex && prev->st==stExpr)
  {
    ex=prev->as<ExprStatement>().expr;
  }
  if(ex)
  {
    Expr* e=findNearestSubExpr(ex,line,col);
    if(!e)
    {
      return;
    }
    if(ns)
    {
      getNsCompletions(e,start,lst);
      return;
    }
    TypeInfo ti;
    cg.getExprType(e,ti);
    for(auto& t:ti)
    {
      ClassInfo* ci=0;
      if(t.vt==vtObject && t.symRef)
      {
        ci=t.symRef.asClass();
      }else
      {
        switch(t.vt)
        {
          case vtString:ci=vm.stringClass;break;
          case vtInt:ci=vm.intClass;break;
          case vtClass:ci=vm.classClass;break;
          case vtArray:ci=vm.arrayClass;break;
          case vtMap:ci=vm.mapClass;break;
          case vtSet:ci=vm.setClass;break;
          case vtCoroutine:ci=vm.corClass;break;
          default:break;
        }
      }
      if(ci)
      {
        ZString* nm;
        SymInfo** ptr;
        SymMap::Iterator it(ci->symMap);
        while(it.getNext(nm,ptr))
        {
          if(!ptr || !*ptr)
          {
            continue;
          }
          if(!start.empty() && !startsWith(start,(*ptr)->name.val.c_str()))
          {
            continue;
          }
          lst.push_back(*ptr);
        }
      }
    }
  }
}

void ZIndexer::getNsCompletions(Expr* ex,const std::string& start,std::list<SymInfo*>& lst)
{
  ScopeSym* arr[2]={cg.si->currentScope,&cg.si->global};
  for(int i=ex->global?1:0;i<2;++i)
  {
    ScopeSym* ptr=arr[i];
    bool first=true;
    std::list<ScopeSym*>::iterator nsIt=ptr->usedNs.begin();
    std::list<ScopeSym*>::iterator nsEnd=ptr->usedNs.end();
    do{
      if(ex->ns)
      {
        for(NameList::iterator it=ex->ns->begin(),end=ex->ns->end();it!=end;++it)
        {
          ptr=(ScopeSym*)ptr->getSymbols()->findSymbol(*it);
          if(!ptr)
          {
            break;
          }
          if(ptr->st!=sytNamespace)
          {
            ptr=0;
            break;
          }
        }
      }
      if(ptr)
      {
        break;
      }
      if(first && nsIt!=nsEnd)
      {
        ptr=*nsIt;
        first=false;
      }else
      {
        ++nsIt;
        if(nsIt!=nsEnd)
        {
          ptr=*nsIt;
        }
      }
    }while(nsIt!=nsEnd);
    if(ptr)
    {
      if(ex->val)
      {
        ScopeSym* ptr2=(ScopeSym*)ptr->getSymbols()->findSymbol(ex->val);
        if(ptr2 && ptr2->st==sytNamespace)ptr=ptr2;
      }
      if(ptr)
      {
        SymMap::Iterator it(ptr->symMap);
        ZString* nm;
        SymInfo** val;
        while(it.getNext(nm,val))
        {
          if(!val)
          {
            continue;
          }
          if(startsWith(start,nm->c_str(&vm)))
          {
            lst.push_back(*val);
          }
        }
      }
    }
  }
}

SymInfo* ZIndexer::findSymbolAt(const std::string& fn,ScopeSym* scope,int line,int col)
{

  vm.symbols.currentScope=scope;
  StmtList* sl=0;
  switch(scope->st)
  {
    case sytGlobalScope:
    {
      auto it=idxMap.find(fn);
      if(it!=idxMap.end())
      {
        sl=&it->second->lastParseResult;
      }
    }break;
    case sytNamespace:break;
    case sytMethod:
    case sytFunction:
    {
      FuncInfo* fi=(FuncInfo*)scope;
      if(fi->def)
      {
        sl=fi->def->as<FuncDeclStatement>().body;
      }
    }break;
    case sytClass:
    {
      ClassInfo* ci=(ClassInfo*)scope;
      if(ci->def)
      {
        ClassDefStatement& cds=ci->def->as<ClassDefStatement>();
        if (cds.parent)
        {

          FileLocation end=cds.parent->name.name.pos;
          end.col+=cds.parent->name.name.val->getLength();
          if (isInside(line, col, cds.parent->name.name.pos, end))
          {
            return vm.symbols.getSymbol(cds.parent->name);
          }
        }
        if(cds.parent && cds.parent->args && isInside(line, col, cds.parent->args->front()->pos, cds.parent->args->back()->end)) {
          for(auto& ex:*cds.parent->args) {
            SymInfo* rv=findSymbolAt(ex, line, col);
            if(rv)return rv;
          }
        }
        sl=cds.body;
      }
    }break;
    default:break;
  }
  return findSymbolAt(sl,line,col);
}

SymInfo* ZIndexer::findSymbolAt(StmtList* stl,int line,int col)
{
  if(!stl)return 0;
  SymInfo* rv;
  std::vector<Expr*> subEx;
  std::vector<StmtList*> subStmt;

  for(auto stptr:*stl)
  {
    if(!isInside(line,col,stptr))continue;
    Statement& st=*stptr;
    st.getChildData(subEx,subStmt);
    for(auto e:subEx)
    {
      if((rv=findSymbolAt(e,line,col)))return rv;
    }
    for(auto s:subStmt)
    {
      if((rv=findSymbolAt(s,line,col)))return rv;
    }
    subEx.clear();
    subStmt.clear();
  }
  return 0;
}

SymInfo* ZIndexer::findSymbolAt(Expr* ex,int line,int col)
{
  if(!ex || !isInside(line,col,ex))return 0;
  if(ex->et==etVar)
  {
    if(isInside(line,col,ex))
    {
      return vm.symbols.getSymbol(ex->getSymbol());
    }
  }else
  {
    if(ex->et==etProp)
    {
      if(isInside(line,col,ex->e1))
      {
        return findSymbolAt(ex->e1,line,col);
      }
      TypeInfo ti;
      cg.getExprType(ex->e1,ti);
      for(auto& t:ti)
      {
        if(t.vt==vtObject && t.symRef)
        {
          SymInfo* sym=t.symRef.asClass()->getSymbols()->findSymbol(ex->e2->val);
          if(sym)
          {
            return sym;
          }
        }
      }
    }else
    {
      SymInfo* rv=0;
      if(ex->e1 && (rv=findSymbolAt(ex->e1,line,col)))
      {
        return rv;
      }
      if(ex->e2 && (rv=findSymbolAt(ex->e2,line,col)))
      {
        return rv;
      }
      if(ex->e3 && (rv=findSymbolAt(ex->e3,line,col)))
      {
        return rv;
      }
      if(ex->lst)
      {
        for(auto e:*ex->lst)
        {
          if((rv=findSymbolAt(e,line,col)))
          {
            return rv;
          }
        }
      }
    }
  }
  return 0;
}


void ZIndexer::indexate(StmtList* stmtLst)
{
  if(!stmtLst)return;
  std::vector<Expr*> subEx;
  std::vector<StmtList*> subStmt;
  for(StmtList::iterator it=stmtLst->begin(),end=stmtLst->end();it!=end;++it)
  {
    Statement& st=**it;
    ScopeSym* saveScope=cg.si->currentScope;
    try{
      switch(st.st)
      {
        /*case stListAssign:
        {
          ListAssignStatement& las=st.as<ListAssignStatement>();
          for(ExprList::iterator it=las.lst1->begin(),end=las.lst1->end();it!=end;++it)
          {
            if(cg.isSimple(*it))
            {
              cg.getArgType(*it,true);
              SymInfo* si=cg.si->getSymbol((*it)->getSymbol());
              if(fillSymList)sl.push_back(SymbolRef((*it)->getSymbol().name,si));
            }else
            {
              indexateExpr(*it);
            }
          }
          indexate(las.lst2);
        }break;*/
        case stFuncDecl:
        {
          FuncDeclStatement& func=st.as<FuncDeclStatement>();
          indexateFunc(func);
        }break;
        case stForLoop:
        {
          ForLoopStatement& fst=st.as<ForLoopStatement>();
          for(NameList::iterator it=fst.vars->begin(),end=fst.vars->end();it!=end;++it)
          {
            SymInfo* sym=cg.si->getSymbol(*it);
            if(!sym)
            {
              bool isLocal;
              cg.si->registerVar(*it,isLocal);
              sym=cg.si->getSymbol(*it);
            }
            if(fillSymList)sl.push_back(SymbolRef(*it,sym));
          }
          indexateExpr(fst.expr);
          indexate(fst.body);
        }break;
        case stClass:
        {
          ClassDefStatement& cds=st.as<ClassDefStatement>();
          SymInfo* pc=0;
          if(cds.parent)
          {
            pc=cg.si->getSymbol(cds.parent->name);
            if(pc && pc->st==sytClass)
            {
              ClassInfo* ci=(ClassInfo*)pc;
              if(!ci->parentClass)
              {
                StmtList tmp;
                tmp.push_back(ci->def);
                try{
                  indexate(&tmp);
                }catch(...)
                {
                }
                tmp.clear();
              }
            }
          }
          ClassInfo* ci=cg.si->enterClass(cds.name);
          if(!ci)
          {
            ci=cg.si->registerClass(cds.name);
          }
          cg.currentClass=&cds;
          if(cds.parent && pc)
          {
            cg.si->deriveMembers(&cds.parent->name);
          }else
          {
            cg.si->deriveMembers(0);
          }
          if(fillSymList)sl.push_back(SymbolRef(cds.name,ci));
          cg.fillClassNames(cds.body);
          indexate(cds.body);
          cg.si->returnScope();
          //for(FuncParamList::iterator it)
        }break;
        case stClassMemberDef:
        {
          ClassMemberDef& m=st.as<ClassMemberDef>();
          SymInfo* si=cg.si->getSymbol(m.name);
          if(fillSymList)sl.push_back(SymbolRef(m.name,si));
        }break;
        case stProp:
        {
          PropStatement& pst=st.as<PropStatement>();
          ClassPropertyInfo* ps=(ClassPropertyInfo*)cg.si->currentClass->symMap.findSymbol(pst.name.val);
          if(pst.getFunc)
          {
            ps->getMethod=cg.si->registerMethod(pst.getFunc->name,0,false);
            ps->getMethod->def=pst.getFunc;
            ps->getMethod->end=pst.getFunc->end;
            indexate(pst.getFunc->body);
            cg.si->returnScope();
          }else if(pst.getName.val.get())
          {
            SymInfo* si=cg.si->getSymbol(pst.getName);
            ps->getIdx=si->index;
            if(fillSymList)sl.push_back(SymbolRef(pst.getName,si));
          }
          if(pst.setFunc)
          {
            MethodInfo* m=cg.si->registerMethod(pst.setFunc->name,1,false);
            m->end=pst.setFunc->end;
            m->def=pst.setFunc;
            ps->setMethod=m;
            int idx=cg.si->registerArg(*pst.setFunc->args->front());
            SymInfo* ai=m->locals[idx];
            if(fillSymList)sl.push_back(SymbolRef(*pst.setFunc->args->front(),ai));
            indexate(pst.setFunc->body);
            cg.si->returnScope();
          }else if(pst.setName.val.get())
          {
            SymInfo* si=cg.si->getSymbol(pst.setName);
            ps->setIdx=si->index;
            if(fillSymList)sl.push_back(SymbolRef(pst.setName,si));
          }
        }break;
        case stNamespace:
        {
          NamespaceStatement& ns=st.as<NamespaceStatement>();
          if(ns.ns)
          {
            for(NameList::iterator it=ns.ns->begin(),end=ns.ns->end();it!=end;++it)
            {
              if(!cg.si->enterNamespace(*it))
              {
                throw CGException("not a namespace",ns.pos);
              }
            }
          }else
          {
            cg.si->enterNamespace(ns.name);
          }
          indexate(ns.body);
          if(ns.ns)
          {
            for(NameList::iterator it=ns.ns->begin(),end=ns.ns->end();it!=end;++it)
            {
              cg.si->returnScope();
            }
          }else
          {
            cg.si->returnScope();
          }
        }break;
        case stUse:
        {
          UseStatement& us=st.as<UseStatement>();
          SymInfo* sym=cg.si->getSymbol(us.expr->getSymbol());
          if(!sym)
          {
            throw UndefinedSymbolException(us.expr->getSymbol());
          }
          if(sym->st==sytNamespace)
          {
            cg.si->currentScope->usedNs.push_back((ScopeSym*)sym);
          }else
          {
            cg.si->addAlias(sym);
          }
        }break;
        default:
        {
          st.getChildData(subEx,subStmt);
          for(auto e:subEx)
          {
            indexateExpr(e);
          }
          for(auto lst:subStmt)
          {
            indexate(lst);
          }
          subEx.clear();
          subStmt.clear();
        }break;
      }
    }catch(std::exception& e)
    {
      fprintf(stderr,"ex:%s\n",e.what());
      cg.si->currentScope=saveScope;
    }
  }
}


void ZIndexer::indexateFunc(FuncDeclStatement& fds)
{
  Name name=fds.name;
  if(fds.isOnFunc)
  {
    name.val="on "+name.val;
  }
  bool isMethod=cg.si->isClass();
  FuncInfo* si=0;
  if(isMethod)
  {
    ClassSpecialMethod csm=csmUnknown;
    if(fds.isOnFunc)
    {
      csm=cg.si->getCsm(fds.name);
    }
    si=cg.si->enterMethod(name);
    if(!si)return;
    if(csm!=csmUnknown)
    {
      cg.si->currentClass->specialMethods[csm]=si->index;
    }
    si->def=&fds;
    if(fillSymList)sl.push_back(SymbolRef(name,si));
  }else
  {
    if(cg.si->currentScope->st!=sytFunction && cg.si->currentScope->st!=sytMethod)
    {
      si=cg.si->enterFunc(name);
    }else
    {
      si=cg.si->registerFunc(fds.name,fds.argsCount());
      si->end=fds.end;
    }
    si->def=&fds;
    if(fillSymList)sl.push_back(SymbolRef(name,si));
  }
  si->locals.clear();
  if(fds.args)
  {
    for(FuncParamList::iterator it=fds.args->begin(),end=fds.args->end();it!=end;++it)
    {
      FuncParam& fp=**it;
      cg.si->registerArg(fp);
      SymInfo* sym=cg.si->getSymbol(fp);
      if(fillSymList)sl.push_back(SymbolRef(fp,sym));
      indexateExpr(fp.defValue);
    }
  }
  if(isMethod)
  {
    cg.si->registerLocalVar(cg.selfName);
  }
  indexate(fds.body);
  cg.si->returnScope();
}

void ZIndexer::indexate(ExprList* el)
{
  if(!el)return;
  for(ExprList::iterator it=el->begin(),end=el->end();it!=end;++it)
  {
    indexateExpr(*it);
  }
}

void ZIndexer::indexateExpr(Expr* expr)
{
  if(!expr)
  {
    return;
  }
  switch(expr->et)
  {
    case etAssign:
    {
      if(expr->e1->et==etVar)
      {
        SymInfo* si=cg.si->getSymbol(expr->e1->getSymbol());
        if(!si)
        {
          bool isLocal;
          cg.si->registerVar(expr->e1->getSymbol().name,isLocal);
          si=cg.si->getSymbol(expr->e1->getSymbol());
        }
        if(fillSymList)sl.push_back(SymbolRef(expr->e1->getSymbol().name,si));
      }else
      {
        indexateExpr(expr->e1);
      }
      indexateExpr(expr->e2);
    }break;

    case etVar:
    {
      SymInfo* si=cg.si->getSymbol(expr->getSymbol());
      if(fillSymList)sl.push_back(SymbolRef(expr->getSymbol().name,si));
    }break;
    case etNumArg:break;
    case etCall:
      indexateExpr(expr->e1);
      indexate(expr->lst);
      break;
    case etFunc:
    {
      char name[32];
      sprintf(name,"anonymous-%p",expr);
      Name nm(vm.mkZString(name),expr->pos);
      expr->func->name=nm;
      cg.si->registerFunc(nm,expr->func->argsCount());
      cg.si->returnScope();
      indexateFunc(*expr->func);
    }break;
    case etSeqOps:
    {
      Symbol symName=expr->e1->et==etVar?expr->e1->getSymbol():expr->e1->e1->getSymbol();
      SymInfo* si=cg.si->getSymbol(symName);
      if(!si)
      {
        bool isLocal;
        cg.si->registerVar(symName.name,isLocal);
        si=cg.si->getSymbol(symName);
      }
      if(fillSymList)sl.push_back(SymbolRef(symName.name,si));
    }
    /* no break */
    default:
    {
      if(expr->e1)indexateExpr(expr->e1);
      if(expr->e2)indexateExpr(expr->e2);
      if(expr->e3)indexateExpr(expr->e3);
      if(expr->lst)indexate(expr->lst);
    }break;
  }
}


void ZIndexer::fillNames(StmtList* sl)
{
  if(!sl)return;
  for(StmtList::iterator it=sl->begin(),end=sl->end();it!=end;++it)
  {
    Statement& st=**it;
    switch(st.st)
    {
      case stFuncDecl:
      {
        FuncDeclStatement& f=st.as<FuncDeclStatement>();
        FuncInfo* fi=getOrRegFunc(f.name,f.argsCount());
        fi->tinfo=TypeInfo(fi);
        fi->def=&st;
        fi->end=f.end;
      }break;
      case stNamespace:
      {
        NamespaceStatement& ns=st.as<NamespaceStatement>();
        ScopeSym* nsPtr;
        if(ns.ns)
        {
          for(NameList::iterator it=ns.ns->begin(),end=ns.ns->end();it!=end;++it)
          {
            if(!cg.si->enterNamespace(*it))
            {
              //throw CGException("not a namespace",ns.pos);
            }
            cg.si->currentScope->end=ns.end;
          }
        }else
        {
          char nsName[64];
          int l=sprintf(nsName,"anonymous-ns-%p",&ns);
          ns.name=Name(vm.mkZString(nsName,l),ns.pos);
          cg.si->enterNamespace(ns.name);
          nsPtr=cg.si->currentScope;
          cg.si->currentScope->end=ns.end;
        }
        fillNames(ns.body);
        if(ns.ns)
        {
          for(NameList::iterator it=ns.ns->begin(),end=ns.ns->end();it!=end;++it)
          {
            cg.si->returnScope();
          }
        }else
        {
          cg.si->returnScope();
          cg.si->currentScope->usedNs.push_back(nsPtr);
        }
      }break;
      case stClass:
      {
        ClassDefStatement& c=st.as<ClassDefStatement>();
        ClassInfo* ci=getOrRegClass(c.name);
        ci->tinfo=TypeInfo(ci,vtClass);
        ci->def=&st;
        ci->end=c.end;
      }break;
      case stAttr:
      {
        AttrDeclStatement& a=st.as<AttrDeclStatement>();
        SymInfo* sym=cg.si->getAttr(a.name);
        if(!sym)
        {
          cg.si->registerAttr(a.name);
        }

      }break;
      case stLiter:
      {
        LiterStatement& lst=st.as<LiterStatement>();
        ScopeSym* curScope=cg.si->currentScope;
        LiterInfo* li=(LiterInfo*)curScope->symMap.findSymbol(lst.name);
        if(li)
        {
          cg.si->currentScope=li;
        }else
        {
          li=cg.si->registerLiter(lst.name,lst.args->size());
        }
        for(LiterArgsList::iterator it=lst.args->begin(),end=lst.args->end();it!=end;++it)
        {
          ScopeSym::LiterList* curLst;
          ScopeSym::LiterList** ll=curScope->literMap.getPtr((*it)->marker.val.get());
          if(!ll)
          {
            curLst=new ScopeSym::LiterList();
            curScope->literMap.insert((*it)->marker.val.get(),curLst);
          }else
          {
            curLst=*ll;
          }
          curLst->push_back(li);
          if(!(*it)->optional)break;
        }
        for(LiterArgsList::iterator it=lst.args->begin(),end=lst.args->end();it!=end;++it)
        {
          li->markers.push_back(**it);
          cg.si->registerArg((*it)->name);
        }

        cg.si->returnScope();
      }break;
      case stEnum:
      {
        EnumStatement& est=st.as<EnumStatement>();
        cg.si->registerGlobalSymbol(est.name,new SymInfo(est.name,sytGlobalVar));
        if(est.items)
        {
          for(auto v:*est.items)
          {
            if(v->et==etVar)
            {
              cg.si->registerGlobalSymbol(v->getSymbol().name,new SymInfo(v->getSymbol().name,sytGlobalVar));
            }else if(v->et==etAssign && v->e1->et==etVar)
            {
              cg.si->registerGlobalSymbol(v->e1->getSymbol().name,new SymInfo(v->e1->getSymbol().name,sytGlobalVar));
            }
          }
        }
      }break;
      case stExpr:
      {
        ExprStatement& est=st.as<ExprStatement>();
        if(est.expr->et==etAssign && est.expr->e1->et==etVar)
        {
          cg.si->registerGlobalSymbol(est.expr->e1->getSymbol().name,new SymInfo(est.expr->e1->getSymbol().name,sytGlobalVar));
        }
      }break;
      default:break;
    }
  }
}

FuncInfo* ZIndexer::getOrRegFunc(const Name& name,int argsCount)
{
  FuncInfo* rv=0;
  SymInfo* sym=cg.si->currentScope->symMap.findSymbol(name);
  if(sym && sym->st==sytFunction)
  {
    rv=(FuncInfo*)sym;
    rv->argsCount=argsCount;
  }
  if(!rv)
  {
    rv=cg.si->registerFunc(name,argsCount);
    cg.si->returnScope();
  }
  return rv;
}

ClassInfo* ZIndexer::getOrRegClass(const Name& name)
{
  ClassInfo* rv=0;
  SymInfo* sym=cg.si->currentScope->symMap.findSymbol(name);
  if(sym && sym->st==sytClass)
  {
    rv=(ClassInfo*)sym;
  }
  if(!rv)
  {
    rv=cg.si->registerClass(name);
    cg.si->returnScope();
  }
  return rv;
}


int ZIndexer::getDepthLevel(const std::string& fn,int line,int col)
{
  IndexEntry* ie=idxMap[fn];
  StmtList* stmtList=0;
  if(ie)
  {
    stmtList=&ie->lastParseResult;
  }
  if(!stmtList)
  {
    return 0;
  }
  return getStmtDepth(line,col,stmtList);
}

int ZIndexer::getStmtDepth(int line,int col,StmtList* stmtList)
{
  if(!stmtList || stmtList->empty())return 0;
  std::vector<StmtList::iterator> begins,ends;
  begins.push_back(stmtList->begin());
  ends.push_back(stmtList->end());
  int rv=0;
  std::vector<StmtList*> subStmt;
  std::vector<Expr*> subExpr;
  while(!begins.empty())
  {
    StmtList::iterator it=begins.back();
    StmtList::iterator end=ends.back();
    begins.pop_back();
    ends.pop_back();
    for(;it!=end;++it)
    {
      Statement& st=**it;
      if(isInsideEx(line,col,&st))
      {
        switch(st.st)
        {
          case stNone:
          case stListAssign:
          case stReturn:
          case stReturnIf:
          case stYield:
          case stThrow:
          case stClassMemberDef:
          case stVarList:
          case stBreak:
          case stNext:
          case stRedo:
          case stAttr:
          case stUse:
            return rv;
          case stExpr:
          {
            return rv+getExprDepth(line,col,st.as<ExprStatement>().expr);
          }break;
          case stProp:
          {
            PropStatement& ps=st.as<PropStatement>();
            if(ps.getFunc && isInside(line,col,ps.getFunc->pos,ps.getFunc->end))
            {
              ++rv;
              if(ps.getFunc->body)
              {
                begins.push_back(ps.getFunc->body->begin());
                ends.push_back(ps.getFunc->body->end());
              }
            }
            if(ps.setFunc && isInside(line,col,ps.setFunc->pos,ps.setFunc->end))
            {
              ++rv;
              if(ps.setFunc->body)
              {
                begins.push_back(ps.setFunc->body->begin());
                ends.push_back(ps.setFunc->body->end());
              }
            }
          }break;
          case stIf:
          {
            IfStatement& ifst=st.as<IfStatement>();
            if(ifst.thenList && ifst.thenList->front()->pos<ifst.cond->pos)
            {
              return rv;
            }
            if(ifst.thenList)
            {
              if(isInside(line,col,ifst.thenList->front()->pos,ifst.thenList->back()->end))
              {
                begins.push_back(ifst.thenList->begin());
                ends.push_back(ifst.thenList->end());
                break;
              }
            }
            if(ifst.elseList)
            {
              if(isInside(line,col,ifst.elseList->front()->pos,ifst.elseList->back()->end))
              {
                begins.push_back(ifst.elseList->begin());
                ends.push_back(ifst.elseList->end());
                break;
              }
            }
            if(ifst.elsifList)
            {
              for(auto s:*ifst.elsifList)
              {
                if(isInside(line,col,s->pos,s->end))
                {
                  IfStatement& elif=s->as<IfStatement>();
                  if(elif.thenList)
                  {
                    begins.push_back(elif.thenList->begin());
                    ends.push_back(elif.thenList->end());
                  }
                  break;
                }
              }
            }

          }break;
          case stWhile:
          case stForLoop:
          {
            if(st.st==stWhile)
            {
              WhileStatement& wst=st.as<WhileStatement>();
              if(wst.body && wst.body->front()->pos<wst.cond->pos)
              {
                return rv;
              }
            }else
            {
              ForLoopStatement& fst=st.as<ForLoopStatement>();
              if(fst.body && fst.body->front()->pos<fst.expr->pos)
              {
                return rv;
              }
            }
          }
          /* no break */
          case stFuncDecl:
          case stClass:
          case stClassAttrDef:
          case stNamespace:
          case stTryCatch:
          case stSwitch:
          case stEnum:
          case stLiter:
          {
            subExpr.clear();
            subStmt.clear();
            st.getChildData(subExpr,subStmt);
            for(auto s:subStmt)
            {
              if(!s->empty())
              {
                if(isInside(line,col,s->front()->pos,s->back()->end))
                {
                  begins.push_back(s->begin());
                  ends.push_back(s->end());
                  break;
                }
              }
            }
          }break;
        }
        ++rv;
        break;
      }
      if(st.pos.line>line)
      {
        break;
      }
    }
  }
  return rv;
}

void ZIndexer::fillTypesAll()
{
  for(auto& v:idxMap)
  {
    cg.si->init();
    cg.fillTypes(&v.second->lastParseResult);
  }
}


int ZIndexer::getExprDepth(int line,int col,Expr* expr)
{
  switch(expr->et)
  {
    case etArray:
    case etSet:
    case etMap:
    case etCall:
    {
      if(expr->lst)
      {
        for(auto e:*expr->lst)
        {
          if(isInside(line,col,e))
          {
            return 1+getExprDepth(line,col,e);
          }
        }
      }
      return 1;
    }break;
    case etFunc:
    {
      return 1+getStmtDepth(line,col,expr->func->body);
    }break;
    default:
      if(expr->e1 && isInside(line,col,expr->e1))return getExprDepth(line,col,expr->e1);
      if(expr->e2 && isInside(line,col,expr->e2))return getExprDepth(line,col,expr->e2);
      if(expr->e3 && isInside(line,col,expr->e3))return getExprDepth(line,col,expr->e3);
      break;
  }
  return 0;
}

void ZIndexer::importApi(const char* fn)
{
  kst::File f;
  f.ROpen(fn);
  std::vector<char> buf(f.Size());
  f.Read(&buf[0],buf.size());
  InputBuffer ib(&buf[0],buf.size());
  vm.symbols.importApi(ib);
}

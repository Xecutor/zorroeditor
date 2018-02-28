#ifndef __ZE_INDEXER_HPP__
#define __ZE_INDEXER_HPP__

#include "Symbolic.hpp"
#include "SynTree.hpp"
#include "CodeGenerator.hpp"
#include "ZorroParser.hpp"
#include "MacroExpander.hpp"
#include "ZorroVM.hpp"

using namespace zorro;

struct SymbolRef{
  Name name;
  SymInfo* si;
  SymbolRef(const Name& argName,SymInfo* argSi):name(argName),si(argSi){}
};

typedef std::list<SymbolRef> SymbolRefList;

class ZIndexer{
public:
  ZIndexer();
  void indexate(FileReader* fr,bool initial=false);
  const SymbolRefList& getList()const
  {
    return sl;
  }

  ScopeSym* getContext(FileReader* fr,int line,int col);
  void getCompletions(ScopeSym* scope,bool skipParent,const std::string& start,std::list<SymInfo*>& lst);
  void getCompletions(const std::string& fn,bool ns,ScopeSym* scope,int line,int col,const std::string& start,std::list<SymInfo*>& lst);

  SymInfo* findSymbolAt(const std::string& fn,ScopeSym* scope,int line,int col);

  SymInfo* findSymbol(ScopeSym* scope,ZString* name)
  {
    vm.symbols.currentScope=scope?scope:&vm.symbols.global;
    Symbol sm;
    if(*name->getDataPtr()=='@')
    {
      sm.name=ZStringRef(&vm,name->substr(&vm,2,name->getLength()-2));
      sm.ns=new NameList;
      sm.ns->push_back(vm.mkZString("MACRO"));
    }else
    {
      sm.name=ZStringRef(&vm,name);
    }

    return vm.symbols.getSymbol(sm);
  }

  int getDepthLevel(const std::string& fn,int line,int col);

  void fillTypesAll();

  void importApi(const char* fn);

protected:

  struct IndexEntry{
    IndexEntry():zp(0){}
    ZParser zp;
    StmtList lastParseResult;
  };
  typedef std::map<std::string,IndexEntry*> IndexMap;
  IndexMap idxMap;

  void fillNames(StmtList* sl);
  FuncInfo* getOrRegFunc(const Name& name,int argsCount);
  ClassInfo* getOrRegClass(const Name& name);
  void indexate(StmtList* sl);
  void indexateStmt(Statement& st);
  void indexateExpr(Expr* ex);
  void indexate(ExprList* el);

  int getStmtDepth(int line,int col,StmtList* stmtList);
  int getExprDepth(int line,int col,Expr* expr);

  SymInfo* findSymbolAt(StmtList* sl,int line,int col);
  SymInfo* findSymbolAt(Expr* ex,int line,int col);

  void getCompletions(StmtList* stmtLst,bool ns,int line,int col,const std::string& start,std::list<SymInfo*>& lst);
  void getNsCompletions(Expr* ex,const std::string& start,std::list<SymInfo*>& lst);

  void indexateFunc(FuncDeclStatement& fds);

  ZorroVM vm;
  ZMacroExpander zmx;
  CodeGenerator cg;
  SymbolRefList sl;
  bool fillSymList=false;

};

#endif

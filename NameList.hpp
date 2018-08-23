#ifndef __ZORRO_SYMLIST_HPP__
#define __ZORRO_SYMLIST_HPP__

#include <list>
#include <string>
#include "ZString.hpp"
#include "FileReader.hpp"
#include <kst/Format.hpp>

namespace zorro{

struct Name{
  ZStringRef val;
  FileLocation pos;
  Name(){}
  Name(const Name& argName):val(argName.val),pos(argName.pos){}
  Name(const ZStringRef& argVal,const FileLocation& argPos=FileLocation()):val(argVal),pos(argPos){}
};

class NameList:public std::list<Name>{
public:
  void dump(std::string& out,const char* sep=",")
  {
    bool first=true;
    for(iterator it=begin(),e=end();it!=e;++it)
    {
      if(first)
      {
        first=false;
      }else
      {
        out+=sep;
      }
      out+=it->val.c_str();
    }
  }
};

struct Symbol{
  Name name;
  NameList* ns;
  bool global;
  Symbol(Name argName=Name(),NameList* argNs=0,bool argGlobal=false):name(argName),ns(argNs),global(argGlobal){}
  Symbol(const Symbol& argOther):name(argOther.name),ns(argOther.ns),global(argOther.global){}
  std::string toString()const
  {
    std::string rv;
    if(ns)
    {
      for(NameList::iterator it=ns->begin(),end=ns->end();it!=end;++it)
      {
        if(!rv.empty())rv+="::";
        rv+=it->val.c_str();
      }
      if(!rv.empty())rv+="::";
    }
    rv+=name.val.c_str();
    if(global)rv.insert(0,"::");
    return rv;
  }
};

inline void customformat(kst::FormatBuffer& buf,const zorro::Name& name,int,int)
{
  CStringWrap wrap(0,0,0);
  name.val.c_str(wrap);
  buf.Append(wrap.c_str(),wrap.getLength());
}

inline void customformat(kst::FormatBuffer& buf,const zorro::Symbol& sym,int,int)
{
  const std::string& str=sym.toString();
  buf.Append(str.c_str(),str.length());
}


}


#endif

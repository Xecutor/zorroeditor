#include "FileReader.hpp"

namespace zorro{

FileRegistry::~FileRegistry()
{
  for(EntryVector::iterator it=entries.begin(),end=entries.end();it!=end;++it)
  {
    delete *it;
  }
  for(std::vector<FileReader*>::iterator it=readers.begin(),end=readers.end();it!=end;++it)
  {
    delete *it;
  }
}

//FileRegistry freg;
FileReader* FileRegistry::newReader(Entry* argEntry)
{
  FileReader* rv=new FileReader(this,argEntry);
  readers.push_back(rv);
  return rv;
}

}

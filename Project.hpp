#ifndef __ZE_PROJECT_HPP__
#define __ZE_PROJECT_HPP__

#include "ui/UIObject.hpp"
#include <string>
#include <map>
#include "FileReader.hpp"
#include "ZIndexer.hpp"

class Editor;
class FileList;
class Console;

class Project{
public:
  Project(Console* con);
  void load(const std::string& argFileName);
  void addFile(const std::string& argFileName);
  Editor* openOrSwitchFile(const std::string& argFileName);

  zorro::FileRegistry& getFileRegistry()
  {
    return freg;
  }
  ZIndexer& getIndexer()
  {
    return idx;
  }
  FileList* getFileList()
  {
    return fl;
  }
  ZorroVM& getZVM()
  {
    return vm;
  }
  void loadScript(FileRegistry::Entry* fe);
  FileReader& getNativeFileReader()
  {
    return natFr;
  }
protected:
  struct ProjectEntry{
    ProjectEntry():editor(0),fr(0){}
    ProjectEntry(const std::string& argFileName):fileName(argFileName),editor(0),fr(0){}
    std::string fileName;
    glider::ui::UIObjectRef editor;
    zorro::FileReader* fr;
  };
  typedef std::map<std::string,ProjectEntry*> FilesMap;
  FilesMap files;
  std::string basePath;
  zorro::FileRegistry freg;
  ZIndexer idx;
  FileList* fl;

  FileRegistry::Entry* natEntry=nullptr;
  FileReader natFr;
  FileRegistry::Entry* scrEntry=nullptr;
  FileReader scrFr;
  ZorroVM vm;

  void registerInZVM();
};

#endif

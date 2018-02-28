#ifndef __GLIDER_USTRING_HPP__
#define __GLIDER_USTRING_HPP__

#include "Utility.hpp"

namespace glider{

class UString{
public:
  UString():str(0),bytes(0),capacity(0),symbols(0),modified(false){}
  UString(const char* argStr,int argLength);
  UString(const UString& argStr);
  ~UString();
  UString& operator+=(const UString& rhs);
  UString& operator=(const char* argStr);
  ushort getNext(int& pos)const;
  static int getNextPos(const char* argStr,int pos);
  static ushort getNextSymbol(const char* argStr,int& pos);
  static int calcLength(const char* str,int bytes);
  int getSize()const
  {
    return bytes;
  }
  int getLength()const
  {
    if(modified)
    {
      updateLength();
    }
    return symbols;
  }

  bool isEmpty()const
  {
    return bytes==0;
  }

  int getLetterOffset(int idx)const;

  static int ucs2ToUtf8(ushort symbol,char* buf);

protected:
  char* str;
  int bytes;
  int capacity;
  mutable int symbols;
  mutable bool modified;

  void updateLength()const;
};

}

#endif

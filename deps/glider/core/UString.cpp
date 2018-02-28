#include "UString.hpp"
#include <memory.h>

namespace glider{

UString::UString(const char* argStr,int argLength)
{
  modified=false;
  symbols=0;
  bytes=argLength;
  capacity=argLength+1;
  str=new char[capacity];
  memcpy(str,argStr,bytes);
  str[bytes]=0;
  updateLength();
}

UString::UString(const UString& argStr)
{
  argStr.updateLength();
  bytes=argStr.bytes;
  capacity=argStr.capacity;
  str=new char[capacity];
  memcpy(str,argStr.str,capacity);
  symbols=argStr.symbols;
  modified=false;
}

UString::~UString()
{
  if(str)delete [] str;
}

UString& UString::operator=(const char* argStr)
{
  bytes=strlen(argStr);
  if(bytes>capacity)
  {
    capacity=bytes+1;
    char* newstr=new char[capacity];
    delete [] str;
    str=newstr;
  }
  memcpy(str,argStr,bytes);
  modified=true;
  return *this;
}


UString& UString::operator+=(const UString& rhs)
{
  if(bytes+rhs.bytes>capacity)
  {
    capacity=bytes+rhs.bytes;
    capacity+=capacity/2+1;
    char* newstr=new char[capacity];
    memcpy(newstr,str,bytes);
    delete [] str;
    str=newstr;
  }
  memcpy(str+bytes,rhs.str,rhs.bytes);
  bytes+=rhs.bytes;
  str[bytes]=0;
  modified=true;
  return *this;
}

int UString::getNextPos(const char* argStr,int pos)
{
  uint c=(unsigned char)argStr[pos];
  pos++;
  if(c<0x80)
  {
    return pos;
  }else if ((c>>5)==0x06)
  {
    return pos+1;
  }else if ((c>>4)==0x0e)
  {
    return pos+2;
  }else if ((c>>3)==0x1e)
  {
    return pos+3;
  }
  return pos;
}

ushort UString::getNext(int& pos)const
{
  if(pos>=bytes)
  {
    return 0;
  }
  uint c=getNextSymbol(str,pos);
  return (ushort)c;
}

int UString::calcLength(const char* str,int bytes)
{
  int symbols=0;
  for(int i=0;i<bytes;symbols++)
  {
    unsigned char c=str[i];
    if(c<0x80)
    {
      i++;
    }else if ((c>>5)==0x06)
    {
      i+=2;
    }else if ((c>>4)==0x0e)
    {
       i+=3;
    }else if ((c>>3)==0x1e)
    {
      i+=4;
    }
    else
    {
      //error?
      i++;
    }
  }
  return symbols;
}

void UString::updateLength()const
{
  symbols=calcLength(str,bytes);
  modified=false;
}

int UString::getLetterOffset(int idx)const
{
  int rv=0;
  for(int i=0;i<idx;i++)
  {
    getNext(rv);
  }
  return rv;
}


ushort UString::getNextSymbol(const char* str,int& pos)
{
  uint c=(unsigned char)str[pos];
  pos++;
  if(c<0x80)
  {
    return (ushort)c;
  }else if ((c>>5)==0x06)
  {
    c=((c<<6)&0x7ff)+(str[pos++] & 0x3f);
  }else if ((c>>4)==0x0e)
  {
    c=((c<<12)&0xffff)|((((uint)(uchar)str[pos++])<<6)&0x0fff);
    c|=(str[pos++]&0x3f);
  }else if ((c>>3)==0x1e)
  {
    c=((c<<18)&0x1fffff)+((((uint)(uchar)str[pos++])<<12)&0x0003ffff);
    c|=(((uint)(uchar)str[pos++])<<6)&0x0fff;
    c|=((uint)(uchar)str[pos++])&0x3f;
  }
  else
  {
    c=' ';
  }
  return (ushort)c;
}

int UString::ucs2ToUtf8(ushort symbol,char* buf)
{
  int rv=0;
  if (symbol<0x80)
  {
      buf[rv++]=symbol&0xff;
  }
  else if (symbol<0x800)
  {
      buf[rv++]=((symbol>>6)|0xc0)&0xff;
      buf[rv++]=(symbol&0x3f)|0x80;
  }
  else
  {
    buf[rv++]=((symbol>>12)|0xe0)&0xff;
    buf[rv++]=((symbol>>6)&0x3f)|0x80;
    buf[rv++]=(symbol&0x3f)|0x80;
  }
  buf[rv]=0;
  return rv;
}


}

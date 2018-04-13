#include "Text.hpp"
#include "SysHeaders.hpp"
#include <memory.h>
#include <iosfwd>
#include "GLState.hpp"
#include "Engine.hpp"

using std::fpos;

namespace glider{

Text::Text(FontRef argFnt,const char* argStr,bool argRawText,int argMaxWidth, bool argWordWrap)
{
  width=0;
  height=0;
  rawText=argRawText;
  fnt=argFnt.get()?argFnt:engine.getDefaultFont();
  setText(argStr,argRawText,argMaxWidth,argWordWrap);
}

Text::Text(const char* str,bool argRawText,int argMaxWidth, bool wordWrap)
{
  width=0;
  height=0;
  fnt=engine.getDefaultFont();
  setText(str,argRawText,argMaxWidth,wordWrap);
}

void Text::setText(const char* argStr,bool argRawText,int argMaxWidth, bool argWordWrap)
{
  str=argStr;
  rawText=argRawText;
  maxWidth=argMaxWidth;
  wordWrap=argWordWrap;
  prepare(argRawText);
}

void Text::prepare(bool rawText)
{
  linesCount=1;
  linesStart.clear();
  int len=str.getLength();
  if(len==0)
  {
    return;
  }
  if(!fnt.get())
  {
    return;
  }
  fnt->prepareForString(str);

  VxVector& vbuf=vb.getVBuf();
  vbuf.reserve(len*4*2);
  VxVector& tbuf=vb.getTBuf();
  tbuf.reserve(len*4*2);
  ClrVector& cbuf=vb.getCBuf();
  cbuf.reserve(len*4*2);
  vbuf.clear();
  tbuf.clear();
  cbuf.clear();

  int fh=fnt->getHeight();
  //int asc=fnt->getAscent();
  float x=0.0,y=0.0;
  int lineWidth=0;
  int lastWordEndOff=0;
  int lastWordEndIdx=0;
  int lastWordEndSymIdx=0;
  bool lastGlyphNonSpace=false;
  bool wasNewLine=false;
  bool haveWordOnLine=false;
  Color curClr=clr;
  Color lastClr;
  bool colorActive=false;
  int symIdx=0;
  width=0;
  for(int i=0;i<str.getSize();++symIdx)
  {
    ushort c=str.getNext(i);
    if(c=='\n')
    {
      x=0.0;
      y+=fh;
      lineWidth=0;
      continue;
    }

    if(!rawText && c=='^')
    {
      c=str.getNext(i);
      if(c!='^')
      {
        if(c=='<')
        {
          curClr=lastClr;
          colorActive=false;
          continue;
        }
        lastClr=curClr;
        colorActive=true;
        switch(c)
        {
          case 'r':curClr=Color(1,0,0);break;
          case 'g':curClr=Color(0,1,0);break;
          case 'b':curClr=Color(0,0,1);break;
          case 'y':curClr=Color(1,1,0);break;
          case 'c':curClr=Color(0,1,1);break;
          case 'v':curClr=Color(1,0,1);break;
          case 'w':curClr=Color(1,1,1);break;
          case '0':
          case '1':
          case '2':
          case '3':
          case '4':
          case '5':
          case '6':
          case '7':
          case '8':
          case '9':
            curClr.r=(c-'0')/9.0f;
            c=str.getNext(i);
            curClr.g=(c-'0')/9.0f;
            c=str.getNext(i);
            curClr.b=(c-'0')/9.0f;
            break;
        }
        continue;
      }
    }



    if(maxWidth>0)
    {
      /*if(wasNewLine && c==' ')
      {
        continue;
      }*/
      if(wordWrap)
      {
        if(c==' ' && lastGlyphNonSpace)
        {
          haveWordOnLine=true;
          lastWordEndOff=vbuf.size();
          lastWordEndIdx=i;
          lastWordEndSymIdx=symIdx;
        }
        lastGlyphNonSpace=c!=' ';
      }
    }

    GlyphInfo gi=fnt->getGlyph(c);

    if(maxWidth>0)
    {
      if(!wordWrap)
      {
        if(lineWidth+gi.adv>maxWidth)
        {
          lineWidth=0;
          x=0.0;
          y+=fh;
          linesCount++;
          linesStart.push_back(symIdx);
          wasNewLine=true;
        }else
        {
          wasNewLine=false;
        }
        lineWidth+=(int)gi.adv;
      }else
      {
        if(lineWidth+gi.adv>maxWidth)
        {
          lineWidth=0;
          x=0.0;
          y+=fh;
          wasNewLine=true;
          linesCount++;
          if(c!=' ' && haveWordOnLine)
          {
            if(colorActive)
            {
              curClr=lastClr;
            }
            vbuf.erase(vbuf.begin()+lastWordEndOff,vbuf.end());
            tbuf.erase(tbuf.begin()+lastWordEndOff,tbuf.end());
            cbuf.erase(cbuf.begin()+lastWordEndOff,cbuf.end());
            i=lastWordEndIdx;
            symIdx=lastWordEndSymIdx;
          }
          linesStart.push_back(symIdx);
          haveWordOnLine=false;
          continue;
        }else
        {
          wasNewLine=false;
        }
        lineWidth+=(int)gi.adv;
      }
    }

    Pos sz=gi.texSize/(float)fnt->getTexSize();
    Rect texRect(gi.texPos,sz);
    Rect verRect(Pos(x+gi.xshift,y+gi.yshift/*+asc-gi.maxy*/),Pos(gi.texSize.x,gi.texSize.y));
    /*if(gi.maxx==gi.texSize.x && gi.minx>0)
    {
      verRect.pos.x-=1;
    }*/

    verRect.pushQuad(vbuf);
    texRect.pushQuad(tbuf);
    cbuf.push_back(curClr);
    cbuf.push_back(curClr);
    cbuf.push_back(curClr);
    cbuf.push_back(curClr);

    x+=gi.adv;
    if(x>width)
    {
      width=(int)x;
    }
  }
  height=(int)(y+fh);
  vb.update();
  vb.setSize(len*4);
}

void Text::getLetterExtent(int idx,Pos& argPos,Pos& argSize)
{
  if(!str.getLength())
  {
    return;
  }
  if(idx>=str.getLength())
  {
    idx=str.getLength()-1;
  }
  if(idx<0)
  {
    idx=0;
  }
  VxVector& v=vb.getVBuf();
  argPos=v[idx*4];
  argSize=v[idx*4+2]-argPos;
  int off=str.getLetterOffset(idx);
  ushort sym=str.getNext(off);
  GlyphInfo gi=fnt->getGlyph(sym);
  //argPos.x-=gi.minx?1:0;
  argSize.x=gi.adv;
}


void Text::updateColors(const ClrVector& clrs,int from,int to)
{
  ClrVector& cbuf=vb.getCBuf();
  if(to==-1)to=clrs.size();
  if(from>=(int)cbuf.size() || to<=from)
  {
    return;
  }
  if(to>(int)cbuf.size() || to<0)
  {
    to=cbuf.size();
  }
  ClrVector::iterator it=vb.getCBuf().begin()+from,end=vb.getCBuf().begin()+to;
  ClrVector::const_iterator cit=clrs.begin(),cend=clrs.end();
  for(;it!=end && cit!=cend;++it,++cit)
  {
    *it=*cit;
  }
  vb.update(ufColors,from,to);
}


Text::~Text()
{
}

void Text::reload()
{
  prepare(rawText);
}

void Text::draw()
{
  if(str.isEmpty())
  {
    return;
  }
  state.loadIdentity();
  state.translate(pos);
  state.enableTexture();
  fnt->getTexture().bind();

  vb.draw();

}

}

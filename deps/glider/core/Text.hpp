#ifndef __GLIDER_TEXT_HPP__
#define __GLIDER_TEXT_HPP__

#include "Utility.hpp"
#include "Font.hpp"
#include "Drawable.hpp"
#include <string>
#include <vector>
#include "UString.hpp"
#include "VertexBuffer.hpp"

namespace glider{


class Text:public Drawable{
public:
  Text(FontRef fnt=0,const char* str="",bool rawText=false,int maxWidth=0, bool wordWrap=true);
  Text(const char* str,bool rawText=false,int maxWidth=0, bool wordWrap=true);
  virtual ~Text();
  void assignFont(FontRef argFnt)
  {
    fnt=argFnt;
    prepare(rawText);
  }
  void setText(const char* str,bool rawText=false,int maxWidth=0, bool wordWrap=true);
  void setPosition(const Pos& argPos)
  {
    pos=argPos;
  }
  const Pos& getPosition()const
  {
    return pos;
  }
  void setColor(const Color& argClr)
  {
    clr=argClr;
    prepare(rawText);
  }
  void draw();
  void reload();

  int getWidth()const
  {
    return width;
  }

  int getHeight()const
  {
    return height;
  }

  void updateColors(const ClrVector& clrs,int from=0,int to=-1);
  const ClrVector& getColors()const
  {
    return vb.getCBuf();
  }

  void getLetterExtent(int idx,Pos& argPos,Pos& argSize);
  int getTextLength()const
  {
    return str.getLength();
  }

  int getLinesCount()const
  {
    return linesCount;
  }

  const std::vector<int>& getLinesStart()const
  {
    return linesStart;
  }

  const UString& getUString()const
  {
    return str;
  }

  FontRef getFont()
  {
    return fnt;
  }

protected:
  Pos pos;
  Color clr;
  FontRef fnt;
  UString str;
  bool rawText;
  int maxWidth;
  bool wordWrap;
  int width;
  int height;
  int linesCount;
  std::vector<int> linesStart;
  VertexBuffer vb;
  void prepare(bool rawText);
};

typedef ReferenceTmpl<Text> TextRef;


}

#endif

#include "Layout.hpp"
#include "UIContainer.hpp"
#include <kst/Throw.hpp>

namespace glider{
namespace ui{


Layout::Layout(const char* code):main(code)
{
}

static const char* skipMarkup(const char* ptr)
{
  int cnt=0;
  const char* org=ptr;
  do{
    if(*ptr=='%' || *ptr=='#')cnt++;
    if(*ptr=='!')cnt--;
    ++ptr;
    if(!ptr)KSTHROW("unbalanced code %s",org);
  }while(cnt>0);
  return ptr;
}

Layout::Area::Area(const char* code):ItemBase(litArea)
{
  hsp=5;
  vsp=5;
  switch(code[0])
  {
    case 'N':lfm=lfmMin;break;
    case 'E':lfm=lfmEven;break;
    case 'X':lfm=lfmMax;break;
    default:KSTHROW("Invaid area code:%s",code);
  }
  switch(code[1])
  {
    case 'L':
    case 'R':lpm=lpmFromLeftToRight;break;
    case 'T':
    case 'B':lpm=lpmFromTopToBottom;break;
    default:KSTHROW("Invaid area code:%s",code);
  }
  switch(code[2])
  {
    case 'L':hla=hlaLeft;break;
    case 'R':hla=hlaRight;break;
    case 'C':hla=hlaCenter;break;
    default:KSTHROW("Invaid area code:%s",code);
  }
  switch(code[3])
  {
    case 'T':vla=vlaTop;break;
    case 'B':vla=vlaBottom;break;
    case 'C':vla=vlaCenter;break;
    default:KSTHROW("Invaid area code:%s",code);
  }
  if(code[4]==0)return;
  const char* ptr=code+4;
  if(*ptr=='[')
  {
    int n;
    if(sscanf(ptr,"[%d,%d]%n",&hsp,&vsp,&n)!=2)
    {
      KSTHROW("Invaid area code:%s",code);
    }
    ptr+=n;
  }
  if(*ptr!=':')
  {
    KSTHROW("Invaid area code:%s",code);
  }
  ++ptr;
  std::string nm;
  while(*ptr && *ptr!='!')
  {
    if(*ptr=='%')
    {
      addItem(new Area(ptr+1));
      ptr=skipMarkup(ptr);
      if(*ptr==',')++ptr;
      continue;
    }
    if(*ptr=='#')
    {
      addItem(new Layout::Grid(ptr+1));
      ptr=skipMarkup(ptr);
      if(*ptr==',')++ptr;
      continue;
    }
    const char* nameStart=ptr;
    while(*ptr && *ptr!=',' && *ptr!='!' && *ptr!='*' && *ptr!='-' && *ptr!='|')++ptr;
    nm.assign(nameStart,ptr-nameStart);
    bool maxH=false, maxV=false;
    if(*ptr=='*')
    {
      maxH=true;
      maxV=true;
      ++ptr;
    }
    if(*ptr=='-')
    {
      maxH=true;
      ++ptr;
    }
    if(*ptr=='|')
    {
      maxV=true;
      ++ptr;
    }
    if(*ptr==',')++ptr;
    addItem(new Object(nm,maxH, maxV));
  }
}


void Layout::Area::calcSize()
{
  r.size=Pos((float)hsp,(float)vsp);
  for(ItemVector::iterator it=items.begin(),end=items.end();it!=end;++it)
  {
    (*it)->calcSize();
    Pos sz=(*it)->getSize();
    if(lpm==lpmFromTopToBottom)
    {
      r.size.y+=sz.y+vsp;
      if(sz.x+hsp*2>r.size.x)
      {
        r.size.x=sz.x+hsp*2;
      }
    }else
    {
      r.size.x+=sz.x+hsp;
      if(sz.y+vsp*2>r.size.y)
      {
        r.size.y=sz.y+vsp*2;
      }
    }
  }
}

void Layout::Area::update(const Pos& argPos,const Pos& argSize)
{
  std::vector<LayoutFillMode> fillModes;
  std::vector<float> sizes;
  std::vector<int> mx;
  std::vector<int> even;

  float Pos::*ptr=lpm==lpmFromLeftToRight?&Pos::x:&Pos::y;
  float Pos::*optr=lpm==lpmFromLeftToRight?&Pos::y:&Pos::x;
  int sp=lpm==lpmFromLeftToRight?hsp:vsp;
  int osp=lpm==lpmFromLeftToRight?vsp:hsp;
  int la=lpm==lpmFromLeftToRight?(int)hla:(int)vla;
  int ola=lpm==lpmFromLeftToRight?(int)vla:(int)hla;

  float mnsz=(float)sp;
  float evenMx=0.0f;
  {
    int idx=0;
    for(ItemVector::iterator it=items.begin(),end=items.end();it!=end;++it,++idx)
    {
      ItemBase& i=*(*it).get();
      const Pos& sz=i.getSize();
      sizes.push_back(sz.*ptr/*+sp*/);
      if(i.lit==litArea)
      {
        Area& a=i.as<Area>();
        fillModes.push_back(a.lfm);
        if(a.lfm==lfmMax)
        {
          mx.push_back(idx);
          mnsz+=sp;
        }else if(a.lfm==lfmEven)
        {
          even.push_back(idx);
          if(i.getSize().*ptr>evenMx)
          {
            evenMx=i.getSize().*ptr;
          }
          mnsz+=sp;
        }else
        {
          mnsz+=i.getSize().*ptr+sp;
        }
      }else if(i.lit==litObject)
      {
        Object& o=i.as<Object>();
        if((o.maximizeH && (lpm==lpmFromLeftToRight || lpm==lpmFromLeftToRight)) ||
           (o.maximizeV && (lpm==lpmFromTopToBottom || lpm==lpmFromTopToBottom)))
        {
          fillModes.push_back(lfmMax);
          mx.push_back(idx);
          mnsz+=sp;
        }else
        {
          fillModes.push_back(lfmMin);
          mnsz+=i.getSize().*ptr+sp;
        }
      }else
      {
        fillModes.push_back(lfmMin);
        mnsz+=i.getSize().*ptr+sp;
      }
    }
  }
  for(size_t i=0;i<even.size();i++)
  {
    int idx=mx[i];
    sizes[idx]=evenMx;
    mnsz+=evenMx;
  }
  for(size_t i=0;i<mx.size();i++)
  {
    int idx=mx[i];
    sizes[idx]=(argSize.*ptr-mnsz)/mx.size();
  }
  if(!mx.empty())
  {
    r.size=argSize;
  }else
  {
    r.size.*ptr=mnsz;
    //r.size.*optr=argSize.*optr;
  }

  Pos pos=argPos;
  Pos sz;
  sz.*optr=argSize.*optr;
  switch(la)
  {
    case 0:pos.*ptr+=sp;break;
    case 1:pos.*ptr+=(int)(argSize.*ptr-r.size.*ptr)/2+sp;break;
    case 2:pos.*ptr+=argSize.*ptr-r.size.*ptr+sp;break;
  }

  {
    int idx=0;
    for(ItemVector::iterator it=items.begin(),end=items.end();it!=end;++it,++idx)
    {
      ItemBase& i=*(*it).get();
      sz.*ptr=sizes[idx]-sp*2;
      switch(ola)
      {
        case 0:pos.*optr=argPos.*optr+osp;break;
        case 1:pos.*optr=argPos.*optr+(int)(argSize.*optr-sz.*optr+sp*2)/2;break;
        case 2:pos.*optr=argSize.*optr-r.size.*optr+osp;break;
      }
      i.update(pos,sz);
      sz=i.getSize();
      pos.*ptr+=sp;
      pos.*ptr+=sizes[idx];
    }
  }
}

void Layout::Object::fillObjects(UIContainer* con)
{
  if(!obj.get())
  {
    obj=con->findByName(name);
    if(!obj.get())
    {
      KSTHROW("object with name %{} not found",name);
    }
  }
}

void Layout::Area::fillObjects(UIContainer* con)
{
  for(ItemVector::iterator it=items.begin(),end=items.end();it!=end;++it)
  {
    (*it)->fillObjects(con);
  }
}

Layout::Grid::Grid(const char* code):ItemBase(litGrid)
{
  hsp=3;
  vsp=3;
  const char* ptr=code;
  while(*ptr && *ptr!=':' && *ptr!='[')
  {
    ColumnInfo ci;
    switch(*ptr)
    {
      case 'L':ci.la=hlaLeft;break;
      case 'R':ci.la=hlaRight;break;
      case 'C':ci.la=hlaCenter;break;
      default:KSTHROW("Invaid grid code:%s",code);
    }
    colInfo.push_back(ci);
    ++ptr;
  }
  width=colInfo.size();
  if(*ptr=='[')
  {
    int n;
    if(sscanf(ptr,"[%d,%d]%n",&hsp,&vsp,&n)!=2)
    {
      KSTHROW("Invaid area code:%s",code);
    }
    ptr+=n;
  }
  if(*ptr!=':')
  {
    KSTHROW("Invaid grid code:%s",code);
  }
  ++ptr;
  std::string nm;
  int y=0;
  while(*ptr && *ptr!='!')
  {
    grid.resize(y+1);
    grid[y].resize(width);
    for(int x=0;x<width;++x)
    {
      if(*ptr=='%')
      {
        setItemAt(x,y,new Area(ptr+1));
        ptr=skipMarkup(ptr);
        continue;
      }
      if(*ptr=='#')
      {
        setItemAt(x,y,new Grid(ptr+1));
        ptr=skipMarkup(ptr);
        continue;
      }
      const char* nameStart=ptr;
      while(*ptr && *ptr!=',' && *ptr!='|' && *ptr!='!')++ptr;
      //if(!*ptr)KSTHROW("Invaid grid code:%s, expected separator at %d",code,(int)(ptr-code));
      nm.assign(nameStart,ptr-nameStart);
      setItemAt(x,y,new Object(nm));
      if(*ptr==',')++ptr;
    }
    if(*ptr=='|')++ptr;
    ++y;
  }
  height=y;
}

void Layout::Grid::calcSize()
{
  r.size=Pos();
  std::vector<int> colWidths(width,0);
  std::vector<int> rowHeights(height,0);
  colInfo.resize(width);
  for(int y=0;y<height;y++)
  {
    for(int x=0;x<width;x++)
    {
      Pos sz=cell(x,y)->getSize();
      if(sz.x>colWidths[x])
      {
        colWidths[x]=(int)sz.x;
        colInfo[x].width=(int)sz.x;
      }
      if(sz.y>rowHeights[y])
      {
        rowHeights[y]=(int)sz.y;
      }
    }
  }
  for(int x=0;x<width;x++)
  {
    r.size.x+=colWidths[x]+hsp*2;
  }
  for(int y=0;y<height;y++)
  {
    r.size.y+=rowHeights[y]+vsp*2;
  }
}

void Layout::Grid::fillObjects(UIContainer* con)
{
  for(int y=0;y<height;y++)
  {
    for(int x=0;x<width;x++)
    {
      cell(x,y)->fillObjects(con);
    }
  }
}

void Layout::Grid::update(const Pos& argPos,const Pos& argSize)
{
  std::vector<int> colWidths(width,0);
  std::vector<int> rowHeights(height,0);
  for(int y=0;y<height;y++)
  {
    for(int x=0;x<width;x++)
    {
      ItemRef ir=cell(x,y);
      Pos sz=ir->getSize();
      if(sz.x>colWidths[x])
      {
        colWidths[x]=(int)sz.x;
        colInfo[x].width=(int)sz.x;
      }
      if(sz.y>rowHeights[y])
      {
        rowHeights[y]=(int)sz.y;
      }
    }
  }
  Pos p;
  p.y=argPos.y+vsp;
  for(int y=0;y<height;y++)
  {
    p.x=argPos.x+hsp;
    for(int x=0;x<width;x++)
    {
      ItemRef ir=cell(x,y);
      Pos sz=ir->getSize();
      int w=colWidths[x];
      Pos pp=p;
      switch(colInfo[x].la)
      {
        case hlaLeft:pp.x+=hsp;break;
        case hlaCenter:pp.x+=(w-sz.x)/2;break;
        case hlaRight:pp.x+=w-sz.x;break;
      }
      ir->update(pp,sz);
      p.x+=hsp+w;
    }
    p.y+=rowHeights[y]+vsp*2;
  }
}


}
}

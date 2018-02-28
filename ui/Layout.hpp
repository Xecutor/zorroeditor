#ifndef __GLIDER_UI_LAYOUT_HPP__
#define __GLIDER_UI_LAYOUT_HPP__

#include "UIObject.hpp"

namespace glider{
namespace ui{

enum LayoutPlacementMode{
  lpmFromTopToBottom,lpmFromLeftToRight,lpmFillMode
};

enum LayoutFillMode{
  lfmMin,lfmEven,lfmMax
};

enum LayoutHAlignment{
  hlaLeft,hlaCenter,hlaRight
};
enum LayoutVAlignment{
  vlaTop,vlaCenter,vlaBottom
};

enum LayoutItemType{
  litObject,
  litArea,
  litGrid
};

class UIContainer;

class Layout:public Managed{
public:
  Layout(const char* code);

  struct ItemBase:Managed{
    ItemBase(LayoutItemType argLit):lit(argLit){}
    LayoutItemType lit;
    virtual Pos getSize()=0;
    virtual void calcSize(){}
    virtual void update(const Pos& argPos,const Pos& argSize)=0;
    virtual void fillObjects(UIContainer* con)=0;
    template <class T>
    T& as()
    {
      return *((T*)this);
    }
  };

  typedef ReferenceTmpl<ItemBase> ItemRef;

  typedef std::vector<ItemRef> ItemVector;

  struct Object:ItemBase{
    UIObjectRef obj;
    std::string name;
    bool maximizeH, maximizeV;
    Object(const std::string& argName,bool argMaximizeH=false, bool argMaximizeV=false):
      ItemBase(litObject),name(argName),maximizeH(argMaximizeH), maximizeV(argMaximizeV){}
    Object(UIObjectRef argObj, bool argMaximizeH=false,bool argMaximizeV=false):
      ItemBase(litObject),obj(argObj),maximizeH(argMaximizeH),maximizeV(argMaximizeV){}
    Pos getSize()
    {
      return obj.get()?obj->getSize():Pos(0,0);
    }
    void calcSize(){}
    void update(const Pos& argPos,const Pos& argSize)
    {
      obj->setPos(argPos);
      if(maximizeH || maximizeV)
      {
        Pos newSz = obj->getSize();
        if(maximizeH)newSz.x=argSize.x;
        if(maximizeV)newSz.y=argSize.y;
        obj->setSize(newSz);
      }
    }
    virtual void fillObjects(UIContainer* con);
  };
  struct Area:ItemBase{
    Area():ItemBase(litArea){}
    LayoutFillMode lfm;
    LayoutPlacementMode lpm;
    LayoutHAlignment hla;
    LayoutVAlignment vla;
    int hsp,vsp;
    Rect r;
    ItemVector items;

    Area(const char* code);

    void addItem(ItemBase* argItem)
    {
      items.push_back(argItem);
    }

    void calcSize();
    void update(const Pos& argPos,const Pos& argSize);

    void fillObjects(UIContainer* con);

    Pos getSize()
    {
      return r.size;
    }
  };

  struct Grid:ItemBase{
    Grid(const char* code);
    Grid(int argWidth,int argHeight):ItemBase(litGrid)
    {
      setDim(argWidth,argHeight);
      hsp=3;
      vsp=3;
    }

    struct ColumnInfo{
      LayoutHAlignment la;
      LayoutFillMode lfm;
      int width;
      ColumnInfo():la(hlaLeft),lfm(lfmMin){}
    };

    typedef std::vector<ColumnInfo> ColumnInfoVector;
    ColumnInfoVector colInfo;

    typedef std::vector<ItemVector> GridVector;
    Rect r;
    GridVector grid;
    int width,height;
    int hsp,vsp;

    void setDim(int argWidth,int argHeight)
    {
      width=argWidth;
      height=argHeight;
      grid.resize(argHeight);
      for(int i=0;i<argHeight;i++)
      {
        grid[i].resize(argWidth);
      }
      colInfo.resize(argWidth);
    }

    void calcSize();
    void update(const Pos& argPos,const Pos& argSize);
    void fillObjects(UIContainer* con);

    void setItemAt(int x,int y,ItemRef item)
    {
      grid[y][x]=item;
    }

    ItemRef cell(int x,int y)
    {
      return grid[y][x];
    }
    Pos getSize()
    {
      return r.size;
    }
  };

  void add(ItemBase* argItem)
  {
    main.addItem(argItem);
  }
  void add(UIObjectRef argObj,bool maximize=false)
  {
    main.addItem(new Object(argObj,maximize));
  }

  Pos calcSize()
  {
    main.calcSize();
    return getSize();
  }

  void update(const Pos& argPos,const Pos& argSize)
  {
    main.calcSize();
    main.update(argPos,argSize);
  }
  Pos getSize()const
  {
    return main.r.size;
  }

  void fillObjects(UIContainer* con)
  {
    main.fillObjects(con);
  }
  Area& getMainArea()
  {
    return main;
  }
protected:
  Area main;

};

typedef ReferenceTmpl<Layout> LayoutRef;


}
}

#endif

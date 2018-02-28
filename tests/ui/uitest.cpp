#include <stdio.h>
#include "Engine.hpp"
#include "Button.hpp"
#include "Layout.hpp"
#include "UIConfig.hpp"
#include "EventCallback.hpp"
#include "UIContainer.hpp"
#include "UIRoot.hpp"
#include "Window.hpp"
#include "Label.hpp"
#include "EditInput.hpp"
#include "ScrollBar.hpp"
#include "ListBox.hpp"
#include "ResourcesManager.hpp"

using namespace glider::ui;

class Test{
public:
  typedef Test ThisClass;
  void method()
  {
    printf("hello\n");
  }
  void method2(const int& x)
  {
    printf("x=%d\n",x);
  }

  void test()
  {
    using namespace glider::ui;
    EventCallback<void> eh=MKCALLBACK0(method);
    eh.execute();
  }
};

class ScrollEventHandler{
protected:
  LabelRef lbl;
public:
  typedef ScrollEventHandler ThisClass;
  ScrollEventHandler(ScrollBar* scr,LabelRef argLbl):lbl(argLbl)
  {
    scr->setValueChangeHandler(MKUICALLBACK(onValueChanged));
  }
  void onValueChanged(const UIEvent& evt)
  {
    lbl->setCaption(FORMAT("value:%{}",(int)((ScrollBar*)evt.sender)->getValue()));
  }
};

int cnt=0;

void onClick(void* data,const glider::ui::UIEvent& e)
{
  using glider::ui::Button;
  Button* b=(Button*)data;
  char buf[32];
  sprintf(buf,"test%d",cnt++);
  b->setCaption(buf);
}

void onClick2(void* data,const glider::ui::UIEvent& e)
{
  glider::engine.exitApp();
}

void onClick3(void* data,const glider::ui::UIEvent& e)
{
  glider::ui::Window* w=(glider::ui::Window*)data;
  w->setVisible(!w->isVisible());
}

void addClick(void* data,const UIEvent& e)
{
  glider::ui::Window* w=(glider::ui::Window*)data;
  static int cnt=0;
  ListBox* lb=w->findByName("lb").as<ListBox>();
  Label* l=new Label(FORMAT("one more %{}",cnt++));
  l->setColor(glider::Color::black);
  lb->addObject(l);
}

int GliderAppMain(int argc,char* argv[])
{
  //Test t;
  //t.test();

  kst::Logger::Init("uitest.log");
  try{
  using namespace glider;
  using namespace glider::ui;
  engine.setVSync(true);
  engine.selectResolution(1024,768,false);
  engine.setClearColor(Color(0,0,0,0.5));
  //engine.selectResolution(800,480,false);
  engine.setResolution();
  engine.setDefaultFont(manager.getFont("FreeMono.ttf"/*"FSEX300.ttf"*/,16));
  uiConfig.init();

  engine.assignHandler(root);
  Button* b=new Button();
  b->setEventHandler(betOnClick,mkEventCallback(b,onClick));
  b->setName("button1");
  b->setCaption("test");
  b->setPos(Pos(100,100));
  root->addObject(b);


  Button* b2=new Button();
  b2->setName("button2");
  b2->setCaption("Quit");
  b2->setPos(Pos(100,100+b->getSize().y+5));
  b2->setEventHandler(betOnClick,mkEventCallback(0,onClick2));
  root->addObject(b2);

  Window* w3=new Window(Pos(400,400),Pos(300,300),"scroller test");
  {
    ListBox* lb=new ListBox;
    lb->setName("lb");
    lb->setSize(Pos(200,200));
    lb->setPos(Pos(1,1));
    Label* l=new Label("hello");l->setColor(Color::black);
    lb->addObject(l);
    l=new Label("world");l->setColor(Color::black);
    lb->addObject(l);
    l=new Label("lalala");l->setColor(Color::black);
    lb->addObject(l);
    w3->addObject(lb);
    Button* b=new Button("Add");
    b->setPos(Pos(202,1));
    b->setEventHandler(betOnClick,mkEventCallback(w3,addClick));
    w3->addObject(b);
  }
  //ScrollBar* sb=new ScrollBar(100);
  //Label* ll=new Label;
  //ScrollEventHandler seh(sb,ll);
  //w3->addObject(sb);
  //w3->addObject(ll);
  root->addObject(w3);

  Window* w=new Window(Pos(200,200),Pos(200,200),"window title");
  Layout* l=new Layout("XLCC");
  Button* b3a=new Button();
  b3a->setName("button3a");
  b3a->setCaption("hello3a");
  l->add(b3a);
  w->addObject(b3a);
  Button* b3b=new Button();
  b3b->setName("button3b");
  b3b->setCaption("hello3b");
  l->add(b3b);
  w->addObject(b3b);
  Label* lbl=new Label("hello!");
  l->add(lbl);
  w->addObject(lbl);

  l->update(Pos(0,0),w->getSize());
  w->setSize(l->getSize()+Pos(0,w->getTitleHeight()));
  w->setLayout(l);
  w->setResizable(false);



  root->addObject(w);

  Window* w2=new Window(Pos(500,200),Pos(200,200),"xxAxx other title");
  Button* b4=new Button();
  b4->setName("button4");
  b4->setCaption("hello4");
  b4->setPos(Pos(10,20));
  w2->addObject(b4);
  Button* b5=new Button();
  b5->setName("button5");
  b5->setCaption("hello5");
  b5->setEventHandler(betOnClick,mkEventCallback(w,onClick3));
  b5->setPos(Pos(10,60));
  w2->addObject(b5);
  EditInput* ed=new EditInput;
  ed->setValue("hello world");
  ed->setPos(Pos(10,90));
  w2->addObject(ed);
  root->addObject(w2);


  //printf("%s\n",b->getFullName().c_str());

  engine.loop(root);
  //engine.loop(&b);
  }catch(std::exception& e) {
    fprintf(stderr, "Exception: %s\n", e.what());
    LOGERROR("main","Exception:%{}", e.what());
  }
  uiConfig.shutdown();
  return 0;
}

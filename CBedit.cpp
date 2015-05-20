#include <stdio.h>
#include <iostream>
#include "Control.cpp"
#include "BufferList.cpp"
#include "Bedit.h"
#include "CursesView.h"
#include "Print.h"

using namespace std;
using namespace BEditor;

void drawList(BufferList<CursesView>& b);
void drawTab(BufferList<CursesView>::bufferStruct &t);

int main (int argc, char *argv[])
{
  Control<CursesView> controller(argc,argv);
  int comm = 0;
   
  mousemask(ALL_MOUSE_EVENTS, 0);
   
  controller.update();
  controller.drawView();
  drawList(controller.buffList);
  controller.updateView();
  controller.clean();
   
  while((comm = getch()) != KEY_F(5)){
    controller.touch();

    comm = comm & CHAR_MASK;
      
    switch(comm){
    case KEY_BACKSPACE:controller.backspaceKey();break;
    case KEY_DC:controller.deleteKey();break;
    case KEY_UP:controller.upKey(comm);break;
    case KEY_DOWN:controller.downKey(comm);break;
    case KEY_LEFT:controller.leftKey(comm);break;
    case KEY_RIGHT:controller.rightKey(comm);break;
    case KEY_F(1):controller.newBuffer();break;
    case KEY_F(2):controller.save(0);break;
    case KEY_F(3):controller.print();break;
    case KEY_F(4):controller.load();break;
    case KEY_F(6):controller.toggleSyntax();break;
    case KEY_F(7):controller.prevBuffer();break;
    case KEY_F(8):controller.nextBuffer();break;
    case KEY_F(9):controller.compileBuffer();break;
    case KEY_F(10):controller.execCommand();break;
    case KEY_F(11):controller.closeBuffer();break;break;
    case KEY_F(12):controller.clearMessages();break;
    case '\t':controller.tabKey();break;
    case KEY_NPAGE:controller.pagedownKey(comm);break;
    case KEY_PPAGE:controller.pageupKey(comm);break;
    case KEY_HOME:controller.homeKey(comm);break;
    case KEY_END:controller.endKey(comm);break;
    case KEY_MOUSE:{
      MEVENT e;
      if(getmouse(&e) != OK)
        controller.buff.view->status(" * Error Getting Mouse Info");
      else{
        //controller.buff.model->insert("MOUSE");
        if(e.bstate & BUTTON1_CLICKED){
          if(e.y > 0)
            controller.buff.view->setCursor(e.x,e.y);
          else{
            controller.buff = controller.buffList.current((e.x-1)/13);
            controller.update();
          }
        }
      }
    }break;
      
    default: controller.generalKey(comm & CHAR_MASK);break;
    }
    
    if(controller.isDirty()){
      controller.drawView();
      drawList(controller.buffList);
      controller.updateView();
      controller.clean();
    } 
  }
}
 
int tabX,tabY;
BufferList<CursesView>::bufferStruct compBuff;

void drawList(BufferList<CursesView>& b)
{
  tabX = 1,
    tabY = 0;
  compBuff = b.current();
  b.foreach(&drawTab);
}

void drawTab(BufferList<CursesView>::bufferStruct &t)
{
  static string s;
  if(t == compBuff)
    CursesView::drawRectangle(tabX,tabY,12,1,1);
  else
    CursesView::drawRectangle(tabX,tabY,12,1,0);

  if(t.model->isDirty())
    s = '*' + t.model->getTitle();
  else s = t.model->getTitle();
  CursesView::drawString(s,tabX+1,tabY,10);
  tabX += 13;
}

#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <unistd.h>
#include <sys/wait.h>
#include <iostream>
#include "Control.cpp"
#include "BufferList.cpp"
#include "Bedit.h"
#include "XView.h"
#include "Print.h"

using namespace std;
using namespace BEditor;

void drawList(BufferList<XView>& b);
void drawTab(BufferList<XView>::bufferStruct &t);

const int tabWidth = 110;

int main (int argc, char *argv[])
{
  Control<XView> controller(argc,argv);
  int comm = 0;
  Display *display = controller.currentView()->viewDisplay();
  XEvent event; 
   
  controller.drawView();
  controller.updateView();
   
  while(1){
    XNextEvent(display, &event);
    switch(event.type) {
    case SelectionRequest:
      controller.buff.view->convertClipboard(event);
      break;
    case SelectionClear:
      break;
    case Expose: controller.updateView();break;
    case ConfigureNotify: controller.update();break;
    case ButtonPress:
      if(event.xbutton.button == Button1){
        controller.touch();
        if(event.xbutton.y < 25){
        }else
          if(event.xbutton.y < 50){
            int i = (event.xbutton.x-5)/tabWidth;
            if(i < controller.buffList.size()){
              controller.buff = controller.buffList.current(i);
              controller.update();
            }
          }else if(event.xbutton.x > controller.buff.view->scrollbarX()){
            controller.buff.view->doScroll(event.xbutton.y);
          }else{
            controller.buff.view->doSelect(event.xmotion.x,event.xbutton.y);
          }
      }else
        if(event.xbutton.button == Button4){
          controller.touch();
          controller.buff.view->scroll(-6);
        }else
          if(event.xbutton.button == Button5){
            controller.touch();
            controller.buff.view->scroll(6);
          }
      break;
                        
    case KeyPress: 
      comm = XKeycodeToKeysym(display,
                              event.xkey.keycode,0);
                
      controller.touch();
      if(event.xkey.state&ShiftMask)
        comm |= SHIFT_MASK;
                  
      if(event.xkey.state&ControlMask)
        controller.commandKey(comm);
      else
        switch(comm & CHAR_MASK){
        case XK_Return:controller.returnKey();break;
        case XK_BackSpace:controller.backspaceKey();break;
        case XK_Delete:controller.deleteKey();break;
        case XK_Up:controller.upKey(comm);break;
        case XK_Down:controller.downKey(comm);break;
        case XK_Left:controller.leftKey(comm);break;
        case XK_Right:controller.rightKey(comm);break;
        case XK_F1:controller.newBuffer();break;
        case XK_F2:controller.save(0);break;
        case XK_F3:controller.print();break;
        case XK_F4:controller.load();break;
        case XK_F5:break;
        case XK_F6:controller.toggleSyntax();break;
        case XK_F7:controller.prevBuffer();break;
        case XK_F8:controller.nextBuffer();break;
        case XK_F9:controller.compileBuffer();break;
        case XK_F10:controller.execCommand();break;
        case XK_F11:controller.configureView();break;
        case XK_F12:controller.clearMessages();break;
        case XK_Tab:controller.tabKey();break;
        case XK_Page_Down:controller.pagedownKey(comm);break;
        case XK_Page_Up:controller.pageupKey(comm);break;
        case XK_Home:controller.homeKey(comm);break;
        case XK_End:controller.endKey(comm);break;
        case XK_Shift_L: case XK_Shift_R:
        case XK_Control_L: case XK_Control_R:break;
        default: comm = XKeycodeToKeysym(display,
                                         event.xkey.keycode,
                                         event.xkey.state&ShiftMask);
          controller.generalKey((char)comm);break;
        }break;
    default:break;
    }    
    if(controller.isDirty()){
      controller.buff.view->clearAll();
      controller.buff.view->redraw();
      drawList(controller.buffList);
      controller.buff.view->update();
    }
    controller.clean();
  }
}
 
int tabX,tabY;
BufferList<XView>::bufferStruct compBuff;

void drawList(BufferList<XView>& b)
{
  tabX = 5,
    tabY = 30;
  compBuff = b.current();
  b.foreach(&drawTab);
}

void drawTab(BufferList<XView>::bufferStruct &t)
{
  static string s;
  if(t != compBuff)
    XView::drawRectangle(tabX,tabY,tabWidth,17,1);
  else
    XView::drawRectangle(tabX,tabY-2,tabWidth,22,1);
  if(t.model->isDirty())
    s = '*' + t.model->getTitle();
  else s = t.model->getTitle();
  XView::drawString(s,tabX+5,tabY,tabWidth-5);
  tabX += tabWidth+2;
}



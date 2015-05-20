// XView.h

#ifndef _XVIEW_H
#define _XVIEW_H

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <string>
#include <set>

#include"Bedit.h"

using namespace BEditor;
using std::string;
using std::set;

class XView : public TextView
{
  int currx,curry;
  static XColor attrColors[11];
  string statusString;
   
  bool syntaxMark;
  bool showNewLine;
  double scrollpix;
  int scrolly,scrollh;
   
  static string exportClip;
   
  static int charWidth,charHeight; 
  static bool view_init; 
  static Display *display;
  static Visual *visual;
  static Window window;
  static Colormap colormap;
  static XGCValues gr_values;
  static XSetWindowAttributes attributes;
  static GC gc;
  static Pixmap pixmap;
  static int refCount;
 
  enum{ NumAttributes = 11, defaultAttr=0,cursorAttr=0x100,keywordAttr=0x200,numberAttr=0x300,
        funcAttr=0x400,statusAttr=0x500,titleAttr=0x600,coordAttr=0x700,
        stringAttr=0x800,localAttr=0x900,selectAttr=0xA00,COLOR_MASK = 0xF};
   
  static void drawstr(string,int,int);
   
 public:
  XView(TextModel& t,Configuration& c);
  ~XView();

  void reconfig(Configuration &c);
  static set<string> keywords;
  static set<string> localKeywords;
  static bool initKeywords(const char* file);
  static bool initLocalKeywords(const char* file);
  static Display *viewDisplay(){return display;}
  static Window viewWindow(){return window;}
  void clear();
  void clearAll();
  void drawBorders();
  void resize();
  int scrollbarX();
  void drawTitle();
  string prompt(string p){return prompt(p,"");}
  string prompt(string,string);
  void convertClipboard(XEvent&e);
  void exportClipboard(string clip);
  string importClipboard();
  void redraw();
  void oldredraw();
  void newredraw();
  void resetDrawing();
  void drawChar(int c);
  void drawCursor(int c,int l);
  static void drawRectangle(int,int,int,int,int=1);
  static void drawString(string,int,int,int=-1);
  void setCursor(int c,int l);
  void update();
  void update(int,int,int,int);
  void updateTitle();
  void status(string);
  bool toggleShowLine(){showNewLine = !showNewLine;return showNewLine;}
  bool toggleSyntax(){syntaxMark = !syntaxMark;return syntaxMark;}
     
  void doScroll(int y);
  void doSelect(int x,int y);
};
  
#endif

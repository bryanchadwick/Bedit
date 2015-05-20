// CursesView.h

#ifndef _CURSESVIEW_H
#define _CURSESVIEW_H

#include<string>
#include<set> 

#include<curses.h>

#ifdef scroll
#undef scroll
#endif

#include"Bedit.h"

using namespace BEditor;
using std::string;
using std::set;

class CursesView : public TextView
{
  int currx,curry;
  enum{ NumAttributes=11,
        defaultAttr=0,cursorAttr=0x100,keywordAttr=0x200,numberAttr=0x300,
        funcAttr=0x400,statusAttr=0x500,titleAttr=0x600,coordAttr=0x700,
        stringAttr=0x800,localAttr=0x900,selectAttr=0xA00,COLOR_MASK = 0xF};
         
  static int attrColors[NumAttributes];
   
 public:
   
  bool syntaxMark;
  bool showNewLine;
   
  static bool cursesinit;
  static set<string> keywords;
  static set<string> localKeywords;
 public:
  CursesView(TextModel& t, Configuration& c);
  ~CursesView();

  static bool initKeywords(const char* file);
  static bool initLocalKeywords(const char* file);
  void clear();
  void clearAll();
  void resize();
  void drawTitle();
  void drawWindow(string,int,int,int,int,char);
  string prompt(string,string="");
  void redraw();
  void resetDrawing();
  void drawChar(int c);
  void drawCursor(int c,int l);
  void update();
  void status(string);
  bool toggleShowLine(){showNewLine = !showNewLine;return showNewLine;}
  bool toggleSyntax(){syntaxMark = !syntaxMark;return syntaxMark;}
     
  static void drawString(string,int,int,int=0);
  static void drawRectangle(int,int,int,int,int);
};

#endif

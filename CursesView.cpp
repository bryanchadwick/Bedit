// CursesView.cpp

#ifndef _CURSESVIEW_CPP
#define _CURSESVIEW_CPP
 
#include"CursesView.h"
 
#include<fstream>
 
using std::ifstream;

bool CursesView::cursesinit = false;
set<string> CursesView::keywords;
set<string> CursesView::localKeywords;
int CursesView::attrColors[NumAttributes] = {0,};
 
CursesView::CursesView(TextModel& t, Configuration& c):TextView(t,c),currx(0),curry(0),
                                                       syntaxMark(true),showNewLine(false)
{
  title = " Bedit : " + t.getTitle()+" ";
  if(!cursesinit){
    cursesinit = true;
    initscr();
    raw();
    cbreak();
    noecho();
    keypad(stdscr,TRUE);
    curs_set(0);
    start_color();
    init_pair(0,COLOR_WHITE,COLOR_BLACK);
    init_pair(1,COLOR_YELLOW,COLOR_WHITE);
    init_pair(2,COLOR_RED,COLOR_WHITE);
    init_pair(3,COLOR_WHITE,COLOR_BLUE);
    init_pair(4,COLOR_MAGENTA,COLOR_WHITE);
    init_pair(5,COLOR_GREEN,COLOR_WHITE);
    init_pair(6,COLOR_BLUE,COLOR_WHITE);
    init_pair(7,COLOR_BLACK,COLOR_WHITE);

    attrColors[defaultAttr >>COLOR_SHIFT] = COLOR_PAIR(7);
    attrColors[keywordAttr >>COLOR_SHIFT] = COLOR_PAIR(6);
    attrColors[localAttr   >>COLOR_SHIFT] = COLOR_PAIR(6);
    attrColors[statusAttr  >>COLOR_SHIFT] = COLOR_PAIR(2);
    attrColors[cursorAttr  >>COLOR_SHIFT] = COLOR_PAIR(3);
    attrColors[titleAttr   >>COLOR_SHIFT] = COLOR_PAIR(4);
    attrColors[stringAttr  >>COLOR_SHIFT] = COLOR_PAIR(4);
    attrColors[coordAttr   >>COLOR_SHIFT] = COLOR_PAIR(5);
    attrColors[numberAttr  >>COLOR_SHIFT] = COLOR_PAIR(5);
    attrColors[funcAttr    >>COLOR_SHIFT] = COLOR_PAIR(1);
    attrColors[selectAttr  >>COLOR_SHIFT] = COLOR_PAIR(0);
  }
  getmaxyx(stdscr,height,width);
  height -= 2;
  visible = height;
  clearAll();
}
  
CursesView::~CursesView()
{
  endwin(); // Check for RefCount on Curses
}
 
void CursesView::resize()
{
  getmaxyx(stdscr,height,width);
  height -= 2;
  clearAll();
  redraw();
}
  
bool CursesView::initKeywords(const char* file)
{
  ifstream in(file);
  if(!in)return true;
  string w;

  while(in >> w)
    keywords.insert(w);
  in.close();
  return false;
} 

bool CursesView::initLocalKeywords(const char* file)
{
  ifstream in(file);
  if(!in)return true;
  string w;

  while(in >> w)
    localKeywords.insert(w);
  in.close();
  return false;
}

void CursesView::clear()
{
  int temp = ' '| attrColors[defaultAttr >>COLOR_SHIFT];
  // Clear from Top to Status(Coord) Line
  for(int i = 0; i < height+1;i++)
    for(int j = 0; j < width;j++)
      mvaddch(i,j,temp);
  drawTitle();
}
  
void CursesView::clearAll()
{
  // Clear All lines
  for(int i = 0; i < height+2;i++)
    for(int j = 0; j < width;j++)
      mvaddch(i,j,' '| attrColors[defaultAttr >>COLOR_SHIFT]);
}
  
void CursesView::drawTitle()
{
  int l = title.length(),
    t = (width - l)/2;
    
  int ch = model.isDirty()?'*':'-';
  ch |= attrColors[titleAttr >>COLOR_SHIFT];
  l += t;
    
  for(int i = 0; i < width; i++){
    if(i >= t && i < l)
      mvaddch(0,i,title[i-t] | attrColors[titleAttr >>COLOR_SHIFT]);
    else
      mvaddch(0,i,ch);
  }
}
 
void CursesView::resetDrawing()
{ 
  currx = 0;curry = 1;
  clear();
  updateBounds();
}
 
void CursesView::drawChar(int c)
{
  int x = currx, y = curry, attr;
   
  if((char)c == '\n'){
    c = showNewLine?'^':' ';
    curry++;
    currx = 0;
  }else
    currx++;
   
  attr = c >> COLOR_SHIFT;
  c = (char)c;
  if(attr & INVERT_FLAG)
    c |= attrColors[selectAttr >>COLOR_SHIFT];
  else
    c |= attrColors[attr];
     
  mvaddch(y,x,c);
}
  
void CursesView::drawCursor(int c,int l)
{
  int ch = *model.current();
  if(ch == '\n')ch = ' ';
  mvaddch(l+1,c,ch | attrColors[cursorAttr >>COLOR_SHIFT]);
  l += startline;
    
  int t = width-12;
  int def = attrColors[defaultAttr >>COLOR_SHIFT],
    cord = attrColors[coordAttr >>COLOR_SHIFT];
    
  mvaddch((height+1),t+1,'['|def);
  addch((c/100%10+'0')| cord);
  addch((c/10%10+'0')| cord);
  addch((c%10+'0')| cord);
  addch(':'| def);
  addch((l/100%10+'0')| cord);
  addch((l/10%10+'0')| cord);
  addch((l%10+'0')| cord);
  addch(']'| def);
}
   
void CursesView::update()
{ 
  refresh(); // I don't think this works :(
}
 
void CursesView::redraw()
{
  if(!syntaxMark){
    TextView::redraw();
    return;
  }
    
  resetDrawing();
  updateBounds();
    
  int current,last = 0;
  TextModelIterator i = start,
    n = start;
  ++n;
  while(i != end){
    current = *i;
    if(*i == '/' && *n == '/'){
      while(i != end && *i != '\n'){
        current = *i | statusAttr;
        drawChar(current);
        last = (char)current;
        ++i;++n;
      }
    }else{ // Strings
      if((current == '\'' || current == '\"') && last != '\\'){ // '
        current |= stringAttr;
        drawChar(current);
        ++i;++n;
        while(i != end){
          last = (char)current;
          current = *i | stringAttr;
          drawChar(current);
          ++i;++n;
          if(((char)current == '\'' || (char)current == '\"') &&  last != '\\') // '
            break;
        }
      }else{ // Numbers
        if(isdigit(*i)){
          while(i != end && isxdigit(*i) || *i == 'x'){
            last = (char)current;
            current = numberAttr | *i;
            drawChar(current);
            ++i;++n;
          }
        }else{
          if(isalnum(*i) || *i == '_'){ // Keywords/Others
            string s;
            unsigned int attr = defaultAttr;
            while(i != end && (isalnum(*i) || *i == '_')){
              s += *i;
              ++i;++n;
            }
            if(keywords.find(s) != keywords.end())
              attr = keywordAttr;
            else if(localKeywords.find(s) != localKeywords.end())
              attr = localAttr;
            else if(*i == '(')attr = funcAttr;

            for(unsigned int i = 0; i < s.length(); i++)
              drawChar(s[i] | attr);
          }else{
            drawChar(*i | defaultAttr);
            ++i;++n;
          }
        }
      }
    }
    last =(char)current;
    // Skip White Space
    while(i != end && isspace(*i)){
      last = *i;
      drawChar(*i | defaultAttr);
      ++i;++n;
    }
  }
  updateCursor();
  update();
}
 
/*void CursesView::drawWindow(string t,int x,int y,int w,int h, char top)
  {
  int ch = '=',
  e = t.length(),
  s = (w-e)/2+x;
  e += s;
   
  for(int i = x; i < x+w;i++){
  if(i >= s && i < e)ch = t[i-s];
  else ch = top;    
  mvaddch(y,i,ch | attrColors[defaultAttr >> COLOR_SHIFT]);
  mvaddch(y+h,i,'-'| attrColors[defaultAttr >> COLOR_SHIFT]);
  }
  for(int i = x; i < x+w;i++)
  for(int j = y+1; j < y+h;j++){
  if(i != x && i != x+w-1)ch = ' ';
  else ch = '|';      
  mvaddch(j,i,ch | attrColors[defaultAttr >> COLOR_SHIFT]);
  }
  update();
  }*/
 
string CursesView::prompt(string t, string p)
{
  char buff[64] = {0};
  status(t+" ");
  move(height+1,t.length()+2);
  echo();
  curs_set(1);
  update();
  scanw("%s",buff);
  curs_set(0);
  noecho();
  return string(buff);
}
 
void CursesView::status(string s)
{
  int n = width-12;
  move(height+1,0);
  attron(attrColors[defaultAttr >> COLOR_SHIFT]);
  while(n--)
    addch('-');
  attroff(attrColors[defaultAttr >> COLOR_SHIFT]);
  move(height+1,2);
  attron(attrColors[statusAttr >> COLOR_SHIFT]);
  addstr((char*)s.c_str());
  addch(' ');
  attroff(attrColors[statusAttr >> COLOR_SHIFT]);
  update();
}
  
void CursesView::drawString(string s,int xx,int yy,int l)
{
  char t[32];
  for(int i = 0; i < s.length();i++)
    t[i] = s[i];
  t[s.length()] = 0;
   
  if(l && s.length()+1 > l ){
    t[l-2] = '>';
    t[l-3] = '>';
    t[l-1] = 0;
  }
    
  move(yy,xx);
  attron(attrColors[titleAttr >> COLOR_SHIFT]);
  addstr((char*)s.c_str());
  attroff(attrColors[titleAttr >> COLOR_SHIFT]);
}
  
void CursesView::drawRectangle(int xx,int yy,int ww,int hh, int cc)
{
  move(yy,xx);
  attron(attrColors[titleAttr >> COLOR_SHIFT]);
  if(cc){
    addch('['); 
  }else{ 
    addch(' ');
  }
  for(int i = 1; i < ww;i++){
    addch(' ');
  }
  if(cc){ 
    addch(']'); 
  }else{ 
    addch(' '); 
  }
  attroff(attrColors[titleAttr >> COLOR_SHIFT]);
}
  
#endif

// XView.cpp

#ifndef _XVIEW_CPP
#define _XVIEW_CPP
 
#include"XView.h"
#include<X11/keysym.h>
#include<X11/Xatom.h> 
#include<string.h> 
#include<fstream>
#include<iostream>
 
using std::cerr;
using std::endl;
using std::flush;
using std::ifstream; 

XColor XView::attrColors[NumAttributes] = {{0,0,0,0,0,0}, // P,R,G,B,f,p
                                           {0,0x0000,0x0000,0x0000,0,0},{0,0x2222,0x2222,0xEEEE,0,0},{0,0x1111,0xBBBB,0x1111,0,0},
                                           {0,0xFFFF,0x5555,0x0000,0,0},{0,0xDDDD,0x0000,0x0000,0,0},{0,0x0000,0x0000,0x0000,0,0},
                                           {0,0x0000,0xDDDD,0x0000,0,0},{0,0x9999,0x1111,0x9999,0,0},{0,0xFFFF,0xBBBB,0x0000,0,0},
                                           {0,0xBBBB,0xBBBB,0xBBBB,0,0}};
    
//        defaultAttr, cursorAttr, keywordAttr, numberAttr, 
//                     funcAttr,   statusAttr,  titleAttr,
//                     coordAttr,  stringAttr,  localAttr,
//                     selectAttr (background)
 
bool XView::view_init = false;  
set<string> XView::keywords;
set<string> XView::localKeywords;
string XView::exportClip;
Atom clipSelection;
Display *XView::display;
Visual *XView::visual;
Window XView::window;
Colormap XView::colormap;
XGCValues XView::gr_values;
XSetWindowAttributes XView::attributes;
GC XView::gc;
Pixmap XView::pixmap;
int XView::charWidth;
int XView::charHeight;
int XView::refCount = 0;
    
const int left = 15, top = 50,right = 25,bottom = 25;
    
XView::XView(TextModel& t, Configuration& c):
  TextView(t,c),currx(0),curry(0),syntaxMark(c.syntaxMark()),showNewLine(false)
{
  refCount++;
  updateTitle();
  if(!view_init){
    charWidth = c.fontWidth();
    charHeight = c.fontHeight();
    display = XOpenDisplay(NULL);
    visual = DefaultVisual(display, 0);
    attributes.background_pixel = XWhitePixel(display, 0);
    window = XCreateWindow(display, XRootWindow(display, 0),
                           50, 50, c.width(),c.height(), 
                           2, DefaultDepth(display, 0),
                           InputOutput, visual, CWBackPixel,&attributes);
    XSelectInput(display, window, ExposureMask | KeyPressMask | ButtonReleaseMask |
                 ButtonPressMask| Button1MotionMask|StructureNotifyMask);
      
    colormap = DefaultColormap(display, 0);
    gr_values.font = XLoadFont(display,c.fontString().c_str());
    gr_values.foreground = XBlackPixel(display, 0);

    XMapWindow(display, window);
    for(int i = 0; i < NumAttributes; i++)
      XAllocColor(display, colormap,&attrColors[i]);
    gr_values.background = attrColors[selectAttr>>COLOR_SHIFT].pixel;
    gc = XCreateGC(display, window, GCFont+GCForeground+GCBackground, &gr_values);
    
    clipSelection = XInternAtom(display,"CLIPBOARD",true);
    if(clipSelection == None){
      cerr << "Changing to Primary Selection, Not Clipboard!\n";
      clipSelection = XA_PRIMARY;
    }
  }
  resize();
  view_init = true;
}
 
void XView::reconfig(Configuration &c)
{
  charWidth = c.fontWidth();
  charHeight = c.fontHeight();
  gr_values.font = XLoadFont(display,c.fontString().c_str());
  gc = XCreateGC(display, window, GCFont+GCForeground+GCBackground, &gr_values);
  XResizeWindow(display,window,c.width(),c.height());
  resize();  
}
  
XView::~XView()
{
  refCount--;
  if(!refCount){
    XUnmapWindow(display, window);
    XUnloadFont(display, gr_values.font);
    XFreePixmap(display,pixmap);
    XFreeGC(display, gc);
    XCloseDisplay(display);
  }
}
 
void XView::resize()
{
  XWindowAttributes winattr;
  XGetWindowAttributes(display,window,&winattr);
  if(width < winattr.width ||
     height < winattr.height){
    if(view_init)
      XFreePixmap(display,pixmap);
    pixmap = XCreatePixmap(display,window,winattr.width,winattr.height,DefaultDepth(display, 0));
  }
  width = winattr.width;
  height = winattr.height;
  visible = (height-top-bottom) / charHeight;
}
  
bool XView::initKeywords(const char* file)
{
  ifstream in(file);
  if(!in)return true;
  keywords.clear();
  string w;
  while(in >> w)
    keywords.insert(w);
  in.close();
  return false;
} 

bool XView::initLocalKeywords(const char* file)
{
  ifstream in(file);
  if(!in)return true;
  localKeywords.clear();
  string w;
  while(in >> w)
    localKeywords.insert(w);
  in.close();
  return false;
}

void XView::clear()
{
  XSetForeground(display,gc,XWhitePixel(display,0));
  XFillRectangle(display,pixmap,gc,0,top+1,width,height);
  drawBorders();
}

void XView::drawBorders()
{
  XSetForeground(display,gc,XBlackPixel(display,0));
  XDrawRectangle(display,pixmap,gc,5,top,width-right-5,height-top-bottom);
  XDrawRectangle(display,pixmap,gc,5,height-bottom+2,width-right-5,bottom-7);
  XDrawRectangle(display,pixmap,gc,width-right+2,top,right-7,height-top-5);
               
  //*** BEGIN scrollBar
  scrollpix = (height-top-10);
  if(model.totalLines() > visible)
    scrollpix /= model.totalLines();

  scrolly = (int)(startline*scrollpix+top+3),
    scrollh = (int)(visible*scrollpix)-4;
  if(visible > model.totalLines())
    scrollh = (int)scrollpix-2;
   
  XSetForeground(display,gc,attrColors[selectAttr>>COLOR_SHIFT].pixel);  
  XFillRectangle(display,pixmap,gc,width-right+6,scrolly,right-12,scrollh);
  // END scrollBar
   
  XSetForeground(display,gc,attrColors[statusAttr>>COLOR_SHIFT].pixel);
  drawstr(statusString,left,height-bottom+charHeight+2);
   
  XDrawRectangle(display,pixmap,gc,width-right+5,scrolly-1,right-13,scrollh-1);
  int t = model.columnOf(model.current())+1;
  string s;
  s += (char)('0'+(t/1000)%10);
  s += (char)('0'+(t/100)%10);
  s += (char)('0'+(t/10)%10);
  s += (char)('0'+(t)%10);
  s += ":";
  t = model.lineOf(model.current())+1;
  s += (char)('0'+(t/1000)%10);
  s += (char)('0'+(t/100)%10);
  s += (char)('0'+(t/10)%10);
  s += (char)('0'+(t)%10);
  XSetForeground(display,gc,attrColors[coordAttr>>COLOR_SHIFT].pixel);
  drawstr(s,width-right-charWidth*11,height-bottom+charHeight+2);
}
   
void XView::clearAll()
{
  XSetForeground(display,gc,XWhitePixel(display,0));
  XFillRectangle(display,pixmap,gc,0,0,width,height);
}
 
int XView::scrollbarX(){return width-right+2;}
  
void XView::drawTitle()
{
  XTextProperty p;
  char str[64],*s = str;
  strcpy(s,title.c_str());
  XStringListToTextProperty(&s,1,&p);
  XSetWMName(display,window,&p);
  XFree(p.value);
}
 
void XView::resetDrawing()
{ 
  currx = left;
  curry = charHeight+top;
  clear();
}
 
void XView::drawChar(int c)
{
  int x = currx, y = curry, attr;
  char t = (char)c;

  if((char)c == '\n'){
    c = showNewLine?'^':' ';
    curry += charHeight;
    currx = left;
  }else{
    currx += charWidth;
    attr = c >> COLOR_SHIFT;
   
    XSetForeground(display,gc,attrColors[attr&COLOR_MASK].pixel);
    if(x < width-charWidth-right)
      if(attr&INVERT_FLAG)
        XDrawImageString(display,pixmap,gc,x,y,&t,1);
      else
        XDrawString(display,pixmap,gc,x,y,&t,1);
  }
}
  
void XView::drawCursor(int c,int l)
{
  int x = c*charWidth-charWidth/2+left,
    y = (l+1)*charHeight+top;
  static char ch[] = "|";
  XSetForeground(display,gc,attrColors[cursorAttr>>COLOR_SHIFT].pixel);
  if(top < y && y < height-bottom)
    XDrawString(display,pixmap,gc,x,y,ch,1);
}
   
void XView::setCursor(int x,int y)
{
  x = (x-left)/charWidth;
  y = (y-top+(charHeight-1))/charHeight;
  TextView::setCursor(x,y);
} 
   
inline void XView::update()
{ 
  update(0,0,width,height);   
}

void XView::update(int x,int y,int w,int h)
{
  XCopyArea(display,pixmap,window,gc,x,y,w,h,x,y);
}
 
void XView::updateTitle()
{ 
  title = " XBedit: " + model.getTitle()+' ';
}

void XView::redraw()
{
  if(!syntaxMark){
    TextView::redraw();
    return;
  }
     
  if(model.validSelection()){
    TextView::redraw();
    newredraw();
  }else
    oldredraw();
} 
   
void XView::newredraw()
{
  int current,last = 0;
  currx = left;
  curry = charHeight+top;
    
  TextModelIterator i = start,
    n = start;
  ++n;
  while(i != end){
    current = *i;
      
    if(current == '/' && *n == '/'){
      while(i != end && *i != '\n'){
        current = *i | statusAttr;
        drawChar(current);
        last = (char)current;
        ++i;++n;
      }
    }else{
      if(current == '/' && *n == '*'){
        do{
          last = (char)current;
          current = *i | statusAttr;
          drawChar(current);
          ++i;++n;
        }while(i != end && (last != '*' || (char)current != '/'));
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
              if(keywords.find(s) != keywords.end())attr = keywordAttr;
              else if(localKeywords.find(s) != localKeywords.end())attr = localAttr;
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
    }
    last = (char)current;

    // Skip White Space
    while(i != end && isspace(*i)){
      last = *i;
      drawChar(*i | defaultAttr);
      ++i;++n;
    }
  }
  updateCursor();
  if(instantUpdate)
    update();
}
 
void XView::oldredraw()
{
  resetDrawing();
  updateBounds();
  newredraw();
}

void XView::drawRectangle(int x, int y, int w, int h, int c)
{
  if(c)
    XSetForeground(display,gc,XBlackPixel(display,0));
  else
    XSetForeground(display,gc,XWhitePixel(display,0));
  XDrawRectangle(display, pixmap, gc, x,y,w,h);
}
 
void XView::drawString(string s, int x, int y, int l)
{
  y += charHeight;
  unsigned int t = l/charWidth;
  XSetForeground(display,gc,XBlackPixel(display,0));
  if(l < 0 || t >= s.length())
    drawstr(s,x,y);
  else{
    drawstr(s.substr(0,t-2),x,y);
    t = ((l/charWidth)-2)*charWidth;
    XDrawString(display,pixmap,gc,x+t,y,".",1);
    XDrawString(display,pixmap,gc,x+t+3,y,".",1);
    XDrawString(display,pixmap,gc,x+t+6,y,".",1);
  }
}
 
void XView::drawstr(string s, int x, int y)
{
  unsigned int i = 0;
   
  while(i < s.length()){
    XDrawString(display,pixmap,gc,x,y,&s[i++],1);
    x+= charWidth;
  }
}
  
string XView::prompt(string p, string sug)
{
  int x = left,
    y = height-bottom+charHeight+2,
    current = sug.length(), len = current, comm = 0;
  const char* cursor = "_";

  XSetForeground(display,gc,XWhitePixel(display,0));
  drawstr(statusString,x,y);
  XSetForeground(display,gc,attrColors[statusAttr>>COLOR_SHIFT].pixel);
  drawstr(p,x,y);
  x += charWidth*p.length();
    
  XEvent event;
  char buff[64]; // *FIX* Small Magic Number
  strcpy(buff,sug.c_str());
   
  XSetForeground(display,gc,attrColors[0].pixel);
  drawstr(sug,x,y);   
  XDrawString(display,pixmap,gc,x+current*charWidth,y,cursor,1);
   
  update();
  while(1){
    XNextEvent(display, &event);
    switch(event.type) {
    case KeyPress: comm = XKeycodeToKeysym(display,event.xkey.keycode,event.xkey.state&1);
      switch(comm){
      case XK_Escape:return "";
      case XK_Return: return  string(buff);
             
      case XK_Shift_L: case XK_Shift_R:
      case XK_Up: case XK_Down:break;
             
      case XK_BackSpace: case XK_Delete: 
      case XK_Left: case XK_Right: default:
        XSetForeground(display,gc,XWhitePixel(display,0));
        drawstr(cursor,x+current*charWidth,y);
        drawstr(buff,x,y);break;
      }
            
      switch(comm){
      case XK_Shift_L: case XK_Shift_R:
      case XK_Up: case XK_Down:break;
      case XK_BackSpace: if(!current)break; current--;
      case XK_Delete:    if(current == len)break;
        for(int i = current; i < len; i++)
          buff[i] = buff[i+1];
        buff[--len] = 0;
        break; 
      case XK_Left: if(current)current--;break;
      case XK_Right: if(current<len)current++;break;
      default: for(int i = len; i > current; i--)
          buff[i] = buff[i-1];
        buff[current++] = (char)comm;
        buff[++len]=0;
      }
           
      switch(comm){
      case XK_Shift_L: case XK_Shift_R:
      case XK_Up: case XK_Down:break;
      case XK_BackSpace: case XK_Delete:
      case XK_Left: case XK_Right: default:
        XSetForeground(display,gc,attrColors[0].pixel);
        drawstr(buff,x,y);
        drawstr(cursor,x+current*charWidth,y);break;
      }
      update();
    }
  }
  return "";
}
 
void XView::status(string s)
{
  statusString = s;
}
 
void XView::exportClipboard(string s)
{
  exportClip = s;
  cerr << "Export : " << 
    XSetSelectionOwner(display,clipSelection,window,CurrentTime) << '\n';
}
  
void XView::convertClipboard(XEvent &e)
{
    
  cerr << " type: " << XGetAtomName(display,e.xselection.target)
       << " where:" << XGetAtomName(display,e.xselection.property)<< '\n';
  XChangeProperty(display,e.xselection.requestor,
                  XInternAtom(display,"CLIPBOARD",true),e.xselection.property,
                  8,PropModeReplace,
                  (unsigned char*)exportClip.c_str(),exportClip.length());
  e.type = SelectionNotify;
  XSendEvent(display,e.xselection.requestor,NoEventMask,0,&e);
    
} 
 
string XView::importClipboard()
{
  cerr << " * Importing Clip\n";
  const int BUFF_SIZE = 4;
  Atom type;
  int form;
  unsigned long len,left,offset = 0;
  Window owner;
  string ret;
  unsigned char *buff;
    
  owner = XGetSelectionOwner(display,clipSelection);
  if(owner == window)
    return exportClip;
  if(owner == None)
    return "NO_CLIPBOARD";
  XConvertSelection(display,clipSelection,XA_STRING,
                    XA_CUT_BUFFER0,window,CurrentTime);
                      
  XEvent e;
  e.type = 0;
  while(e.type != SelectionNotify)
    XNextEvent(display,&e);
  do{
    XGetWindowProperty(display,window,e.xselection.property,
                       offset,BUFF_SIZE,0,
                       AnyPropertyType,&type,&form,
                       &len,&left,&buff);
    if(type != None){
      len *= form>>3;
      offset += len>>2;
      buff[len] = 0;
      ret += string((const char*)buff);
      XFree(buff);
    }else{
      break;
    }
  }while(left > 0);
  return ret;
}
  
void XView::doScroll(int y)
{
  // Fix to Leave the Tabs at Top
  XEvent e;
  int d,mx,my;
  Window dw;
  e.type = 0;
  if(scrolly < y && y < scrolly+scrollh)
    while(e.type != ButtonRelease){
      XNextEvent(display,&e);
      if(e.type == MotionNotify){
        XQueryPointer(display,window,&dw,&dw,&d,&d,&mx,&my,(unsigned int*)&d);
        d = my - y;
        if(d > scrollpix || d < -scrollpix){
          while(e.type != ButtonRelease && XQLength(display) > 1)
            XNextEvent(display,&e);
          scroll((int)(d/scrollpix));
          y = my;
          redraw();
          update();
        }
      }
    }
  else{
    if(y < scrolly){
      scroll(-(visible-visible>>2));
      redraw();
      update();
    }else{
      scroll(visible-visible>>2);
      redraw();
      update();
    } 
  }
}
  
void XView::doSelect(int x,int y)
{
  // Same Fix as Above
  XEvent e;
  int d,mx,my;
  Window dw;
  e.type = 0;
  setCursor(x,y);
  model.setSelection();
  while(e.type != ButtonRelease){
    XNextEvent(display,&e);
    if(e.type == MotionNotify){
      XQueryPointer(display,window,&dw,&dw,&d,&d,&mx,&my,(unsigned int*)&d);
      while(e.type != ButtonRelease && XQLength(display) > 1)
        XNextEvent(display,&e);
      setCursor(mx,my);
      updateScroll();
      redraw();
      update();
    }
  }
  if(model.selection() == model.current())
    model.invalidateSelection();
}
  
#endif

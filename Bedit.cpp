// Bedit.cpp

#ifndef _BEDIT_CPP
#define _BEDIT_CPP

#include"Bedit.h"
#include<fstream>
#include<iostream>
#include<cctype>
#include<unistd.h>
#include<stdlib.h>
#include<sys/fcntl.h>
#include<fcntl.h>

using std::cerr;
using std::ifstream;
using std::ofstream;

#define UndoSize 62

namespace BEditor{
 
  Configuration::Configuration():
    font("*-courier-*"),
    message("NO_CONFIG_FILE"),
    keyword("NO_CONFIG_FILE"),
    localKeyword("NO_CONFIG_FILE"),
    fontH(13),fontW(7),
    windowW(600),windowH(700),
    printS(10),printW(7),printH(11),
    syntax(false)
  {
  }
 
  string Configuration::getConfig(string &set,string& cf)
  {
    int idx = cf.find(set),
      end;
    string val;
       
    if(idx >= 0){
      idx += set.length();
      end = cf.find(";",idx);
      val = cf.substr(idx,end-idx);
    }else
      cerr << "Couldn\'t load Pref: \'" << set << "\'!! \n From \"" << cf << "\"\n";
    return val;
  }
 
  void Configuration::getConfigString(string set, string& val ,string& cf)
  {
    string tmp = getConfig(set,cf);
    if(tmp.length() > 0)
      val = tmp;
  }
  
  void Configuration::getConfigInt(string set, unsigned int& val, string& cf)
  {
    string tmp = getConfig(set,cf);
    if(tmp.length() > 0)
      val = atoi(tmp.c_str());
  }
  
  void Configuration::load(const char* file)
  {
    ifstream in(file);
    string config;
    char buffer[256];
   
    if(in){
      while(in.getline(buffer,128,'\n')){
        if(buffer[0] != '#'){
          config += buffer;
          config += ';';
        }
      }
      
      string tmp;
      getConfigString("font=",font,config);
      getConfigString("message=",message,config);
      getConfigString("keyword=",keyword,config);
      getConfigString("local=",localKeyword,config);
      getConfigString("syntax=",tmp,config);
      syntax = (tmp == "true");
    
      getConfigInt("fwidth=",fontW,config);
      getConfigInt("fheight=",fontH,config);
      getConfigInt("wwidth=",windowW,config);
      getConfigInt("wheight=",windowH,config);

      getConfigInt("psize=",printS,config);
      getConfigInt("pwidth=",printW,config);
      getConfigInt("pheight=",printH,config);    
      in.close();
      return;
    }
    cerr << " *** Bedit : Config Not Found **** \n";  
  }
  
  TextModelIterator::TextModelIterator(TextPosition p):TextPosition(p){}
  
  const char TextModelIterator::operator*()const
  {
    return section->text[position];
  }
  
  TextModelIterator& TextModelIterator::operator++()
  {
    if(!section->next){
      if(position >= section->length-1)
        position = section->length;    // to enable end() iteration
      else
        position++;
      return *this;
    }
   
    if(position < section->length-1)
      position++;
    else{
      section = section->next;
      position = 0;
    }
    return *this;
  }
 
  TextModelIterator& TextModelIterator::operator--()
  {
    if(position > 0)
      position--;
    else 
      if(section->prev){
        section = section->prev;
        position = section->length-1;
      }
    return *this;
  }
 
  bool TextModelIterator::operator==(TextModelIterator i)const
  {
    return (section == i.section && position == i.position);
  }
 
  bool TextModelIterator::operator!=(TextModelIterator i)const
  {
    return (section != i.section || position != i.position);
  } 
  
  TextModel::TextModel():curr_position(0,0),undoStack(UndoSize),dirty(false),
                         valid_select(false),store_undo(true)
  {
    text_data = new TextSection(0,0,0,0);

    last_section = text_data;
    curr_position.section = text_data;
    curr_select = curr_position;
    total_lines = 0;
    total_sections  = 1;
    title = std::string("untitled");
  }
 
  TextModel::TextModel(const char* file):curr_position(0,0),undoStack(UndoSize),
                                         valid_select(false),store_undo(true)
  {
    text_data = new TextSection(0,0,0,0);

    last_section = text_data;
    curr_position.section = text_data;
    curr_select = curr_position;
    total_lines = 0;
    total_sections  = 1;
    load(file);
    dirty = false;
  }

  TextModel::~TextModel()
  {
    TextSection *i = text_data;
    while(i != 0){
      i = text_data;
      if(i)text_data = i->next;
      delete i;
    }
  }
 
  void TextModel::clear()
  {
    // No UNDOS !!!!
    TextSection *i = text_data->next,*p;
    text_data->next = 0;
    last_section = text_data;
    text_data->length = 0;
    text_data->lines = 0;
    total_lines = 0;
    curr_position.section = text_data;
    curr_position.position = 0;
    curr_select = curr_position;
    valid_select = false;
    
    while(i){
      p = i;
      i = i->next;
      delete p;
    }
  }  
  
  void TextModel::pipe(int p)
  {
    // No Updates for the View
    char buffer[65];
    int len = 0;
    while((len = read(p,buffer,64))){
      buffer[len] = 0;
      insert(buffer);
    }
  }
  
  void TextModel::insert(const char* s, TextPosition& p, bool sundo)
  {
    dirty = true;
    if(sundo)
      undoStack.push(new UndoItem(Insert,charNumberOf(p),string(s)));
    while(*s)
      insert(*s++,p,false);
  }
  
  void TextModel::insert(char c, TextPosition& p, bool sundo)
  {
    dirty = true;
    if(sundo){
      char str[2] = {c,0};
      undoStack.push(new UndoItem(Insert,charNumberOf(p),str));
    }
   
    int i = p.section->length;
    if(i == TEXT_BLOCK_SIZE){
      // New block
      // for now, move the last 1/4 of the block and update the position
      total_sections++; 
      i = (TEXT_BLOCK_SIZE)/4;
      TextSection *sect = new TextSection(i,0,p.section,p.section->next),
        *psect = p.section;

      psect->next = sect;
      if(sect->next)
        sect->next->prev = sect;
      if(last_section == psect)
        last_section = sect;

      psect->length -= i;   
      if(p.position >= (3*TEXT_BLOCK_SIZE)/4){
        p.position -= (3*TEXT_BLOCK_SIZE)/4;
        p.section = sect;
      }

      char* s = &psect->text[TEXT_BLOCK_SIZE-1];
      while(i > 0){
        if(*s == '\n'){
          psect->lines--;
          sect->lines++;
        }
        sect->text[--i] = *s--;
      }
    }
    
    i = p.section->length;
    p.section->length++;

    while(i && i > p.position){
      p.section->text[i] = p.section->text[i-1];
      i--;
    }
    p.section->text[i] = c;
    if(c == '\n'){
      p.section->lines++;
      total_lines++;
    }
    p.position++;
  }

  int TextModel::removeNext(TextPosition &p, bool sundo)
  {
    dirty = true;
   
    if(sundo){
      TextModelIterator i = p;
      char str[2] = {*i,0};
      undoStack.push(new UndoItem(Remove,charNumberOf(i),str));
    }
    
    int i = p.position;
    TextSection *sect = p.section;

    if(i == sect->length-1){
      if(sect == last_section)
        return 0;
      else{
        p.position = 0;
        p.section = sect->next;
      }
    }

    sect->length--;
    if(sect->text[i] == '\n'){
      sect->lines--;
      total_lines--;
    }
    while(i < sect->length){
      sect->text[i] = sect->text[i+1];
      i++;
    }
      
    if(!sect->length){
      if(sect == text_data){
        if(total_sections > 1){
          text_data = text_data->next;
          text_data->prev = 0;
        }else
          return 1;
      }else{
        sect->prev->next = sect->next;
        if(sect->next)
          sect->next->prev = sect->prev;
        else
          last_section = sect->prev;
      }
      total_sections--;
      sect->next = 0;
      sect->prev = 0;
     
      delete sect;
    }
    return 1;
  }
 
  int TextModel::reorderIterators(TextModelIterator& a, TextModelIterator& b)
  {
    int na = charNumberOf(a), nb = charNumberOf(b),
      len = nb-na;
   
    if(len < 0){
      TextModelIterator t = a;
      a = b;
      b = t;
      len = -len;
    }
    return len;
  }
  
  int TextModel::removePrev(TextPosition &p, bool sundo)
  {
    if(curr_position.section == text_data &&
       curr_position.position == 0)return 0;
   
    setPosition(--current());
    return removeNext(p, sundo);
  }
  
  int TextModel::remove(bool sundo)
  {
    TextModelIterator s = curr_select, e = curr_position;
    int len = reorderIterators(s,e);
   
    if(sundo)
      undoStack.push(new UndoItem(Remove,charNumberOf(s),selectionString()));
     
    setPosition(s);
    while(len--){
      removeNext(curr_position,false);
    }
    valid_select = false; 
    return 0;
  }
  
  void TextModel::recompile()
  {
    // Long Complicated function ...
  }
  
  TextModelIterator TextModel::begin()
  {
    return TextModelIterator(TextPosition(text_data,0));
  }
  
  TextModelIterator TextModel::end()
  {
    return TextModelIterator(TextPosition(last_section,last_section->length));
  }
  
  TextModelIterator TextModel::current()
  {
    return TextModelIterator(curr_position);
  }
  
  TextModelIterator TextModel::line(int l)
  {
    // Could be done faster
    int tl = 0;
    TextSection *sect = text_data;
    while(sect && (tl+sect->lines) < l){
      tl += sect->lines;
      sect = sect->next;
    }

    if(!sect)
      return end();

    TextModelIterator i(TextPosition(sect,0));
    while(tl < l){
      if(*i == '\n')tl++;
      ++i;
    }
    return i;
  }

  TextModelIterator TextModel::charNumber(long long c)
  {
    TextSection *sect = text_data;
    while(sect && (c - sect->length) >= 0){
      c -= sect->length;
      sect = sect->next;
    }

    if(!sect || c >= sect->length)
      return end();

    return TextPosition(sect,c);
  }

  TextModelIterator TextModel::position(int l, int col)
  {
    TextModelIterator i = line(l);
    while(col-- && *i != '\n')
      ++i;
    return i;
  }

  int TextModel::lineOf(TextModelIterator i)
  { 
    TextSection* t = i.section;
    int l = 0;
    char* c = &i.section->text[i.position-1]; 

    while(c >= i.section->text)
      if(*c-- == '\n')l++;
    t = t->prev;
    while(t){
      l += t->lines;
      t = t->prev;
    }
    return l;
  }
 
  long long TextModel::charNumberOf(TextModelIterator i)
  {
    TextSection *t = text_data;
    long long n = i.position;
    while(t && t != i.section){
      n += t->length;
      t = t->next;
    }
    return n;
  }

  int TextModel::columnOf(TextModelIterator i)
  {
    // Assumes no line wrapping
    // Could be done faster
    TextModelIterator t = line(lineOf(i));
    int col = 0;
    while(t != i)
      ++t,col++;
    return col;
  }

  string TextModel::lineString(int l)
  {
    // Could (possibly) be done faster
    TextModelIterator i = line(l);
    string s = "";

    while(i != end() && *i != '\n'){
      s += *i;
      ++i;
    }
    return s;
  }
  string TextModel::selectionString()
  {
    TextModelIterator s = curr_select, e = curr_position;
    if(!valid_select)return "";
   
    int len = reorderIterators(s,e);
    string str = "";
    while(len--){
      str += *s;
      ++s;
    }
    return str;
  }
  
  bool TextModel::load(const char* file)
  {
    // *FIX* : magic numbers 64/65
    int f  = open(file,O_RDONLY);
    char temp[65] = {0};

    if(f > 0){
      title = std::string(file);
      valid_title = true;
    }else{
      title = std::string("untitled");
      insert(" ! File Error : No Access to or non existent \"");
      insert(file);
      insert("\"\n");
      valid_title = false;
      return true;
    }
   
    int num;
    char* t;
    while(num = read(f,temp,64)){
      t = temp;
      for(int i = 0; i < num; i++,t++)
        if(!*t)*t='~';
      temp[num] = 0;
      insert(temp,curr_position,false);
    }
    
    close(f);
    setPosition(begin());
    dirty = false;
    return false;
  }
 
  bool TextModel::save(const char* file)
  {
    // *FIX* : magic number 64/65
    ofstream out(file);

    if(!out){
      valid_title = false;
      return true;
    }

    TextSection *t = text_data;
    while(t){   
      out.write(t->text,t->length);
      t = t->next;
    }
    out.close();
    dirty = false;
    title = std::string(file);
    valid_title = true;
    return false;
  }

  bool TextModel::undo()
  {
    if(!undoStack.atStart()){
      UndoItem *t = undoStack.current();
      undoStack.prev();
      if(!t)return false;
     
      TextModelIterator i = charNumber(t->pos);
      int len = t->text.length();
     
      if(t->type == Insert)
        while(len--)
          removeNext(i,false);
      else
        insert(t->text.c_str(),i,false);
      curr_position = charNumber(t->pos);
      return true;
    }
    return false;
  }

  bool TextModel::redo()
  {
    if(!undoStack.atEnd()){
      undoStack.next();
      UndoItem *t = undoStack.current();
      if(!t)return false;

      TextModelIterator i = charNumber(t->pos);
      int len = t->text.length();
         
      if(t->type == Insert)
        insert(t->text.c_str(),i,false);
      else
        while(len--)
          removeNext(i,false);
      curr_position = charNumber(t->pos);
      return true;
    }
    return false;
  }
  
  int TextView::scroll(int L)
  {
    startline += L;
    updateBounds();
    return L;
  }
 
  void TextView::pageUp()
  {
    model.setPosition(model.line(model.lineOf(model.current())-(visible-2)));
    updateScroll();
  }
  
  void TextView::pageDown()
  {
    model.setPosition(model.line(model.lineOf(model.current())+visible-2));
    updateScroll();
  }
 
  void TextView::updateBounds()
  {
    if(startline+visible-1 > model.totalLines())
      startline = model.totalLines()-visible+1;
    start = model.line(startline);
    startline = model.lineOf(start);
    end = model.line(startline+visible);
  } 

  void TextView::redraw()
  {
    resetDrawing();
    updateBounds();
    if(model.validSelection()){
      bool in = false;
      TextModelIterator s = model.selection(), c = model.current();
      int ns = model.charNumberOf(s),nc = model.charNumberOf(c),
        nb = model.charNumberOf(start),ne = model.charNumberOf(end);
          
      if(ns > nc){
        TextModelIterator t = s;
        int nt = ns;
        s = c;ns = nc;
        c = t;nc = nt;
      }
      if(nc > nb && ns < ne)
        if(ns < nb)
          s = start;
      if(nc > ne)
        c = end;
        
      for(TextModelIterator i = start; i != end; ++i){
        if(i == s)in = true;
        if(i == c)in = false;
        if(in)drawChar(*i | (INVERT_FLAG << COLOR_SHIFT));
        else drawChar(*i);
      }
    }
    else
      for(TextModelIterator i = start; i != end; ++i)
        drawChar(*i);  
    updateCursor();
    if(instantUpdate)
      update();
  }
 
  void TextView::updateCursor()
  {
    TextModelIterator c = model.current();
    int line = model.lineOf(c) - model.lineOf(start),
      col = model.columnOf(c);
    drawCursor(col,line);
  }
  
  void TextView::updateScroll()
  {
    int l = model.lineOf(model.current());
    if(l < startline)
      startline = l - 2;
    else if(l >= startline + visible)
      startline = l-visible+2;
    updateBounds();
  }

  void TextView::setCursor(int c, int l)
  {
    model.setPosition(model.position(l-1+startline,c));
    updateBounds();
  }
}
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <iostream>
#include "BufferList.cpp"
#include "Bedit.h"
#include "Print.h"

#define SHIFT_MASK 0x10000
#define CHAR_MASK 0xFFFF

using namespace std;
namespace BEditor{

  template<class VIEW>
  class Control
  {
  public:
    typedef typename BufferList<VIEW>::bufferStruct TBuffer;
     
    int pipeSetup();   
    void doCompile();
    void doCommand(string comm);
    void setupSelection(int s);
    bool quit();
    void load();
    void save(int);
    void print();
    void printHTML();
     
    VIEW* currentView(){return buff.view;}
    TextModel* currentModel(){return buff.model;}
    void touch(){dirty = true;}
    void clean(){dirty = false;}
    bool isDirty(){return dirty;}
    void updateView(){buff.view->update();}
    void drawView(){buff.view->redraw();}
    void update();
    void search();
    void searchReplace();
    int find(string s);
    void configureView();
    void commandKey(int comm);
    void returnKey();
    void backspaceKey();
    void deleteKey();
    void upKey(int key);
    void downKey(int key);
    void leftKey(int key);
    void rightKey(int key);
    void newBuffer();
    void toggleSyntax();
    void prevBuffer();
    void nextBuffer();
    void compileBuffer();
    void execCommand();
    void clearMessages();
    void tabKey();
    void pagedownKey(int key);
    void pageupKey(int key);
    void homeKey(int key);
    void endKey(int key);
    void generalKey(int key);
    void closeBuffer();
    void exportClipboard();
    void importClipboard();
    Control(int argc, char *argv[]);
     
  private:
    string clipboard,lastFind,configpath;
    bool dirty;
    Configuration configure;

  public:
    TBuffer messages,
      buff;
    BufferList<VIEW> buffList;
  };
  
  template<class VIEW> 
  Control<VIEW>::Control(int argc, char *argv[]): dirty(true),
                                                  buffList(configure)
  {
    configpath += getenv("HOME");
    configpath += "/.bedit";
    configure.load(configpath.c_str());
    
    messages = buffList.newBuffer(configure.messageFile().c_str());
    messages.model->setTitle("Messages");
    messages.view->updateTitle();
    
    if(argc >= 2)
      while(argc-- > 1)
        buffList.newBuffer(argv[argc]);
    buff = buffList.current();
   
    buff.view->status(" * Welcome to the BEditor");
    if(VIEW::initKeywords(configure.keywordFile().c_str()))
      messages.model->insert(" * Could not open Keyword File\n");
    if(VIEW::initLocalKeywords(configure.localKeywordFile().c_str()))
      messages.model->insert(" * Could not open Local Keyword File\n");
    buff.view->drawTitle();
    buff.view->redraw();
  }
  
  template<class VIEW>
  void Control<VIEW>::update()
  {
    buff.view->resize();
    buff.view->updateTitle();
    buff.view->drawTitle();
    dirty = true;
  }
  
  template<class VIEW>
  void Control<VIEW>::configureView()
  {
    configure.load(configpath.c_str());
    buff.view->reconfig(configure);
    dirty = true;
  }

  template<class VIEW>
  void Control<VIEW>::exportClipboard()
  {
    buff.view->exportClipboard(clipboard);
  }
  
  template<class VIEW>
  void Control<VIEW>::importClipboard()
  {
    clipboard = buff.view->importClipboard();
  }
       
  template<class VIEW>
  void Control<VIEW>::search()
  {
    string s(""),p;
    if(buff.model->validSelection())
      s = buff.model->selectionString();
    else
      s = lastFind;
    p = buff.view->prompt(" * Find ? ",s);
    buff.model->invalidateSelection();
    if(p.length() == 0)
      buff.view->status(" * find canceled");
    else{
      lastFind = p;
      if(find(p))
        buff.view->status(" * \'"+p+"\' found");
      else 
        buff.view->status(" *! \'"+p+"\' NOT found");
    }
  }
  
  template<class VIEW>
  void Control<VIEW>::searchReplace()
  {
    string s(""),p,f,r;
    bool done = false;
    
    if(buff.model->validSelection())s = buff.model->selectionString();
    else s = lastFind;
    
    p = buff.view->prompt(" * Find&Replace f|r ? ",s);
    buff.model->invalidateSelection();
    
    if(p.length() == 0)
      buff.view->status(" * find/replace cancelled");
    else{
      int i = p.find("|");
      if(i == 0){
        buff.view->status(" *! nothing to find!");
        return;
      }
      
      if(i < 0)
        i = p.length();
          
      f = p.substr(0,i);
      r = p.substr(i+1,p.length()-i-1);
      
      lastFind = f;
      while(!done){
        if(find(f)){
          update();
          buff.view->clearAll();
          drawView();
          updateView();
          s = buff.view->prompt(" * \'"+f+"\' found, replace with \'"+r+"\'? [y/n] ","n");
          if(s.length() == 0){
            buff.view->status(" * find/replace cancelled");
            done = true;
          }
          if(s[0] == 'y'){
            buff.model->remove();
            buff.model->insert(r.c_str());
          }
          buff.model->invalidateSelection();
        }else{ 
          buff.view->status(" *! \'"+f+"\' NOT found");
          done = true;
        }
      }
    }
  }
   
  template<class VIEW>
  int Control<VIEW>::find(string s)
  {
    TextModelIterator t(buff.model->current()),
      p(t);
    const char *cs = s.c_str(),
      *ct = cs;
    int i,found = 0;
    
    while(!found){
      while(t != buff.model->end() && *t != s[0])
        ++t;
      if(t == buff.model->end()){
        found = -1;
        break;
      }
      p = t;
      i = 0;
      
      while(i < s.length() && *p == s[i]){
        ++p;++ct;
        i++;
      }
      if(i == s.length())
        found = 1;
      else 
        ++t;
    }
     
    if(found > 0){
      buff.model->setPosition(t);
      buff.model->setSelection();
      buff.model->setPosition(p);
      buff.view->updateScroll();
      return 1;
    }
    return 0; 
  }
                        
  template<class VIEW>
  void Control<VIEW>::commandKey(int comm)
  {
    switch((char)comm){
    case 'c':if(buff.model->validSelection()){
        clipboard = buff.model->selectionString();
        if(comm & SHIFT_MASK){
          exportClipboard();
          cerr << "EXPORT\n";
        }
      }break;
    case 'r':searchReplace();break;
    case 'z':if(comm & SHIFT_MASK)
        if(!buff.model->redo())buff.view->status(" * Can\'t Redo");
        else buff.view->status(" * Redone");
      else
        if(!buff.model->undo())buff.view->status(" * Can\'t Undo");
        else buff.view->status(" * Undone");break;
    case 'v':if(buff.model->validSelection())buff.model->remove();
      if(comm & SHIFT_MASK)
        importClipboard();
      if(clipboard.length() > 0){
        buff.model->insert(clipboard.c_str());
        buff.view->updateScroll();
      }break;
    case 'w':closeBuffer();break;
    case 'n':newBuffer();break;              
    case 'x':if(buff.model->validSelection()){
        clipboard = buff.model->selectionString();
        buff.model->remove();
      }break;
    case 'f':search();break;
    case 'g':if(find(lastFind)) 
        buff.view->status(" * \'"+lastFind+"\' found");
      else 
        buff.view->status(" *! \'"+lastFind+"\' NOT found");break;
    case 'q':if(quit())exit(0);break;
    case 's':save(comm);break;
    case 'p':print();break;
    case 'h':printHTML();break;            
    case 'o':load();
      buff = buffList.current();
      dirty = true;break;
    }
  }
 
  template<class VIEW>
  void Control<VIEW>::returnKey()
  {
    buff.model->insert('\n');
    buff.view->resize();
    TextModelIterator i = buff.model->line(buff.model->lineOf(buff.model->current())-1);
    int ws = 0;
    while(*i == ' ')
      ++ws,++i;
    if(ws)
      while(ws--)
        buff.model->insert(' ');
    buff.view->updateScroll();
  }
 
  template<class VIEW>
  void Control<VIEW>::backspaceKey()
  {
    if(buff.model->validSelection())
      buff.model->remove();
    else{
      buff.model->removePrev(buff.model->getPosition());
      buff.view->updateScroll();
    }
  }
 
  template<class VIEW>
  void Control<VIEW>::deleteKey()
  { 
    if(buff.model->validSelection())
      buff.model->remove();
    else{
      buff.model->removeNext(buff.model->getPosition());
      buff.view->updateScroll();
    }
  }
  
  template<class VIEW>
  void Control<VIEW>::upKey(int key)
  {
    setupSelection(key);
    buff.model->setPosition(buff.model->position(buff.model->lineOf(buff.model->current())-1,
                                                 buff.model->columnOf(buff.model->current())));
    buff.view->updateScroll();
  }

  template<class VIEW>
  void Control<VIEW>::downKey(int key)
  {
    setupSelection(key);
    buff.model->setPosition(buff.model->position(buff.model->lineOf(buff.model->current())+1,
                                                 buff.model->columnOf(buff.model->current())));
    buff.view->updateScroll();
  }
   
  template<class VIEW>
  void Control<VIEW>::leftKey(int key)
  { 
    setupSelection(key);
    buff.model->setPosition(--buff.model->current());
    buff.view->updateScroll();
  }
  
  template<class VIEW>
  void Control<VIEW>::rightKey(int key)
  {
    setupSelection(key);
    buff.model->setPosition(++buff.model->current());
    buff.view->updateScroll();
  }
  
  template<class VIEW>
  void Control<VIEW>::newBuffer()
  {
    buff = buffList.newBuffer();
    buff.model->setTitle("untitled");
    update();
  }
 
  template<class VIEW>
  void Control<VIEW>::toggleSyntax()
  {
    buff.view->toggleSyntax();
  }
 
  template<class VIEW>
  void Control<VIEW>::prevBuffer()
  {
    buff = buffList.prev();
    update();
  } 
 
  template<class VIEW>
  void Control<VIEW>::nextBuffer()
  {
    buff = buffList.next();
    update();
  }
 
  template<class VIEW>
  void Control<VIEW>::compileBuffer()
  {
    int res = pipeSetup();
    TBuffer t = buff;
    while(t != messages)
      t = buffList.next();
    t.view->redraw();t.view->update();
    if(res)
      messages.model->insert("\n ** Error Attempting to Compile\n");
    else
      doCompile();
  }
 
  template<class VIEW>
  void Control<VIEW>::execCommand()
  {
    doCommand(buff.model->lineString(buff.model->lineOf(buff.model->current())));
  }
 
  template<class VIEW>
  void Control<VIEW>::clearMessages()
  { 
    messages.model->clear();
  }
  
  template<class VIEW>
  void Control<VIEW>::tabKey()
  {
    int t = 4 - (buff.model->columnOf(buff.model->current())%4);
    while(t--)buff.model->insert(' ');
  }
 
  template<class VIEW>
  void Control<VIEW>::pagedownKey(int key)
  {
    setupSelection(key);
    buff.view->pageDown();
  }
 
  template<class VIEW>
  void Control<VIEW>::pageupKey(int key)
  {
    setupSelection(key);
    buff.view->pageUp();
  }
  
  template<class VIEW>
  void Control<VIEW>::homeKey(int key)
  {
    setupSelection(key);
    buff.model->setPosition(buff.model->line(buff.model->lineOf(buff.model->current())));
  }
 
  template<class VIEW>
  void Control<VIEW>::endKey(int key)
  {
    setupSelection(key);
    buff.model->setPosition(--buff.model->line(buff.model->lineOf(buff.model->current())+1));
  }
  
  template<class VIEW>
  void Control<VIEW>::generalKey(int key)
  {
    if(buff.model->validSelection())buff.model->remove();
    buff.model->insert((char)key);
    buff.view->updateScroll();
  }
 
  template<class VIEW>
  void Control<VIEW>::save(int key)
  {
    if(!buff.model->isDirty() && !(key & SHIFT_MASK))return;
    if(buff.model->validTitle() && !(key & SHIFT_MASK)){
      if(!buff.model->save())
        buff.view->status(" * File \""+buff.model->getTitle()+"\" saved");
      else 
        buff.view->status(" *! Save Error");
    }else{
      string s = buff.view->prompt(" SaveAs ? ",buff.model->getTitle());
      if(s.length() == 0)
        buff.view->status(" * SaveAs canceled");
      else
        if(!buff.model->save(s.c_str()))
          buff.view->status(" * File saved as \"" + buff.model->getTitle()+"\"");
        else
          buff.view->status(" *! SaveAs Error");
      buff.view->updateTitle();
    }
  }

  template<class VIEW>
  void Control<VIEW>::print()
  {
    string s = buff.view->prompt(" PrintToFile ? ",buff.model->getTitle()+".ps");
    if(s.length() == 0)
      buff.view->status(" * Print canceled");
    else{
      if(!printModel(*buff.model,s.c_str(),configure))
        buff.view->status(" * File printed to \""+s+"\"");
      else
        buff.view->status(" *! Printing Error (File Problem)");
    }
  }
 
  template<class VIEW>
  void Control<VIEW>::printHTML()
  {
    string s = buff.view->prompt(" HTMLToFile ? ",buff.model->getTitle()+".html");
    if(s.length() == 0)
      buff.view->status(" * HTML canceled");
    else{
      if(!htmlModel(*buff.model,s.c_str(),buff.view->keywords,buff.view->localKeywords))
        buff.view->status(" * File HTMLed to \""+s+"\"");
      else
        buff.view->status(" *! HTML Error (File Problem)");
    }
  }
 
  template<class VIEW>
  void Control<VIEW>::load()
  {
    string s = buff.view->prompt(" LoadFile ? ");
    if(s.length() == 0)
      buff.view->status(" * load canceled");
    else{
      buff = buffList.newBuffer(s.c_str());
      if(buff.model->validTitle())
        buff.view->status(" * Loaded \"" + buff.model->getTitle()+"\"");
      else
        buff.view->status(" *! Load Error");
    }
    buff.view->updateTitle();
    buff.view->drawTitle();
    dirty = true;
  }
 
  template<class VIEW>
  void Control<VIEW>::setupSelection(int key)
  {
    if(key & SHIFT_MASK){
      if(!buff.model->validSelection())
        buff.model->setSelection();
    }else
      buff.model->invalidateSelection();
  }


  template<class VIEW>
  int Control<VIEW>::pipeSetup()
  {
    if(buff.model->isDirty() && buff.model->save()){ // Then We Need to Save
      string s = buff.view->prompt(" SaveAs ? ");
      if(s.length() == 0 || buff.model->save(s.c_str()))
        return 1;
    }
    buff.view->updateTitle();
    return 0;
  }     

  template<class VIEW>
  bool Control<VIEW>::quit()
  {
    // Use a ForEach function to check on each of the 
    //   Buffers, save if needed, but ask first
    //  - somehow skip the messages buffer
    return 1;
  }

  template<class VIEW>
  void Control<VIEW>::doCompile()
  {
    char name[64],nameo[64];
    strcpy(name,buff.model->getTitle().c_str());
    strcpy(nameo,name);
    char *p = nameo;
    while(*p && *p++ != '.'); // Empty on purpose
    *p++ = 'o';
    *p = 0;
    string comm = "g++ -c -o " + string(nameo) + " " + string(name);
    buff = messages;
    doCommand(comm);
  }

  template<class VIEW>
  void Control<VIEW>::closeBuffer()
  {
    if(buff != messages)buff = buffList.deleteCurrent();
    else buff.view->status(" * You Can\'t close the Message Buffer");
    buff.view->resize();
    dirty = true;
  }

  template<class VIEW>
  void Control<VIEW>::doCommand(string comm)
  {
    int p[2],ch,i = 0,stat = 0;
    char buffer[256];
    pipe(p);
   
    buff.view->status(" * running command ...");
    buff.view->redraw();buff.view->update();
   
    if(!(ch = fork())){
      dup2(p[1],1);
      dup2(p[1],2);
      close(p[0]);
      execlp("bash","bash","-c",comm.c_str(),NULL);
      cerr << " ** serious internal error !!!";
      exit(1);
    }
    close(p[1]);
    buff.model->setPosition(buff.model->line(buff.model->lineOf(buff.model->current())+1));
   
    while((i = read(p[0],buffer,254))){
      buffer[i] = 0;
      buff.model->insert(buffer);
      buff.view->updateScroll();
      buff.view->redraw();
      buff.view->update();
    }

    close(p[0]); 
    wait(&stat);
    if(!WEXITSTATUS(stat))buff.view->status(" * successful");
    else buff.view->status(" * unsuccessful");
  }

} // BEditor namespace



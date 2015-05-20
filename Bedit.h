// Bedit.h

#ifndef _BEDIT_H
#define _BEDIT_H

#include<string>
#include"CircularStack.cpp"

namespace BEditor{
 
  using std::string; 
 
  class Configuration{
    string font,
      message,
      keyword,
      localKeyword;
    unsigned int fontH,
      fontW,
      fontS,
      windowH,
      windowW,
      printS,
      printW,
      printH;
    // Margins?
    bool syntax;
    string getConfig(string&,string&);
    void getFontSize();
    void getConfigString(string,string&,string&);
    void getConfigInt(string, unsigned int&, string&);
     
  public:
    Configuration();
    void load(const char* f); 
    string fontString()const{return font;};
    string messageFile()const{return message;};
    string keywordFile()const{return keyword;};
    string localKeywordFile()const{return localKeyword;};
    unsigned int fontSize()const{return fontS;};
    unsigned int fontHeight()const{return fontH;};
    unsigned int fontWidth()const{return fontW;};
    unsigned int width()const{return windowW;};
    unsigned int height()const{return windowH;};
    unsigned int printSize()const{return printS;};
    unsigned int printWidth()const{return printW;};
    unsigned int printHeight()const{return printH;};
    bool syntaxMark()const{return syntax;};
  };
  
  const int TEXT_BLOCK_SIZE = 1024;

  typedef char text_block[TEXT_BLOCK_SIZE];

  struct TextSection{
    text_block text;
   
    int length,
      lines;
    TextSection *prev,*next;

  TextSection(int len,int ln,TextSection*p,TextSection*n)
  :length(len),lines(ln),prev(p),next(n){}
  };

  struct TextPosition{
    TextSection *section;
    int position;  // the char after the position
  TextPosition():section(0),position(0){}
  TextPosition(TextSection* s, int p):section(s),position(p){}
  };
  
  struct TextSelection: public TextPosition{
    int length;
  };
  
  class TextModelIterator: public TextPosition{
  public:
    TextModelIterator(TextPosition p);
    const char operator*()const;
    TextModelIterator& operator++();
    TextModelIterator& operator--();
    bool operator==(TextModelIterator i)const;
    bool operator!=(TextModelIterator i)const;
  };
 
  struct UndoItem{
    unsigned char type;
    long long pos;
    string text;
    
  UndoItem(unsigned char t, int p, string s):
    type(t),pos(p),text(s){}
  };
 
  class TextModel{
  private:
    TextSection *text_data,    // first section
      *last_section;
    TextPosition curr_position;
    TextPosition curr_select; // Start or end of the Selection
    CircularStack<UndoItem> undoStack;
    int total_lines,
      total_sections;
    string title;
    bool dirty,valid_title,valid_select,store_undo;
    int reorderIterators(TextModelIterator &a, TextModelIterator& b);
     
    enum{Insert,Remove};
        
  public:
    TextModel();
    ~TextModel();
    TextModel(const char* file);
    TextPosition& getPosition(){return curr_position;}
    void setPosition(TextPosition p){curr_position = p;}     
    int getLines(){return total_lines;}
    string getTitle(){return title;}
    void setTitle(string s){title = s;}
     
    void setSelection(){curr_select = curr_position;valid_select = true;}
    bool validSelection(){return valid_select;}
    void invalidateSelection(){valid_select = false;}
    TextModelIterator selection(){return curr_select;}
    string selectionString();
    int selectionLength();
     
    void pipe(int p);
    void insert(const char* s, TextPosition& p,bool=true);
    void insert(char c, TextPosition& p,bool=true);
    void insert(char c){insert(c,curr_position);}
    void insert(const char* c){insert(c,curr_position);}
    void clear();
    int removeNext(TextPosition &p,bool=true);
    int removePrev(TextPosition &p,bool=true);
    int remove(bool=true);            // Remove the selection
    void recompile();
    TextModelIterator begin();        // iterator for the beginning
    TextModelIterator end();          //  "" end
    TextModelIterator current();      //  "" current position
    TextModelIterator line(int l);    //  "" start of line
    TextModelIterator charNumber(long long c);
    TextModelIterator position(int line, int col);
    int lineOf(TextModelIterator i);  //  line# of (i)
    string lineString(int l);
    int totalLines(){return total_lines;}
    int columnOf(TextModelIterator i);//  col# of (i)
    long long charNumberOf(TextModelIterator i);
    bool load(const char* file); // Faster to let it save itself
    bool save(const char* file); //  - for plain text only, iter. for others
    bool save(){if(!valid_title)return true;return save(title.c_str());}
    bool isDirty(){return dirty;}
    bool validTitle(){return valid_title;}
    string toString();
     
    void setUndo(bool nundo){store_undo = nundo;}
    bool undo();
    bool redo();
  };

  class TextView{
    // restricted to viewing full lines...
  protected:
    TextModel& model;
    int startline;           // startline will be the foundation!!

    TextModelIterator start; // top left of the view (first line)
    TextModelIterator end;   // Bottom == begin + height
    int width;   // width
    int height;  // height
    int visible; // lines visible (height)
    bool instantUpdate;
    string title;
    enum{COLOR_SHIFT = 8,     // Color/Attr is upper byte
         INVERT_FLAG = 0x10}; // For Selections
  public:
  TextView(TextModel& t, Configuration& c,int w=0,int h=0):model(t),startline(0),
      start(model.begin()),end(model.line(h+1)),width(w),height(h),
      visible(h),instantUpdate(false){};
    virtual ~TextView(){};
    void setTitle(string s){title = s;}
    void updateTitle(){title = model.getTitle();}
    int scroll(int l);
    void pageUp();
    void pageDown();
    void updateCursor();
    void updateBounds();
    void updateScroll();
    void setCursor(int c, int l);

    // ** Implementation Dependent
    virtual void redraw() = 0;
    virtual void resetDrawing() = 0;
    virtual void drawChar(int) = 0;
    virtual void drawCursor(int c,int l) = 0;
    virtual void update() = 0;
  };
}
#endif

// BufferList.h
// Circular list of Buffers (TextModels and Acc. Views)

#ifndef _BUFFERLIST_H
#define _BUFFERLIST_H
 
#include<string>
#include"Bedit.h"
 
namespace BEditor{
   
  using std::string;

  template<class VIEW>   
    class BufferList 
    {
      struct bufferNode{
        TextModel model;
        VIEW view;
        bufferNode *prev,*next;
        
      bufferNode(bufferNode* p, bufferNode *n, Configuration &c):model(),view(model,c),prev(p),next(n){}
      bufferNode(const char* file, bufferNode* p, bufferNode *n, Configuration &c):
        model(file),view(model,c),prev(p),next(n){}
      };

      bufferNode* head;
      bufferNode* curr;
      int buffcount;
      Configuration &configure;
      
    public:
      
      struct bufferStruct{
        TextModel *model;
        VIEW *view;
      bufferStruct():model(0),view(0){};
      bufferStruct(TextModel *m, VIEW *v):model(m),view(v){};
        bufferStruct& operator =(bufferStruct b){ model = b.model;view = b.view;return *this;};
        bool operator ==(bufferStruct& b){ return view == b.view;};
        bool operator !=(bufferStruct& b){ return view != b.view;};
      };    

      typedef void (*Func)(bufferStruct&);

      BufferList(Configuration &c);
      ~BufferList();
      BufferList(string name,Configuration &c); // Create one buffer with title == name
      bufferStruct newBuffer(); // Add a buffer to the list and return pointers
      bufferStruct newBuffer(const char* file);  // Same with FileName
      int size(){return buffcount;}
      void foreach(Func f);
      bufferStruct deleteCurrent();
      bufferStruct current(int);
      bufferStruct next();
      bufferStruct prev();
      bufferStruct current();
    };
}

#endif

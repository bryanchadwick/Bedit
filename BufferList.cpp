// BufferList.cpp
// Circular list of Buffers (TextModels and Acc. Views)

#ifndef _BUFFERLIST_CPP
#define _BUFFERLIST_CPP

#include"BufferList.h"
#include<iostream>

using namespace std;

namespace BEditor{

  template<class VIEW> 
  BufferList<VIEW>::BufferList(Configuration &c):head(0),curr(0),buffcount(0),configure(c){}

  template<class VIEW> 
  BufferList<VIEW>::BufferList(string name, Configuration &c):head(0),curr(0),buffcount(0),configure(c)
  {
    newBuffer();
    curr->model.setTitle(name);
    curr->view.updateTitle();
  }
 
  template<class VIEW> 
  BufferList<VIEW>::~BufferList()
  {
    bufferNode* t = head;
    
    while(head && buffcount){
      t = head;
      head = head->next;
      delete t;
      buffcount--;
    }
  }

  template<class VIEW> 
  typename BEditor::BufferList<VIEW>::bufferStruct BufferList<VIEW>::newBuffer()
  {
    // Makes new Buffer the Current one
    if(!head){
      head = new bufferNode(0,0,configure);
      head->next = head;
      head->prev = head;
      curr = head;
    }else{
      bufferNode *temp = new bufferNode(curr,curr->next,configure);
      curr->next->prev = temp;
      curr->next = temp;
      curr = temp;
    }
    buffcount++;
    return current();
  }
  
  template<class VIEW>
  typename BEditor::BufferList<VIEW>::bufferStruct BufferList<VIEW>::newBuffer(const char* file)
  {
    if(!head){
      head = new bufferNode(file,0,0,configure);
      head->next = head;
      head->prev = head;
      curr = head;
    }else{
      bufferNode *temp = new bufferNode(file,curr,curr->next,configure);
      curr->next->prev = temp;
      curr->next = temp;
      curr = temp;
    }
    buffcount++;
    return current();
  }
  
  template<class VIEW>
  typename BEditor::BufferList<VIEW>::bufferStruct BufferList<VIEW>::deleteCurrent()
  {
    if(!buffcount || !curr)
      return current();
    
    if(buffcount == 1){
      head = 0;
      delete curr;
      curr = 0;
    }else{
      curr->next->prev = curr->prev;
      curr->prev->next = curr->next;
      bufferNode *temp = curr;
      curr = curr->next;
      delete temp;
    }
    buffcount--;
    return current();
  }
  
  template<class VIEW>
  typename BEditor::BufferList<VIEW>::bufferStruct BufferList<VIEW>::next()
  {
    if(curr)
      curr = curr->next;
    else
      cerr << "BufferList<>::current is NULL !!!!" << flush;
    return current();
  }
  
  template<class VIEW>
  typename BEditor::BufferList<VIEW>::bufferStruct BufferList<VIEW>::prev()
  {
    if(curr)
      curr = curr->prev;
    return current();
  }
  
  template<class VIEW> 
  inline typename BEditor::BufferList<VIEW>::bufferStruct BufferList<VIEW>::current()
  {
    if(curr)
      return bufferStruct(&curr->model,&curr->view);
    else
      return bufferStruct(0,0);
  }
 
  template<class VIEW> 
  typename BEditor::BufferList<VIEW>::bufferStruct BufferList<VIEW>::current(int n)
  {
    if(n >= buffcount)
      return current();
    bufferNode *p = head;
    while(n--)
      p = p->next;
    curr = p;
    return current();
  }
 
 
  template<class VIEW> 
  void BufferList<VIEW>::foreach(Func f)
  {
    bufferNode* n = head;
    bufferStruct b;
    if(!n)return;

    do{
      b = bufferStruct(&n->model,&n->view);
      f(b);
      n = n->next;
    }while(n != head);
  }
}
#endif

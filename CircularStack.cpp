// CircularStack.h

template<class T>
class CircularStack
{
  T** stack;
  int top,  // One Past the Top of the Stack (aka, Next Open Slot)
    base, // The first spot filled in the Stack (aka, index of BASE)
    curr, // One Past the current Spot (aka, == top,  after push())
    size; // Size of the stack
 
public:
  
  CircularStack(int s):top(0),base(0),curr(0),size(s){
    stack = new T*[size];
    for(int i = 0; i < size; i++)
      stack[i] = 0;
  }
  
  ~CircularStack(){
    for(int i = base; i != top; i = NEXT(i)){
      if(stack[i])
        delete stack[i];   
    }
    delete [] stack;
  }
    
  bool isEmpty(){return (top == base);}
  bool isFull(){return NEXT(top) == base;}
  bool atStart(){return (curr == base);}
  bool atEnd(){return curr == top;}
  
  T* current(){
    if(!isEmpty())
      return stack[PREV(curr)];
    return (T*)0;
  }
  
  T* pop(){ 
    if(!isEmpty()){
      top = PREV(top);
      return stack[top];
    }
    return (T*)0;
  }
   
  void push(T* t)
  {
    if(!atEnd()){
      for(int i = curr; i != top; i = NEXT(i)){
        delete stack[i];
        stack[i] = 0;
      }
      top = curr;
    }else
      if(isFull()){ // Move the Base, cut off the bottom of the Stack
        delete stack[base];
        stack[base] = 0;
        base = NEXT(base);
      }
    stack[top] = t;
    top = NEXT(top);
    curr = top;
  }
  
  void prev() // "Undo" == undo(curr-1); prev();
  {
    if(!atStart())
      curr = PREV(curr);
  }
   
  void next() // "Redo" == next();redo(curr-1);
  {
    if(!atEnd())
      curr = NEXT(curr);
  }
  
  int NEXT(int i){return (i+1)%size;}
  int PREV(int i){return (i+size-1)%size;}
}; 

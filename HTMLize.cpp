#include "Print.h"
#include <stdlib.h> 
#include <iostream> 

using std::cerr;
using std::ifstream;
using std::ofstream;
using std::set;

const int MAX = 256;

set<string> readKeys(const char* file);

// Convert a "source" file into an HTML page with syntax highlighting
int main(int argc, const char** argv){
  Configuration configure;
  
  string configpath = getenv("HOME");
  configpath += "/.bedit";
  configure.load(configpath.c_str());
    
  set<string> keywords = readKeys(configure.keywordFile().c_str());
  set<string> localKeywords = readKeys(configure.localKeywordFile().c_str());

  for(int i = 1; i < argc; i++){
    string out = argv[i];
    out += ".html";

    TextModel in(argv[i]);
    htmlModel(in, out.c_str(), keywords, localKeywords);
    cerr << argv[i] << " --> " << out << "\n";
  }
  return 0;
}


set<string> readKeys(const char* file){
  set<string> keywords;
  ifstream in(file);
  if(in){
    string w;    
    while(in >> w)
      keywords.insert(w);
  }
  in.close();
  return keywords;
}

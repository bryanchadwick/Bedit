// Print.cpp

#ifndef _PRINT_CPP
#define _PRINT_CPP

#include"Print.h"
#include<iostream>
const int PAGE_WIDTH   = 612,
  PAGE_HEIGHT  = 792,
  LEFT_MARGIN  = 45,
  RIGHT_MARGIN = PAGE_WIDTH-30,
  TOP_MARGIN   = PAGE_HEIGHT-45;
int FONT_POINT_SIZE = 10,
  CHAR_WIDTH   = 7,
  CHAR_HEIGHT  = 11,
  LINES_PER_PAGE = (TOP_MARGIN)/CHAR_HEIGHT - 2;

void printHeader(ofstream& file, string& title, int pnum)
{
  char page[] = {pnum/10%10+'0',
                 pnum%10+'0','\0'};
                 
  file << LEFT_MARGIN << " " << TOP_MARGIN+CHAR_HEIGHT << " moveto\n"
       << "(" << title << ") show\n"
       << RIGHT_MARGIN-(7*CHAR_WIDTH) << " " << TOP_MARGIN+CHAR_HEIGHT << " moveto\n"
       << "(Page " << page << ") show\n"
       << LEFT_MARGIN << " " << TOP_MARGIN+CHAR_HEIGHT/2 << " moveto\n"
       << RIGHT_MARGIN << " " << TOP_MARGIN+CHAR_HEIGHT/2 
       << " lineto stroke newpath\n";
}

int printModel(TextModel& m, const char *fname, const Configuration& config)
{
  ofstream file(fname);
  if(!file)
    return 1;
  FONT_POINT_SIZE = config.printSize();
  CHAR_WIDTH   = config.printWidth();
  CHAR_HEIGHT  = config.printHeight();
  LINES_PER_PAGE = (TOP_MARGIN)/CHAR_HEIGHT - 2;
 
 
  string title = m.getTitle();
  int lines = m.getLines();
  int pages = (lines+LINES_PER_PAGE-1)/LINES_PER_PAGE;
  string line;
  const char* ch;
 
  file <<  "%!PS-Adobe-2.0\n"
       <<  "% FILE : " << title 
       <<"\n% Printed with the Beditor\n"
       <<  "% *c* Bryan Chadwick, 2003\n\n"
       <<  "newpath\n/Courier findfont " << FONT_POINT_SIZE
       <<  " scalefont setfont\n";
 
  for(int i = 0; i < pages; i++,lines -= LINES_PER_PAGE){
    printHeader(file,title,1+i);
    for(int k = 0; k < LINES_PER_PAGE && k < lines; k++){
      file << LEFT_MARGIN << " " << TOP_MARGIN-(CHAR_HEIGHT*(1+k)) << " moveto\n";
      file << "(";
      line = m.lineString(i*LINES_PER_PAGE+k);
      ch = line.c_str();
      for(unsigned int j = 0; j < line.length(); j++){
        if(*ch == '\\' || *ch == ')' || *ch == '(')
          file << '\\';
        file << *ch++;
      }
      file << ") show\n";
    }
    file << " showpage\n";
  }
 
  file.close();
  return 0;
}

void newColor(ofstream &f, string c)
{
  static string color = "";
  if(color != c){
    f << "</span><span class=\""<< c << "\">";
    color = c;
  }
}

void drawChar(ofstream &f, int ch)
{
  switch(ch){
  case '>': f << "&gt;";break;
  case '<': f << "&lt;";break;
  default: f << (char)ch;
  }
}

int htmlModel(TextModel& m, const char *fname, std::set<string> keywords, std::set<string> localKeywords)
{
  ofstream file(fname);
  if(!file)
    return 1;
  string title = m.getTitle();
  int lines = m.getLines(),
    current = 0, last = 0;
 
 
  file <<  "<html><head><title>" << title << "</title></head>"
       << "\n<!-- HTMLized with the Beditor\n"
       << "     *c* Bryan Chadwick, 2003 -->\n"
       << "<style type='text/css'><!--\n"
       << "   .def{ color: #000000; }\n"
       << "   .grade{\n"
       << "      border: solid #FF0000 1px;\n"
       << "      background-color: #FFFF60;\n"
       << "   }\n"
       << "   .gcom{ font-weight: bold; background-color: #FFC0C0; }\n"
       << "   .com{ font-style: italic; color: #880000; }\n"
       << "   .keyw{ font-weight: bold; color: #000088; }\n"
       << "   .num{ color: #00AA00; }\n"
       << "   .func{ color: #BB7733; }\n"
       << "   .str{ color: #CC00AB; }\n"
       << "   .prim{ color: #0000FF; }\n"
       << "--></style>\n"
       << "<body><pre><span class=\"def\">\n";
 
  TextModelIterator i = m.begin(),
    n = m.begin();
  ++n;
  while(i != m.end()){
    current = *i;
      
    if(current == '/' && *n == '/'){
      TextModelIterator t = n;
      ++t;
      bool grade = (*t == '>');
      if(grade)newColor(file,"gcom");
      else newColor(file,"com");
      while(i != m.end() && *i != '\n'){
        current = *i;
        drawChar(file,current);
        last = (char)current;
        ++i;++n;
      }
    }else{
      if(current == '/' && *n == '*'){
        newColor(file,"com");
        do{
          last = (char)current;
          current = *i;
          drawChar(file,current);
          ++i;++n;
        }while(i != m.end() && (last != '*' || (char)current != '/'));
      }else{ // Strings
        if((current == '\'' || current == '\"') && last != '\\'){ // '
          newColor(file,"str");
          drawChar(file,current);
          ++i;++n;
          while(i != m.end()){
            last = (char)current;
            current = *i;
            drawChar(file,current);
            ++i;++n;
            if(((char)current == '\'' || (char)current == '\"') &&  last != '\\') // '
              break;
          }
        }else{ // Numbers
          if(isdigit(*i)){
            newColor(file,"num");
            while(i != m.end() && isxdigit(*i) || *i == 'x'){
              last = (char)current;
              current = *i;
              drawChar(file,current);
              ++i;++n;
            }
          }else{
            if(isalnum(*i) || *i == '_'){ // Keywords/Others
              string s;
              while(i != m.end() && (isalnum(*i) || *i == '_')){
                s += *i;
                ++i;++n;
              }
              if(keywords.find(s) != keywords.end())
                newColor(file,"keyw");
              else if(localKeywords.find(s) != localKeywords.end())
                newColor(file,"prim");
              else if(*i == '(')
                newColor(file,"func");

              for(unsigned int i = 0; i < s.length(); i++)
                drawChar(file,s[i]);
            }else{
              newColor(file,"def");
              drawChar(file,*i);
              ++i;++n;
            }
          }
        }
      }
    }
    last =(char)current;
    newColor(file,"def");
    // Skip White Space
    while(i != m.end() && isspace(*i)){
      last = *i;
      drawChar(file,*i);
      ++i;++n;
    }
  }
  file << "</span></pre></body></html>";
  file.close();
  return 0;
}
#endif

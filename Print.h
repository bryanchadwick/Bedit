// Print.h

#ifndef _PRINT_H
#define _PRINT_H

#include"Bedit.h"
#include<string>
#include<fstream>
#include<set>

using std::string;
using std::ofstream;
using namespace BEditor;

void printHeader(ofstream& file, string& title, int pnum);
int printModel(TextModel& m, const char *fname, const Configuration& config);
int htmlModel(TextModel& m, const char *fname, std::set<string> keywords, std::set<string> localKeywords);

#endif

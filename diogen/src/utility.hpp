#ifndef _H_utility
#define _H_utility

#include <map>
#include <string>

using namespace std;

typedef map<string, string, less<string> > SymbolTable;
typedef pair<string, string> StringPair;

void printStringMap( SymbolTable t );

#endif

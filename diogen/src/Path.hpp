#ifndef _H_Path
#define _H_Path

#include <vector>
#include <ostream>
#include "utility.hpp"

using namespace std;

class Node;

class Path : public vector<Node *> {
private:
   string dio;
   pair<Node *, Node*> * txGateNode;

   string getModel( SymbolTable params, string modelPrefix, Node * switchNode, pair<Node *, Node *> * txGateNode );
   int distanceBetween( Node * n1, Node * n2 );


   vector<Node *> transistors; // transistors in the path
   vector<Node *> internalNodes;

public:
   Path();// : vector<Node *>();
   Path( reverse_iterator a, reverse_iterator b ); // : vector<Node *>(a, b) {};
   Path( iterator a, iterator b ); // : vector<Node *>(a, b) {};

   string getDio();
   void checkForTxGates();
   pair<Node *, Node *> * getTxGateNode();
   string replaceModelString( string model, string r, string w );

   void generateDio( SymbolTable params, string modelPrefix, Node * outputNode, Node * switchNode );

   //void reverse();
   void append( Path p );
   void print( ostream &o );
   void separateElements();
   bool contains( Node * n );
   bool containsAntiCorrOrIntPrechargeInputs( map<string,string> AntiCorrInputMap, map<string, int> IntPrechargeInputMap );
};

#endif

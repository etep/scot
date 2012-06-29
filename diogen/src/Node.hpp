#ifndef _H_node
#define _H_node

#include <strings.h>

#include <string>
#include <vector>

#include "Type.hpp"
#include "utility.hpp"

using namespace std;

class Path;

/*
 * This class defines the types of devices that are legal in the
 * circuit parser.  The devices are categorized according to the
 * number of terminals that they have, for example, a transistor
 * is a subclass of the three terminal device.
 */
struct ltstr {
   bool operator()( string s1, string s2 ) const {
      return strcasecmp( s1.c_str(), s2.c_str() );
      //return strcmp(s1, s2) < 0;
   }
};

class Node {
   friend class Input;

protected:

   string name; // as defined in the spice netlist
   Type * type;

   vector<Node *> connections;
   int numMaxConnections;

   bool hasVisited; // for path generation


   string fallCapExpr;
   string riseCapExpr;

   string avgCapExpr;
   string wireCapExpr;

   SymbolTable parameters;


   // activity factors
   double lowToHighTransitionFactor;
   double highToLowTransitionFactor;


   static const int notRestricted = -1;
   virtual void addConnectionAt( Node * n, int connectionNum );



public:
   //typedef vector<Node *> Path;

   virtual void addConnection( Node * n );
   Node * getConnectionAt( int i ) {
      return connections[i];
   }
   vector<Node *> getConnections() {
      return connections;
   }

   virtual string toString();
   Type * getType();
   string getName();

   virtual string getAvgCapExpr() {
      return "";
   }
   virtual string getAvgCapExpr( SymbolTable s, string m, Node * o ) {
      return "";
   }
   virtual string getWireCapExpr() {
      return "";
   }

   virtual string getFallCapExpr() {
      return "";
   }
   virtual string getFallCapExpr( SymbolTable s, string m, Node * o ) {
      return "";
   }

   virtual string getRiseCapExpr() {
      return "";
   }
   virtual string getRiseCapExpr( SymbolTable s, string m, Node * o ) {
      return "";
   }

   virtual string getParameter( string key ) {
      return "";
   }

   void resetHasVisited();

   virtual vector<Node *> getChildren( Node * goal );

   bool concatToPath( Path &path, vector<Node *> outputNodeVec, map<string,string> AntiCorrInputMap );

   void findPathOnChildren ( Node * end, vector<Node *> outputVec, Path &currentPath, vector<Path> &finishedPaths, map<string,string> AntiCorrInputMap );
   void findPathBetween    ( Node * end, vector<Node *> outputVec, Path &currentPath, vector<Path> &finishedPaths, map<string,string> AntiCorrInputMap );
   bool equals( Node * e );

   virtual bool existConnectionTo( Node * );

   void setLowToHighTransitionFactor( double f );
   void setHighToLowTransitionFactor( double f );
   double getLowToHighTransitionFactor();
   double getHighToLowTransitionFactor();

   virtual ~Node() {}
};



#endif





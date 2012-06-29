#include <stdlib.h>
#include <iostream>
#include "Node.hpp"
#include "Path.hpp"

Type * Node::getType() {
   return this->type;
}

string Node::getName() {
   return this->name;
}

void Node::setHighToLowTransitionFactor( double f ) {
   this->highToLowTransitionFactor = f;
}

void Node::setLowToHighTransitionFactor( double f ) {
   this->lowToHighTransitionFactor = f;
}

double Node::getHighToLowTransitionFactor() {
   return highToLowTransitionFactor;
}

double Node::getLowToHighTransitionFactor() {
   return lowToHighTransitionFactor;
}

void Node::resetHasVisited() {
   hasVisited = false;
}


string Node::toString() {
   return "You are invoking toString on the base node type!";
}


void Node::addConnectionAt( Node * n, int connectionNum ) {

   if( numMaxConnections != notRestricted && connectionNum >= numMaxConnections ) {
      cerr << "You are trying to insert into a node that has ";
      cerr << "fewer max number of connections allowed." << endl;
      exit( 1 );
   }

   int numMissing = connectionNum - connections.size() + 1;

   if( numMissing != 0 ) {
      connections.resize( connectionNum + 1 );
   }

   connections[connectionNum] = n;
}

void Node::addConnection( Node * n ) {
   connections.push_back( n );
}

bool Node::concatToPath( Path &path, vector<Node *> outputNodeVec, map<string,string> AntiCorrInputMap ) {
   //check if this has been visited or goes thru any of the outputs.
   bool notOut = true;
   for( unsigned i = 0; i< outputNodeVec.size(); i++ ) {
      if( this == outputNodeVec[i] ) {
         notOut = false;
         break;
      }
   }
   //check if its a cap, if yes do not concatenate it.
   if( getType()->equals( Type::intCapType ) ) {
      hasVisited =  true;
      return false;
   }
   // If the path length is small enough do not check for loops.
   if( !hasVisited &&  notOut && path.size() < 3 ) {
      path.push_back( this );
      hasVisited = true;
      return true;
   }

   // Check for loops, check against all the nodes in the current path.
   Node * goal;
   vector<Node *> children = getChildren( goal );
   //	cout << getName() << " is the parent " << endl;
   bool noClash = true;
   for( unsigned i = 0; i< children.size(); i++ ) {
      //			 cout << children[i]->getName() << " is a child " << endl;
      if( path[0] == children[i] ) {
         noClash = false;
      }
   }
   for( unsigned i = 0; i< path.size(); i++ ) {
      //			 cout << path[i]->getName() << " is being compared " << endl;
      if( this == path[i] ) {
         noClash = false;
      }
   }
   if ( notOut && noClash ) {
      path.push_back( this );
      hasVisited = true;
      return true;
   }

   return false;
}

vector<Node *> Node::getChildren( Node * goal ) {
   vector<Node *> v;
   return v;
}

void Node::findPathOnChildren( Node * end, vector<Node *> outputVec, Path &currentPath, vector<Path> &finishedPaths, map<string,string> AntiCorrInputMap ) {

   vector<Node *> children = this->getChildren( end );

   //if (children == NULL) return;

   vector<Node *>::iterator it;
   //	cout << "The possible children of current node " << getName() << " are ";
   //   while ( i != children.end() )
   //	{
   //			 cout << (*i)->getName() << " ";
   //			 i++;
   //	}
   //	 cout << endl;
   //	i = children.begin();
   for( it = children.begin(); it != children.end(); it++ ) {
      Path newPath = currentPath;
      ( *it )->findPathBetween( end, outputVec, newPath, finishedPaths, AntiCorrInputMap );
   }
}


void Node::findPathBetween( Node * end, vector<Node *> outputVec, Path &currentPath, vector<Path> &finishedPaths, map<string,string> AntiCorrInputMap ) {

   // cout << "This Node is " << getName() << " and end node is " << end->getName() << endl;
   if( this->equals( end ) ) { // play end
      //this->hasVisited = true;
      currentPath.push_back( this );
      finishedPaths.push_back( currentPath );
      cout << "Initially recorded ";
      currentPath.print( cout );
      return;
   }

   if( this->concatToPath( currentPath, outputVec, AntiCorrInputMap ) ) {
      this->findPathOnChildren( end, outputVec, currentPath, finishedPaths, AntiCorrInputMap );
   }
   return;
}

bool Node::existConnectionTo( Node * e ) {
   for( int i = 0; i < connections.size(); i++ ) {
      if( connections[i]->equals( e ) ) {
         return true;
      }
   }
   return false;
}

bool Node::equals( Node * e ) {
   return ( strcasecmp( name.c_str(), e->getName().c_str() ) == 0 ); //&& type == e->getType() );
}




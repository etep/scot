#include <stdlib.h>
#include <stdio.h>

#include "Path.hpp"
#include "Node.hpp"
#include "ModelPrefix.hpp"
#include "Transistor.hpp"



Path::Path() {
   separateElements();
   txGateNode = NULL;
}

Path::Path( reverse_iterator a, reverse_iterator b ) : vector<Node *>( a, b ) {
   Path();
}

Path::Path( iterator a, iterator b ) : vector<Node *>( a, b ) {
   Path();
}

string Path::getDio() {
   return dio;
}

pair<Node *, Node *> * Path::getTxGateNode() {
   return txGateNode;
}

void Path::append( Path p ) {
   Path::iterator it;
   for( it = p.begin(); it != p.end(); it++ ) {
      push_back( *it );
   }
}


void Path::print( ostream & o ) {

   Path::iterator it;

   o << "path includes:" << endl;
   for( it = begin(); it != end(); it++ ) {
      
      o << ( *it )->getName() << endl;
   }
}

void Path::generateDio( SymbolTable params, string modelPrefix, Node * outputNode, Node * switchNode ) {

   bool rise = ( modelPrefix == fallToRiseStd ||
                 modelPrefix == fallToRiseNor );

   string cap = "";
   if( rise ) {
      cap = ( *this )[0]->getRiseCapExpr( params, modelPrefix, outputNode );
   }
   else{
      cap = ( *this )[0]->getFallCapExpr( params, modelPrefix, outputNode );
   }

   if( cap == "" ) {
      return;
   }

   //print(cout);

   //pair<Node *, Node *> * txGateNodePair = NULL;

   string model;

   checkForTxGates();

   if( this->txGateNode != NULL ) {
      
      int distanceToOutput = distanceBetween( ( *this )[0], txGateNode->first );

      char distToOutputString[10];

      sprintf( distToOutputString, "%d", distanceToOutput );

      string modelPrefixWithTransmission = modelPrefix + transistorPrefix + string( distToOutputString ) + "_";

      model = getModel( params, modelPrefixWithTransmission, switchNode, txGateNode );
      
   }
   else {
      model = getModel( params, modelPrefix, switchNode, NULL );
   }

   string dio = cap + " * " + model;


   iterator i = begin();
   if( !switchNode->equals( *++i ) ) {
      i++;
      Path nextPath( i, end() );
      nextPath.separateElements();
      nextPath.generateDio( params, modelPrefix, outputNode, switchNode );

      string nextPathDio = nextPath.getDio();

      this->dio = dio + ( ( nextPathDio == "" ) ? "" : " + " + nextPathDio );
   }
   else {
      this->dio = dio;
   }
}


// count number of transistors between n1 and n2
int Path::distanceBetween( Node * n1, Node *n2 ) {
   iterator i = begin();
   int count = 0;
   bool foundN1 = false;
   bool foundN2 = false;

   while ( i != end() ) {
      if ( ( *i )->equals( n1 ) ) foundN1 = true;
      if ( ( *i )->equals( n2 ) ) foundN2 = true;

      if ( foundN1 && foundN2 ) return count;

      if ( ( ( foundN1 && !foundN2 ) || ( !foundN1 && foundN2 ) ) &&
            ( ( *i )->getType()->equals( Type::nmosType ) ||
              ( *i )->getType()->equals( Type::pmosType ) ) )
         count++;

      i++;
   }

   return -1; // not found
}


void Path::separateElements() {
   for( int i = 0; i < size(); i++ ) {
      
      const bool isNmosType = ( *this )[i]->getType()->equals( Type::nmosType );
      const bool isPmosType = ( *this )[i]->getType()->equals( Type::pmosType );
      const bool isXstrType = isNmosType || isPmosType;
      
      if( isXstrType ) {
         transistors.push_back   ( ( *this )[i] );
      }
      else {
         internalNodes.push_back ( ( *this )[i] );
      }
   }
}

string Path::getModel( SymbolTable params, string modelPrefix, Node * switchNode, pair<Node *, Node *> * txNode ) {

   int numTransistors = size() / 2;

   char numTransistorsStr[10];
   sprintf( numTransistorsStr, "%d", numTransistors );

   string lookupString = modelPrefix + string( numTransistorsStr );

   #ifdef DEBUG
   cout << "lookupstring = " << lookupString << endl;
   #endif

   if( params.find( lookupString ) == params.end() ) {
      cerr << "model " + lookupString + " was not found!" << endl;
      return modelNotFound;
   }

   string model = params[lookupString];
   string widthPrefix = width;

   if( txNode != NULL ) {
      // assume that a tx gate only has two nodes
      model = replaceModelString( model, txGateWidthPrefix + string( "1" ), txNode->first->getParameter  ( effectiveWidth ) );
      model = replaceModelString( model, txGateWidthPrefix + string( "2" ), txNode->second->getParameter ( effectiveWidth ) );
   }

   int transistorNumberToReplace = 1;
   for( int i = 0; i < transistors.size(); i++ ) {

      if ( txNode == NULL || ( !transistors[i]->equals( txNode->first  ) && !transistors[i]->equals( txNode->second ) ) ) {

         sprintf( numTransistorsStr, "%d", transistorNumberToReplace );
         string stringToReplaceInModel = widthPrefix + string( numTransistorsStr );

         model = replaceModelString( model, stringToReplaceInModel, transistors[i]->getParameter( effectiveWidth ) );
         transistorNumberToReplace++;
      }
   }

   return model;
}

void Path::checkForTxGates() {

   for( int i = 0; i < transistors.size(); i++ ) {
      
      Node * currentTransistor = transistors[i];
      Node * source = currentTransistor->getConnectionAt( Transistor::SourceConnection );
      Node * drain  = currentTransistor->getConnectionAt( Transistor::DrainConnection );
      const bool isNmos = ( currentTransistor->getType()->equals( Type::nmosType ) );

      vector<Node *> sourceConnections = source->getConnections();
      
      for( int j = 0; j < sourceConnections.size(); j++ ) {

         const bool nmos2pmos  =  isNmos && sourceConnections[j]->getType()->equals( Type::pmosType );
         const bool pmos2nmos  = !isNmos && sourceConnections[j]->getType()->equals( Type::nmosType );
         // -- TODO -- check the logic here... -- const bool outputNode = nmos2pmos || pmos2nmos;
         // -- TODO -- should it be (nmos2pmos || pmos2nmos) && rest-of-test
         
         if( nmos2pmos || pmos2nmos && !currentTransistor->equals( sourceConnections[j] ) ) {

            if( drain->existConnectionTo( sourceConnections[j] ) ) {

               this->txGateNode = new pair<Node *, Node *>( currentTransistor, sourceConnections[j] );
               return;
            }
         }
      }
   }

   this->txGateNode = NULL;
}

string Path::replaceModelString( string model, string toReplace, string replaceWith ) {
   /*
   int foundPos = 0;
   while ( (foundPos = model.find(toReplace, foundPos)) != string::npos)
     {
       model.replace(foundPos, toReplace.size(), replaceWith);
     }
   */
   int foundPos   = 0;
   int currentPos = 0;

   for( currentPos = 0; currentPos < model.length(); currentPos++ ) {

      foundPos = model.find( toReplace, currentPos );
      if( foundPos != string::npos ) {
         model.replace( foundPos, toReplace.size(), replaceWith );
      }
      currentPos++;
   }

   return model;
}

bool Path::contains( Node * n ) {
   
   Path::iterator it;

   for( it = begin(); it != end(); it++ ) {
      if( ( *it )->getName() == n->getName() ) {
         return true;
      }
   }
   return false;
}


bool Path::containsAntiCorrOrIntPrechargeInputs( map<string,string> AntiCorrInputMap, map<string, int> IntPrechargeInputMap ) {
   //		 cout <<  "Reached here1" << endl;
   for( Path::iterator it0 = begin(); it0 != end(); it0++ ) {
      
      const bool isNmos0 = ( *it0 )->getType()->equals( Type::nmosType );
      const bool isPmos0 = ( *it0 )->getType()->equals( Type::pmosType );
      const bool isXstr0 = isNmos0 || isPmos0;
      if( isXstr0 ) {
         
         string in1 = ( *it0 )->getConnectionAt( Transistor::GateConnection )->getName();
         
         if( IntPrechargeInputMap.find( in1 ) != IntPrechargeInputMap.end() ) {
            return true;
         }
         
         //							 cout << "Testing " << in1 << " and ";
         for( Path::iterator it1 = it0+1; it1 != end(); it1++ ) {
            
            const bool isNmos1 = ( *it1 )->getType()->equals( Type::nmosType );
            const bool isPmos1 = ( *it1 )->getType()->equals( Type::pmosType );
            const bool isXstr1 = isNmos1 || isPmos1;
            
            if( isXstr1 ) {
               
               string in2 = ( *it1 )->getConnectionAt( Transistor::GateConnection )->getName();
               //												 cout <<  in2 << endl;
               if( AntiCorrInputMap.find( in1 ) != AntiCorrInputMap.end()
                     && AntiCorrInputMap.find( in1 )->second == in2
                     || AntiCorrInputMap.find( in2 ) != AntiCorrInputMap.end()
                     && AntiCorrInputMap.find( in2 )->second == in1 ) {
                  //															cout <<  "Reached here" << endl;
                  return true;
               }
            }
         }
      }
   }
   return false;
}


/*
void Path::reverse()
{
  Path * reversedPath = new Path();

  Path::iterator i = rbegin();

  while ( i != rend() )
    {
      reversedPath->push_back(*i);
      i++;
    }

  this = reversedPath;
}
*/

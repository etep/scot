#include "Transistor.hpp"
#include <iostream>
#include "Vdd.hpp"
#include "Gnd.hpp"
#include "ModelPrefix.hpp"

Transistor::Transistor( string name, Type * type, Node * drainNode, Node * sourceNode, Node * gateNode, Node * bodyNode, SymbolTable parameters ) {
   
   this->name = name;
   this->type = type;
   this->numMaxConnections = 4;
   this->parameters = parameters;

   addConnectionAt( drainNode,  DrainConnection  );
   addConnectionAt( sourceNode, SourceConnection );
   addConnectionAt( gateNode,   GateConnection   );
   addConnectionAt( bodyNode,   BodyConnection   );
}

Node * Transistor::getConnectionAt( int location ) {
   if( location < 4 && getType()->equals( Type::nmosType ) ) {
      return connections[location];
   }
   else if( location < 4 && getType()->equals( Type::pmosType ) ) {
      return connections[location];
   }
   else if ( location < 2 && getType()->equals( Type::intCapType ) ) {
      return connections[location];
   }
   else {
      cerr << "you are trying to get an invalid connection on a transistor" << endl;
      exit( 1 );
   }
}

string Transistor::getParameter( string key ) {
   if( key == effectiveWidth ) {

      string m = ( parameters.find( multiplier ) != parameters.end() ) ? parameters[multiplier] : "" ;

      if( m != "" ) {
         return ( "( " + m + " * " + parameters[width] + " )" );
      }
      else {
         return parameters[width];
      }
   }
   if( key == intCapVal && !( getType()->equals( Type::intCapType ) ) ) {
      return "";
   }
   return parameters[key];
}

void Transistor::addConnectionAt( Node * n, int where ) {
   Node::addConnectionAt( n, where );
}

vector<Node *> Transistor::getChildren( Node * goal ) {

   Vdd * vdd = new Vdd();
   Gnd * gnd = new Gnd();

   vector<Node *> v;
   /*
   if (type->equals(Type::nmosType))
     {
       if (goal->equals(gnd))
   v.push_back(connections[SourceConnection]);
       else if (goal->equals(vdd)) {} // this should be error
       else v.push_back(connections[DrainConnection]);
     }
   else if (type->equals(Type::pmosType))
     {
       if (goal->equals(vdd))
   v.push_back(connections[SourceConnection]);
       else if (goal->equals(gnd)) {} // this should be error
       else v.push_back(connections[DrainConnection]);
     }
   */
   v.push_back( connections[SourceConnection] );
   v.push_back( connections[DrainConnection] );

   return v;
}

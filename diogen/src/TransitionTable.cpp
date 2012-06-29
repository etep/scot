#include <stdlib.h>
#include <iostream>
#include "TransitionTable.hpp"

void TransitionTable::createEntry( string name, int value ) {

   if( table.find( name ) == table.end() ) {
      table[name] = new TransitionTableEntry( name, value );
      //cout << "creating new entry: " << name << endl;
   }
   else {
      cerr << "trying to creating an existing entry " << name << endl;
   }
}

void TransitionTable::updateEntry( string name, int value ) {

   if( table.find( name ) == table.end() ) {
      
      createEntry( name, value );
      //cerr << "trying to update a non-existing entry " << name << endl;
      return;
   }

   if( table[name]->lastValue == 0 && value == 1 ) {
      table[name]->numLowToHighTransitionsFound++;
   }
   else if( table[name]->lastValue == 1 && value == 0 ) {
      table[name]->numHighToLowTransitionsFound++;
   }


   table[name]->lastValue = value;
   table[name]->numTransitions++;
}

void TransitionTable::printLowToHighFactors( ostream & o ) {
   
   map<string, TransitionTableEntry *>::iterator it;

   for( it = table.begin(); it != table.end(); it++ ) {

      TransitionTableEntry * e = it->second;

      double factor = ( double ) e->numLowToHighTransitionsFound / e->numTransitions;

      o << "name = "    << e->name            << " factor = " << factor;
      o << " number = " << e->numTransitions  << endl;
   }
}

void TransitionTable::calculateActivityFactors( map<string, Node *> internals ) {

   map<string, TransitionTableEntry *>::iterator it;
   
   for( it = table.begin(); it != table.end(); it++ ) {

      TransitionTableEntry * e = it->second;

      double lowToHighFactor = ( double ) e->numLowToHighTransitionsFound / e->numTransitions;
      double highToLowFactor = ( double ) e->numHighToLowTransitionsFound / e->numTransitions;

      // irsim is case insensitive
      if( internals.find( e->name ) == internals.end() ) {
         cerr << "error in calculating activity factors" << endl;
         cerr << "unable to find node " << e->name << endl;
         exit( 1 );
      }

      internals[e->name]->setLowToHighTransitionFactor( lowToHighFactor );
      internals[e->name]->setHighToLowTransitionFactor( highToLowFactor );
   }
}


TransitionTableEntry::TransitionTableEntry( string name, int value ) {
   
   this->name = name;
   this->numTransitions = 0;
   this->numLowToHighTransitionsFound = 0;
   this->numHighToLowTransitionsFound = 0;
   this->lastValue = value;
}


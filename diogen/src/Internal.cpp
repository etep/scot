#include "Internal.hpp"
#include "ModelPrefix.hpp"

Internal::Internal( string name ) {
   this->name = name;
   type = Type::intType;
   numMaxConnections = notRestricted;
}

void Internal::addConnection( Node * n ) {
   Node::addConnection( n );
}

string Internal::getFallCapExpr() {
   return fallCapExpr;
}

string Internal::getFallCapExpr( SymbolTable constants, string modelPrefix, Node * outputNode ) {

   if( fallCapExpr == "" ) {
      calculateCapExpr( constants, modelPrefix, outputNode, false );
   }

   return fallCapExpr;
}

string Internal::getAvgCapExpr() {
   return avgCapExpr;
}


string Internal::getAvgCapExpr( SymbolTable constants, string modelPrefix, Node * outputNode ) {
   
   if( avgCapExpr == "" ) {
      calculateCapExpr( constants, modelPrefix, outputNode, false );
   }

   return avgCapExpr;
}

string Internal::getWireCapExpr() {
   return wireCapExpr;
}

string Internal::getRiseCapExpr() {
   return riseCapExpr;
}

string Internal::getRiseCapExpr( SymbolTable constants, string modelPrefix, Node * outputNode ) {

   if( riseCapExpr == "" ) {
      calculateCapExpr( constants, modelPrefix, outputNode, true );
   }

   return riseCapExpr;
}


void Internal::calculateCapExpr( SymbolTable constants, string modelPrefix, Node * outputNode, bool rise ) {
   
   string currentCapExpr;
   string currentAvgCapExpr; // for energy generation
   string currentWireCapExpr; // for only wire energy generation

   string edgeConstant;
   string avgEdgeConstant;
   string covConstant;
   string widthConstant;
   string avgWidthConstant;
   string width;
   string intCap;


   for( int i = 0; i < connections.size(); i++ ) {
      if( ! connections[i]->getType()->equals( Type::intCapType ) ) {

         width = connections[i]->getParameter( effectiveWidth );

         if( connections[i]->getType()->equals( Type::nmosType ) ) {
            
            covConstant = constants[CovN];
            edgeConstant = rise ? constants[CparN_r_edge] : constants[CparN_f_edge];
            avgEdgeConstant = constants[CenergyN_edge];
            widthConstant = rise ?constants[CparN_r_width] : constants[CparN_f_width];
            avgWidthConstant = constants[CenergyN_width];
         }
         else {
            covConstant = constants[CovP];
            //width = "WP";
            edgeConstant = rise ? constants[CparP_r_edge] : constants[CparP_f_edge];
            avgEdgeConstant = constants[CenergyP_edge];
            widthConstant = rise ? constants[CparP_r_width] : constants[CparP_f_width];
            avgWidthConstant = constants[CenergyP_width];
         }

         if( currentCapExpr.length() == 0 ) {
            currentCapExpr = "( " + edgeConstant + " + ( " + covConstant + " + " + widthConstant + " ) * " + width;
         }
         else {
            currentCapExpr += ( " + " + edgeConstant + " + ( " + covConstant + " + " + widthConstant + " ) * " + width );
         }

         if( currentAvgCapExpr.length()  == 0 ){
            currentAvgCapExpr = "( " + avgEdgeConstant + " + ( " + covConstant + " + " + avgWidthConstant + " ) * " + width;
         }
         else{
            currentAvgCapExpr += ( " + " + avgEdgeConstant + " + ( " + covConstant + " + " + avgWidthConstant + " ) * " + width );
         }
      }
      else {
         
         intCap = connections[i]->getParameter( intCapVal );
         //					 if(currentAvgCapExpr.length() == 0)
         //								currentAvgCapExpr = "( (" + intCap + ")";
         //					 else
         //					 currentAvgCapExpr += " + (" + intCap + ")";
         if( currentCapExpr.length() == 0 ) {
            currentCapExpr = "( (" + intCap + ")";
         }
         else {
            currentCapExpr +=  " + (" + intCap + ")";
         }

         if( currentWireCapExpr.length() == 0 ) {
            currentWireCapExpr = "( " + intCap;
         }
         else {
            currentWireCapExpr += ( " + " + intCap );
         }
      }
   }
   
   if( currentWireCapExpr.length() != 0 ) {
      currentWireCapExpr += " )";
   }

   if( currentAvgCapExpr.length() != 0 ) {
      currentAvgCapExpr += " )";
   }

   if( equals( outputNode ) ) {
      if( rise ) {
         currentCapExpr += " + _LOAD_R_";
      }
      else {
         currentCapExpr += " + _LOAD_F_";
      }
   }

   if( currentCapExpr.length() != 0 ) {
      currentCapExpr += " )";
   }

   if( rise ) {
      riseCapExpr = currentCapExpr;
      //riseAvgCapExpr = currentAvgCapExpr;
   }
   else {
      fallCapExpr = currentCapExpr;
      //fallAvgCapExpr = currentAvgCapExpr;
   }
   
   avgCapExpr = currentAvgCapExpr;
   wireCapExpr = currentWireCapExpr;

   //  capExpr = currentCapExpr;
   //return capExpr;
}

#include "CircuitParser.hpp"
#include <stdlib.h>
#include "Instance.hpp"
#include <fstream>
#include "ModelPrefix.hpp"


void CircuitParser::setConstants( SymbolTable constants ) {
   this->constants = constants;
}

void CircuitParser::setSubcircuits( map<string, Subcircuit *> sub ) {
   this->subcircuits = sub;
}

Subcircuit * CircuitParser::getSubcircuit( string name ) {
   if( subcircuits.find( name ) != subcircuits.end() ) {
      return subcircuits[name];
   }
   else return NULL;
}

void CircuitParser::setElements( map<string, Node *> elements ) {
   this->elements = elements;
}

void CircuitParser::insertInternal( string name, Node * internal ) {
   internals[convertToLower( name )] = internal;
}

void CircuitParser::insertCapNetNode( string CCCname, Node * net ) {
   if( capNetNodeMap.find( CCCname ) == capNetNodeMap.end() ) {
      vector<Node *> tmp;
      capNetNodeMap[CCCname] = tmp;
   }
   capNetNodeMap[CCCname].push_back( net );
}

vector<Node *> CircuitParser::getCapNetNodes( string CCCname ) {
   return capNetNodeMap[CCCname];
}

void CircuitParser::setAntiCorrInfo( string name, string in1, string in2 ) {
   if( getSubcircuit( name ) == ( Subcircuit * ) NULL ) {
      cout << "No CCC of the name " << name << " exists for setting the anticorrelation of inputs " << in1 << " and " <<  in2 << endl;
      exit( -1 );
   }
   getSubcircuit( name )->putAntiCorrInput( in1, in2 );
}

void CircuitParser::setIntPrechargeInputInfo( string name, vector<string> preIns ) {
   if( getSubcircuit( name ) == ( Subcircuit * ) NULL ) {
      cout << "No CCC of the name " << name << " exists setting the Precharge Inputs" << endl;
      exit( -1 );
   }
   getSubcircuit( name )->recIntPrechargeInputs( preIns );
}


string CircuitParser::convertToLower( string in ) {
   string out( in );
   string::iterator i = in.begin();
   string::iterator j = out.begin();

   while ( i != in.end() ) {
      *j = tolower( *i );
      j++;
      i++;
   }

   return out;
}

Node * CircuitParser::getInternal( string name ) {
   if( internals.find( name ) != internals.end() ) {
      return internals[name];
   }
   return NULL;
}

void CircuitParser::setPrimaryInputs( vector<pair<string,double> > inputs ) {
   this->primaryInputs = inputs;
}


void CircuitParser::setIrsimData( vector<SymbolTable> irsimData ) {
   this->irsimData = irsimData;
   //printf("data = %d\n", irsimData.size());
}





void CircuitParser::generateDio( ostream &o, bool printToFile ) {
   map<string, Subcircuit *>::iterator i = subcircuits.begin();

   while ( i != subcircuits.end() ) {
      Subcircuit * currentSub = i->second;
      currentSub->generateDio( o, constants, printToFile );
      i++;
   }
}

void CircuitParser::generateTxGateConstraints( ostream &o ) {
   map<string, Subcircuit *>::iterator i = subcircuits.begin();

   int print = 0;
   o << endl << ".TRANSMISSION" << endl;

   while ( i != subcircuits.end() ) {
      Subcircuit * currentSub = i->second;
      print += currentSub->generateTxGateConstraints( o );
      o << endl;
      i++;
   }
   o << ".ENDS" << endl;
}

#define MAX_NUM_NODES_PER_WATCH_STMT 10

void CircuitParser::generateIrsimInput( ostream &o, string filename, int numRuns, double defActFact ) {
   o << "stepsize 1000" << endl;

   string primaryInputsString = "";//vector primary_inputs ";

   vector<pair<string, double> >::iterator i = primaryInputs.begin();
   vector<double> primaryInputAf;

   while( i != primaryInputs.end() ) {
      primaryInputsString += i->first;
      primaryInputAf.push_back( i->second );
      primaryInputsString += " ";
      i++;
   }
   o << "vector primary_inputs " << primaryInputsString << endl;


   //get other nets in the circuit and watch them
   // due to the limit in the max line length in irsim...
   int numWatchStatements = internals.size() / MAX_NUM_NODES_PER_WATCH_STMT;
   if ( internals.size() % MAX_NUM_NODES_PER_WATCH_STMT != 0 )
      numWatchStatements++;

   map<string, Node *>::iterator ii = internals.begin();

   for( int j = 0; j < numWatchStatements; j++ ) {
      int currNodeNum = 0;
      string nodesToWatch = "w ";

      while ( ii != internals.end() && currNodeNum < MAX_NUM_NODES_PER_WATCH_STMT ) {
         if ( ii->second->getType() != Type::gndType &&
               ii->second->getType() != Type::vddType ) {
            nodesToWatch += ii->first;
            nodesToWatch += " ";
            currNodeNum++;
         }

         ii++;
      }
      o << nodesToWatch << endl;
   }


   o << "logfile " << filename << ".out" << endl;
   //set the initial vector
   string stimulus;
   std::vector<std::string> prevStimulus;
   prevStimulus.resize( primaryInputs.size() );
   for( int i = 0; i < primaryInputs.size(); i++ ) {
      int num = rand();
      if ( num < RAND_MAX/2 )
         prevStimulus[i] = "0";
      else
         prevStimulus[i] = "1";
   }

   for( int i = 0; i < numRuns; i++ ) {
      stimulus += "set primary_inputs ";
      stimulus += generateRandomVector( primaryInputs.size(),prevStimulus, primaryInputAf, defActFact );
      stimulus += "\ns\n";
   }

   o << stimulus << endl;
   o << "logfile" << endl;
   o << "exit" << endl;
}

string CircuitParser::generateRandomVector( int length, std::vector<std::string> prevStimulus, vector<double> inputAf, double defActFact ) {
   string randomVector;
   double actFact;


   for( int i = 0; i < length; i++ ) {
      if( inputAf[i] < 0 ) {
         actFact = 0.25;
      }
      else if( inputAf[i] > 0.5 ) {
         cout << "Activity Factor > 0.5 not accepted."
              << " To use >0.5 scale all other AFs such that"
              << " the max(AF) < 0.5"<< endl;
         exit( 1 );
      }
      else {
         actFact = inputAf[i];
      }
      int num = rand();
      if( num > 2*actFact*RAND_MAX ) {
         randomVector += prevStimulus[i];
      }
      else if( prevStimulus[i] == "0" ) {
         randomVector += "1";
         prevStimulus[i] =  "1";
      }
      else {
         randomVector += "0";
         prevStimulus[i] = "0";
      }
   }

   return randomVector;
}

void CircuitParser::generateActivityFactors( ostream &o ) {
   TransitionTable * t = new TransitionTable();
   vector<SymbolTable>::iterator i = irsimData.begin();

   // create the first entries
   //createTransitionTableEntries(t, *i);
   //i++;

   while( i != irsimData.end() ) {
      processOneTransitionEntry( t, *i );
      i++;
   }

   t->calculateActivityFactors( internals );
   //t->printLowToHighFactors(o);


   // print out the activity factors (.POWER section)
   o << "\n.POWER" << endl;


   //ofstream nodeFile("factors.dat", ios_base::app);

   map<string, Node *>::iterator ei = elements.begin();

   while( ei != elements.end() ) {
      
      const bool isInstanceType = ei->second->getType() == Type::instanceType;
      const bool isInputType    = ei->second->getType() == Type::inputType;
      
      if( isInstanceType || isInputType ) {
         Instance * elementInstance = ( ( Instance * )ei->second );
         for( unsigned k=0; k <elementInstance->getNumOutputs(); k++ ) {

            Node * output = elementInstance->getOutputNode( k );

            string outputNameLowerCase = convertToLower( output->getName() );
            if( outputNameLowerCase != output->getName() ) {
               // conversion performed

               double lowToHigh = internals[outputNameLowerCase]->getLowToHighTransitionFactor();
               double highToLow = internals[outputNameLowerCase]->getHighToLowTransitionFactor();

               output->setLowToHighTransitionFactor( lowToHigh );
               output->setHighToLowTransitionFactor( highToLow );
            }

            double combinedTransitionFactor =
               //  internals[outputName]->getLowToHighTransitionFactor() +
               //internals[outputName]->getHighToLowTransitionFactor();
               output->getLowToHighTransitionFactor() +
               output->getHighToLowTransitionFactor();


            o << output->getName() << ": "
              << combinedTransitionFactor
              // new format as of 5/10/04
              //	    << ", "
              //	    << elementInstance->getOutputParasiticCap()
              << " ;" << endl;


            /*	  if (output->getName() == "Xor_b[5]")
              {
                cout << output->getName() << ": " << combinedTransitionFactor <<
            		endl;
                cout << output->getLowToHighTransitionFactor() << " "
            		   << output->getHighToLowTransitionFactor() << endl;
              }
            */


            //	  nodeFile << combinedTransitionFactor << " ";
         }
      }
      ei++;
   }

   //  nodeFile << endl;
   //nodeFile.close();
   o << ".ENDS" << endl;

}


void CircuitParser::generateLeakageExpressionsInDioFile( ostream &o ) {

   map<string, Subcircuit *>::iterator it;
   //  = subcircuits.begin();
   string LeakCurr;
   
   for( it = subcircuits.begin(); it != subcircuits.end(); it++ ) {
      
      Subcircuit * currentSub = it->second;
      
      for( unsigned k = 0; k < currentSub->getNumOutputs(); k ++ ) {
         LeakCurr = currentSub->getLeakCurrExpr( pLeakNom,constants,k );
         if( LeakCurr != "" ) {
            o << currentSub->getName() << ".LEAK_POWER_P_NOM.";
            o << currentSub->getOutputNode( k )->getName();
            o << " : " << LeakCurr << " ;" << endl;
         }
         LeakCurr = currentSub->getLeakCurrExpr( pLeakStat,constants,k );
         if( LeakCurr != "" ) {
            o << currentSub->getName() << ".LEAK_POWER_P_STAT.";
            o << currentSub->getOutputNode( k )->getName();
            o << " : " << LeakCurr << " ;" << endl;;
         }
         LeakCurr = currentSub->getLeakCurrExpr( nLeakNom,constants,k );
         if( LeakCurr != "" ) {
            o << currentSub->getName() << ".LEAK_POWER_N_NOM.";
            o << currentSub->getOutputNode( k )->getName();
            o << " : " << LeakCurr << " ;" << endl;
         }
         LeakCurr = currentSub->getLeakCurrExpr( nLeakStat,constants,k );
         if( LeakCurr != "" ) {
            o << currentSub->getName() << ".LEAK_POWER_N_STAT.";
            o << currentSub->getOutputNode( k )->getName();
            o << " : " << LeakCurr << " ;" << endl;
         }
      }
      o << endl << endl;
   }
}

void CircuitParser::generatePowerExpressionsInDioFile( ostream &o ) {

   map<string, Subcircuit *>::iterator it;

   for( it = subcircuits.begin(); it != subcircuits.end(); it++ ) {

      Subcircuit * currentSub = it->second;

      //			for(unsigned no=0; no < currentSub->getNumOutputs(); no ++)
      //			{
      //					 string combinedCap = "";
      //					 combinedCap += currentSub->getOutputNode(no)->getAvgCapExpr();
      //					 o << currentSub->getName() << ".CPAR_POWER." << currentSub->getOutputNode(no)->getName() << " : " << combinedCap
      //	<< " ;" << endl << endl;
      //			}
      for( unsigned nint=0; nint < currentSub->getNumInternals(); nint ++ ) {
         
         string combinedCap = currentSub->getInternalNode( nint )->getAvgCapExpr();
         string wireCap = currentSub->getInternalNode( nint )->getWireCapExpr();
         if( combinedCap != "" ) {
            o << currentSub->getName() << ".CPAR_POWER." << currentSub->getInternalNode( nint )->getName() << " : " << combinedCap << " ;" << endl << endl;
         }
         if( wireCap != "" ) {
            o << currentSub->getName() << ".CWIRE_POWER." << currentSub->getInternalNode( nint )->getName() << " : " << wireCap << " ;" << endl << endl;
         }
      }
   }


   /*
     map<string, Subcircuit  *>::iterator ei = subcircuits.begin();

     while (ei != subcircuits.end())
       {
         if (ei->second->getType() == Type::instanceType)
   	{
   	  Instance * elementInstance = ((Instance *)ei->second);
   	  Node * output = elementInstance->getOutputNode();
   	  double combinedTransitionFactor =
   	    output->getLowToHighTransitionFactor() +
   	    output->getHighToLowTransitionFactor();


   	  o << output->getName() << ".CPAR_POWER : "
   	    << elementInstance->getOutputParasiticCap()
   	    << " ;" << endl << endl;


   	}
         ei++;
       }
   */

}

void CircuitParser::createTransitionTableEntries( TransitionTable * t, SymbolTable s ) {

   SymbolTable::iterator it;
   
   for( it = s.begin(); it != s.end(); it++ ) {
      t->createEntry( it->first, atoi( it->second.c_str() ) );
   }
}

void CircuitParser::processOneTransitionEntry( TransitionTable * t, SymbolTable s ) {

   SymbolTable::iterator it;

   for( it = s.begin(); it != s.end(); it++ ) {
      //cout << "key = " << i->first
      //     << "; value = " << i->second << endl;
      t->updateEntry( it->first, atoi( it->second.c_str() ) );
   }
}

void CircuitParser::printConstants() {
   
   SymbolTable::iterator it;
   
   for( it = constants.begin(); it != constants.end(); it++ ) {
      cout << "key = "     << it->first;
      cout << "; value = " << it->second << endl;
   }
}

/*
void cmp_nocase(const string& s, const string& s2)
{
  string::const_iterator p = s.begin();
  string::const_iterator p2 = s2.begin();

  while (p != s.end() && p2 != s2.end())
    {
      if (toupper(*p) != toupper(*p2)) return (toupper(*p) < toupper(*p2))
	? -1 : 1;
      ++p;
      ++p2;
    }

  return (s2.size() == s.size()) ? 0 : (s.size() < s2.size()) ? -1 : 1;
}
*/

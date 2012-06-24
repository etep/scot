%{

// include files
#include <iostream>

#include "../src/parser.h"
#include "../src/utility.hpp"
#include "../src/Internal.hpp"
#include "../src/Transistor.hpp"
#include "../src/CircuitParser.hpp"
#include "../src/Vdd.hpp"
#include "../src/Gnd.hpp"
#include "../src/Capacitor.hpp"

using namespace std;

// function declarations
 void yyerror(const char * msg);
 Node * LookupInInternalMap(map<string, Node *> * map, string key);
 vector<Node *> TranslateNodeNamesToNodes(vector<string> nodeNames);

 Transistor * CreateNewTransistor(string name, string drainName,
				  string gateName, string sourceName,
				  string bodyName, 
				  string type, SymbolTable params);


 Transistor * CreateNewTransistor(string name, 
					 string name1, string name2, 
				   SymbolTable params);
 
 
 Subcircuit * CreateNewSubcircuit(string name, vector<string> outports,
					vector<string> inports,
				  SymbolTable params, 
				  vector<Transistor *> transistors,
				  map<string, Node *> ints);
 

 Instance * CreateNewInstance(string instanceName, 
			      string typeName, vector<string> outPorts,
			      vector<string> inPorts);
				
 void InitParser();
   

 //extern SymbolTable constants;
 extern CircuitParser * parser;

// parsing specific global variables
 map<string, Node *> * subcircuitNodes;

 map<string, Subcircuit *> subcircuitTypes;

%}

%union
{
  string * identifier;
  
  // include all types in parser.h
  vector<string> * strVector;
  vector<pair<string,double> > * strPairVector;
  pair<string,double> * pairStrDouble;
  pair<string, string> * pair;
  map<string, string, less<string> > * symTable;

  vector<Transistor *> * transistorVector;
  Transistor * transistor;

  map<string, Subcircuit *> * subcircuitMap;
  Subcircuit * subcircuit;

  map<string, Node *> * instanceMap;
  Node * instance;

  vector<SymbolTable> * symTableVector;
}

%token T_EQUAL
%token T_SEMICOLON
%token T_COLON
%token T_COMMA
%token T_VERTICAL_BAR
%token T_TIME
%token T_TIME_EXPR

%token <identifier> T_CONST_NAME
%token <identifier> T_CONST_EXPR

%token T_SUB_CIRCUIT
%token <identifier> T_IDENTIFIER
%token <identifier> T_DOUBLE_PAIR
%token <identifier> T_DEVICE_NAME
%token <identifier> T_MOS_TYPE

%token <identifier> T_INSTANCE_NAME
%token <identifier> T_CAP_NAME
%token ANTICORR INTPRECHARGE



%type <symTable> ConstExprList;
%type <pair> ConstExpr;

%type <strVector> PortList;
%type <symTable> ParameterList;
%type <pair> Parameter;


%type <transistorVector> TransistorList;
%type <transistor> Transistor;

%type <subcircuitMap> SubCircuitList;
%type <subcircuit> SubCircuit;

%type <instanceMap> InstanceList;
%type <instance> Instance;

%type <strPairVector> PrimaryInputsList;
%type <pairStrDouble> PrimaryInput;

//%type <strPairVector> PrimaryOutputsList;
//%type <pairStrDouble> PrimaryOutput;

//%type <void> GlobalConstraintsList;
//%type <void> GlobalConstraint;
//%type <void> IdentifierList;



%type <symTable> IrsimExpr;
%type <symTableVector> IrsimExprList;

%%

S : DioFileGrammar 
  | SpiceFileGrammar
  | IrsimGrammar
  ;

IrsimGrammar : IrsimExprList
               {
	   cout << "Parsing the IRSIM log file" << endl;
		 @1;
		 parser->setIrsimData(*$1);
	       }
             ;

IrsimExprList : IrsimExprList IrsimExpr
{
  if ($2 != NULL)
    ($$ = $1)->push_back(*$2);
}
                | IrsimExpr
{
  $$ = new vector<SymbolTable>();
  if ($1 != NULL)
    $$->push_back(*$1);
}
;

IrsimExpr :  T_VERTICAL_BAR ParameterList
             {
	       //$$ = new SymbolTable();
	       $$ = $2;	       
	     } 
          | T_VERTICAL_BAR T_TIME T_EQUAL T_TIME_EXPR
            { 
	      // do nothing 
	      $$ = NULL;
	    }
          ;


/* --------------------------------------------------------------- */

DioFileGrammar : ConstExprList
                 {
                      cout << "Parsing the dio file" << endl;
											@1;
											//constants.insert($1->begin(), $1->end());
			                parser->setConstants(*$1);
		 }
               ;


ConstExprList : ConstExprList ConstExpr { ( $$ = $1 )->insert( *$2 ); }
            | ConstExpr 
              {
									 $$ = new SymbolTable();		
									 $$->insert( *$1 );
							}
            ;


ConstExpr : T_CONST_NAME T_EQUAL T_CONST_EXPR T_SEMICOLON
          { 
							 $$ = new StringPair(*$1, *$3); 
					}
          |
					 T_CONST_NAME T_EQUAL T_CONST_NAME T_SEMICOLON
          { 
							 $$ = new StringPair(*$1, *$3); 
					}
	        ;

/* --------------------------------------------------------------- */

SpiceFileGrammar : SubCircuitList  
    {
      cout << "Parsing the spice file" << endl;
      @1; // needed to generate yylloc variable
      parser->setSubcircuits(*$1);
//      parser->setElements(*$2);
//      cout << "num instances = " << $2->size() << endl;
//      parser->setPrimaryInputs(*$3);
//      cout << "Spice file parsing done" << endl;
    }
InstanceList PrimaryInputsList
{
		 parser->setElements(*$3);
     cout << "num instances = " << $3->size() << endl;
		 parser->setPrimaryInputs(*$4);
     cout << "Spice file parsing done" << endl;
		 //since the subcircuits have been created, one can record the anticorrelated inputs 
		 //if any, here. 
}
;

PrimaryInputsList : PrimaryInputsList PrimaryInput
{
		 if($2->first != "NotAnInput") //this is if the statment to be recorded is actually a global AntiCorr 
					// constraint statement, in which case there is no Primary Input pointer formed.
         ( $$ = $1 )->push_back(*$2);
		 else //see that the space given to the false input is deleted.
				 delete $2;
}
| PrimaryInput
{
  $$ = new vector<pair<string, double> >();
  $$->push_back(*$1);
}
;

PrimaryInput : ParameterList T_SEMICOLON
{
	if($1->find("af") == $1->end())
	{
			 $$ = new pair<string, double>($1->find("name")->second,-1);
	}
	else
	{
			$$ = new pair<string, double>($1->find("name")->second,atof(($1->find("af")->second).c_str())); 
	}
  //cout << "value = " << *$$ << endl;
}
|
//Using the same construct for ANTICORR inputs. This is a hack and needs to be properly done at 
//some point. Right now we do not look at PrimaryOutputs or any of the power, duty or glbcnstr 
//sections, except for this AntiCorr statement in the glbcnstr section. Hence it is clubbed with 
ANTICORR T_IDENTIFIER T_IDENTIFIER T_IDENTIFIER T_SEMICOLON
{
		 //create an impression of an input 
		 $$ = new pair<string, double>("NotAnInput",-1);
		 //Recording the anticorrelation info.
		 if(*$3 == *$4)
		 {
					lineno++;
					cout << "An input cannot be anti-correlated with itself!" << endl;
					cout << "Error at line " << lineno << endl;
					exit(-1);
		 }
		 cout << "Recording the anti correlation of inputs " << *$3 << " and " << *$4 << " of ccc " << *$2 << endl;
		 parser->setAntiCorrInfo(*$2, *$3, *$4);
		 delete $2,$3,$4;

}

|
INTPRECHARGE T_IDENTIFIER PortList T_SEMICOLON
{
		 //create an impression of an input 
		 $$ = new pair<string, double>("NotAnInput",-1);
		 //Recording the precharge ports info.
		 cout << "Recording the precharge inputs of ccc " << *$2 << endl;
		 parser->setIntPrechargeInputInfo(*$2, *$3);
		 delete $2;

}


;

 /*
PrimaryOutputsList : PrimaryOutputsList PrimaryOutput
{
}
| PrimaryOutput
{
}
;

PrimaryOutput : ParameterList T_SEMICOLON
{
}
;


GlobalConstraintsList : GlobalConstraintsList GlobalConstraint
{
}
|
GlobalConstraint
{
}
;

GlobalConstraint : IdentifierList T_SEMICOLON
{
		 //constraints that are not for modelling.
}
|
ANTICORR T_IDENTIFIER T_IDENTIFIER T_IDENTIFIER T_SEMICOLON
{
		 //this is the one we are looking for.
		 cout << "Reached here no problem" << endl;
}
;

IdentifierList : IdentifierList T_IDENTIFIER
{
}
|
T_IDENTIFIER
{
}
;
*/


InstanceList : InstanceList Instance
{
  ( $$ = $1 )->insert(pair<string, Node *>
		      ($2->getName(), $2));
}
| Instance
{
  $$ = new map<string, Node *>();
  $$->insert(pair<string, Node *>
	     ($1->getName(), $1));
}
;

// ParameterList is optional
Instance :T_IDENTIFIER PortList T_COLON PortList T_COLON T_IDENTIFIER T_COLON T_IDENTIFIER ParameterList T_SEMICOLON
{
  $$ = CreateNewInstance(*$1, *$6, *$2, *$4);
}
| T_IDENTIFIER PortList T_COLON PortList T_COLON T_IDENTIFIER T_COLON T_IDENTIFIER T_SEMICOLON
{
  $$ = CreateNewInstance(*$1, *$6, *$2, *$4);
}
| T_IDENTIFIER PortList T_COLON PortList T_COLON T_IDENTIFIER ParameterList T_SEMICOLON
{
  $$ = CreateNewInstance(*$1, *$6, *$2, *$4);
}
| T_IDENTIFIER PortList T_COLON PortList T_COLON T_IDENTIFIER T_SEMICOLON
{
  $$ = CreateNewInstance(*$1, *$6, *$2, *$4);
}
| T_IDENTIFIER T_COLON PortList ParameterList T_SEMICOLON
{
	//remember that a voltage source will be treated as a cap.
	string capName = *$1;
//	cout << "The name of the cap is " << *$1 << endl;
		
  vector<Node *> ports = TranslateNodeNamesToNodes(*$3);

  $$ = new Capacitor(capName, ports, *$4);
} 
;

SubCircuitList : SubCircuitList SubCircuit 
                 { 
											($$ = $1)->insert(pair<string, Subcircuit *> 
																				($2->getName(), $2));
											subcircuitNodes = new map<string, Node *>();
	
                 }
               | SubCircuit 
                 { 
											$$ = new map<string, Subcircuit *>();
											$$->insert(pair<string, Subcircuit *>
																($1->getName(), $1));
											subcircuitNodes = new map<string, Node *>();

                 }
;

// ParameterList is optional
SubCircuit : T_SUB_CIRCUIT T_IDENTIFIER PortList T_COLON PortList ParameterList T_SEMICOLON TransistorList
{ 
  // might add subcircuitNodes later
  $$ = CreateNewSubcircuit(*$2, *$3, *$5, *$6, *$8, *subcircuitNodes); 
}
| T_SUB_CIRCUIT T_IDENTIFIER PortList T_COLON PortList T_SEMICOLON TransistorList
{ 
  // might add subcircuitNodes later
  SymbolTable dummy;
  $$ = CreateNewSubcircuit(*$2, *$3, *$5, dummy, *$7, *subcircuitNodes); 
}







;


/* Transistor list consists of tansistors, but they can also contain parasitic
 * capacitances as well. All parasitic caps are modelled as transistors with drain
 * having the main cap and everything else being grounded. Since InternalCap is 
 * derived from the main class
 */
TransistorList : TransistorList Transistor { ($$ = $1 )->push_back( $2 ); }
               | Transistor 
                 {
											$$ = new vector<Transistor *>();
											$$->push_back( $1 );
                 }
               ;


/* M_0 egress in vdd vdd pmos W=WP L=6 GEO=0  */
Transistor : 
T_DEVICE_NAME T_IDENTIFIER T_IDENTIFIER T_IDENTIFIER T_IDENTIFIER
T_MOS_TYPE ParameterList T_SEMICOLON
{ 
  if (subcircuitNodes == NULL)
    subcircuitNodes = new map<string, Node *>();
  
  $$ = CreateNewTransistor(*$1, *$2, *$3, *$4, *$5, *$6, *$7);
  
}
| T_DEVICE_NAME T_IDENTIFIER T_IDENTIFIER ParameterList T_SEMICOLON
{
	if (subcircuitNodes == NULL)
    subcircuitNodes = new map<string, Node *>();
  
  $$ = CreateNewTransistor(*$1, *$2, *$3, *$4);
}
| T_DEVICE_NAME T_COLON T_IDENTIFIER T_IDENTIFIER ParameterList T_SEMICOLON
{
	if (subcircuitNodes == NULL)
    subcircuitNodes = new map<string, Node *>();
  
  $$ = CreateNewTransistor(*$1, *$3, *$4, *$5);
}
;




PortList : PortList T_IDENTIFIER { ( $$ = $1 )->push_back( *$2 ); }
         | T_IDENTIFIER
           {
	              $$ = new vector<string>();
	              $$->push_back( *$1 );
           }
         ;

ParameterList : ParameterList Parameter 
								{ ( $$ = $1 )->insert( *$2 );
								}
								
              | Parameter
                {
		  					$$ = new SymbolTable();
		  					$$->insert( *$1 );
//										 std::cout << $1->second << "::::" << std::endl;
                }
              ;

Parameter : T_IDENTIFIER T_EQUAL T_IDENTIFIER
           { $$ = new StringPair(*$1, *$3); }
					 |
					 T_IDENTIFIER T_EQUAL T_DOUBLE_PAIR
           { $$ = new StringPair(*$1, *$3);}
          ;
        


%%

void yyerror(const char * msg) {  
		 cout << "Syntax error at line " <<  lineno << " before " << yytext << endl;
		 yyltype * loc = &yylloc; 
     exit(-1);
}


Instance * CreateNewInstance(string instanceName, 
			     string typeName, vector<string> outPorts,
			     vector<string> inPorts)
{

  //cout << "type name = " << typeName << endl;
  //Subcircuit * type = parser->getSubcircuit(typeName);
  Subcircuit * type = subcircuitTypes.find(typeName)->second;

  vector<Node *> inportNodes = TranslateNodeNamesToNodes(inPorts);
  vector<Node *> outportNodes = TranslateNodeNamesToNodes(outPorts);
	vector<Node *> intCapNetNodes = parser->getCapNetNodes(typeName);
  vector<Node *>::iterator iNode =intCapNetNodes.begin();
	
	// Record the internal cap nets for activity factor registration.
	while(iNode != intCapNetNodes.end())
	{
			 string netName = instanceName + "." + (*iNode)->getName();
			 parser->insertInternal(netName,*iNode);
			 iNode ++;
	}

  // insert ports into parser
  
  return new Instance(instanceName, type, outportNodes, inportNodes);
  
}


/*---------------------------------------------------------*/
Subcircuit * CreateNewSubcircuit(string name, vector<string> outports,
				 vector<string> inports,
				 SymbolTable params, 
				 vector<Transistor *> transistors,
				 map<string, Node *> internalTable)
{
  vector<string>::iterator iIn = inports.begin();
  vector<string>::iterator iOut = outports.begin();
  vector<Transistor *>::iterator iTran = transistors.begin();
  vector<Node *> foundPorts;
  vector<Node *> internalNodes;
  vector<Node *> outputPorts;

  while (iOut != outports.end())
    {
				 Node * outputPort = LookupInInternalMap(subcircuitNodes, *iOut);
				 outputPorts.push_back(outputPort);
				 iOut++;
		}
  
  while (iIn != inports.end())
    {
      Node * intNode = LookupInInternalMap(subcircuitNodes, *iIn);
      Input * inputNode = new Input(intNode,outports.size());
      foundPorts.push_back(inputNode);
      iIn++;
    }
	
  while (iTran != transistors.end())
    {
				 Transistor * t = *iTran;
				 if(t->getType()->equals(Type::intCapType))
				 {
//							if(outports.find(t->getConnectionAt(0)->getName()) == outports.end())
//							{		 
//									 string intName = t->getConnectionAt(0)->getName();
									 parser->insertCapNetNode(name, t->getConnectionAt(0));
//							}
				 }
				 iTran++;
		}

  map<string, Node *>::iterator si = internalTable.begin();

  while ( si != internalTable.end() )
    {
      Node * n = (*si).second;
      internalNodes.push_back(n);
      si++;
    }
  

  Subcircuit * newSubcircuit = new Subcircuit(name, outputPorts, foundPorts, params, transistors, 
					      internalNodes);

  subcircuitTypes.insert(pair<string, Subcircuit *>(name, newSubcircuit));
  return newSubcircuit;
}
/*-----------------------------------------------------------*/



Transistor * CreateNewTransistor(string name, string drainName,
				 string gateName, string sourceName,
				 string bodyName,
				 string type, SymbolTable params)
{
  Node * drainNode = LookupInInternalMap(subcircuitNodes, drainName);
  Node * gateNode = LookupInInternalMap(subcircuitNodes, gateName);
  Node * sourceNode = LookupInInternalMap(subcircuitNodes, sourceName);
  Node * bodyNode = LookupInInternalMap(subcircuitNodes, bodyName);

  Type * tp = ( type == "nmos" || type == "nch" || type == "NMOS") ?
    Type::nmosType : Type::pmosType;

  Transistor * t = 
    new Transistor(name, tp, drainNode, sourceNode, gateNode, bodyNode,  params);

  drainNode->addConnection(t);
  sourceNode->addConnection(t);
  gateNode->addConnection(t);

  return t;
  
}

Transistor * CreateNewTransistor(string name, 
				 string name1, string name2,
	       SymbolTable params)
{
	string capname;
	string vddgnd;
	if(name1 == "Gnd" || name1 == "gnd" || name1 == "Vdd" || name1 == "vdd" ) 
	{
			 capname = name2; 
			 vddgnd = name1;
	}
	else if(name2 == "Gnd" || name2 == "gnd" || name2 == "Vdd" || name2 == "vdd" )
	{
			 capname = name1;
			 vddgnd = name2;
	}
	else
	{
			 yyerror("The capacitance net name is not a standard one");
	}
  Node * drainNode = LookupInInternalMap(subcircuitNodes, capname);
  Node * gateNode = LookupInInternalMap(subcircuitNodes, vddgnd);
  Node * sourceNode = LookupInInternalMap(subcircuitNodes, vddgnd);
  Node * bodyNode = LookupInInternalMap(subcircuitNodes, vddgnd);

  Type * tp = Type::intCapType;

  Transistor * t = 
    new Transistor(name, tp, drainNode, sourceNode, gateNode, bodyNode,  params);

  drainNode->addConnection(t);

  return t;
  
}




vector<Node *> TranslateNodeNamesToNodes(vector<string> nodeNames)
{
  vector<string>::iterator i = nodeNames.begin();
  vector<Node *> portNodes;

  while ( i != nodeNames.end())
  {
      Node * n = parser->getInternal(*i);
      if (n == NULL)
	    {
					 if (strcasecmp(i->c_str(), "gnd") == 0)
								n = new Gnd();
					 else if (strcasecmp(i->c_str(), "vdd") == 0)
								n = new Vdd();
					 else 
								n = new Internal(*i);
					 
					 parser->insertInternal(*i, n);
			}
      portNodes.push_back(n);
      i++;
	}
  return portNodes;
}


Node * LookupInInternalMap(map<string, Node *> * map, string key)
{
  if ( map->find( key ) == map->end() )
    {
      pair<string, Node *>newPair;
      newPair.first = key;
      
      if (strcasecmp(key.c_str(), "vdd") == 0)
	newPair.second = new Vdd();
      else if (strcasecmp(key.c_str(), "gnd") == 0)
	newPair.second = new Gnd();
      else newPair.second = new Internal( key );
      map->insert(newPair);
    }

  return map->find( key )->second;
}

void InitParser()
{
  yydebug = false;
}

/*
      map<string, string>::iterator i = constants.begin();

      while (i != constants.end())
	{
	  cout << "field = " << i->first 
	       << " value = " << i->second << endl;
	  i++;
	}
*/

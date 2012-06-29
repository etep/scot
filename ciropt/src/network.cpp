#include "randomc.hpp"

#include <map>
#include <cstring>
#include <iostream>

#include <math.h>
#include <string.h>
#include <stdlib.h>

#include "opt.hpp"
#include "cnvt.hpp"
#include "network.hpp"

using namespace std;

#define EXPT true
//extern unsigned ciropt_line_number;
//extern unsigned dio_line_number;
int ciropterror( const string & );
int dioerror( const string & );
int errorReport( const string & );

extern string ERROR_STRING;
extern string EMPTY_STRING;
extern string KEEPER_INPUT;
extern double MAX_PATH_LENGTH;

/*
 * CLASS network MEMBER FUNCTION DEFINITIONS
 */

network::~network() {
   
   for( map<string, ccc * >::const_iterator it = cccMap.begin();  it != cccMap.end();  it++ ) {
      delete it->second;
   }
   for( map<string, node *>::const_iterator it = nodeMap.begin(); it != nodeMap.end(); it++ ) {
      delete it->second;
   }
   for( map<string, edge *>::const_iterator it = edgeMap.begin(); it != edgeMap.end(); it++ ) {
      delete it->second;
   }
}

bool network::isGNDorVDD( const string & name ) {
   return ( name == "GND"
            || name == "gnd"
            || name == "VDD"
            || name == "vdd"
            || name == "Vdd"
            || name == "Gnd"
            || name == "VSS"
            || name == "Vss" );

}

//Insert the map of correlated MOS in a ccc in a map of CCCs
//containing the correlated

void network::putCorrMos( const string & ccc_name, map<string, string> & corr_mos ) {
   if( corr_mos_info.find( ccc_name ) != corr_mos_info.end() ) {
      ciropterror( "multiple definition of cccs with name \""+ccc_name+"\"" );
      return;
   }
   corr_mos_info.insert( make_pair( ccc_name,corr_mos ) );
   return;
}

//From the MOS list, find the nets that are correlated and
//make a table of them.
void network::MakeCorrelatedEdgeTable() {
   map<string, node *>::const_iterator nitr;
   string ccName, in_name1, in_name2;
   int in_num1, in_num2;
   bool isPMOS1, isPMOS2;
   for( nitr = nodeMap.begin(); nitr != nodeMap.end(); nitr ++ ) {
      if( nitr->second->isCapacitor() || nitr->second->isVRL() )
         continue;
      ccName = nitr->second->getCCC().getName();
      if( corr_mos_info.find( ccName ) != corr_mos_info.end() ) {
         map<string, string>::const_iterator mitr;
         map<string, string> & tmp = corr_mos_info[ccName];
         for( mitr = tmp.begin(); mitr !=tmp.end(); mitr++ ) {
            in_num1 =   nitr->second->getCCC().get_gate_num( mitr->first );
            nitr->second->putCorrInputnum( in_num1 );
            in_num2 =   nitr->second->getCCC().get_gate_num( mitr->second );
            nitr->second->putCorrInputnum( in_num2 );
            in_name1 =  nitr->second->getInEdge( in_num1 ).getName();
            in_name2 =  nitr->second->getInEdge( in_num2 ).getName();
            isPMOS1 =   nitr->second->getCCC().isPMOS( mitr->first );
            isPMOS2 =   nitr->second->getCCC().isPMOS( mitr->second );
            putCorrNets( in_name1,isPMOS1,in_name2,isPMOS2 );
            putCorrNets( in_name2,isPMOS2,in_name1,isPMOS1 );

            //cout << "the mosses names are " << in_name1 << "   " <<  in_name2 << endl;
         }
      }
   }
   cout<< "Made the Correlated nets table " <<endl;
   // The inclusion of the correlated nets is correct ...so below is commented.
   //map<string, edge *>::const_iterator eitr;
   //for( eitr = edgeMap.begin(); eitr != edgeMap.end(); eitr ++ )
   //   eitr->second->print_corr_edges();
}


void network::putVRLC_info( const string name,const string net1, const string net2, string type, gposy & val,symbol_table & st ) {
   if( VRLC_info.find( name ) != VRLC_info.end() )
      ciropterror( "The element " + name + " of type " + type + " is defined more than once" );
   map<string, string> tmp1;
   map<string, gposy * > tmp2;
   tmp1[net1] = net2;
   tmp2[type] = & val;
   VRLC_info[name] = tmp1;
   VRLC_type[name] = tmp2;
   VRLC_symtab[name] = st;
}

void network::MakeVRLCnodes() {
   //make the voltage nodes. Note that to make that one has to assign the input
   //and the output of the voltage source in the right fashion. Since there could
   //be more than one VRL connected in series we start
   //with a voltage source that we find has an existing edge connected to it.
   //Soon the voltage sources made will make new edges and all the things should
   //be full. This means that for voltage sources that are truly unconnected to
   //the entire netlist, no node will be formed. Also though it might happen that
   //at the end, a voltage source is connected to only one proper net and the
   //other is just dangling. But I expect that it will be captured when checking
   //the connections of all the edges.
   map<string, map<string, string> >::iterator itr;
   map<string, string> edgeInfo; // the second variable of itr
   map<string, gposy * > attr; // the second variable of gtr
   vector<string> done;
   map<string, string>::iterator sitr;
   map<string, gposy * >::iterator aitr;
   string VRLname;
   bool unconnect = false;
   bool madeNode = true;
   //first put only the VRL nodes since they are in series and need to instantiate edges
   //for caps to be put in.
   while( madeNode ) {
      madeNode = false;
      //repeatedly go thru the map and figure out
      //and at each step erase the elements whose nodes are made.
      for( itr = VRLC_info.begin(); itr != VRLC_info.end(); itr++ ) {
         VRLname = itr->first;
         edgeInfo = VRLC_info[VRLname];
         attr = VRLC_type[VRLname];
         symbol_table & st = VRLC_symtab[VRLname];

         sitr = edgeInfo.begin();
         aitr = attr.begin();
         if( edgeMap.find( sitr->first ) != edgeMap.end()
               || edgeMap.find( sitr->second ) != edgeMap.end() ) {
            if( aitr->first != "c" ) {
               //									 		 		 cout << "Instantiating the VRL node " << VRLname << endl;
               putVRLNode( VRLname, sitr->first, sitr->second, aitr->first, * aitr->second,  st );
               madeNode = true;
               done.push_back( VRLname );
            }
         }
      }
      for( unsigned i = 0; i < done.size(); i++ ) {
         VRLC_info.erase( done[i] );
         VRLC_type.erase( done[i] );
         VRLC_symtab.erase( done[i] );
      }
      done.clear();
   }
   //now put the caps.
   for( itr = VRLC_info.begin(); itr != VRLC_info.end(); itr++ ) {
      VRLname = itr->first;
      edgeInfo = VRLC_info[VRLname];
      attr = VRLC_type[VRLname];
      symbol_table & st = VRLC_symtab[VRLname];
      sitr = edgeInfo.begin();
      aitr = attr.begin();
      if( aitr->first != "c" ) {
         errorReport( "The VRL " + VRLname + " is dangling in the netlist\n" );
      }
      //						cout << "Instantiating capacitor " << VRLname << endl;
      putCapNode( VRLname, sitr->first, sitr->second, *aitr->second,  st, VRLname );
      VRLC_info.erase( VRLname );
      VRLC_type.erase( VRLname );
      VRLC_symtab.erase( VRLname );
   }
   cout << "Recorded the VRLC nodes, if any " << endl;
}

/*
 *  No need for this now since all checks are done before this and we allow for consecutive VRL nodes.
  void network::CheckVRLnodes()
  {
			 map<string, map<string, string> >::iterator itr;
			 map<string, string >::iterator ttr;
			 itr = VRLC_info.begin();
			 ttr = VRLC_type.begin();
			 map<string, string> tmp;
			 map<string, string>::iterator sitr;
			 map<string, edge *>::iterator eitr1, eitr2;
			 string VRLname;
			 while(itr != VRLC_info.end())
			 {
						VRLname = itr->first;
            tmp = itr->second;
						for(sitr = tmp.begin(); sitr != tmp.end(); sitr++)
						{
								 string n1 = sitr->first;
								 eitr1 = edgeMap.find(n1);
								 if(eitr1 == edgeMap.end())
											errorReport("The edge " +  n1 + " of VRL node " + VRLname + " is unconnected");
								 string n2 = sitr->second;
								 eitr2 = edgeMap.find(n2);
								 if(eitr2 == edgeMap.end())
											errorReport("The edge " +  n2 + " of VRL node " + VRLname + " is unconnected");
								 if(eitr1->second->getDriverNode() == (node *)NULL && eitr2->second->getDriverNode() == (node *)NULL )
											if(! (eitr1->second->isPI() or eitr2->second->isPI()))
													 errorReport("Nither of edges " + n1 + " and " + n2 + " are driven by any source or are primary input");
						}
						itr++;
			 }
			 cout << "Checked the VRL nodes, if any " << endl;
			 //checkForConsecutiveVRLnodes
			 map<string, node *>::iterator nitr;
			 for(nitr = nodeMap.begin();nitr != nodeMap.end(); nitr ++)
			 {
						if((nitr->second)->isVRL())
						{
								 if(nitr->second->getOutEdge(0).getNumFanoutNodes() == 1
													 && nitr->second->getOutEdge(0).getFanoutNode(0).isVRL())
								 {
											string n1 = nitr->first;
											string n2 = nitr->second->getOutEdge(0).getFanoutNode(0).getName();
											errorReport("Error :  VRL nodes " + n1 + " and " + n2 + " are consecutive");
								 }
						}
			 }
  }

*/


void network::CheckConnectionsOfEdges() {
   map<string, edge *>::iterator itr;
   for( itr = edgeMap.begin(); itr != edgeMap.end(); itr ++ ) {
      if( itr->second->isPI() ) continue;
      if( itr->second->getDriverNode() == ( node * ) NULL ) {
         if( itr->second->isGNDorVDD() ) {
            errorReport( "Found a Gnd or Vdd net in your circuit.\n"
                         "If you wish to have it as input, please declare it as a primary input with a large capacitance and a small risefall timing" );
         }
         errorReport( "The net " + itr->first + " is not driven, neither is a primary input" );
         exit( -1 );
      }
   }
}


void network::put( ccc & c ) {
   assert( cccMap.find( c.getName() )==cccMap.end() );
   cccMap[c.getName()] = &c;
}

void network::putNode( const  string & node_name,
                       const string & ccc_name,
                       vector<string> & out_edge_name_list,
                       vector<string> & in_edge_name_list,
                       const string & var_name ) {
   if( nodeMap.find( node_name ) != nodeMap.end() ) {
      ciropterror( "multiple definition of gates with name \""+node_name+"\"" );
      return;
   }

   // find ccc type with name {ccc_name} reference it by {cc}
   map<string, ccc *>::const_iterator cccitr;
   if( ( cccitr = cccMap.find( ccc_name ) ) == cccMap.end() ) {
      ciropterror( "ccc type " +ccc_name +" is not defined in .SUBCKT section" );
      return;
   }
   ccc & cc = * cccitr->second;

   // find an edge with {out_edge_name} or make one
   // reference it by {outEdge}
   //edge & outEdge = findOrAddEdge( out_edge_name );

   vector<edge *> inEdgeVec;
   vector<edge *> outEdgeVec;

   vector<edge *> constInEdgeVec;
   vector<edge *> constOutEdgeVec;

   for( int i = 0; i < in_edge_name_list.size(); i ++ ) {
      edge & te = findOrAddEdge( in_edge_name_list[i] );
      inEdgeVec.push_back( &te );
      constInEdgeVec.push_back( &te );
   }

   for( int i = 0; i < out_edge_name_list.size(); i ++ ) {
      edge & te = findOrAddEdge( out_edge_name_list[i] );
      outEdgeVec.push_back( &te );
      constOutEdgeVec.push_back( &te );
   }


   // create a node and store it in the map
   string n = node_name;
   node & newnode = *( new node( n, constOutEdgeVec, cc, constInEdgeVec,var_name ) );

   nodeMap[newnode.getName()] = & newnode;


   // make a link to this node at each out_edge
   for( int i = 0; i < outEdgeVec.size(); i ++ )
      //outEdge.putOutNode( newnode );
      outEdgeVec[i]->putOutNode( newnode );

   // make a link to this node at each in_edge
   for( int i = 0; i < inEdgeVec.size(); i ++ )
      inEdgeVec[i]->putInNode( newnode, i );

   return;
}

void network::putCapNode
(  const string & node_name,
   const string & e1,  const string & e2, gposy & c,
   symbol_table & st, const string & var_name ) {
   if( nodeMap.find( node_name ) != nodeMap.end() ) {
      ciropterror( "multiple definition of subcircuit \"" + node_name + "\"" );
      return;
   }

   edge * ein1;
   edge * ein2;

   if( isGNDorVDD( e1 ) && isGNDorVDD( e2 ) ) {
      errorReport( "The cap " + node_name + " is between Vdd and Gnd nodes, ignored for del/area/power\n" );
      return;
   } else if( isGNDorVDD( e1 ) ) {
      ein2 = &findOrAddEdge( e2 );
      node & newnode = *( new node( node_name, *ein2, c , st, var_name ) );
      nodeMap[newnode.getName()] = & newnode;
      ein2->putInNode( newnode, 0 );
   } else if( isGNDorVDD( e2 ) ) {
      ein1 = &findOrAddEdge( e1 );
      node & newnode = *( new node( node_name, *ein1, c , st, var_name ) );
      nodeMap[newnode.getName()] = & newnode;
      ein1->putInNode( newnode, 0 );
   } else {
      bool couple = false; // for the time being let it assume there is no coupling.
      cout << "Recording cap " << node_name << " as a potential coupling cap" << endl;
      ein1 = &findOrAddEdge( e1 );
      ein2 = &findOrAddEdge( e2 );
      node & newnode = *( new node( node_name, *ein1, *ein2, c , st, var_name, couple ) );
      nodeMap[newnode.getName()] = & newnode;
      ein1->putInNode( newnode, 0 );
      ein2->putInNode( newnode, 0 );
   }
}


void network::putVRLNode( const string & VRLname,
                          const string & e1, const string & e2,
                          const string & type, gposy & val, symbol_table & st ) {
   if( nodeMap.find( VRLname ) != nodeMap.end() ) {
      ciropterror( "multiple definition of subcircuit \"" + VRLname + "\"" );
      return;
   }
   if( isGNDorVDD( e1 ) && isGNDorVDD( e2 ) ) {
      cout << " The VRL node " << VRLname << " is shorted or connected between gnd and Vdd" << endl;
      cout << "Ignored for the delay calculation" << endl;
      return;
   } else if( isGNDorVDD( e1 ) ) {
      edge & teOut = findOrAddEdge( e2 );
      edge & teIn = findOrAddEdge( e1 );
      node & newnode = *( new node( VRLname, teIn, teOut, type, val, st ) );
      nodeMap[newnode.getName()] = & newnode;
      teIn.putInNode( newnode, 0 );
      teOut.putOutNode( newnode );
   } else if( isGNDorVDD( e2 ) ) {
      edge & teOut = findOrAddEdge( e1 );
      edge & teIn = findOrAddEdge( e2 );
      node & newnode = *( new node( VRLname, teIn, teOut, type, val, st ) );
      nodeMap[newnode.getName()] = & newnode;
      teIn.putInNode( newnode, 0 );
      teOut.putOutNode( newnode );
   } else {
      //			 cout << "came here for nodemaking of  " << VRLname << endl;
      edge * tmpe1 = edgeMap[e1];
      edge * tmpe2 = edgeMap[e2];
      if( tmpe1==( edge * )NULL  && tmpe2==( edge * )NULL )
         errorReport( "Error: The edges " + e1 + " and " + e2 + " are both unconnected." );
      edge & te1 = findOrAddEdge( e1 );
      edge & te2 = findOrAddEdge( e2 );
      if( te1.getDriverNode() == ( node * )NULL && te2.getDriverNode() == ( node * )NULL ) {
         //						if(te1->isPI() || te2->isPI())
         //								 errorReport("Voltage sources are not allowed at the input : " + VRLname);
         /* The following portion is for the future when I will find a way to
          * implement the voltage sources at the input as well.*/
         if( te1.isPI() ) {
            node & newnode = *( new node( VRLname, te1, te2, type, val, st ) );
            nodeMap[newnode.getName()] = & newnode;
            te1.putInNode( newnode, 0 );
            te2.putOutNode( newnode );
         } else if( te2.isPI() ) {
            node & newnode = *( new node( VRLname, te2, te1, type, val, st ) );
            nodeMap[newnode.getName()] = & newnode;
            te2.putInNode( newnode, 0 );
            te1.putOutNode( newnode );
         } else if( tmpe1 != ( edge * )NULL  && tmpe2==( edge * )NULL ) {
            // if one of the nets is not null, it has to be an input to some gate.
            node & newnode = *( new node( VRLname, te2, te1, type, val, st ) );
            nodeMap[newnode.getName()] = & newnode;
            te2.putInNode( newnode, 0 );
            te1.putOutNode( newnode );
         } else if( tmpe1 == ( edge * )NULL  && tmpe2!=( edge * )NULL ) {
            node & newnode = *( new node( VRLname, te1, te2, type, val, st ) );
            nodeMap[newnode.getName()] = & newnode;
            te1.putInNode( newnode, 0 );
            te2.putOutNode( newnode );
         } else {
            errorReport( "Neither of edges " + e1 + " or " + e2 + " is driven or is a PI" );
         }
      } else if( te1.getDriverNode() != ( node * )NULL && te2.getDriverNode() != ( node * )NULL )
         errorReport( "Edges " + e1 + " and " + e2 + " are both driven into the voltage source" );
      else if( te1.getDriverNode() == ( node * )NULL ) {
         node & newnode = *( new node( VRLname, te2, te1, type, val, st ) );
         nodeMap[newnode.getName()] = & newnode;
         te2.putInNode( newnode, 0 );
         te1.putOutNode( newnode );
      } else {
         node & newnode = *( new node( VRLname, te1, te2, type, val, st ) );
         nodeMap[newnode.getName()] = & newnode;
         te1.putInNode( newnode, 0 );
         te2.putOutNode( newnode );
      }
   }
}



void network::putCorrNets( const string & netname, const bool risefall,  const string & corr_net, const bool corr_risefall ) {
   map<string, edge *>::const_iterator itr;

   itr = edgeMap.find( netname );

   if( itr == edgeMap.end() ) {
      ciropterror( "no net name :" + netname + ": error while putting correlated nets: " );
      return;
   }

   if( risefall )
      itr->second->putCorrNetTfall( corr_net, corr_risefall );
   else
      itr->second->putCorrNetTrise( corr_net, corr_risefall );
}

void network::putSwitchingFactor
( const string & netname, double sf ) {
   map<string, edge *>::const_iterator itr;

   itr = edgeMap.find( netname );

   if( itr == edgeMap.end() ) {
      ciropterror( "no net name error: " + netname );
      return;
   }

   itr->second->putSwitchingFactor( sf );

   return;
}

void network::putSwitchingFactor
( const string & gatename, const string & netname, double sf ) {
   map<string, node *>::const_iterator itr;

   itr = nodeMap.find( gatename );

   if( itr == nodeMap.end() ) {
      ciropterror( "Could not find cell " + gatename + " to record AF of internal net " + netname );
      return;
   }

   itr->second->putIntNetAF( netname, sf );

   return;
}





void network::putDutyFactor
( const string & nodename, double df,const string & out ) {
   map<string, node *>::const_iterator itr;

   itr = nodeMap.find( nodename );

   if( itr == nodeMap.end() ) {
      ciropterror( "no node name error: " + nodename );
      return;
   }

   itr->second->putDutyFactor( df, out );

   return;
}


void network::setPathLengthTables() {
   MAX_PATH_LENGTH = 0.0;
   map<string, double> PLforward;
   map<string, double> PLreverse;
   map<string, edge *>::const_iterator eitr;
   map<string, node *>::const_iterator nitr;
   string in;
   string out;
   //Initialize all the tables before the looping of
   //Bellman-Ford Algorithm to find the longest paths
   //from inputs and outputs. This is done basically by finding
   //the shortest paths using negative weights.
   //Here note that the wires(called edges in ciropt) are the nodes and the
   //input output connection of the various gates (which are nodes
   //here) are the edges as far as this algorithm is concerned.
   for( eitr = edgeMap.begin(); eitr != edgeMap.end(); eitr ++ ) {
      PLforward[eitr->first] = 0;
      PLreverse[eitr->first] = 0;
   }
   //Start looping for the algo. Here we are traversing forward as
   //well as backward and the results are stored in two separate
   //maps that are initialized above. For forward the sinks of the graph
   // are the
   //primary inputs and for backward, the outputs.
   for( eitr = edgeMap.begin(); eitr != edgeMap.end(); eitr ++ ) {
      for( nitr = nodeMap.begin(); nitr != nodeMap.end(); nitr ++ ) {
         if( nitr->second->isCapacitor() || nitr->second->isVRL() )
            continue;
         for( unsigned ni =0; ni < nitr->second->getNumberOfInputs(); ni++ )
            for( unsigned no =0; no < nitr->second->getNumberOfOutputs(); no++ ) {
               //First assume that its an internal net
               // and then correct for PI and PO
               in = nitr->second->getInEdge( ni ).getName();
               out = nitr->second->getOutEdge( no ).getName();
               if( PLforward[out] > PLforward[in] - 1 )
                  PLforward[out] = PLforward[in] - 1;
               if( PLreverse[in] > PLreverse[out] - 1 )
                  PLreverse[in] = PLreverse[out] -1;
               if( nitr->second->getInEdge( ni ).isPI() )
                  PLforward[eitr->first] = 0;
               else if( nitr->second->getInEdge( ni ).isPO() )
                  PLreverse[eitr->first] = 0;
            }
      }
   }
   //insert the longest path values in the nodes.
   for( nitr = nodeMap.begin(); nitr != nodeMap.end(); nitr ++ ) {
      if( nitr->second->isCapacitor() || nitr->second->isVRL() )
         continue;
      for( unsigned ni =0; ni < nitr->second->getNumberOfInputs(); ni++ )
         for( unsigned no =0; no < nitr->second->getNumberOfOutputs(); no++ ) {
            in = nitr->second->getInEdge( ni ).getName();
            out = nitr->second->getOutEdge( no ).getName();
            double pl  = -1*( PLforward[in] + PLreverse[out] ) + 1;
            if( MAX_PATH_LENGTH < pl )
               MAX_PATH_LENGTH = pl;
            nitr->second->put_path_length( ni,no,pl );
            //												 cout << nitr->first << " " << ni << " " << no << " " << pl << endl;
         }
   }
}


ccc & network::findCCC( const string & ccc_name ) {
   map<string, ccc *>::const_iterator
   itr = cccMap.find( ccc_name );
   assert( itr!=cccMap.end() );

   return *( itr->second );
}


edge & network::findEdge( const string & edgeName ) {
   map<string, edge *>::const_iterator
   itr = edgeMap.find( edgeName );

   if( itr==edgeMap.end() )
      errorReport( "The edge name " + edgeName + " is not valid" );

   return *( itr->second );
}

edge & network::findOrAddEdge( const string & edgeName ) {
   edge * te = edgeMap[edgeName];
   if( te==( edge * )NULL ) te = edgeMap[edgeName] = new edge( edgeName );

   return *te;
}

void network::print() {
   map<string,ccc *>::const_iterator itr1 = cccMap.begin();
   map<string,node *>::const_iterator itr2 = nodeMap.begin();
   map<string,edge *>::const_iterator itr3 = edgeMap.begin();

   for( ; itr1 != cccMap.end();  itr1 ++ ) itr1->second->print();
   for( ; itr2 != nodeMap.end(); itr2 ++ ) itr2->second->print();
   for( ; itr3 != edgeMap.end(); itr3 ++ ) itr3->second->print();
}

void network::montecarlo
( map<string, ProbDist *> & mc,
  map<string, vector<unsigned> > & netSelectVecMap,
  const monte_carlo & monte, double p,
  map<string,double> & optVs, bool noTriseTfall ) {
   const string & dist = monte.getDistribution();
   const size_t N = monte.getN();

   if( monte.isIndependent() ) {
      cout << "Independent distributions" << endl;
      montecarlo( mc, netSelectVecMap, dist, N, p, optVs, noTriseTfall );
   } else {
      const string depType = monte.getDependencyType();
      if( depType == "type1" ) {
         cout << "Dependent distribution type1" << endl;
         // assign prob. dist. for each gate
         map<string,ProbDist *> gatePDMap;

         map<string, node *>::const_iterator nitr;
         for( nitr = nodeMap.begin(); nitr != nodeMap.end(); nitr ++ )
            gatePDMap[nitr->first] = new ProbDist( dist, N, 0.0, 1.0 );

         // do montecarlo
         montecarlo( mc, netSelectVecMap, gatePDMap, monte, p, optVs, noTriseTfall );

         map<string,ProbDist *>::const_iterator gitr;
         for( gitr = gatePDMap.begin(); gitr != gatePDMap.end(); gitr ++ )
            delete gitr->second;
      } else
         errorReport( "The distribution dependence type is not supported" );
   }

   return;
}

void network::montecarlo
( map<string, ProbDist *> & mc,
  map<string, vector<unsigned> > & netSelectVecMap,
  const string & dist, unsigned N, double p,
  map<string,double> & optVs, bool noTriseTfall ) {
   assert( mc.size() == ( map<string,ProbDist*>::size_type )0 );
   map<string, edge *>::const_iterator itr;

   for( itr = edgeMap.begin(); itr != edgeMap.end(); itr ++ ) {
      itr->second->resetNumOfMCVisits();
      itr->second->montecarlo( mc, netSelectVecMap, dist, N, p, optVs,noTriseTfall );
   }

   return;
}

void network::montecarlo
( map<string,ProbDist *> & mc,
  map<string,vector<unsigned> > & netSelectMap,
  map<string,ProbDist *> & gatePDMap,
  const monte_carlo & monte, double p,
  map<string,double> & optVs, bool noTriseTfall ) {
   assert( mc.size() == ( map<string,ProbDist *>::size_type )0 );
   map<string, edge *>::const_iterator itr;

   for( itr = edgeMap.begin(); itr != edgeMap.end(); itr ++ )
      itr->second->montecarlo( mc, netSelectMap, gatePDMap, monte, p, optVs,noTriseTfall );

   return;
}


void network::nominalAnalysis( map<string,double> & nomAnlys, map<string,unsigned> & netSelectVec, map<string,vector<double> > & pathLengthMap, map<string,vector<double> > & pathVarianceMap, map<string,double > & gateDioMap, map<string,double> & optVs, const string cccName, bool noTriseTfall ) {

   assert( nomAnlys.size() == ( map<string,double>::size_type ) 0 );
   assert( netSelectVec.size() == 0 );
   assert( pathLengthMap.size() == 0 );
   map<string, node *>::const_iterator nitr;
   
   if( cccName != "" ) {
      if( ( nitr=nodeMap.find( cccName ) ) == nodeMap.end() ) {
         errorReport( "Error: The specified gate for Dio analysis" + cccName + " is not present in the netlist" );
      }
      if( nitr->second->isCapacitor() ) {
         errorReport( "Error: The gate requested for Dio analysis" + cccName + " is a capacitor" );
      }
      nitr->second->dioAnalysis( gateDioMap, optVs,noTriseTfall );
      cout << "Dio Analysis on " << nitr->second->getName() <<  " finished" <<endl;
      return;
   }

   map<string,edge *>::const_iterator itr;

   for( itr = edgeMap.begin(); itr != edgeMap.end(); itr ++ ) {
      cout << "Considering edge " << itr->second->getName() << endl;
      itr->second->nominalAnalysis( nomAnlys, netSelectVec, pathLengthMap, pathVarianceMap,gateDioMap, optVs,noTriseTfall );
   }
   /*
   for( unsigned i = 0; i < poEdgeVec.size(); i ++ )
   {
     vector<double> rPLs, fPLs;
     poEdgeVec[i]->nominalAnalysis( nomAnlys, netSelectVec, rPLs, fPLs, optVs );

     pathLengths.insert<vector<double>::const_iterator>
       (pathLengths.end(),rPLs.begin(),rPLs.end());

     pathLengths.insert<vector<double>::const_iterator>
       (pathLengths.end(),fPLs.begin(),fPLs.end());

     rPLs.clear(); fPLs.clear();
   }
   */
   cout << "Nominal Analysis finished" <<endl;
}

void network::setKappaBetaForAllNodes( double kappa, double beta ) {
   map<string, edge *>::const_iterator itr;

   for( itr = edgeMap.begin(); itr != edgeMap.end(); itr ++ ) {
      if( !itr->second->isPI() )
         ( itr->second->getDriverNode() )->setKappaBeta( kappa, beta );
   }

   return;
}

/*
 * CLASS ccc MEMBER FUNCTION DEFINITIONS
 */

string ccc::FCLOAD_NAME = "_LOAD_F_";
string ccc::RCLOAD_NAME = "_LOAD_R_";

ccc::ccc
( const string & nm, vector<string> & outputnames,
  vector<string> & inputnames )
   : name( nm ), outputNameVec( outputnames ) , inputNameVec( inputnames ) {
   numInputs = inputNameVec.size();
   numOutputs = outputNameVec.size();
   assert( numInputs > 0 );
   assert( numOutputs > 0 );

   dioVec.clear();
   dioVec.resize( numInputs );
   stdVec.clear();
   stdVec.resize( numInputs );
   wnVec.clear();
   wnVec.resize( numInputs );
   wpVec.clear();
   wpVec.resize( numInputs );

   for( unsigned i = 0; i < dioVec.size(); i ++ ) {
      for( unsigned j = 0; j < numOutputs ; j ++ ) {
         dioVec[i][outputNameVec[j]] = ( dio * )NULL;
      }
   }

   for( unsigned i = 0; i < stdVec.size(); i ++ ) {
      for( unsigned j = 0; j < numOutputs ; j ++ ) {
         stdVec[i][outputNameVec[j]] = ( dio * )NULL;
      }
   }
   for( unsigned i = 0; i < numOutputs ; i ++ ) {
      parCap[outputNameVec[i]] = ( gposy * )NULL;
      leakPowPmean[outputNameVec[i]] = ( gposy * )NULL;
      leakPowNmean[outputNameVec[i]] = ( gposy * )NULL;
      leakPowPstat[outputNameVec[i]]= ( gposy * )NULL;
      leakPowNstat[outputNameVec[i]] = ( gposy * )NULL;
   }

   return;
}

void ccc::putMosList( vector<mos> & mos_list ) {
   devices = mos_list;
   for( unsigned i = 0; i < devices.size(); i ++ ) {
      mos & ms = devices[i];
      //		cout << getName() << " : this is the width for " << ms.getMosName() << " ";
      //		ms.getWidthName()->print(symtab);
      //		cout << endl;
      if( ! ms.isCAP() ) {
         assignWidthToInput( ms );
      }
   }

   checkWidthAssignment();
}

void ccc::putInternalNets() {
   for( unsigned i = 0; i < devices.size(); i ++ ) {
      mos & ms = devices[i];
      if( ! ms.isCAP() ) {
         if( parCap.find( ms.getDrainName() ) == parCap.end()
               && ! ms.isGNDorVDD( ms.getDrainName() ) ) {
            parCap[ms.getDrainName()] = ( gposy * )NULL;
            intNetNameVec.push_back( ms.getDrainName() );
         } else if( parCap.find( ms.getSourceName() ) == parCap.end()
                    && ! ms.isGNDorVDD( ms.getSourceName() ) ) {
            parCap[ms.getSourceName()] = ( gposy * )NULL;
            intNetNameVec.push_back( ms.getSourceName() );
         } else if( parCap.find( ms.getBodyName() ) == parCap.end()
                    && ! ms.isGNDorVDD( ms.getBodyName() ) ) {
            parCap[ms.getBodyName()] = ( gposy * )NULL;
            intNetNameVec.push_back( ms.getBodyName() );
         }
      }
   }
}


ccc::~ccc() {
   for( unsigned i = 0; i < dioVec.size(); i ++ ) {
      for( unsigned j = 0; j < numOutputs ; j ++ ) {
         delete dioVec[i][outputNameVec[j]];
      }
   }

   for( unsigned i = 0; i < stdVec.size(); i ++ ) {
      for( unsigned j = 0; j < numOutputs ; j ++ ) {
         delete stdVec[i][outputNameVec[j]];
      }
   }
}

void ccc::putMeanDio( dio & d, const string & in, const string & out ) {
   for( unsigned i = 0; i < inputNameVec.size(); i ++ )
      for( unsigned j = 0; j < outputNameVec.size(); j ++ )
         if( inputNameVec[i] == in && outputNameVec[j] == out ) {
            if( dioVec[i][out] != ( dio * )NULL ) {
               dioerror( "mean dio model for \"" + in + "\" of \"" + getName() + "\" is defined more than once" );
               return;
            }

            dioVec[i][out] = &d;
            return;
         }

   dioerror( "no input name \"" + in + "\" or output name \"" + out + "\" in \"" + getName() + "\"" );

   return;
}

void ccc::putSTDDio( dio & d, const string & in, const string & out ) {
   for( unsigned i = 0; i < inputNameVec.size(); i ++ )
      for( unsigned j = 0; j < outputNameVec.size(); j ++ )
         if( inputNameVec[i] == in && outputNameVec[j] == out ) {
            if( stdVec[i][out] != ( dio * )NULL ) {
               dioerror( "std dio model for \"" + in + "\" to \"" + out +"\" of \"" + getName() + "\" is defined more than once" );
               return;
            }

            stdVec[i][out] = &d;
            return;
         }

   dioerror( "no input name \"" + in + "\" or output name \"" + out + "\" in \"" + getName() + "\"" );

   return;
}

void ccc::putParCap( gposy * parcap, const string & net ) {
   for( unsigned j = 0; j < outputNameVec.size(); j ++ )
      if( outputNameVec[j] == net ) {
         if( parCap[net] != ( gposy * )NULL ) {
            dioerror( "Parasitic Cap model for \"" + net +"\" of \""  + getName() + "\" is defined more than once" );
            return;
         }
         parCap[net] = parcap;
         return;
      }

   for( unsigned j = 0; j < intNetNameVec.size(); j ++ )
      if( intNetNameVec[j] == net ) {
         if( parCap[net] != ( gposy * )NULL ) {
            dioerror( "Parasitic Cap model for \"" + net +"\" of \""  + getName() + "\" is defined more than once" );
            return;
         }
         parCap[net] = parcap;
         return;
      }
   dioerror( "no net name \"" + net + "\" in \"" + getName() + "\"" );
   return;
}

void ccc::putWireCap( gposy * wirecap, const string & net ) {
   for( unsigned j = 0; j < outputNameVec.size(); j ++ )
      if( outputNameVec[j] == net ) {
         if( wireCap[net] != ( gposy * )NULL ) {
            dioerror( "Parasitic Cap model for \"" + net +"\" of \""  + getName() + "\" is defined more than once" );
            return;
         }
         wireCap[net] = wirecap;
         return;
      }

   for( unsigned j = 0; j < intNetNameVec.size(); j ++ )
      if( intNetNameVec[j] == net ) {
         if( wireCap[net] != ( gposy * )NULL ) {
            dioerror( "Parasitic Cap model for \"" + net +"\" of \""  + getName() + "\" is defined more than once" );
            return;
         }
         wireCap[net] = wirecap;
         return;
      }
   dioerror( "no net name \"" + net + "\" in \"" + getName() + "\"" );
   return;
}

void ccc::putLeakPowNmean(  gposy * leakpow, const string & out ) {
   for( unsigned j = 0; j < outputNameVec.size(); j ++ )
      if( outputNameVec[j] == out ) {
         if( leakPowNmean[out] != ( const gposy * )NULL ) {
            dioerror( "LeakPowNmean model for \"" + out +"\" of \"" + getName() + "\" is defined more than once" );
            return;
         }
         leakPowNmean[out] = leakpow;
         return;
      }
   dioerror( "no output name \"" + out + "\" or output name \"" + out + "\" in \"" + getName() + "\"" );
   return;
}


void ccc::putLeakPowPmean(  gposy * leakpow, const string & out ) {
   for( unsigned j = 0; j < outputNameVec.size(); j ++ )
      if( outputNameVec[j] == out ) {
         if( leakPowPmean[out] != ( const gposy * )NULL ) {
            dioerror( "LeakPowPmean model for \"" + out +"\" of \""+ getName() + "\" is defined more than once" );
            return;
         }
         leakPowPmean[out] = leakpow;
         return;
      }
   dioerror( "no output name \"" + out + "\" or output name \"" + out + "\" in \"" + getName() + "\"" );
   return;
}


void ccc::putLeakPowNstat(  gposy * leakpow, const string & out ) {
   for( unsigned j = 0; j < outputNameVec.size(); j ++ )
      if( outputNameVec[j] == out ) {
         if( leakPowNstat[out] != ( const gposy * )NULL ) {
            dioerror( "LeakPowNstat model for \"" + out +"\" of \"" + getName() + "\" is defined more than once" );
            return;
         }
         leakPowNstat[out] = leakpow;
         return;
      }
   dioerror( "no output name \"" + out + "\" or output name \"" + out + "\" in \"" + getName() + "\"" );
   return;
}


void ccc::putLeakPowPstat(  gposy * leakpow, const string & out ) {
   for( unsigned j = 0; j < outputNameVec.size(); j ++ )
      if( outputNameVec[j] == out ) {
         if( leakPowPstat[out] != ( const gposy * )NULL ) {
            dioerror( "LeakPowPstat model for \"" + out +"\" of \"" + getName() + "\" is defined more than once" );
            return;
         }
         leakPowPstat[out] = leakpow;
         return;
      }
   dioerror( "no output name \"" + out + "\" or output name \"" + out + "\" in \"" + getName() + "\"" );
   return;
}

int ccc::get_gate_num( const string & mos_name ) {
   vector<mos>::const_iterator mositr;
   string gate_name;
   for( mositr = devices.begin(); mositr != devices.end(); mositr++ )
      if( mositr->getMosName() == mos_name ) {
         // return mositr->getGateName();
         //gatename =  mositr->getGateName();
         for( unsigned i = 0 ; i < inputNameVec.size(); i ++ )
            if ( inputNameVec[i] == mositr->getGateName() )
               return i;
      }
   ciropterror( "the requested mos name : " + mos_name + "was not found in \n ccc : " + getName() );
   return -1;
}


bool ccc::isPMOS( const string & mos_name ) {
   vector<mos>::const_iterator  mositr;
   for( mositr = devices.begin(); mositr != devices.end(); mositr++ )
      if( mositr->getMosName() == mos_name )
         return mositr->isPMOS();

   ciropterror( "the requested mos name : " + mos_name + "was not found in \n ccc : " + getName() );
   return false;
}

string & ccc::getInputName( unsigned num ) {
   //cout << "Number is " << num << " & total Inputs " <<getNumberOfInputs() << " for gate " << getName() << endl;
   assert( num < getNumberOfInputs() );
   return inputNameVec[num];
}

string & ccc::getOutputName( unsigned num ) {
   assert( num < getNumberOfOutputs() );
   return outputNameVec[num];
}

int ccc::getSymbolIndex( const string & s ) {
   return symtab.index( s );
}

const string & ccc::getSymbol( unsigned i ) {
   return symtab.getSymbol( i );
}


string ccc::getWPName( unsigned num ) {

   assert( num < numInputs );

   if( wpVec[num].empty() ) {
      //	  cout << "Input " << getInputName( num ) << " in ccc " << getName() << " is only NMOS connected" << endl;
      //    assert( false );
      //  errorReport( "no corresponding P-mos width for the input "
      //   + getInputName( num ) + " in ccc " + getName() );

      return EMPTY_STRING;
   }
   string PNames = "(" + wpVec[num].front()->toString( symtab );
   for( int i = 1; i < wpVec[num].size(); i++ ) {
      PNames = PNames + " + " + wpVec[num][i]->toString( symtab );
   }
   PNames += ")";
   const string PN = PNames;
   return PN;
}

string ccc::getWNName( unsigned num ) {
   assert( num < numInputs );

   if( wnVec[num].empty() ) {
      //	  cout << "Input " << getInputName( num ) << " in ccc " << getName() << " is only PMOS connected" << endl;
      //    assert( false );
      //    errorReport( "no corresponding N-mos width for the input "
      //      + getInputName( num ) + " in ccc " + getName() );

      return EMPTY_STRING;
   }
   string NNames = "(" + wnVec[num].front()->toString( symtab );
   for( int i = 1; i < wnVec[num].size(); i++ ) {
      NNames = NNames + wnVec[num][i]->toString( symtab );
   }
   NNames += ")";
   const string NN = NNames;
   return NN;
}

vector<const gposy *>  ccc::getWPNameVec ( unsigned num ) {
   assert( num < numInputs );
   if( wpVec[num].empty() ) {
      //					cout << "Input " << getInputName( num ) << " in ccc " << getName() << " is only NMOS connected" << endl;
      //    assert( false );
      //  errorReport( "no corresponding P-mos width for the input "
      //   + getInputName( num ) + " in ccc " + getName() );

   }
   return  wpVec[num];
}


vector<const gposy *> ccc::getWNNameVec ( unsigned num ) {
   assert( num < numInputs );
   if( wnVec[num].empty() ) {
      //					cout << "Input " << getInputName( num ) << " in ccc " << getName() << " is only PMOS connected" << endl;
      //    assert( false );
      //    errorReport( "no corresponding N-mos width for the input "
      //      + getInputName( num ) + " in ccc " + getName() );

   }
   return  wnVec[num];
}

bool ccc::isMeanDioAssigned( unsigned ni, unsigned no ) {
   return ( dioVec[ni][outputNameVec[no]] != ( dio* ) NULL );
}


bool ccc::isStdDioAssigned( unsigned ni, unsigned no ) {
   return ( stdVec[ni][outputNameVec[no]] != ( dio* ) NULL );
}

bool ccc::isParCapAssigned( unsigned num ) {
   return ( parCap[outputNameVec[num]] != ( gposy * )NULL );
}
bool ccc::isMeanLeakPowAssigned( unsigned num ) {
   return ( leakPowNmean[outputNameVec[num]] != ( gposy * )NULL && leakPowPmean[outputNameVec[num]] != ( gposy * )NULL );
}

bool ccc::isStatLeakPowAssigned( unsigned num ) {
   return ( leakPowNstat[outputNameVec[num]] != ( gposy * )NULL && leakPowPstat[outputNameVec[num]] != ( gposy * )NULL );
}


string ccc::getLeakPowNnom( node & nd,  unsigned num ) {
   assert( isMeanLeakPowAssigned( num ) ); //checks if leakP or N is there or not
   if( leakPowNmean[outputNameVec[num]] == ( gposy * )NULL )
      return EMPTY_STRING;

   return leakPowNmean[outputNameVec[num]]->toString( symtab,nd.getvName(),EMPTY_STRING,EMPTY_STRING );
}


string ccc::getLeakPowPnom( node & nd, unsigned num ) {
   assert( isMeanLeakPowAssigned( num ) ); //checks if leakP or N is there or not
   if( leakPowPmean[outputNameVec[num]] == ( gposy * )NULL )
      return EMPTY_STRING;

   return leakPowPmean[outputNameVec[num]]->toString( symtab,nd.getvName(),"","" );
}



string ccc::getLeakPowNstat( node & nd,  unsigned num ) {
   assert( isStatLeakPowAssigned( num ) ); //checks if leakP or N is there or not
   if( leakPowNstat[outputNameVec[num]] == ( gposy * )NULL )
      return EMPTY_STRING;

   return leakPowNstat[outputNameVec[num]]->toString( symtab,nd.getvName(),EMPTY_STRING,EMPTY_STRING );
}


string ccc::getLeakPowPstat( node & nd, unsigned num ) {
   assert( isStatLeakPowAssigned( num ) ); //checks if leakP or N is there or not
   if( leakPowPstat[outputNameVec[num]] == ( gposy * )NULL )
      return EMPTY_STRING;

   return leakPowPstat[outputNameVec[num]]->toString( symtab,nd.getvName(),EMPTY_STRING,EMPTY_STRING );
}

string ccc::getParCap( node & nd,  unsigned num ) {
   assert( isParCapAssigned( num ) );

   return parCap[outputNameVec[num]]->toString( symtab,nd.getvName(),EMPTY_STRING,EMPTY_STRING );
}

string ccc::getParCap( node & nd, string intNet ) {
   //  assert( isParCapAssigned(num) );

   if( parCap[intNet] == ( gposy* )NULL )
      ciropterror( "The parCap of internal net " + intNet + " of node " + nd.getName() + " is not found" );

   return parCap[intNet]->toString( symtab,nd.getvName(),EMPTY_STRING,EMPTY_STRING );
}

string ccc::getWireCap( node & nd, string intNet ) {
   //  assert( isParCapAssigned(num) );

   if( wireCap[intNet] == ( gposy* )NULL )
      ciropterror( "The parCap of internal net " + intNet + " of node " + nd.getName() + " is not found" );

   return wireCap[intNet]->toString( symtab,nd.getvName(),EMPTY_STRING,EMPTY_STRING );
}

string ccc::getDioRF
( unsigned ni, unsigned no,
  const string & nodename, const string & fload ) {
   if( dioVec[ni][outputNameVec[no]] == ( dio * )NULL ) {
      errorReport( "CCCgetRF 1: No mean dio model defined for the input "
                   + getInputName( ni ) + " to output " + outputNameVec[no] + " in ccc " + getName() );
      return ERROR_STRING;
   }

   try {
      return string( "(" )
             + dioVec[ni][outputNameVec[no]]->getRF().toString( symtab,nodename,FCLOAD_NAME,fload ) + ")";
   } catch( noRefException ) {
      errorReport( "no mean rf dio model defined for the input "
                   + getInputName( ni ) + " to output " + getOutputName( no ) +  " in ccc " + getName() );
      return ERROR_STRING;
   }
}

string ccc::getDioFR
( unsigned ni, unsigned no,
  const string & nodename, const string & rload ) {
   if( dioVec[ni][outputNameVec[no]] == ( dio * )NULL ) {
      errorReport( "CCCgetFR 2: No mean dio model defined for the input "
                   + getInputName( ni ) + " to output " + getOutputName( no ) + " in ccc " + getName() );
      return ERROR_STRING;
   }

   try {
      return string( "(" )
             + dioVec[ni][outputNameVec[no]]->getFR().toString( symtab,nodename,RCLOAD_NAME,rload ) + ")";
   } catch( noRefException ) {
      cout << "no mean fr dio model defined for the input " <<
           getInputName( ni ) << " to output " << getOutputName( no )  << " in ccc " + getName() << endl ;
      //errorReport( "no mean fr dio model defined for the input "
      //    + getInputName( num ) + " in ccc " + getName() );
      return ERROR_STRING;
   }
}

string ccc::getSTDRF
( unsigned ni, unsigned no,
  const string & nodename, const string & fload )
throw( noRefException ) {
   if( stdVec[ni][outputNameVec[no]] == ( dio * )NULL ) {
      errorReport( "no std dio model defined for the input "
                   + getInputName( ni ) + " to output " + getOutputName( no ) + " in ccc " + getName() );
      return ERROR_STRING;
   }

   return string( "(" )
          + stdVec[ni][outputNameVec[no]]->getRF().toString( symtab,nodename,FCLOAD_NAME,fload ) + ")";
}

string ccc::getSTDFR
( unsigned ni, unsigned no,
  const string & nodename, const string & rload )
throw( noRefException ) {
   if( stdVec[ni][outputNameVec[no]] == ( dio * )NULL ) {
      errorReport( "no std dio model defined for the input "
                   + getInputName( ni ) + " to output " + getOutputName( no ) + " in ccc " + getName() );
      return ERROR_STRING;
   }

   return string( "(" )
          + stdVec[ni][outputNameVec[no]]->getFR().toString( symtab,nodename,RCLOAD_NAME,rload ) + ")";
}

string ccc::getVARRF
( unsigned ni, unsigned no,
  const string & gatename, const string & fload )
throw( noRefException ) {
   return string( "(" ) + getSTDRF( ni, no,gatename,fload ) + "^2)";
}

string ccc::getVARFR
( unsigned ni, unsigned no,
  const string & gatename, const string & rload )
throw( noRefException ) {
   return string( "(" ) + getSTDFR( ni, no, gatename,rload ) + "^2)";
}

double ccc::meanDioRF( unsigned ni, unsigned no,
                       const string & gatename,
                       map<string,double> & optVs,
                       const edge & outedge ) {
   //cout << "Reached the CCC meanDioRF 1" << endl;
   return meanDioRF( ni, no, gatename, optVs, outedge.valFCLoad( optVs ) );
}

double ccc::meanDioFR( unsigned ni, unsigned no,
                       const string & gatename,
                       map<string,double> & optVs,
                       const edge & outedge ) {
   return meanDioFR( ni, no, gatename, optVs, outedge.valRCLoad( optVs ) );
}

double ccc::stdDioRF( unsigned ni, unsigned no,
                      const string & gatename,
                      map<string,double> & optVs,
                      const edge & outedge ) {
   return stdDioRF( ni, no, gatename, optVs, outedge.valFCLoad( optVs ) );
}

double ccc::stdDioFR( unsigned ni, unsigned no,
                      const string & gatename,
                      map<string,double> & optVs,
                      const edge & outedge ) {
   return stdDioFR( ni, no, gatename, optVs, outedge.valRCLoad( optVs ) );
}

double ccc::varDioRF( unsigned ni, unsigned no,
                      const string & gatename,
                      map<string,double> & optVs,
                      const edge & outedge ) {
   return varDioRF( ni, no, gatename, optVs, outedge.valFCLoad( optVs ) );
}

double ccc::varDioFR( unsigned ni, unsigned no,
                      const string & gatename,
                      map<string,double> & optVs,
                      const edge & outedge ) {
   return varDioFR( ni,no, gatename, optVs, outedge.valRCLoad( optVs ) );
}

double ccc::meanDioRF( unsigned ni, unsigned no,
                       const string & gName,
                       map<string,double> & optVs, double fload ) {
   //cout << "Getting meanDioRF from ccc " << endl;
   if( dioVec[ni][outputNameVec[no]] == ( dio * )NULL ) {
      errorReport( "CCCRF 1: No mean dio model defined for the input "
                   + getInputName( ni ) + " in ccc " + getName() );
      return 0.0;
   }

   try {
      //dioVec[ni][outputNameVec[no]]->getRF().print(symtab);
      //cout << "FCLOAD_NAME string is " << FCLOAD_NAME << endl;
      return dioVec[ni][outputNameVec[no]]->getRF().evaluate( symtab,gName,optVs,FCLOAD_NAME,fload );
   } catch( noRefException ) {
      cout << "Warning: no mean fr dio model defined for the input "
           << getInputName( ni ) << " in ccc " << getName() << endl;
      return 0.0;
   }
}

double ccc::meanDioFR
( unsigned ni, unsigned no, const string & gName,
  map<string,double> & optVs, double rload ) {
   if( dioVec[ni][outputNameVec[no]] == ( dio * )NULL ) {
      errorReport( "CCC 1: No mean dio model defined for the input "
                   + getInputName( ni ) + " to " + getOutputName( no ) + " in ccc " + getName() );
      return 0.0;
   }

   try {
      //dioVec[ni][outputNameVec[no]]->getFR().print(symtab);
      //cout << "RCLOAD_NAME string is " << RCLOAD_NAME << endl;
      return dioVec[ni][outputNameVec[no]]->getFR().evaluate( symtab,gName,optVs,RCLOAD_NAME,rload );
   } catch( noRefException ) {
      cout << "Warning no mean rf dio model defined for the input "
           << getInputName( ni ) << " to output " << getOutputName( no )
           << " in ccc " << getName() << endl;;
      return 0.0;
   }
}

double ccc::stdDioRF
( unsigned ni, unsigned no, const string & gName,
  map<string,double> & optVs, double fload ) {
   if( stdVec[ni][outputNameVec[no]] == ( dio * )NULL ) {
      errorReport( "no std dio model defined for the input "
                   + getInputName( ni ) + " to output "+ getOutputName( no ) + " in ccc " + getName() );
      return 0.0;
   }

   try {
      return stdVec[ni][outputNameVec[no]]->getRF().evaluate( symtab,gName,optVs,FCLOAD_NAME,fload );
   } catch( noRefException ) {
      cout << "Warning: Exception in stdDio rf model defined for the input "
           << getInputName( ni ) << " to output "<< getOutputName( no ) << " in ccc " << getName() << endl;;
      return 0.0;
   }
}

double ccc::stdDioFR
( unsigned ni, unsigned no, const string & gName,
  map<string,double> & optVs, double rload ) {
   if( stdVec[ni][outputNameVec[no]] == ( dio * )NULL ) {
      errorReport( "no std dio model defined for the input "
                   + getInputName( ni )+ " to " + getOutputName( no ) + " in ccc " + getName() );
      return 0.0;
   }

   try {
      return stdVec[ni][outputNameVec[no]]->getFR().evaluate( symtab,gName,optVs,RCLOAD_NAME,rload );
   } catch( noRefException ) {
      cout << "Warning: Exception in stdDio fr model defined for the input "
           << getInputName( ni ) << " to " << getOutputName( no ) << " in ccc " << getName() << endl;;
      return 0.0;
   }
}

string ccc::getArea( const string & nodename ) {
   string area;
   for( int i = 0; i < symtab.getNumberOfSymbols(); i ++ )
      if( symtab[i] != FCLOAD_NAME && symtab[i] != RCLOAD_NAME )
         if( area.length() == 0 ) area += nodename + "." + symtab[i];
         else area += " + " + nodename + "." + symtab[i];

   return area;
}

void ccc::print() {
   cout << "ccc name = " << name
        << ", # of inputs = " << numInputs << endl;

   map<string,dio *>::iterator ditr;
   for( int i = 0; i < dioVec.size(); i ++ ) {
      ditr = dioVec[i].begin();
      for( ditr = dioVec[i].begin(); ditr != dioVec[i].end(); ditr++ )
         ditr->second->print( symtab );
      cout << endl;
   }
}

void ccc::print( bool varName ) {
   if( !varName ) {
      print();
      return;
   }

   cout << "ccc name = " << name
        << ", # of inputs = " << numInputs << endl;

   map<string,dio *>::iterator ditr;
   for( int i = 0; i < dioVec.size(); i ++ ) {
      ditr = dioVec[i].begin();
      for( ditr = dioVec[i].begin(); ditr != dioVec[i].end(); ditr++ )
         ditr->second->print( symtab );
      cout << endl;
   }
}

void ccc::assignWidthToInput( mos & ms ) {
   unsigned inputNum = getInputNumber( ms.getGateName() );
   assert( inputNum < getNumberOfInputs() );

   //  void putCorrNets( const string & netname, const string & risefall,  const string & corr_net, const string & corr_risefall );
   if( ms.isPMOS() ) wpVec[inputNum].push_back( ms.getWidthName() );
   else wnVec[inputNum].push_back( ms.getWidthName() );

   return;
}

void ccc::checkWidthAssignment() { //
   assert( inputNameVec.size() == getNumberOfInputs() );
   // assert( wpVec.size() == getNumberOfInputs() );
   //assert( wnVec.size() == getNumberOfInputs() );

   for( unsigned i = 0; i < getNumberOfInputs(); i ++ ) {
      if( wpVec[i].empty() ) {
         cout << inputNameVec[i] << " in ccc " << getName() << " seems to be a NMOS domino input" << endl;
         //     ciropterror( "p-mos width for " + inputNameVec[i] + " is not defined in ccc " + getName() );
      }

      if( wnVec[i].empty() ) {
         //    ciropterror( "n-mos width for " + inputNameVec[i] + " is not defined in ccc " + getName() );
         cout << inputNameVec[i] << " in ccc " << getName() << " seems to be a PMOS domino input" << endl;
      }
   }

   return;
}

/*
void ccc::assignWidthToInput
( const string & widthName, const string & inputName )
{
  int inputNum = getInputNumber( inputName );
  //int varNum = symtab.index(widthName);
  // assert(varNum>=0);

  if( widthName.length() < 2 )
    ciropterror( "the length of mos width name must be great than 1" );

  unsigned ind;
  if( widthName[1] == 'N'
      || (ind=widthName.find('.')) < widthName.length()
         && widthName[ind+2] == 'N' )
  { wnVec[inputNum] = widthName; }

  else if( widthName[1] == 'P'
      || (ind=widthName.find('.')) < widthName.length()
         && widthName[ind+2] == 'P' )
  { wpVec[inputNum] = widthName; }

  else
    ciropterror( widthName + " is neither a p-mos name nor a n-mos name" );

  return;
}
*/

unsigned ccc::getInputNumber( const string & inputName ) {
   for( int i = 0; i < inputNameVec.size(); i ++ )
      if( inputNameVec[i] == inputName ) return i;

   ciropterror( getName() + " does not have " + inputName + " as an input name" );

   return 0;
}

/*
 * CLASS dio MEMBER FUNCTION DEFINITIONS
 */

void dio::putRF( gposy & gp ) {
   if( dioRF != ( gposy * ) NULL ) {
      dioerror( "rf dio model is defined more than once" );
      return;
   }

   dioRF = & gp;
   return;
}

void dio::putFR( gposy & gp ) {
   if( dioFR != ( gposy * ) NULL ) {
      dioerror( "rf dio model is defined more than once" );
      return;
   }

   dioFR = & gp;
   return;
}

void dio::putRR( gposy & gp ) {
   if( dioRR != ( gposy * ) NULL ) {
      dioerror( "rf dio model is defined more than once" );
      return;
   }

   dioRR = & gp;
   return;
}

void dio::putFF( gposy & gp ) {
   if( dioFF != ( gposy * ) NULL ) {
      dioerror( "rf dio model is defined more than once" );
      return;
   }
   dioFF = & gp;
   return;
}

gposy & dio::getRF() throw( noRefException ) {
   if( dioRF == ( gposy * )NULL )
      throw noRefException();
   return *dioRF;
}

gposy & dio::getFR() throw( noRefException ) {
   if( dioFR == ( gposy * )NULL )
      throw noRefException();
   return *dioFR;
}

void dio::print() {
   if( dioRF != ( gposy * )NULL ) {
      cout << "rf: ";
      dioRF->print();
      cout << endl;
   }
   if( dioFR != ( gposy * )NULL ) {
      cout << "fr: ";
      dioFR->print();
      cout << endl;
   }
   if( dioRR != ( gposy * )NULL ) {
      cout << "rr: ";
      dioRR->print();
      cout << endl;
   }
   if( dioFF != ( gposy * )NULL ) {
      cout << "ff: ";
      dioFF->print();
      cout << endl;
   }
}

void dio::print( symbol_table & symtab ) {
   if( dioRF != ( gposy * )NULL ) {
      dioRF->print( symtab,"rf: " );
      cout << endl;
   }
   if( dioFR != ( gposy * )NULL ) {
      dioFR->print( symtab,"fr: " );
      cout << endl;
   }
   if( dioRR != ( gposy * )NULL ) {
      dioRR->print( symtab,"rr: " );
      cout << endl;
   }
   if( dioFF != ( gposy * )NULL ) {
      dioFF->print( symtab,"ff: " );
      cout << endl;
   }
}

/*
 * CLASS edge MEMBER FUNCTION DEFINITIONS
 */

edge::edge( const string & nm )
   : name( nm ),
     tRiseName( nm + "." + string( "Trise" ) ),
     tFallName( nm + "." + string( "Tfall" ) ),
     tName( nm + "." + string( "T" ) ),
     switching_factor( -1.0 ) {
   pi = ( piinfo * )NULL;
   po = ( poinfo * )NULL;
   driverNode = ( node * )NULL;
   CorrEdgeTrise.clear();
   CorrEdgeTfall.clear();
   RiseFallTrise.clear();
   RiseFallTfall.clear();
   numOfMonteCarloVisits = 0;
}

edge::~edge() {
   delete pi;
   delete po;
}

double edge::getSwitchingFactor() {
   // assert( !isPI() );
   if( switching_factor > 1.0 || ( switching_factor != -1.0 && switching_factor < 0 ) ) {
      errorReport( "Switching factor for edge " + getName() + " is out of limits" );
      return 0;
   }
   return switching_factor;
}

void edge::put( piinfo & i ) {
   if( isPO() ) {
      ciropterror( "illegal attempt to declare " + getName()
                   + " as a primary input after already declared"
                   + " as a primary output" );
      return;
   }

   if( isPI() ) {
      ciropterror( "illegal attempt to declare " + getName()
                   + " as a primary input more than once" );
      return;
   }

   if( driverNode != ( node * )NULL ) {
      ciropterror( "illegal attempt to declare " + getName()
                   + " as a primary input net while it is the output net of"
                   + " the gate " + driverNode->getName() );
      return;
   }

   pi = & i;
}

void edge::put( poinfo & o ) {
   if( isPI() ) {
      ciropterror( "illegal attempt to declare " + getName()
                   + " as a primary output after already declared"
                   + " as a primary input" );
      return;
   }

   if( isPO() ) {
      ciropterror( "illegal attempt to declare " + getName()
                   + " as a primary output more than once" );
      return;
   }
   /*
    * Its okay for the output to be an input to another gate
    * to produce another output.
     if( fanoutNodeVec.size() != 0 )
     {
       ciropterror( "illegal attempt to declare " + getName()
                    + " as a primary output net while it is an input net of"
   		 + " the gate " + fanoutNodeVec[0]->getName() );
       return;
     }
   */
   po = & o;
}

void edge::print_corr_edges() {
   if( CorrEdgeTrise.size() != 0 )
      for( unsigned int i = 0; i!=CorrEdgeTrise.size(); i++ )
         cout << getName() << " rise has " << RiseFallTrise[i] << " of " << CorrEdgeTrise[i] << endl;

   if( CorrEdgeTfall.size() != 0 )
      for( unsigned int i = 0; i!=CorrEdgeTfall.size(); i++ )
         cout << getName() << " fall has " << RiseFallTfall[i] << " of " << CorrEdgeTfall[i] << endl;
}

string edge::getCorrNets( const string & risefall ) {
   string corrstr;
   if( risefall == "rise" )
      for( unsigned i = 0; i < CorrEdgeTrise.size() ; i++ ) {
         if( corrstr.length() ==0 ) corrstr = CorrEdgeTrise[i]+".T"+RiseFallTrise[i];
         else
            corrstr = CorrEdgeTrise[i]+ ".T" + RiseFallTrise[i] + " , " + corrstr;
      }
   if( risefall == "fall" )
      for( unsigned i = 0; i < CorrEdgeTfall.size() ; i++ ) {
         if( corrstr.length() ==0 ) corrstr = CorrEdgeTfall[i]+".T"+RiseFallTfall[i];
         else
            corrstr = CorrEdgeTfall[i]+ ".T" + RiseFallTfall[i] + " , " + corrstr;
      }
   return corrstr;
}

void edge::putCorrNetTrise( const string & in, const bool rf ) {
   string risefall;
   if( rf )risefall = "fall";
   else risefall = "rise";
   for( unsigned i = 0; i < CorrEdgeTrise.size(); i ++ )
      if( CorrEdgeTrise[i] == in && RiseFallTrise[i] == risefall ) {
         // cout << "netname correlations repeated for or reflected for:" << in << endl;
         return;
      }
   CorrEdgeTrise.push_back( in );
   RiseFallTrise.push_back( risefall );
}

node & edge::getFanoutNode( unsigned i ) {
   if( ! ( i < fanoutNodeVec.size() ) )
      errorReport( "Error: Asking for the " + cnvt::doubleToString( i ) + "th fanout node of edge " + getName() + " whereas there are only " + cnvt::doubleToString( fanoutNodeVec.size() ) + " nodes"  );
   return * fanoutNodeVec[i];
}



void edge::putCorrNetTfall( const string & in, const bool rf ) {
   string risefall;
   if( rf )risefall = "fall";
   else risefall = "rise";
   for( unsigned i = 0; i < CorrEdgeTfall.size(); i ++ )
      if( CorrEdgeTfall[i] == in && RiseFallTfall[i] == risefall ) {
         // cout << "netname correlations repeated for or reflected for:" << in << endl;
         return;
      }
   CorrEdgeTfall.push_back( in );
   RiseFallTfall.push_back( risefall );
}

void edge::putOutNode( node & on ) {
   if( driverNode != ( node * )NULL ) {
      ciropterror( "illegal attempt to declare " + getName()
                   + " as the output net of the gate " + on.getName()
                   + "; " + getName() + " has been already declared as"
                   + " the output net of " + driverNode->getName() );
      return;
   }

   if( on.isCapacitor() ) {
      ciropterror( "illegal attempt to declare " + getName()
                   + " as the output net of the gate " + on.getName()
                   + " while " + on.getName() + " is a capacitor" );
      return;
   }

   if( isPI() ) {
      ciropterror( "illegal attempt to declare " + getName()
                   + " as the output net of the gate " + on.getName()
                   + " while it is a primary input net" );
      return;
   }

   driverNode = & on;
}




void edge::putInNode( node & in, unsigned num ) {
   assert( fanoutNodeVec.size() == inNumVec.size() );
   //assert that this connection is not already set.
   for( unsigned i = 0; i < inNumVec.size(); i ++ )
      assert( !( fanoutNodeVec[i] == & in && inNumVec[i] == num ) );

   assert( in.isCapacitor() || in.isVRL() || num < in.getNumberOfInputs() );
   //its okay for an output to be an input to some other gate to produce another output.
   //  if( isPO() )
   //  {
   //    ciropterror( "illegal attempt to declare " + getName()
   //                 + " as the " + cnvt::intToString(num+1)
   //		 + "th input net of the gate " + in.getName()
   //                 + " while it is a primary output net" );
   //    return;
   //  }
   fanoutNodeVec.push_back( & in );
   inNumVec.push_back( num );
}

void edge::putSwitchingFactor( double sf ) {
   assert( sf >= 0 );
   switching_factor = sf;

   return;
}


//for keepers and dummy outputs with caps.
bool edge::skipEdgeTiming() {
   //Include dummy outputs connected to capacitorss
   if( getNumFanoutNodes() == 1
         && ( ! isPO() )
         && getFanoutNode( 0 ).isCapacitor() ) {
      cout << "Including dummy output " << getName() << " in the delay constraint" << endl;
      return false;
   } else if( isPO() ) return false;


   if( getNumFanoutNodes() == 1
         && ( ! isPO() )
         && getFanoutNode( 0 ).isVRL() ) {
      cout << "Found consecutive VRL nodes, traversing " << getFanoutNode( 0 ).getName() << " for the keeper" << endl;
      return getFanoutNode( 0 ).getOutEdge( 0 ).skipEdgeTiming();
   }


   //again the stuff about keeper inputs and inputs
   if( getNumFanoutNodes()==1
         && ( ! getFanoutNode( 0 ).isVRL()
              && getFanoutNode( 0 ).getCCC().getInputName( getInCCCNum( 0 ) ).substr( 0,3 ) == KEEPER_INPUT )
         ||
         getFanoutNode( 0 ).isVRL()
         && getFanoutNode( 0 ).getOutEdge( 0 ).getNumFanoutNodes() == 1
         && getFanoutNode( 0 ).getOutEdge( 0 ).getFanoutNode( 0 ).getCCC().getInputName( getInCCCNum( 0 ) ).substr( 0,3 ) == KEEPER_INPUT )
      return true; //if this edge is connected solely to a keeper input, then obv its not timed. So do not include it.
   else
      return false;

}


bool edge::isGNDorVDD() const {
   return ( getName()=="gnd"
            || getName() == "GND"
            || getName() == "Gnd"
            || getName() == "vdd"
            || getName() == "VDD"
            || getName() == "Vdd"
            || getName() == "VSS"
            || getName() == "Vss" );
}

double edge::getPreValue( bool rf ) {
   double val = 0.0;

   if( isPI() ) {
      piinfo & pii = getPI();

      if( rf )
         val = pii.getRSlope() * atof( ( opt_prob_generator::getVthn() ).c_str() )
               / 2.0 / atof( ( opt_prob_generator::getVdd() ).c_str() );
      //      val = pii.getRSlope() * 1/5;
      else
         val = pii.getFSlope() * atof( ( opt_prob_generator::getVthp() ).c_str() )
               / 2.0 / atof( ( opt_prob_generator::getVdd() ).c_str() );
      //      val = pii.getFSlope() * 1/5;

      if( ! ( val > 0 ) )
         errorReport( "One of the input rise/fall time is not positive" );

      double msl = atof( ( opt_prob_generator::getMinSl() ).c_str() );
      if( msl > val )
         val = msl;
   }

   return val;
}


string edge::getEnergy( bool noLoad, bool UseDefActFact ) {
   //	cout << "Edge in question is " << getName() << endl;
   //  double vdd = opt_prob_generator::getVdd();

   double sf = getSwitchingFactor();
   // assert( sf >= 0.0 );
   double min_sf = atof( ( opt_prob_generator::getMinSF() ).c_str() );
   //cout << "The offending net is " << getName() << endl;
   if( sf < 0.0 ) {
      if( UseDefActFact )
         sf = atof( ( opt_prob_generator::getDefSF() ).c_str() );
      else
         errorReport( "The AF of net " + getName() + " is < 0 \n May be its not initialized.\n If you wish to use the default activity factor, use UseDefActFactor" );
   } else if( sf <= min_sf )
      sf = min_sf;

   string edgeEnergy;
   if( sf == 0.0 )
      edgeEnergy = EMPTY_STRING;
   else {
      //return "( " + cnvt::doubleToString(vdd) + " ^ 2 * "
      if( getEnergyCLoad( noLoad ) == EMPTY_STRING )
         if( getParCap() == EMPTY_STRING )
            edgeEnergy = EMPTY_STRING;
         else
            edgeEnergy = cnvt::doubleToString( sf )  + " ( " + getParCap() + " )";
      else if( getParCap() == EMPTY_STRING )
         edgeEnergy = cnvt::doubleToString( sf )  + " ( " + getEnergyCLoad( noLoad ) + " )";
      else
         edgeEnergy = cnvt::doubleToString( sf )  + " ( " + getParCap() + " + " + getEnergyCLoad( noLoad ) + " )";

   }
   return edgeEnergy;
}

string edge::getLogicEnergy( bool noLoad, bool UseDefActFact ) {
   //	cout << "Edge in question is " << getName() << endl;
   //  double vdd = opt_prob_generator::getVdd();

   double sf = getSwitchingFactor();
   // assert( sf >= 0.0 );
   double min_sf = atof( ( opt_prob_generator::getMinSF() ).c_str() );
   //cout << "The offending net is " << getName() << endl;
   if( sf < 0.0 ) {
      if( UseDefActFact )
         sf = atof( ( opt_prob_generator::getDefSF() ).c_str() );
      else
         errorReport( "The AF of net " + getName() + " is < 0 \n May be its not initialized.\n If you wish to use the default activity factor, use UseDefActFactor" );
   } else if( sf <= min_sf )
      sf = min_sf;

   string edgeEnergy;
   if( sf == 0.0 )
      edgeEnergy = EMPTY_STRING;
   else {
      //return "( " + cnvt::doubleToString(vdd) + " ^ 2 * "
      if( getEnergyCLoad( noLoad ) == EMPTY_STRING )
         if( getParCap() == EMPTY_STRING )
            edgeEnergy = EMPTY_STRING;
         else
            edgeEnergy = cnvt::doubleToString( sf )  + " ( " + getParCap() + " )";
      else if( getParCap() == EMPTY_STRING )
         edgeEnergy = cnvt::doubleToString( sf )  + " ( " + getEnergyCLoad( noLoad ) + " )";
      else
         edgeEnergy = cnvt::doubleToString( sf )  + " ( " + getParCap() + " + " + getEnergyCLoad( noLoad ) + " )";

   }
   return edgeEnergy;
}


string edge::getWireEnergy( bool UseDefActFact ) {
   //	cout << "Edge in question is " << getName() << endl;
   //  double vdd = opt_prob_generator::getVdd();

   double sf = getSwitchingFactor();
   // assert( sf >= 0.0 );
   double min_sf = atof( ( opt_prob_generator::getMinSF() ).c_str() );
   //cout << "The offending net is " << getName() << endl;
   if( sf < 0.0 ) {
      if( UseDefActFact )
         sf = atof( ( opt_prob_generator::getDefSF() ).c_str() );
      else
         errorReport( "The AF of net " + getName() + " is < 0; May be its not initialized.\n If you wish to use the default activity factor, use UseDefActFactor" );
   } else if( sf <= min_sf )
      sf = min_sf;

   string edgeEnergy;
   if( sf == 0.0  || ( ( edgeEnergy = getWireCap() ) == EMPTY_STRING ) )
      edgeEnergy = EMPTY_STRING;
   else {
      edgeEnergy = cnvt::doubleToString( sf )  + " ( " + edgeEnergy + " )";
   }
   return edgeEnergy;
}

string edge::getLoadEnergy( bool UseDefActFact ) {
   assert( isPO() );
   double sf = getSwitchingFactor();
   // assert( sf >= 0.0 );
   double min_sf = atof( ( opt_prob_generator::getMinSF() ).c_str() );

   if( sf < 0.0 ) {
      if( UseDefActFact )
         sf = atof( ( opt_prob_generator::getDefSF() ).c_str() );
      else
         errorReport( "The AF of net " + getName() + " is < 0 \n May be its not initialized.\n If you wish to use the default activity factor, use UseDefActFactor" );
   } else if( sf <= min_sf )
      sf = min_sf;

   if( sf == 0.0 )
      return EMPTY_STRING;
   else
      return cnvt::doubleToString( sf )  + "*" + string( "(" ) + po->capToString() + ")";
}


//obtain the total width of the fanout transistors connected to this edge



string edge::getWidth() {
   if( fanoutNodeVec.size() == 0 && !isPO() ) {
      ciropterror( "net " + getName() + " is either fictious or contains no gates on its fanout for which areas has to be considered " );
      return EMPTY_STRING;
   }
   //else {this net is an input to some gate}
   string fcl;
   string WidthN;
   string WidthP;
   for( unsigned i = 0; i < fanoutNodeVec.size(); i ++ ) {
      if ( fanoutNodeVec[i]->isCapacitor()
            || fanoutNodeVec[i]->isVRL() ) continue;
      WidthP = fanoutNodeVec[i]->getWPName( inNumVec[i] );
      WidthN = fanoutNodeVec[i]->getWNName( inNumVec[i] );
      if( fcl.length() == 0 && WidthP != EMPTY_STRING ) {
         fcl = WidthP;
         if( WidthN != EMPTY_STRING )
            fcl = WidthN + " + " + fcl;
         continue;
      }
      if( fcl.length() == 0 && WidthN != EMPTY_STRING ) {
         fcl = WidthN;
         if( WidthP != EMPTY_STRING )
            fcl = WidthP + " + " + fcl;
         continue;
      }
      if( WidthP != EMPTY_STRING )
         fcl = WidthP + " + " + fcl;
      if( WidthN != EMPTY_STRING )
         fcl = WidthN + " + " + fcl;
   }
   return fcl;
}

string edge::getRCLoad( opt_prob_generator & opt ) {
   if( fanoutNodeVec.size() == 0 && !isPO() ) {
      errorReport( "net " + getName() + " is neither connected to an input"
                   " of a gate, nor is a primary output.\n"
                   "If you want this net do be a dummy output, put a dummy cap on it of a small value, say 0.01(f)" );
      return ERROR_STRING;
   }

   // if this is a primay output net
   if( fanoutNodeVec.size() == 0 )
      return string( "(" ) + po->capToString() + ")";

   // if this net is an input to some gate
   string rcl;
   string tmp;
   for( unsigned i = 0; i < fanoutNodeVec.size(); i ++ ) {
      if( rcl == EMPTY_STRING ) {
         if( ( tmp = fanoutNodeVec[i]->getRCLoad( inNumVec[i],opt ) ) != EMPTY_STRING )
            rcl = "( " + tmp;
      } else {
         if( ( tmp = fanoutNodeVec[i]->getRCLoad( inNumVec[i],opt ) ) != EMPTY_STRING )
            rcl += " + " + tmp;
      }
      //			 cout << "for edge name " << getName() << "node name is " << fanoutNodeVec[i]->getName() << endl;
   }
   //assert( !isPO() );
   //if this is also an output, add the output load as well
   if( isPO() ) rcl += " + " + po->capToString();

   rcl += " )";

   return rcl;
}

string edge::getFCLoad( opt_prob_generator & opt ) {
   if( fanoutNodeVec.size() == 0 && !isPO() ) {
      errorReport( "net " + getName() + " is neither connected to an input"
                   " of a gate, nor is a primary output.\n"
                   "If you want this net do be a dummy output, put a dummy cap on it of a small value, say 0.01(f)" );
      return ERROR_STRING;
   }

   // if this is a primay output net
   if( fanoutNodeVec.size() == 0 )
      return string( "(" ) + po->capToString() + ")";

   // if this net is an input to some gate
   string fcl;
   string tmp;
   for( unsigned i = 0; i < fanoutNodeVec.size(); i ++ )
      if( fcl == EMPTY_STRING ) {
         if( ( tmp = fanoutNodeVec[i]->getFCLoad( inNumVec[i],opt ) ) != EMPTY_STRING )
            fcl = "( " + tmp;
      } else {
         if( ( tmp = fanoutNodeVec[i]->getFCLoad( inNumVec[i],opt ) ) != EMPTY_STRING )
            fcl += " + " + tmp;
      }
   //assert( !isPO() );
   //if this is also a PO add the output load as well.
   if( isPO() ) fcl += " + " + po->capToString();

   fcl += " )";

   return fcl;
}

string edge::getEnergyCLoad( bool noLoad ) {
   if( fanoutNodeVec.size() == 0 && !isPO() ) {
      errorReport( "net " + getName() + " is neither connected to an input"
                   " of a gate, nor is a primary output.\n"
                   "If you want this net do be a dummy output, put a dummy cap on it of a small value, say 0.01(f)" );
      return ERROR_STRING;
   }

   // if this is only a primary output net
   if( fanoutNodeVec.size() == 0 ) {
      if( noLoad )
         return EMPTY_STRING;
      else
         return string( "(" ) + po->capToString() + ")";
   }
   // if this net is an input to some gate
   string tmp;
   string fcl;
   //	= "( " + fanoutNodeVec[0]->getEnergyCLoad(inNumVec[0]);
   for( unsigned i = 0; i < fanoutNodeVec.size(); i ++ )
      if( ! fanoutNodeVec[i]->isCapacitor() && ! fanoutNodeVec[i]->isVRL() ) //account for only Logic energy
         if( ( tmp = fanoutNodeVec[i]->getEnergyCLoad( inNumVec[i] ) ) != EMPTY_STRING )
            if( fcl == EMPTY_STRING )
               fcl = "( " + tmp;
            else
               fcl += " + " + tmp;

   //  assert( !isPO() );
   //if this is also a PO add the output load as well.
   if( isPO() && !noLoad )
      fcl += " + " + po->capToString();

   if( fcl != EMPTY_STRING )
      fcl += " )";
   return fcl;
}

string edge::getParCap() {
   if( isPI() || getDriverNode()->isVRL() )
      return EMPTY_STRING;
   // cout << "This edge has a problem : " << getName() <<  endl;
   assert( driverNode != ( node * )NULL );

   for( unsigned i = 0; i < driverNode->getNumberOfOutputs(); i ++ ) {
      if ( driverNode->getOutEdge( i ).getName() == getName() )
         return driverNode->getParCap( i );
   }
   errorReport( "The edge " + getName() + " does not have a parasitic Cap" );
   return ERROR_STRING;
}

string edge::getWireCap() {
   string wirecap = EMPTY_STRING;
   for( unsigned i = 0; i < getNumFanoutNodes(); i ++ ) {
      //			 cout << "reached here for " << i << " while upper limit is " << getNumFanoutNodes() << endl;
      if ( getFanoutNode( i ).isCapacitor() ) { //only include capacitors
         assert( !getFanoutNode( i ).isVRL() );
         if( wirecap == EMPTY_STRING )
            wirecap = getFanoutNode( i ).getEnergyCLoad( 0 );
         else
            wirecap += ( " + " + getFanoutNode( i ).getEnergyCLoad( 0 ) );
      }
   }
   return wirecap;
}

double edge::valRCLoad( map<string,double> & optVs )const {
   if( fanoutNodeVec.size() == 0 && !isPO() ) {
      errorReport( "net " + getName() + " is neither connected to an input"
                   " of a gate, nor is a primary output.\n"
                   "If you want this net do be a dummy output, put a dummy cap on it of a small value, say 0.01(f)" );
      return 0.0;
   }


   if( fanoutNodeVec.size() == 0 )
      return po->getCap( optVs );

   double rcl = 0.0;

   for( int i = 0; i < fanoutNodeVec.size(); i ++ )
      rcl += fanoutNodeVec[i]->valRCLoad( inNumVec[i],optVs );

   if( isPO() ) rcl += po->getCap( optVs );
   //cout << "RCload is " << rcl << endl;
   return rcl;
}

double edge::valFCLoad( map<string,double> & optVs )const {
   if( fanoutNodeVec.size() == 0 && !isPO() ) {
      errorReport( "net " + getName() + " is neither connected to an input"
                   " of a gate, nor is a primary output.\n"
                   "If you want this net do be a dummy output, put a dummy cap on it of a small value, say 0.01(f)" );
      return 0.0;
   }

   if( fanoutNodeVec.size() == 0 )
      return po->getCap( optVs );

   double fcl = 0.0;

   for( int i = 0; i < fanoutNodeVec.size(); i ++ ) {
      /*
      if(fanoutNodeVec[i]->isCapacitor())
      		cout << "Reached the capacitor " << fanoutNodeVec[i]->getName() << endl;
      else if(fanoutNodeVec[i]->isVRL())
      		cout << "Reached the VRL node " << fanoutNodeVec[i]->getName() << endl;
      else
      		cout << "Reaching Input " << inNumVec[i] << " i.e. " << (fanoutNodeVec[i]->getCCC()).getInputName(inNumVec[i])  << " of node " << fanoutNodeVec[i]->getName() << endl;
      		*/

      fcl += fanoutNodeVec[i]->valFCLoad( inNumVec[i],optVs );
   }
   if( isPO() ) fcl += po->getCap( optVs );
   //cout << "FCload is " << fcl << endl;
   return fcl;
}

string edge::getTRiseName_Corr( const string & pre, bool isNoRiseFall ) {
   string name ;
   string corr_name;
   /*  if( isPI() )
     {
       if( pi->getT() == 0.0 ) return EMPTY_STRING;
       else return pre + cnvt::doubleToString(pi->getT());
     }
     else
     { */
   if( CorrEdgeTrise.size() == 0 )
      return pre + tRiseName;
   else
      name = tRiseName + ")" ;
   if( isNoRiseFall ) {
      for ( unsigned int i = 0; i < CorrEdgeTrise.size(); i++ ) {
         name = 	CorrEdgeTrise[i] + ".Trise"+ "," + name;
      }
      return ( pre + "max( " + name );
   } else {
      for ( unsigned int i = 0; i < CorrEdgeTrise.size(); i++ ) {
         name = 	CorrEdgeTrise[i] + ".T"+RiseFallTrise[i] + "," + name;
      }
      return ( pre + "max( " + name );
   }

   //  assert( false );
   //  return EMPTY_STRING;
}


string edge::getTRiseName( const string & pre ) {
   //  if( isPI() )
   //  {
   //    if( pi->getT() == 0.0 ) return EMPTY_STRING;
   //    else return pre + cnvt::doubleToString(pi->getT());
   //  }
   //  else
   //  {
   return pre + tRiseName;
   //  }

   //  assert( false );
   //  return EMPTY_STRING;
}

string edge::getTRiseName
( unsigned num, const string & pre ) {
   /*
   	if( isPI() )
     {
       if( pi->getT() == 0.0 ) return EMPTY_STRING;
       else return pre + cnvt::doubleToString(pi->getT());
     }
     else
   	*/
   return getTRiseName( pre ) + "_" + cnvt::intToString( num+1 );

   //  assert( false );
   //  return EMPTY_STRING;
}

string edge::getMonteTRiseName( const string & pre ) {
   return pre + tRiseName;

   assert( false );
   return EMPTY_STRING;
}

string edge::getTFallName_Corr( const string & pre ) {
   string name ;
   /*  if( isPI() )
     {
       if( pi->getT() == 0.0 ) return EMPTY_STRING;
       else return pre + cnvt::doubleToString(pi->getT());
     }
     else
     {*/
   if( CorrEdgeTfall.size() == 0 )
      return pre + tFallName;
   else
      name = tFallName + ")";
   for ( unsigned int i = 0; i < CorrEdgeTfall.size(); i++ ) {
      name = 	CorrEdgeTfall[i] + ".T"+RiseFallTfall[i] + "," + name;
   }
   return ( pre + "max( " + name );
   //	}

   //  assert( false );
   //  return EMPTY_STRING;
}

string edge::getTFallName( const string & pre ) {
   //  if( isPI() )
   //  {
   //    if( pi->getT() == 0.0 ) return EMPTY_STRING;
   //    else return pre + cnvt::doubleToString(pi->getT());
   //  }
   //  else
   //  {
   return pre + tFallName;
   //  }

   //  assert( false );
   //  return EMPTY_STRING;
}

string edge::getTFallName
( unsigned num, const string & pre ) {
   if( isPI() ) {
      if( pi->getT() == 0.0 ) return EMPTY_STRING;
      else return pre + cnvt::doubleToString( pi->getT() );
   } else
      return getTFallName( pre ) + "_" + cnvt::intToString( num+1 );

   assert( false );
   return EMPTY_STRING;
}

string edge::getMonteTFallName( const string & pre ) {
   return pre + tFallName;

   assert( false );
   return EMPTY_STRING;
}

void edge::print() {
   cout << "edge name = " << name;

   if( driverNode!=( node* )NULL )
      cout << ", output node = " << driverNode->getName();

   if( fanoutNodeVec.size() > 0 )
      cout << ", input nodes =";
   for( int i = 0; i < fanoutNodeVec.size(); i ++ )
      cout << " " << fanoutNodeVec[i]->getName() << " " << inNumVec[i];
   cout << endl;

   return;
}

void edge::montecarlo
( map<string,ProbDist *> & mc,
  map<string,vector<unsigned> > & netSelectMap,
  const string & dist, unsigned N, double p,
  map<string,double> & optVs, bool noTriseTfall ) {
   map<string,ProbDist *>::iterator itr;
   if( noTriseTfall ) {
      // if already visited, check if this is the last time this edge will be visited and  return.
      // This option is only available with noTriseFall
      if( ( itr = mc.find( getMonteTRiseName() ) ) != mc.end() ) {
         numOfMonteCarloVisits++;
         if( numOfMonteCarloVisits == ( getNumFanoutNodes() + 1 ) ) //the +1 is for the initial monte carlo loop which goes on to every net
            mc.erase( itr );
         return;
      }
      if( isPI() ) {
         mc[getMonteTRiseName()] = new ProbDist( getPI().getT() );
         return;
      }
   } else {
      // make sure that either Monte Carlo has been done for both rise and fall nets
      // or for none
      if( mc.find( getMonteTRiseName() ) != mc.end()
            && mc.find( getMonteTFallName() ) == mc.end() )
         assert( false );

      if( mc.find( getMonteTRiseName() ) == mc.end()
            && mc.find( getMonteTFallName() )!=mc.end() )
         assert( false );

      // if already visited, just return
      if( mc.find( getMonteTRiseName() ) != mc.end()
            && mc.find( getMonteTFallName() ) != mc.end() )
         return;

      if( isPI() ) {
         mc[getMonteTRiseName()] = new ProbDist( getPI().getT() );


         mc[getMonteTFallName()] = new ProbDist( getPI().getT() );

         return;
      }
   }

   // make sure that this net is output of a gate in the network
   // (since it's not a primary input
   if( driverNode == ( node * )NULL )
      errorReport( "The net " + getName() + " is neither a PI nor is driven by any gate, please check" );

   // do Monte Carlo for the gate whose output is this net
   // (there is only one such gate by definition of my network at this point)
   driverNode->montecarlo( mc, netSelectMap, dist, N, p, optVs,noTriseTfall );
   return;
}

void edge::montecarlo
( map<string,ProbDist *> & mc,
  map<string,vector<unsigned> > & netSelectMap,
  map<string,ProbDist *> & gatePDMap,
  const monte_carlo & monte, double p,
  map<string,double> & optVs, bool noTriseTfall ) {
   map<string,ProbDist *>::iterator itr;
   if( noTriseTfall ) {
      // if already visited, just return
      if( ( itr =mc.find( getMonteTRiseName() ) ) != mc.end() ) {
         numOfMonteCarloVisits++;
         if( numOfMonteCarloVisits == ( getNumFanoutNodes() + 1 ) ) //the +1 is for the initial monte carlo loop which goes on to every net
            mc.erase( itr );
         return;
      }
      if( isPI() ) {
         mc[getMonteTRiseName()] = new ProbDist( getPI().getT() );
         return;
      }
   } else {
      // make sure that either Monte Carlo has been done for both rise and fall nets
      // or for none
      if( mc.find( getMonteTRiseName() ) != mc.end()
            && mc.find( getMonteTFallName() ) == mc.end() )
         assert( false );

      if( mc.find( getMonteTRiseName() ) == mc.end()
            && mc.find( getMonteTFallName() )!=mc.end() )
         assert( false );

      // if already visited, just return
      if( mc.find( getMonteTRiseName() ) != mc.end()
            && mc.find( getMonteTFallName() ) != mc.end() )
         return;

      if( isPI() ) {
         mc[getMonteTRiseName()] = new ProbDist( getPI().getT() );


         mc[getMonteTFallName()] = new ProbDist( getPI().getT() );

         return;
      }
   }

   // make sure that this net is output of a gate in the network
   // (since it's not a primary input
   if( driverNode == ( node * )NULL )
      errorReport( "The net " + getName() + " is neither a PI nor is driven by any gate, please check" );

   // do Monte Carlo for the gate whose output is this net
   // (there is only one such gate by definition of my network at this point)
   driverNode->montecarlo( mc, netSelectMap, gatePDMap, monte, p, optVs,noTriseTfall );

   return;
}

void edge::nominalAnalysis
( map<string,double> & nomAnlys,
  map<string,unsigned> & netSelectVec,
  map<string,vector<double> > & pathLengthMap,
  map<string,vector<double> > & pathVarianceMap,
  map<string,double > & gateDioMap,
  map<string,double> & optVs, bool noTriseTfall ) {
   const string & risename = getMonteTRiseName();
   const string & fallname = getMonteTFallName();

   //cout << "Risename is " << risename << " and fall name is " << fallname << endl;
   // make sure that either nominal analysis has been done
   // for both rise and fall nets or for none, unless there is only one timing per net
   if( !noTriseTfall ) {
      assert( nomAnlys.find( risename ) == nomAnlys.end()
              && nomAnlys.find( fallname ) == nomAnlys.end()
              ||    nomAnlys.find( risename ) != nomAnlys.end()
              && nomAnlys.find( fallname ) != nomAnlys.end() );

      assert( pathLengthMap.find( risename ) == pathLengthMap.end()
              && pathLengthMap.find( fallname ) == pathLengthMap.end()
              ||    pathLengthMap.find( risename ) != pathLengthMap.end()
              && pathLengthMap.find( fallname ) != pathLengthMap.end() );

      assert( pathVarianceMap.find( risename ) == pathVarianceMap.end()
              && pathVarianceMap.find( fallname ) == pathVarianceMap.end()
              ||    pathVarianceMap.find( risename ) != pathVarianceMap.end()
              && pathVarianceMap.find( fallname ) != pathVarianceMap.end() );
      // if already visited, just return
      if(    nomAnlys.find( risename ) != nomAnlys.end()
             && nomAnlys.find( fallname ) != nomAnlys.end() ) {
         //I am not keeping this as && so that dynamic circuits can be handled.
         assert( pathLengthMap.find( risename ) != pathLengthMap.end()
                 || pathLengthMap.find( fallname ) != pathLengthMap.end() );
         assert( pathVarianceMap.find( risename ) != pathVarianceMap.end()
                 || pathVarianceMap.find( fallname ) != pathVarianceMap.end() );

         return;
      }
   } else { //if visited, just return.
      if(    nomAnlys.find( risename ) != nomAnlys.end() ) {
         assert( pathLengthMap.find( risename ) != pathLengthMap.end() );
         assert( pathVarianceMap.find( risename ) != pathVarianceMap.end() );
         return;
      }
   }

   assert( nomAnlys.find( risename ) == nomAnlys.end()
           && nomAnlys.find( fallname ) == nomAnlys.end() );

   assert( pathLengthMap.find( risename ) == pathLengthMap.end()
           && pathLengthMap.find( fallname ) == pathLengthMap.end() );

   assert( pathVarianceMap.find( risename ) == pathVarianceMap.end()
           && pathVarianceMap.find( fallname ) == pathVarianceMap.end() );

   //cout << "Nominal Analysis on edge " << getName() << endl;
   if( isPI() ) {
      nomAnlys[risename] = getPI().getT() + getPreValue( true );
      pathLengthMap[risename].push_back( ( getPI().getT() + getPreValue( true ) ) );
      pathVarianceMap[risename].push_back( 0.0 );
      if( noTriseTfall )
         return;

      nomAnlys[fallname] = getPI().getT() + getPreValue( false );
      pathLengthMap[fallname].push_back( ( getPI().getT()  + getPreValue( false ) ) );
      pathVarianceMap[fallname].push_back( 0.0 );

      return;
   }

   // make sure that this net is output of a gate in the network
   // (since it's not a primary input
   if( driverNode == ( node * )NULL ) {
      errorReport( "The edge " + getName() + " is neither an input nor driven by any node!\n" );
   }

   // do nominal analysis for the gate whose output is this net
   // (there is only one such gate by definition of my network)
   driverNode->nominalAnalysis
   ( nomAnlys, netSelectVec, pathLengthMap, pathVarianceMap,gateDioMap, optVs,noTriseTfall );

   return;
}

void edge::addOneToCriticalNets
( map<string,double> & criticality,
  map<string,unsigned> & netSelectMap, bool rf, bool noTriseTfall ) {
   map<string,double>::iterator citr;
   map<string,unsigned>::const_iterator smitr;
   if( isPI() ) {
      //make a criticality chart for input edges as well.
      if( rf ) {
         if( ( citr = criticality.find( getMonteTRiseName() ) ) != criticality.end() )
            citr->second ++;
         else
            criticality[getMonteTRiseName()] = 1;
      } else {
         if( ( citr = criticality.find( getMonteTFallName() ) ) != criticality.end() )
            citr->second ++;
         else
            criticality[getMonteTFallName()] = 1;
      }
      return;
   }

   assert( driverNode != ( node * )NULL );

   unsigned n;

   if( rf ) {
      citr = criticality.find( getMonteTRiseName() );
      assert( citr != criticality.end() );
      citr->second ++;

      smitr = netSelectMap.find( getMonteTRiseName() );
      assert( smitr != netSelectMap.end() );
      n = smitr->second;
   } else {
      citr = criticality.find( getMonteTFallName() );
      assert( citr != criticality.end() );
      citr->second ++;

      smitr = netSelectMap.find( getMonteTFallName() );
      assert( smitr != netSelectMap.end() );
      n = smitr->second;
   }

   getDriverNode()->addOneToCriticalNets( criticality, netSelectMap, n, rf, noTriseTfall );

   return;
}

/*
 * CLASS piinfo MEMBER FUNCTION DEFINITIONS
 */

void piinfo::put( const string & s, const string & nm ) {
   if( s == "name" ) {
      if( nameB ) {
         ciropterror( "more than one name for a primary input" );
         return;
      }

      name = nm;
      nameB = true;
   } else
      assert( false );

   return;
}

void piinfo::put( const string & s, double value ) {
   if ( s == "at" ) {
      if( tB ) {
         ciropterror( "more than one starting time for a primary input" );
         return;
      }

      t = value;
      tB = true;
   } else
      assert( false );

   return;
}

void piinfo::put( const string & s, double v1, double v2 ) {
   if( s == "s" ) {
      if( slopeB ) {
         ciropterror( "more than one slope value assignment for a primary input" );
         return;
      }

      slope_r = v1;
      slope_f = v2;
      slopeB = true;
   } else
      assert( false );

   return;
}

void piinfo::put
( const string & s, gposy & cp,symbol_table & st ) {
   if( s == "c" ) {
      if( capB ) {
         ciropterror( "more than one input capacitor constraint"
                      " for a primary input" );
         return;
      }

      cap = & cp;
      symtab = & st;
      capB = true;
      capString = cap->toString( * symtab );
   } else
      assert( false );

   return;
}

/*
 * CLASS poinfo MEMBER FUNCTION DEFINITIONS
 */

void poinfo::put( const string & s, const string & nm ) {
   if( s == "name" ) {
      if( nameB ) {
         ciropterror( "more than one name for a primary output" );
         return;
      }

      name = nm;
      nameB = true;
   } else
      assert( false );

   return;
}

void poinfo::put
( const string & s, gposy & cp, symbol_table & st ) {
   if( s == "c" ) {
      if( capB ) {
         ciropterror( "more than one primary output load capacitor" );
         return;
      }

      cap = & cp;
      symtab = & st;
      capB = true;
      capString = cap->toString( * symtab );
   } else
      assert( false );

   return;
}

double poinfo::getCap( map<string,double> & vls ) {
   assert( capB );
   vector<double> vvs;

   const unsigned n = symtab->size();
   map<string,double>::const_iterator itr;

   for( unsigned i = 0; i < n; i ++ ) {
      assert( ( itr = vls.find( ( *symtab )[i] ) ) != vls.end() );
      vvs.push_back( itr->second );
   }

   return cap->evaluate( vvs );
}

/*
 * CLASS node MEMBER FUNCTION DEFINITIONS
 */

const bool node::KAPPA_ADDITIVE = true;
const bool node::KAPPA_CLIPPING = true;
const bool node::BETA_ADDITIVE  = true;
const bool node::BETA_CLIPPING  = true;
//The lower and upper bounds for changing kappa.
const double node::KAPPA_M_LWR = 0.9;
const double node::KAPPA_M_UPR = 1.1;
const double node::KAPPA_A_LWR = 0.3;
const double node::KAPPA_A_UPR = 3;

const double node::BETA_M_LWR = 0.9;
const double node::BETA_M_UPR = 1.1;
const double node::BETA_A_LWR = 0.9;
const double node::BETA_A_UPR = 1.1;

const double node::Dbeta = 0; // I dont know what this does, but
//I am keeping it deactivated by making it 0.
const double node::Dkappa = 1;
//the starting kappa needs to be 0 for the criticality to be real.
const double node::InitKappa = 1;
const double node::MaxKappa = 1;
const double node::MinCrit = 0.001;
const double node::ZeroCrit = 0.0001;
const string node::BETWEEN_NETS = "_and_";

node::node
( const string & nm,
  vector<edge *> & outedgeVec, ccc & c,
  vector<edge *> & inedgeVec,
  const string & vnm )
   : name( nm ), outEdgeVec( outedgeVec ), cc( &c ), inEdgeVec( inedgeVec ),  vname( vnm ) {
   cout << "Making a CCC node " << nm << endl;
   type = "g";
   if( !( inedgeVec.size() == cc->getNumberOfInputs() ) ) {
      ciropterror( "number of input of ccc " + cc->getName()
                   + " is " + cnvt::intToString( cc->getNumberOfInputs() ) );
      return;
   }
   if( !( outedgeVec.size() == cc->getNumberOfOutputs() ) ) {
      ciropterror( "number of input of ccc " + cc->getName()
                   + " is " + cnvt::intToString( cc->getNumberOfOutputs() ) );
      return;
   }
   duty_factor.clear();
   capIntNetAFmap.clear();
   wireIntNetAFmap.clear();
   InputCritMapRF.clear();
   InputCritMapFR.clear();
   for( unsigned i = 0; i < getNumberOfOutputs(); i ++ ) {
      InputCritMapRF[getOutEdge( i ).getName()].resize( getNumberOfInputs() );
      InputCritMapFR[getOutEdge( i ).getName()].resize( getNumberOfInputs() );
      duty_factor.push_back( -1.0 );
   }

   path_length_table.clear();
   rfSlopeRecord.clear();
   frSlopeRecord.clear();
   path_length_table.resize( getNumberOfInputs() );
   rfSlopeRecord.resize( getNumberOfInputs() );
   frSlopeRecord.resize( getNumberOfInputs() );
   for( unsigned i = 0; i < getNumberOfInputs(); i ++ ) {
      path_length_table[i].resize( getNumberOfOutputs() );
      rfSlopeRecord[i].resize( getNumberOfOutputs() );
      frSlopeRecord[i].resize( getNumberOfOutputs() );
   }
   //Give the default path length as one so that if
   //UniformKappa is set, we get 1/path_length = 1
   //while formulating the recursive delay constraints.
   for( unsigned i = 0; i < getNumberOfInputs(); i ++ )
      for( unsigned j = 0; j < getNumberOfOutputs(); j ++ ) {
         path_length_table[i][j]=1;
         rfSlopeRecord[i][j]=0;
         frSlopeRecord[i][j]=0;
      }


   CorrInputnum.clear();
   // STAT
   // initialize beta's and kappa's
   RBeta = FBeta = Dbeta;
   RKappas = new double[getNumberOfOutputs()];
   FKappas = new double[getNumberOfOutputs()];

   for( unsigned i = 0; i < getNumberOfOutputs(); i ++ )
      RKappas[i] = FKappas[i] = InitKappa;

   return;
}

node::node( const string & nm, edge & ie,
            gposy & c , symbol_table & st,
            const string & vnm )
   : name( nm ), cc( ( ccc * )NULL ), VRLCval( & c ) , symtab( st ), vname( vnm ),couple( false ) {
   cout << "Making a Capacitor node " << nm << endl;
   outEdgeVec.push_back( &ie );
   inEdgeVec.push_back( &ie );
   duty_factor.push_back( -1.0 );
   capIntNetAFmap.clear();
   CorrInputnum.clear();
   InputCritMapRF.clear();
   InputCritMapFR.clear();
   type = "c";
   RKappas = FKappas = ( double * ) NULL;
}

node::node( const string & nm, edge & ie1, edge & ie2,
            gposy & c , symbol_table & st,
            const string & vnm, bool couple )
   : name( nm ), cc( ( ccc * )NULL ), VRLCval( & c ) , symtab( st ), vname( vnm ),couple( couple ) {
   cout << "Making a Capacitor node " << nm << endl;
   //	outEdgeVec.push_back(&ie);
   inEdgeVec.push_back( &ie1 );
   inEdgeVec.push_back( &ie2 );
   duty_factor.push_back( -1.0 );
   capIntNetAFmap.clear();
   CorrInputnum.clear();
   InputCritMapRF.clear();
   InputCritMapFR.clear();
   type = "c";
   RKappas = FKappas = ( double * ) NULL;
}

node::node( const string & VRLname, edge & in,
            edge & out, const string type ,gposy & val , symbol_table & st )
   : name( VRLname ), cc( ( ccc * )NULL ), type( type ), symtab( st ), VRLCval( & val ) {
   cout << "Making a VRL node " << VRLname << endl;
   outEdgeVec.push_back( &out );
   inEdgeVec.push_back( &in );
   duty_factor.push_back( -1.0 );
   capIntNetAFmap.clear();
   CorrInputnum.clear();
   InputCritMapRF.clear();
   InputCritMapFR.clear();
   RBeta = FBeta = 0;
   RKappas = new double[getNumberOfOutputs()];
   FKappas = new double[getNumberOfOutputs()];
   for( unsigned i = 0; i < getNumberOfOutputs(); i ++ ) {
      InputCritMapRF[getOutEdge( i ).getName()].resize( getNumberOfInputs() );
      InputCritMapFR[getOutEdge( i ).getName()].resize( getNumberOfInputs() );
   }
   for( unsigned i = 0; i < getNumberOfInputs(); i ++ )
      RKappas[i] = FKappas[i] = 0;
}

node::~node() {
   delete RKappas, FKappas;
   delete VRLCval;
}

void node::setCoupling() {
   if( !isCapacitor() )
      errorReport( "Illegal attempt to set coupling variable for a non capacitor node " + getName() );
   else
      couple = true;
}

double node::getDutyFactor( unsigned no ) {
   // assert( !isPI() );
   if( duty_factor[no] == -1.0 ) {
      cout << "Using default duty factor: for node " << getName() << endl;
      return atof( ( opt_prob_generator::getDefDF() ).c_str() );
   }
   if( duty_factor[no] > 1.0 || ( duty_factor[no] != -1.0 && duty_factor[no] < 0.0 ) ) {
      errorReport( "duty factor for gate " + getName() + " is out of limits" );
      return 0;
   }
   return duty_factor[no];
}



unsigned node::getNumberOfInputs() {
   if( isCapacitor() ) {
      errorReport( "attempt to ask number of inputs for capacitor node" );
      return 0;
   }
   if( isVRL() ) {
      return 1;
   }

   assert( cc != ( ccc * )NULL );
   return cc->getNumberOfInputs();
}

unsigned node::getNumberOfOutputs() {
   if( isCapacitor() ) {
      errorReport( "attempt to ask number of outputs for capacitor node" );
      return 0;
   }
   if( isVRL() ) {
      return 1;
   }

   assert( cc != ( ccc * )NULL );
   return cc->getNumberOfOutputs();
}




void node::putCorrInputnum( unsigned int num ) {
   for( unsigned i = 0; i< CorrInputnum.size(); i ++ )
      if( CorrInputnum[i] == num ) return;

   CorrInputnum.push_back( num );
}

void node::putDutyFactor( double df , const string & out ) {
   assert( df >=0 && df <= 1 );
   for( unsigned i =0; i < getNumberOfOutputs(); i ++ ) {
      if( cc->getOutputName( i ) == out ) {
         duty_factor[i] = df;
         return;
      }
   }
   ciropterror( " The output :" + out +" does not belong to " + getName()+ " :error while recording duty factor" );

   return;
}

void node::put_path_length( unsigned ni, unsigned no, double pl ) {
   assert( ni < getNumberOfInputs() && no < getNumberOfOutputs() );
   path_length_table[ni][no] = pl;
}

void node::putIntNetAF( string netName, double af ) {
   if( af < 0.0 )
      ciropterror( "The activity factor of internal net " + netName + " in cell " + getName() + " is negative" );
   capIntNetAFmap[netName] = af;
   wireIntNetAFmap[netName] = af;
}

edge & node::getInEdge( unsigned num ) {
   assert( getNumberOfInputs() == inEdgeVec.size() );
   assert( num < inEdgeVec.size() );
   return * inEdgeVec[num];
}


edge & node::getOutEdge( unsigned num ) {
   if( isVRL() )
      return * outEdgeVec[num];
   assert( getNumberOfOutputs() == outEdgeVec.size() );
   assert( num < outEdgeVec.size() );
   return * outEdgeVec[num];
}

string node::getWNName( unsigned num ) {
   assert( num < getNumberOfInputs() );
   vector<const gposy *> Nnames =  cc->getWNNameVec( num );
   if(	Nnames.size() == 0 )
      return EMPTY_STRING;
   string list = Nnames.front()->toString( cc->getSymbolTable(),getvName(),EMPTY_STRING, EMPTY_STRING );
   for( int i = 1; i < Nnames.size(); i++ ) {
      list += " + " + Nnames[i]->toString( cc->getSymbolTable(),getvName(),EMPTY_STRING, EMPTY_STRING );
   }
   if( Nnames.size() > 1 ) list = "(" + list + ")";
   return list;

}

string node::getWPName( unsigned num ) {
   assert( num < getNumberOfInputs() );
   vector<const gposy *> Pnames = cc->getWPNameVec( num );
   if( Pnames.size() == 0 )
      return EMPTY_STRING;
   string list = Pnames.front()->toString( cc->getSymbolTable(),getvName(),EMPTY_STRING, EMPTY_STRING );
   for( int i = 1; i < Pnames.size(); i++ ) {
      list += " + " + Pnames[i]->toString( cc->getSymbolTable(),getvName(),EMPTY_STRING, EMPTY_STRING );
   }
   if( Pnames.size() > 1 ) list = "(" + list + ")";
   return list;
}

string	node::getCapIntNetEnergyStt() {
   string intEnergy = EMPTY_STRING;
   map<string,double>::iterator itr;
   map<string,double> tmp = getCapIntNetAFmap();
   for( itr = tmp.begin(); itr != tmp.end(); itr++ ) {
      double sf;
      string nodeName = getName();
      //	 	cout << "Including energy for the internal net " << itr->first << endl;
      sf = itr->second;
      //	  cout << "Reached here1" << endl;
      if( sf <= 0.0 )
         continue;
      if( getCCC().getParCap( *this,itr->first ) == EMPTY_STRING )
         ciropterror( "Warning: net " + itr->first + " in CCC " + getName() + " has an activity factor but not parasitic cap" );
      if( intEnergy == EMPTY_STRING )
         intEnergy = "LogicIntEnergy_" + nodeName + "__" + itr->first;
      else
         intEnergy += ( " + LogicIntEnergy_" + nodeName + "__" + itr->first );
   }
   //	cout << "This is the energy" << intEnergy << " YE SYE" << endl;
   return intEnergy;
}

string	node::getWireIntNetEnergyStt() {
   //	cout << "reached here" << endl;
   string intEnergy = EMPTY_STRING;
   map<string,double>::iterator itr;
   map<string,double> tmp = getWireIntNetAFmap();
   for( itr = tmp.begin(); itr != tmp.end(); itr++ ) {
      double sf;
      string nodeName = getName();
      //	 	cout << "Including energy for the internal net " << itr->first << endl;
      sf = itr->second;
      //	  cout << "Reached here1" << endl;
      if( sf <= 0.0 )
         continue;
      if( getCCC().getWireCap( *this,itr->first ) == EMPTY_STRING )
         ciropterror( "Warning: net " + itr->first + " in CCC " + getName() + " has an activity factor but no wire cap" );
      if( intEnergy == EMPTY_STRING )
         intEnergy = "WireIntEnergy_" + nodeName + "__" + itr->first;
      else
         intEnergy += ( " + WireIntEnergy_" + nodeName + "__" + itr->first );
   }
   //	cout << "This is the energy" << intEnergy << " YE SYE" << endl;
   return intEnergy;
}

/* We do not use this function as we account sparately for all the internal parasitics
string	node::getCapIntNetEnergy()
{
	double sf;
  double min_sf = atof((opt_prob_generator::getMinSF()).c_str());
	//get energy in the internal net of the CCC driving this net.
  string intEnergy = EMPTY_STRING;
	map<string,double>::iterator itr;
	map<string,double> tmp = getCapIntNetAFmap();
	for(itr = tmp.begin();itr != tmp.end(); itr++)
	{
//	 	cout << "Including energy for the internal net " << itr->first << endl;
	 	sf = itr->second;
//	  cout << "Reached here1" << endl;
	 	if(sf <= 0.0)
	 			 continue;
	 	if(sf < min_sf)
	 			 sf = min_sf;
		string tmpStt;
		if((tmpStt = getCCC().getParCap(*this,itr->first)) == EMPTY_STRING)
				 ciropterror("Warning: net " + itr->first + " in CCC " + getName() + " has an activity factor but not parasitic cap");
		if(intEnergy == EMPTY_STRING)
				 intEnergy = cnvt::doubleToString(sf) + "*(" + tmpStt + ")\n";
		else
				 intEnergy += (" + " +  cnvt::doubleToString(sf) + "*(" + tmpStt + ")\n");
	}
//	cout << "This is the energy" << intEnergy << " YE SYE" << endl;
	return intEnergy;
}
*/

void node::getCapIntNetEnergy( ostream &os, string Vdd ) {
   double sf;
   double min_sf = atof( ( opt_prob_generator::getMinSF() ).c_str() );
   //get energy in the internal net of the CCC driving this net.
   map<string,double>::iterator itr;
   map<string,double> tmp = getCapIntNetAFmap();
   string tmpStt;
   for( itr = tmp.begin(); itr != tmp.end(); itr++ ) {
      //	 	cout << "Including energy for the internal net " << itr->first << endl;
      sf = itr->second;
      //	  cout << "Reached here1" << endl;
      if( sf <= 0.0 )
         continue;
      if( sf < min_sf )
         sf = min_sf;
      if( ( tmpStt = getCCC().getParCap( *this,itr->first ) ) == EMPTY_STRING )
         ciropterror( "Warning: net " + itr->first + " in CCC " + getName() + " has an activity factor but not parasitic cap" );
      os << "LogicIntEnergy_" << getName() << "__" << itr->first << " : " << Vdd << " ^ 2 ( " << cnvt::doubleToString( sf ) << "*(" << tmpStt << ") ) < LogicIntEnergy_" << getName() << "__" + itr->first << ";" << endl;
   }
}

void node::getWireIntNetEnergy( ostream &os, string Vdd ) {
   //	cout << "reached here 1" << endl;
   double sf;
   double min_sf = atof( ( opt_prob_generator::getMinSF() ).c_str() );
   //get energy in the internal net of the CCC driving this net.
   map<string,double>::iterator itr;
   map<string,double> tmp = getWireIntNetAFmap();
   string tmpStt;
   for( itr = tmp.begin(); itr != tmp.end(); itr++ ) {
      //	 	cout << "Including energy for the internal net " << itr->first << endl;
      sf = itr->second;
      //	  cout << "Reached here1" << endl;
      if( sf <= 0.0 )
         continue;
      if( sf < min_sf )
         sf = min_sf;
      if( ( tmpStt = getCCC().getWireCap( *this,itr->first ) ) == EMPTY_STRING )
         ciropterror( "Warning: net " + itr->first + " in CCC " + getName() + " has an activity factor but not parasitic cap" );
      os << "WireIntEnergy_" << getName() << "__" << itr->first << " : " << Vdd << " ^ 2 ( " << cnvt::doubleToString( sf ) << "*(" << tmpStt << ") ) < WireIntEnergy_" << getName() << "__" + itr->first << ";" << endl;
   }
}

string node::getRCLoad
( unsigned num,opt_prob_generator & opt ) {
   if( isCapacitor() ) {
      return VRLCval->toString( symtab );
   }
   if( isVRL() ) {
      //			 cout << "reached here for " << getName() << endl;
      return outEdgeVec[0]->getRCLoad( opt );
   }

   assert( num < getNumberOfInputs() );

   // for paper
   string WPName;
   string WNName;
   string RCL;
   WPName = opt.getCPrise() + " " + getWPName( num );
   WNName = opt.getCNrise() + " " + getWNName( num );

   if( getWPName( num ) == EMPTY_STRING )
      return ( string( "(" ) + WNName + ")" );
   else if( getWNName( num ) == EMPTY_STRING )
      return ( string( "(" ) + WPName + ")" );
   else
      return ( string( "(" ) + WPName + " + " + WNName + ")" );
}

string node::getFCLoad
( unsigned num, opt_prob_generator & opt ) {
   if( isCapacitor() ) {
      return  VRLCval->toString( symtab );;
   }
   if( isVRL() ) {
      return outEdgeVec[0]->getFCLoad( opt );
   }

   assert( num < getNumberOfInputs() );

   // for paper
   string WPName;
   string WNName;
   string RCL;
   WPName = opt.getCPfall() + " " + getWPName( num );
   WNName = opt.getCNfall() + " " + getWNName( num );
   if( getWPName( num ) == EMPTY_STRING )
      return ( string( "(" ) + WNName + ")" );
   else if( getWNName( num ) == EMPTY_STRING )
      return ( string( "(" ) + WPName + ")" );
   else
      return ( string( "(" ) + WPName + " + " + WNName + ")" );
}

double node::valRCLoad
( unsigned num, map<string,double> & optVs ) {
   if( isCapacitor() ) {
      //			 cout << getName() << " is a cap with value :" << cap->toString(* symtab) << endl;
      double value = VRLCval->evaluate( symtab, EMPTY_STRING, optVs, EMPTY_STRING, 0.0 );
      //			 cout << getName() << " is a cap with return value :" << value << endl;
      return value;
   }
   if( isVRL() ) {
      //			 cout << getName() << " is a VRL with load :" << cap->toString(* symtab) << endl;
      double value = outEdgeVec[0]->valRCLoad( optVs );
      //			 cout << getName() << " is a VRL with return value :" << value << endl;
      return value;
   }

   assert( num < getNumberOfInputs() );
   //first record cap per unit width
   double cprise = atof( ( opt_prob_generator::getCPrise() ).c_str() );
   double cnrise = atof( ( opt_prob_generator::getCNrise() ).c_str() );

   vector<const gposy *> Pnames = cc->getWPNameVec( num );
   double valRCLoad =  0;
   for( int i = 0; i < Pnames.size(); i++ ) {
      valRCLoad  += cprise*Pnames[i]->evaluate( cc->getSymbolTable(),getvName(),optVs,EMPTY_STRING,0.0 );
   }
   vector<const gposy *> Nnames = cc->getWNNameVec( num );
   for( int i = 0; i < Nnames.size(); i++ ) {
      valRCLoad  += cnrise*Nnames[i]->evaluate( cc->getSymbolTable(),getvName(),optVs,EMPTY_STRING,0.0 );
   }
   return valRCLoad;
}

double node::valFCLoad
( unsigned num, map<string,double> & optVs ) {
   if( isCapacitor() ) {
      //			 cout << getName() << " is a cap with value :" << cap->toString(* symtab) << endl;
      double value = VRLCval->evaluate( symtab, EMPTY_STRING, optVs, EMPTY_STRING, 0.0 );
      //			 cout << getName() << " is a cap with return value :" << value << endl;
      return value;
   }
   if( isVRL() ) {
      //			 cout << getName() << " is a cap with value :" << cap->toString(* symtab) << endl;
      double value = outEdgeVec[0]->valFCLoad( optVs );
      //			 cout << getName() << " is a cap with return value :" << value << endl;
      return value;
   }

   assert( num < getNumberOfInputs() );


   double cpfall = atof( ( opt_prob_generator::getCPfall() ).c_str() );
   double cnfall = atof( ( opt_prob_generator::getCNfall() ).c_str() );
   vector<const gposy *> Pnames = cc->getWPNameVec( num );
   double valFCLoad =  0;

   for( int i = 0; i < Pnames.size(); i++ ) {
      valFCLoad  += cpfall*Pnames[i]->evaluate( cc->getSymbolTable(),getvName(),optVs,EMPTY_STRING,0.0 );
   }
   vector<const gposy *> Nnames = cc->getWNNameVec( num );
   for( int i = 0; i < Nnames.size(); i++ ) {
      valFCLoad  += cnfall*Nnames[i]->evaluate( cc->getSymbolTable(),getvName(),optVs,EMPTY_STRING,0.0 );
   }
   return valFCLoad;
}

string node::getEnergyCLoad( unsigned num ) {
   if( isCapacitor() )
      return VRLCval->toString( symtab );
   if( isVRL() )
      return outEdgeVec[0]->getEnergyCLoad( true );

   assert( num < getNumberOfInputs() );

   string WPName;
   string WNName;
   string RCL;
   unsigned int test = 0;
   WPName = opt_prob_generator::getEnergyCP() + " " + getWPName( num );
   WNName = opt_prob_generator::getEnergyCN() + " " + getWNName( num );

   if( getWPName( num ) == EMPTY_STRING )
      return ( string( "(" ) + WNName + ")" );
   else if( getWNName( num ) == EMPTY_STRING )
      return ( string( "(" ) + WPName + ")" );
   else
      return ( string( "(" ) + WPName + " + " + WNName + ")" );
   /*
   	return
       ( string("(")
         + cnvt::doubleToString(opt_prob_generator::getEnergyCP())
         + " " + getWPName( num ) + " + "
         + cnvt::doubleToString(opt_prob_generator::getEnergyCN())
         + " " + getWNName( num ) + ")" );
   */
}

string node::getParCap( unsigned num ) {
   if( isCapacitor() ) {
      errorReport( "Asking for Par Cap from a capacitor node" );
      return EMPTY_STRING;
   }
   if( isVRL() ) {
      errorReport( "Asking for Par Cap from a VRL node" );
      return EMPTY_STRING;
   }
   //  cout << "Getting parcap of node: " << getName() << endl;
   assert( cc->isParCapAssigned( num ) );
   return cc->getParCap( *this, num );
}

string node::getLeakPow( bool stat,unsigned num ) {
   if( isCapacitor() ) {
      errorReport( "Attempting to find the leakage of a capacitor node" );
   }
   if( isVRL() ) {
      errorReport( "Attempting to find the leakage of a VRL node" );
   }

   double df = getDutyFactor( num );
   assert( ! ( df < 0.0 ) );
   double ddf =  1 - df;
   if( stat ) {
      assert( cc->isStatLeakPowAssigned( num ) );
      if( df > 0.001 && df < 0.999 )
         return "("+cnvt::doubleToString( df ) + " (" + cc->getLeakPowNstat( *this, num )+ ") + " + cnvt::doubleToString( ddf ) + " (" + cc->getLeakPowPstat( *this, num ) + "))";
      else if( df > 0.001 )
         return "("+cnvt::doubleToString( df ) + " (" + cc->getLeakPowNstat( *this, num )+ "))";
      else
         return "(" + cnvt::doubleToString( ddf ) + " (" + cc->getLeakPowPstat( *this, num ) + "))";
   } else {
      assert( cc->isMeanLeakPowAssigned( num ) );
      if( df > 0.001 && df < 0.999 )
         return "("+cnvt::doubleToString( df ) + " (" + cc->getLeakPowNnom( *this, num )+ ") + " + cnvt::doubleToString( ddf ) + " (" + cc->getLeakPowPnom( *this, num ) + "))";
      else if( df > 0.001 )
         return "("+cnvt::doubleToString( df ) + " (" + cc->getLeakPowNnom( *this, num )+ "))";
      else
         return "(" + cnvt::doubleToString( ddf ) + " (" + cc->getLeakPowPnom( *this, num ) + "))";
   }
}

string node::getArea() {
   return getArea( EMPTY_STRING );
}

string node::getArea( const string & pre ) {
   if( isCapacitor() || isVRL() ) return EMPTY_STRING;
   return pre + cc->getArea( getName() );
}

double node::getRBeta() {
   return RBeta;
}

double node::getFBeta() {
   return FBeta;
}

double * node::getRKappas() {
   return RKappas;
}

double * node::getFKappas() {
   return FKappas;
}

double node::getPathLength( unsigned ni, unsigned no ) {
   return path_length_table[ni][no];
}

string node::getDioRF( unsigned ni, unsigned no,  opt_prob_generator & opt ) {
   //	cout << "reached here rf  for" << getName() << endl;
   if( isCapacitor() || isVRL() ) {
      errorReport( "Attempting to obtain RF Dio expressions for a Cap node" );
      return EMPTY_STRING;
   }
   if( isVRL() ) {
      errorReport( "Attempting to obtain RF Dio expressions for a VRL node" );
      return EMPTY_STRING;
   }
   if( cc->isMeanDioAssigned( ni,no ) ) {
      return cc->getDioRF( ni, no, getvName(), outEdgeVec[no]->getFCLoad( opt )  );
   } else {
      cout << "No mean RF dio model for the input " << cc->getInputName( ni ) << " to output " << cc->getOutputName( no ) << " of node " << getName() << endl;
      return EMPTY_STRING;
   }
}

string node::getDioFR( unsigned ni, unsigned no,  opt_prob_generator & opt ) {
   //	cout << "reached here fr  for" << getName() << endl;
   if( isCapacitor() ) {
      errorReport( "Attempting to obain FR Dio from a cap node" );
      return EMPTY_STRING;
   }
   if( isVRL() ) {
      errorReport( "Attempting to obain FR Dio from a VRL node" );
      return EMPTY_STRING;
   }

   if( cc->isMeanDioAssigned( ni,no ) ) {
      return cc->getDioFR( ni, no, getvName(), outEdgeVec[no]->getRCLoad( opt ) );
   } else {
      cout << "No mean FR dio model for the input " << cc->getInputName( ni ) << " to output " << cc->getOutputName( no ) << " of node " << getName()<< endl;
      return EMPTY_STRING;
   }
}

string node::getSTDRF( unsigned ni, unsigned no,  opt_prob_generator & opt )
throw( noRefException ) {
   if( isCapacitor() ) {
      errorReport( "Attempting to get STDRF for a cap node" );
      return EMPTY_STRING;
   }
   if( isVRL() ) {
      errorReport( "Attempting to obain STDRF from a VRL node" );
      return EMPTY_STRING;
   }

   return cc->getSTDRF( ni, no, getvName(), outEdgeVec[no]->getFCLoad( opt ) );
}

string node::getSTDFR( unsigned ni, unsigned no,  opt_prob_generator & opt )
throw( noRefException ) {
   if( isCapacitor() ) {
      errorReport( "Attempting to get STDFR for a cap node" );
      return EMPTY_STRING;
   }
   if( isVRL() ) {
      errorReport( "Attempting to obain STDFR from a VRL node" );
      return EMPTY_STRING;
   }


   return cc->getSTDFR( ni, no, getvName(), outEdgeVec[no]->getRCLoad( opt ) );
}

string node::getVARRF( unsigned ni, unsigned no,  opt_prob_generator & opt )
throw( noRefException ) {
   if( isCapacitor() ) {
      errorReport( "Attempting to get VARRF for a cap node" );
      return EMPTY_STRING;
   }
   if( isVRL() ) {
      errorReport( "Attempting to obain VARRF from a VRL node" );
      return EMPTY_STRING;
   }


   return cc->getVARRF( ni, no, getvName(), outEdgeVec[no]->getFCLoad( opt ) );
}

string node::getVARFR( unsigned ni, unsigned no,  opt_prob_generator & opt )
throw( noRefException ) {
   if( isCapacitor() ) {
      errorReport( "Attempting to get VARFR for a cap node" );
      return EMPTY_STRING;
   }
   if( isVRL() ) {
      errorReport( "Attempting to obain VARFR from a VRL node" );
      return EMPTY_STRING;
   }


   return cc->getVARFR( ni, no, getvName(), outEdgeVec[no]->getRCLoad( opt ) );
}

double node::meanDioRF
( unsigned ni, unsigned no,  map<string,double> & optVs ) {
   //cout << "Reached the node meanDioRF" << endl;
   if( isCapacitor() ) {
      errorReport( "Attempting to get meanDioRF value for a cap node" );
      return 0.0;
   }
   if( isVRL() ) {
      double tmp =  VRLCval->getConstant();
      if( getType() == "r" && ( ! VRLCval->isConstant() || ( VRLCval->isConstant()
                                && tmp > atof( ( opt_prob_generator::getMinRes() ).c_str() ) ) ) ) {
         double delay = VRLCval->evaluate( symtab, EMPTY_STRING, optVs, EMPTY_STRING, 0.0 ) * outEdgeVec[0]->valRCLoad( optVs );
         return delay;
      } else {
         //						errorReport("Attempting to obain meanDioFR value from a V or L node");
         return 0.0;
      }
   }
   return cc->meanDioRF( ni, no, getvName(), optVs, * outEdgeVec[no] );
}

double node::meanDioFR
( unsigned ni, unsigned no,  map<string,double> & optVs ) {
   if( isCapacitor() ) {
      errorReport( "Attempting to get meanDioFR value for a cap node" );
      return 0.0;
   }
   if( isVRL() ) {
      double tmp =  VRLCval->getConstant();
      if( getType() == "r" && ( ! VRLCval->isConstant() || ( VRLCval->isConstant()
                                && tmp > atof( ( opt_prob_generator::getMinRes() ).c_str() ) ) ) ) {
         double delay = VRLCval->evaluate( symtab, EMPTY_STRING, optVs, EMPTY_STRING, 0.0 ) * outEdgeVec[0]->valFCLoad( optVs );
         return delay;
      } else {
         //						errorReport("Attempting to obain meanDioFR value from a V or L node");
         return 0.0;
      }
   }
   return cc->meanDioFR( ni, no, getvName(), optVs, * outEdgeVec[no] );
}

double node::stdDioRF
( unsigned ni, unsigned no, map<string,double> & optVs ) {
   if( isCapacitor() ) {
      errorReport( "Attempting to get stdDioRF value for a cap node" );
      return 0.0;
   }
   if( isVRL() ) {
      cout << "Attempting to obain stdDioRF value from a VRL node : returning 0" << endl;
      return 0.0;
   }


   return cc->stdDioRF( ni, no, getvName(), optVs, * outEdgeVec[no] );
}

double node::stdDioFR
( unsigned ni, unsigned no, map<string,double> & optVs ) {
   if( isCapacitor() ) {
      errorReport( "Attempting to get stdDioFR value for a cap node" );
      return 0.0;
   }
   if( isVRL() ) {
      cout << "Attempting to obain stdDioRF value from a VRL node: returning 0" << endl;
      return 0.0;
   }
   return cc->stdDioFR( ni, no, getvName(), optVs, * outEdgeVec[no] );
}

double node::varDioRF
( unsigned ni, unsigned no, map<string,double> & optVs ) {
   if( isCapacitor() ) {
      errorReport( "Attempting to get varDioRF value for a cap node" );
      return 0.0;
   }
   if( isVRL() ) {
      cout << "Attempting to obain varDioRF value from a VRL : node returning 0" << endl;
      return 0.0;
   }
   return cc->varDioRF( ni, no, getvName(), optVs, * outEdgeVec[no] );
}

double node::varDioFR
( unsigned ni, unsigned no, map<string,double> & optVs ) {
   if( isCapacitor() ) {
      errorReport( "Attempting to get varDioFR value for a cap node" );
      return 0.0;
   }
   if( isVRL() ) {
      cout << "Attempting to obain varDioRF value from a VRL node : returning 0" << endl;
      return 0.0;
   }
   return cc->varDioFR( ni, no, getvName(), optVs, * outEdgeVec[no] );
}

void node::print() {
   if( cc == ( ccc * )NULL ) {
      cout << "node name = " << name
           << ", c_value = " << VRLCval
           << ", input edge = " << inEdgeVec[0]->getName()
           << endl;

      return;
   }

   cout << "node name = " << name;
   cout << ", ccc type = " << cc->getName();

   for( int i = 0; i < inEdgeVec.size(); i ++ )
      for( int j = 0; j < outEdgeVec.size(); j ++ )
         cout << ", output= "<< outEdgeVec[i]->getName() << ", input=" << inEdgeVec[i]->getName()<< endl;
   cout << endl;
}

// OPTIMIZATION & MONTECARLO

void node::setKappaBeta( double kappa, double beta ) {
   if( isCapacitor() ) {
      errorReport( "Attempting to set Kappa and Beta value for a cap node" );
   }
   if( isVRL() ) {
      //set VRl node kappa and beta to 0
      return;
      //			 errorReport("Attempting to set Kappa and Beta value from a VRL node");
   }

   RBeta = FBeta = beta;

   for( unsigned i = 0; i < getNumberOfOutputs(); i ++ )
      RKappas[i] = FKappas[i] = kappa;

   return;
}

void node::montecarlo
( map<string,ProbDist *> & mc,
  map<string,vector<unsigned> > & netSlctMap,
  const string & dist, unsigned N, double p,
  map<string,double> & optVs, bool noTriseTfall ) {
   if( isCapacitor() ) assert( false );

   map<string,ProbDist *>::const_iterator riseItr, fallItr;
   vector<ProbDist *> riseDists, fallDists;
   ProbDist * tmpPD;
   vector<double> Rt90s, Ft90s;

   for( unsigned no = 0; no < outEdgeVec.size(); no ++ ) {
      for( unsigned ni = 0; ni < inEdgeVec.size(); ni ++ ) {
         if( !isVRL() && !getCCC().isMeanDioAssigned( ni,no ) ) continue;
         double mean, std, t90; //the last variable is the quantile time.

         inEdgeVec[ni]->montecarlo( mc,netSlctMap,dist,N,p,optVs,noTriseTfall );

         string rfname = getRFInternalNetName( ni, no );
         if( mc.find( rfname ) != mc.end() )
            errorReport( "Monte carlo analysis for RF transition from input " + getCCC().getInputName( ni ) + " to " + getCCC().getOutputName( no ) + " in node " + getName() + " is repeated" );

         // montecarlo for "rise to fall"
         assert( ( riseItr=mc.find( inEdgeVec[ni]->getMonteTRiseName() ) )!=mc.end() );

         mean = meanDioRF( ni,no,optVs ) + inEdgeVec[ni]->getPreValue( true );

         std  = stdDioRF ( ni,no,optVs );

         tmpPD = &( ( * riseItr->second ) + ProbDist( dist,N,mean,std ) );
         mc[rfname] = tmpPD;
         fallDists.push_back( tmpPD );

         // update kappas
         t90 = riseItr->second->getPercentilePoint( p );
         //      updateKappa( RKappas[no], tmpPD->getPercentilePoint(p), t90, mean, std );
         Rt90s.push_back( t90 );
         //			if(isVRL()) cout << "Reached here7" << endl;


         if( noTriseTfall ) //In this case, we care about only RF transition delay, This delay is then
            continue;  //set as the Trise delay of the output. That is in this function below.


         string frname = getFRInternalNetName( ni, no );
         if( mc.find( frname ) != mc.end() )
            errorReport( "Monte carlo analysis for FR transition from input " + getCCC().getInputName( ni ) + " to " + getCCC().getOutputName( no ) + " in node " + getName() + " is repeated" );

         // montecarlo for "fall to rise"
         assert( ( fallItr = mc.find( inEdgeVec[ni]->getMonteTFallName() ) ) != mc.end() );

         mean = meanDioFR( ni,no,optVs ) + inEdgeVec[ni]->getPreValue( false );
         std  = stdDioFR ( ni,no,optVs );

         tmpPD = &( ( * fallItr->second ) + ProbDist( dist,N,mean,std ) );
         mc[frname] = tmpPD;
         riseDists.push_back( tmpPD );

         // update kappas
         t90 = fallItr->second->getPercentilePoint( p );
         //      updateKappa( FKappas[no], tmpPD->getPercentilePoint(p), t90, mean, std );
         Ft90s.push_back( t90 );
      }

      // calculate distribution for falling output net
      string ofName = outEdgeVec[no]->getMonteTFallName();
      if( noTriseTfall ) //In this case the convention is to label the timing of all nets as Trise
         ofName = outEdgeVec[no]->getMonteTRiseName();
      // assertions to check if these have been calculated already.
      assert( mc.find( ofName ) == mc.end() );
      assert( netSlctMap.find( ofName ) == netSlctMap.end() );

      vector<unsigned> & fNSM = netSlctMap[ofName];
      tmpPD = ProbDist::createMaxProbDist( fNSM, fallDists );
      vector<double> & fCrit = InputCritMapRF[outEdgeVec[no]->getName()];
      tmpPD = ProbDist::createMaxProbDistwithCriticality( fCrit, fallDists );
      mc[ofName] = tmpPD;
      // update beta
      //    updateBeta( RBeta, tmpPD->getPercentilePoint(p), cnvt::max(Rt90s) );

      if( noTriseTfall )
         continue;

      // calculate distribution for rising output net
      const string & orName = outEdgeVec[no]->getMonteTRiseName();
      // assertions to check if these have been calculated already.
      assert( mc.find( orName ) == mc.end() );
      assert( netSlctMap.find( orName ) == netSlctMap.end() );

      vector<unsigned> & rNSM = netSlctMap[orName];
      tmpPD = ProbDist::createMaxProbDist( rNSM, riseDists );
      vector<double> & rCrit = InputCritMapFR[outEdgeVec[no]->getName()];
      tmpPD = ProbDist::createMaxProbDistwithCriticality( rCrit, riseDists );
      mc[orName] = tmpPD;

      // update beta
      //    updateBeta( FBeta, tmpPD->getPercentilePoint(p), cnvt::max(Ft90s) );
   }
   return;
}

void node::montecarlo
( map<string,ProbDist *> & mc,
  map<string,vector<unsigned> > & netSlctMap,
  map<string,ProbDist *> & gatePDMap,
  const monte_carlo & monte, double p,
  map<string,double> & optVs, bool noTriseTfall ) {
   cout << "Warning: The noRiseFallTiming option is not available for this kind of monte carlo analysis. \n Please check to see that this is not what you want!" << endl;
   assert( !monte.isIndependent() );

   if( isCapacitor() ) assert( false );

   map<string,ProbDist *>::const_iterator riseItr, fallItr;
   vector<ProbDist *> riseDists, fallDists;
   ProbDist * tmpPD;
   vector<double> Rt90s, Ft90s;

   ProbDist * PD;
   double numRelGates, depFactor;
   for( unsigned no = 0; no < outEdgeVec.size(); no ++ ) {
      if( monte.getDependencyType() == "type1" ) {
         ProbDist * mainPD;
         vector<ProbDist *> relPDs;
         map<string,ProbDist *>::const_iterator gitr;
         depFactor = monte.getDependencyFactor();

         gitr = gatePDMap.find( getName() );
         assert( gitr != gatePDMap.end() );
         mainPD = gitr->second;

         if( !outEdgeVec[no]->isPO() ) {
            unsigned num = outEdgeVec[no]->getNumFanoutNodes();
            assert( num > 0 );

            for( unsigned i = 0; i < num; i ++ ) {
               gitr = gatePDMap.find( outEdgeVec[no]->getFanoutNode( i ).getName() );
               assert( gitr != gatePDMap.end() );
               relPDs.push_back( gitr->second );
            }
         }

         for( unsigned i = 0; i < inEdgeVec.size(); i ++ ) {
            const edge & tempEdge = * inEdgeVec[i];

            if( !tempEdge.isPI() && !tempEdge.isGNDorVDD() ) {
               gitr = gatePDMap.find( tempEdge.getDriverNode()->getName() );
               assert( gitr != gatePDMap.end() );
               relPDs.push_back( gitr->second );
            }
         }

         PD = ProbDist::createLinCom( mainPD, 1.0, relPDs, depFactor );
         numRelGates = relPDs.size();
      } else
         assert( false );

      for( unsigned ni = 0; ni < inEdgeVec.size(); ni ++ ) {
         if( !isVRL() && !getCCC().isMeanDioAssigned( ni,no ) ) continue;
         double mean, std, t90;

         inEdgeVec[ni]->montecarlo( mc,netSlctMap,gatePDMap,monte,p,optVs,noTriseTfall );

         string rfname = getRFInternalNetName( ni, no );
         string frname = getFRInternalNetName( ni, no );

         //    assert( mc.find(rfname) == mc.end() && mc.find(frname) == mc.end() );
         if( mc.find( rfname ) != mc.end() && mc.find( frname ) != mc.end() )
            continue;

         // montecarlo for "rise to fall"
         assert( ( riseItr=mc.find( inEdgeVec[ni]->getMonteTRiseName() ) )!=mc.end() );

         mean = meanDioRF( ni,no,optVs ) + inEdgeVec[ni]->getPreValue( true );
         std  = stdDioRF ( ni,no,optVs );

         std /= sqrt( 1.0 + numRelGates * depFactor * depFactor );

         tmpPD = &( ( * riseItr->second ) + ProbDist( *PD,mean,std ) );
         mc[rfname] = tmpPD;
         fallDists.push_back( tmpPD );

         // update kappas
         t90 = riseItr->second->getPercentilePoint( p );
         //    updateKappa( RKappas[no], tmpPD->getPercentilePoint(p), t90, mean, std );
         Rt90s.push_back( t90 );


         // montecarlo for "fall to rise"
         assert( ( fallItr = mc.find( inEdgeVec[ni]->getMonteTFallName() ) ) != mc.end() );

         mean = meanDioFR( ni,no,optVs ) + inEdgeVec[ni]->getPreValue( false );
         std = stdDioFR ( ni,no,optVs );

         std /= sqrt( 1.0 + numRelGates * depFactor * depFactor );

         tmpPD = &( ( * fallItr->second ) + ProbDist( *PD,mean,std ) );
         mc[frname] = tmpPD;
         riseDists.push_back( tmpPD );

         // update kappas
         t90 = fallItr->second->getPercentilePoint( p );
         //    updateKappa( FKappas[no], tmpPD->getPercentilePoint(p), t90, mean, std );
         Ft90s.push_back( t90 );
      }

      const string & orName = outEdgeVec[no]->getMonteTRiseName();
      const string & ofName = outEdgeVec[no]->getMonteTFallName();

      // assertions
      assert( mc.find( orName ) == mc.end() );
      assert( mc.find( ofName ) == mc.end() );

      assert( netSlctMap.find( orName ) == netSlctMap.end() );
      assert( netSlctMap.find( ofName ) == netSlctMap.end() );

      // calculate distribution for falling output net

      vector<unsigned> & fNSM = netSlctMap[ofName];
      tmpPD = ProbDist::createMaxProbDist( fNSM, fallDists );
      mc[ofName] = tmpPD;

      // update beta
      updateBeta( RBeta, tmpPD->getPercentilePoint( p ), cnvt::max( Rt90s ) );


      // calculate distribution for rising output net
      vector<unsigned> & rNSM = netSlctMap[orName];
      tmpPD = ProbDist::createMaxProbDist( rNSM, riseDists );
      mc[orName] = tmpPD;

      // update beta
      updateBeta( FBeta, tmpPD->getPercentilePoint( p ), cnvt::max( Ft90s ) );


      delete PD;
   }

   return;
}

void node::nominalAnalysis
( map<string,double> & nomAnlys,
  map<string,unsigned> & netSelectVec,
  map<string,vector<double> > & pathLengthMap,
  map<string,vector<double> > & pathVarianceMap,
  map<string,double > & gateDioMap,
  map<string,double> & optVs, bool noTriseTfall ) {
   //cout << "Nominal Analysis on node " << getName() << endl;
   if( isCapacitor() ) assert( false );

   for( unsigned no=0; no < outEdgeVec.size(); no ++ ) {
      map<string,double>::const_iterator riseItr, fallItr;
      map<string,vector<double> >::const_iterator risePItr, fallPItr;
      map<string,vector<double> >::const_iterator riseVItr, fallVItr;
      vector<double> riseVals, fallVals;

      const string nameRO = outEdgeVec[no]->getMonteTRiseName();
      const string nameFO = outEdgeVec[no]->getMonteTFallName();
      assert( pathLengthMap.find( nameRO ) == pathLengthMap.end() );
      assert( pathLengthMap.find( nameFO ) == pathLengthMap.end() );

      assert( pathVarianceMap.find( nameRO ) == pathVarianceMap.end() );
      assert( pathVarianceMap.find( nameFO ) == pathVarianceMap.end() );

      vector<double> & riseOutPathLengths = pathLengthMap[nameRO];
      vector<double> & riseOutPathVariances = pathVarianceMap[nameRO];


      vector<double> & fallOutPathLengths = pathLengthMap[nameFO];
      vector<double> & fallOutPathVariances = pathVarianceMap[nameFO];
      for( unsigned ni = 0; ni < inEdgeVec.size(); ni ++ ) {
         //check for dio availability but only for CCCs.
         if( !isVRL()  && !getCCC().isMeanDioAssigned( ni,no ) ) continue;
         double dly;
         double var;

         // do nominal analysis for each input net first
         inEdgeVec[ni]->nominalAnalysis
         ( nomAnlys,netSelectVec,pathLengthMap,pathVarianceMap,gateDioMap,optVs,noTriseTfall );

         string dioRF;
         string dioFR;
         if( getType() == "v" || getType() == "l"
               || ( getType() == "r"
                    && VRLCval->isConstant()
                    &&  VRLCval->getConstant() < atof( ( opt_prob_generator::getMinRes() ).c_str() ) ) ) {
            //the names are by the output rise and fall transistion
            dioRF = getName()+"inToOut_FF";
            dioFR = getName()+"inToOut_RR";
         } else {
            dioRF = getName()+"__"+cc->getInputName( ni )+"__"+cc->getOutputName( no )+"__RF";
            dioFR = getName()+"__"+cc->getInputName( ni )+"__"+cc->getOutputName( no )+"__FR";
         }
         assert( gateDioMap.find( dioRF ) == gateDioMap.end() );
         assert( gateDioMap.find( dioFR ) == gateDioMap.end() );

         /*
          * "rise to fall"
          */

         const string nameRI = inEdgeVec[ni]->getMonteTRiseName();
         const string rfname = getRFInternalNetName( ni, no );

         // make sure that NO nominal analysis has been done for this dio
         // of the current gate (node).
         if( nomAnlys.find( rfname ) != nomAnlys.end() )
            continue;

         // and make sure that nominal analysis HAS been done for this input net
         assert( ( riseItr = nomAnlys.find( nameRI ) ) != nomAnlys.end() );
         gateDioMap[dioRF] = meanDioRF( ni,no,optVs );
         //	cout << "Reached here in nom analysis1" << endl;
         dly = gateDioMap[dioRF] ; // + inEdgeVec[ni]->getPreValue(true) no needed since inputs are now taken separately.
         //	cout << "Reached here in nom analysis2" << endl;
         var = varDioRF ( ni,no,optVs );
         fallVals.push_back( ( nomAnlys[rfname] = ( riseItr->second ) + dly ) );

         // add mean and variance of the delay to corresponding element of
         // the map
         assert( ( risePItr=pathLengthMap.find( nameRI ) )   != pathLengthMap.end() );
         assert( ( riseVItr=pathVarianceMap.find( nameRI ) ) != pathVarianceMap.end() );
         assert( risePItr->second.size() == riseVItr->second.size() );
         //			cout << "Reached here in nom analysis" << endl;
         for( unsigned i = 0; i < risePItr->second.size(); i ++ ) {
            if( noTriseTfall ) {
               riseOutPathLengths.push_back( risePItr->second[i] + dly ); //enter values in correct pointers
               riseOutPathVariances.push_back( riseVItr->second[i] + var );
               continue;
            }
            fallOutPathLengths.push_back( risePItr->second[i] + dly );
            fallOutPathVariances.push_back( riseVItr->second[i] + var );
         }


         if( noTriseTfall ) //In this option every delay is an rf delay and the timing on the net is always .Trise
            continue;
         /*
          * "fall to rise"
          */

         const string nameFI = inEdgeVec[ni]->getMonteTFallName();
         const string frname = getFRInternalNetName( ni, no );

         // make sure that NO nominal analysis has been done for this dio
         // of the current gate (node).
         if( nomAnlys.find( frname ) != nomAnlys.end() )
            continue;

         // and make sure that nominal analysis HAS been done for this input net
         assert( ( fallItr = nomAnlys.find( nameFI ) ) != nomAnlys.end() );
         gateDioMap[dioFR] = meanDioFR( ni,no,optVs );
         dly = gateDioMap[dioFR] ; //+ inEdgeVec[ni]->getPreValue(false)  not needed as included in the input.;
         //			cout<< getName() << ": dly FR:" << dly << endl;
         //dly = meanDioFR(ni,getName(),optVs,outEdge.valRCLoad(optVs))
         //    + inEdgeVec[ni]->getPreValue(false);
         var = varDioFR ( ni,no,optVs );
         riseVals.push_back( ( nomAnlys[frname] = ( fallItr->second ) + dly ) );

         // add mean and variance of the delay to corresponding element of
         // the map
         assert( ( fallPItr=pathLengthMap.find( nameFI ) )   != pathLengthMap.end() );
         assert( ( fallVItr=pathVarianceMap.find( nameFI ) ) != pathVarianceMap.end() );
         assert( fallPItr->second.size() == fallVItr->second.size() );
         for( unsigned i = 0; i < fallPItr->second.size(); i ++ ) {
            riseOutPathLengths.push_back( fallPItr->second[i] + dly );
            riseOutPathVariances.push_back( riseVItr->second[i] + var );
         }
      }

      if( noTriseTfall ) {
         assert( nomAnlys.find( nameRO ) == nomAnlys.end() );
         assert( netSelectVec.find( nameRO ) == netSelectVec.end() );
         nomAnlys[nameRO] = cnvt::max( netSelectVec[nameRO], fallVals );
         continue;
      }

      // rise to fall
      assert( nomAnlys.find( nameFO ) == nomAnlys.end() );
      assert( netSelectVec.find( nameFO ) == netSelectVec.end() );
      nomAnlys[nameFO] = cnvt::max( netSelectVec[nameFO], fallVals );


      // fall to rise
      assert( nomAnlys.find( nameRO ) == nomAnlys.end() );
      nomAnlys[nameRO] = cnvt::max( netSelectVec[nameRO], riseVals );
   }
   return;
}

void node::dioAnalysis( map<string,double > & gateDioMap,
                        map<string,double> & optVs, bool noTriseTfall ) {
   cout <<  "Not using the noTriseFall option yet" << endl;
   //cout << "Dio Analysis on node " << getName() << endl;
   if( isCapacitor() ) assert( false );

   for( unsigned no=0; no < outEdgeVec.size(); no ++ ) {
      map<string,double>::const_iterator riseItr, fallItr;
      map<string,vector<double> >::const_iterator risePItr, fallPItr;
      map<string,vector<double> >::const_iterator riseVItr, fallVItr;
      vector<double> riseVals, fallVals;

      for( unsigned ni = 0; ni < inEdgeVec.size(); ni ++ ) {
         //check for dio availability but only for CCCs.
         if( !isVRL()  && !getCCC().isMeanDioAssigned( ni,no ) )
            continue;
         double dly;
         double var;

         string dioRF;
         string dioFR;
         if( getType() == "v" || getType() == "l"
               || ( getType() == "r"
                    && VRLCval->isConstant()
                    &&  VRLCval->getConstant() < atof( ( opt_prob_generator::getMinRes() ).c_str() ) ) ) {
            //the names are by the output rise and fall transistion
            dioRF = getName()+"inToOut_FF";
            dioFR = getName()+"inToOut_RR";
         } else {
            dioRF = getName()+"__"+cc->getInputName( ni )+"__"+cc->getOutputName( no )+"__RF";
            dioFR = getName()+"__"+cc->getInputName( ni )+"__"+cc->getOutputName( no )+"__FR";
         }
         assert( gateDioMap.find( dioRF ) == gateDioMap.end() );
         assert( gateDioMap.find( dioFR ) == gateDioMap.end() );

         /*
          * "rise to fall"
          */
         //			cout << "Reached here in nom analysis" << endl;
         /*
         		if(!isVRL())
         			cout << "Reached here in nom analysis0 " << getCCC().getInputName(ni) << " to " << getCCC().getOutputName(no) << endl;
         		else
         				 cout << "VRL node" << endl;
         	*/
         gateDioMap[dioRF] = meanDioRF( ni,no,optVs );
         //		cout << "Reached here in nom analysis1" << endl;
         dly = gateDioMap[dioRF] ; // + inEdgeVec[ni]->getPreValue(true) no needed since inputs are now taken separately.
         //		cout << "Reached here in nom analysis2" << endl;
         //			cout<< getName() << ": dly RF:" << dly << endl;
         //dly = meanDioRF(ni,getName(),optVs,outEdge.valFCLoad(optVs))
         //    + inEdgeVec[ni]->getPreValue(true);
         var = varDioRF ( ni,no,optVs );

         /*
          * "fall to rise"
          */

         gateDioMap[dioFR] = meanDioFR( ni,no,optVs );
         dly = gateDioMap[dioFR] ; //+ inEdgeVec[ni]->getPreValue(false)  not needed as included in the input.;
         //			cout<< getName() << ": dly FR:" << dly << endl;
         //dly = meanDioFR(ni,getName(),optVs,outEdge.valRCLoad(optVs))
         //    + inEdgeVec[ni]->getPreValue(false);
         var = varDioFR ( ni,no,optVs );

      }

   }
   return;
}


void node::addOneToCriticalNets( map<string,double> & criticality,
                                 map<string,unsigned> & netSelectMap,
                                 unsigned n, bool rf, bool noTriseTfall ) {
   assert( n < getNumberOfInputs() );

   if( noTriseTfall )
      getInEdge( n ).addOneToCriticalNets( criticality,netSelectMap, rf, noTriseTfall );
   else
      getInEdge( n ).addOneToCriticalNets( criticality,netSelectMap, !rf, noTriseTfall );

   return;
}

bool node::skipDelayExpression( unsigned num ) {
   if( getOutEdge( num ).getNumFanoutNodes() == 1
         && ( !getOutEdge( num ).isPO() )
         && getOutEdge( num ).getFanoutNode( 0 ).isCapacitor() ) {
      cout << "The output of CCC " << getName() << " is not a primary output but has a capacitor. Assuming net " << getOutEdge( num ).getName() << " as dummy primary output" << endl;
      return false;
   }
   if( getOutEdge( num ).getNumFanoutNodes() == 1
         && ( ! getOutEdge( num ).isPO() )
         && ( ( ! getOutEdge( num ).getFanoutNode( 0 ).isVRL() )
              && getOutEdge( num ).getFanoutNode( 0 ).getCCC().getInputName( getOutEdge( num ).getInCCCNum( 0 ) ).substr( 0,3 ) == KEEPER_INPUT
              && getOutEdge( num ).getFanoutNode( 0 ).getCCC().getInputName( getOutEdge( num ).getInCCCNum( 0 ) ).substr( 0,4 ) != DUM_KEEPER_INPUT
              ||
              getOutEdge( num ).getFanoutNode( 0 ).isVRL()
              && getOutEdge( num ).getFanoutNode( 0 ).getOutEdge( 0 ).getNumFanoutNodes() == 1
              && getOutEdge( num ).getFanoutNode( 0 ).getOutEdge( 0 ).getFanoutNode( 0 ).getCCC().getInputName( getOutEdge( num ).getInCCCNum( 0 ) ).substr( 0,3 ) == KEEPER_INPUT
              && getOutEdge( num ).getFanoutNode( 0 ).getOutEdge( 0 ).getFanoutNode( 0 ).getCCC().getInputName( getOutEdge( num ).getInCCCNum( 0 ) ).substr( 0,4 ) != DUM_KEEPER_INPUT ) )
      return true;
   else
      return false;
}

ostream & node::delayConstraintToOstream
( ostream & os, opt_prob_generator & opt, bool stt ) {
   //  This is the start for the recursive gate delay constraints.
   if( isCapacitor() )
      return os;
   if( isVRL() )
      //			 if(inEdgeVec[0]->isGNDorVDD() || outEdgeVec[0]->isGNDorVDD())
      //						return os;
      //			 else
   {
      //						if(!skipDelayExpression(0))
      if( !getOutEdge( 0 ).skipEdgeTiming() ) {
         //if its a resistors and and the resistance is an expression or is a number
         //that is significant to be considered..consider it, else its as good as a
         //voltage source.
         double tmp =  VRLCval->getConstant();
         if( getType() == "r" && ( ! VRLCval->isConstant() || ( VRLCval->isConstant()
                                   && tmp > atof( ( opt.getMinRes() ).c_str() ) ) ) ) {
            if( opt.isNoRiseFall() )
               os << "RECURSIVE__" << getName() << "_RR : " <<
                  inEdgeVec[0]->getTRiseName() <<  " + " << " 1e-3 *( " <<
                  VRLCval->toString( symtab ) << " )* " << getRCLoad( 0,opt ) <<
                  " < " << outEdgeVec[0]->getTRiseName() << ";" << endl;
            else {
               os << "RECURSIVE__" << getName() << "_RR : " <<
                  inEdgeVec[0]->getTRiseName() <<  " + " << " 1e-3 *( " <<
                  VRLCval->toString( symtab ) << " )* " << getRCLoad( 0,opt ) <<
                  " < " << outEdgeVec[0]->getTRiseName() << ";" << endl;

               os << "RECURSIVE__" << getName() << "_FF : " <<
                  inEdgeVec[0]->getTFallName() << " + " << " 1e-3 *(" <<
                  VRLCval->toString( symtab ) << " )* " << getFCLoad( 0,opt ) <<
                  " < " << outEdgeVec[0]->getTFallName() << ";" << endl;
            }
         } else {
            if( opt.isNoRiseFall() )
               os << "RECURSIVE__" << getName() << "_RR : " << inEdgeVec[0]->getTRiseName() << " < " << outEdgeVec[0]->getTRiseName() << ";" << endl;
            else {
               os << "RECURSIVE__" << getName() << "_RR : " << inEdgeVec[0]->getTRiseName() << " < " << outEdgeVec[0]->getTRiseName() << ";" << endl;
               os << "RECURSIVE__" << getName() << "_FF : " << inEdgeVec[0]->getTFallName() << " < " << outEdgeVec[0]->getTFallName() << ";" << endl;
            }
         }
      }
      return os;
   }


   // cout << "this delay constraint generator is used in nominal case\n" << endl;
   // normal behavior
   string delay;
   for( unsigned i = 0; i < getNumberOfInputs(); i ++ ) {
      // If this CCC has a keeper input, dont include that delay.
      if( getCCC().getInputName( i ).substr( 0,3 ) == KEEPER_INPUT ) {
         cout << "The input " << getCCC().getInputName( i ) << " of node " << getName() <<" is keeper input - ignored for timing" << endl;
         continue;
      }
      //if the CCC's input is connected to Vdd or Gnd, continue with a warning
      if( inEdgeVec[i]->isGNDorVDD() ) {
         cout << "The input " << getCCC().getInputName( i ) << " of node " << getName() <<" is connected to gnd or Vdd - considered as input with infinite capacitance" << endl;
      }
      for( unsigned j = 0; j < getNumberOfOutputs(); j ++ ) {
         // If this output is going to a keeper input only, dont produce the
         // constrains for this section. But if that keeper is not a real keeper,
         // i.e used only for a dummy reason, then it will have the name DUM_KEEPER_INPUT.
         // In that case, produce it.
         // Here the node is usually an inverter and needs to be sized minimum
         // Of course the sizing of PMOS in it can be ensured to be twice/thrice
         // NMOS by using a local constraint for such keeper feedback inverters.

         //Also take care to go thru the voltage sources, resistors and inductors that might be
         //in the way. This is done by the bool function. writeDelayExpression.
         //					 if(skipDelayExpression(j))
         if( getOutEdge( j ).skipEdgeTiming() ) {
            cout << "The node " << getName() <<" feeds a dummy input only, hence ignored for timing" << endl;
            continue;
         }
         if( getCCC().isMeanDioAssigned( i,j ) ) {
            if( opt.isNoRiseFall() ) {
               if( getWNName( i ) != EMPTY_STRING && getDioRF( i,j,opt ) != EMPTY_STRING && getDioRF( i,j,opt ) != ERROR_STRING ) {
                  cout << " --- Simple Recursion constraints for " << getName() << " input: " << getCCC().getInputName( i ) << " output: " << getCCC().getOutputName( j ) << endl;
                  os << "RECURSION__" << getName() << "__" << getCCC().getInputName( i ) << "To" << getCCC().getOutputName( j ) << ": ";
                  rfDelayConstraintToOstream( os,i,j,opt,stt );
               }
            } else {
               if( getWNName( i ) != EMPTY_STRING && getDioRF( i,j,opt ) != EMPTY_STRING && getDioRF( i,j,opt ) != ERROR_STRING ) {
                  cout << " --- RF Recursion constraints for " << getName() << " input: " << getCCC().getInputName( i ) << " output: " << getCCC().getOutputName( j ) << endl;
                  os << "RECURSION__" << getName() << "__" << getCCC().getInputName( i ) << "To" << getCCC().getOutputName( j ) << "_RF : ";
                  rfDelayConstraintToOstream( os,i,j,opt,stt );
               }
               if( getWPName( i ) != EMPTY_STRING && getDioFR( i,j,opt ) != EMPTY_STRING && getDioFR( i,j,opt ) != ERROR_STRING ) {
                  cout << " --- FR Recursion constraints for " << getName() << " input: " << getCCC().getInputName( i ) << " output: " << getCCC().getOutputName( j ) << endl;
                  os << "RECURSION__" << getName() << "__" << getCCC().getInputName( i ) << "To" << getCCC().getOutputName( j ) <<"_FR : ";
                  frDelayConstraintToOstream( os,i,j,opt,stt );
               }
            }
         }
      }
   }
   return os;
}


ostream & node::delayConstraintToOstream
( ostream & os, opt_prob_generator & opt, opt_spec & osp ) {
   if( isCapacitor() || isVRL() ) return os;

   // normal behavior
   for( unsigned i = 0; i < getNumberOfInputs(); i ++ ) {
      if( getCCC().getInputName( i ).substr( 0,3 ) == KEEPER_INPUT ) continue;
      if( inEdgeVec[i]->isGNDorVDD() ) {
         cout << "The input " << getCCC().getInputName( i ) << " of node " << getName() <<" is connected to gnd or Vdd" << endl;
      }

      for( unsigned j = 0; j < getNumberOfOutputs(); j ++ )
         if( getCCC().isMeanDioAssigned( i,j ) ) {
            if( opt.isNoRiseFall() ) {
               if( getWNName( i ) != EMPTY_STRING ) {
                  os << "RECURSION__" << getName() << "__" << getCCC().getInputName( i ) << "To" << getCCC().getOutputName( j ) << ": " ;
                  delayConstraintToOstream( os,i,j,opt,true,osp );
               }
            } else {
               if( getWNName( i ) != EMPTY_STRING ) {
                  os << "RECURSION__" << getName() << "__" << getCCC().getInputName( i ) << "To" << getCCC().getOutputName( j ) << "_RF : " ;
                  delayConstraintToOstream( os,i,j,opt,true,osp );
               }
               if( getWPName( i ) != EMPTY_STRING ) {
                  os << "RECURSION__" << getName() << "__" << getCCC().getInputName( i ) << "To" << getCCC().getOutputName( j ) <<"_FR : " ;
                  delayConstraintToOstream( os,i,j,opt,false,osp );
               }
            }
         }
   }
   return os;
}

ostream & node::delayConstraintToOstream( ostream & os,
      unsigned ni, unsigned no,opt_prob_generator & opt, bool rf, bool stt ) {
   //  cout << "I reach input " << ni << " and output " << no << " of " << getName()  << endl;
   double k;
   assert( ni < getNumberOfInputs() );

   //  double val = inEdgeVec[ni]->getPreValue(rf);
   string stdRF;
   string stdRFcnstr;
   string stdFR;
   string stdFRcnstr;
   string meanRF;
   string meanRFcnstr;
   string meanFR;
   string meanFRcnstr;

   // if( val != 0.0 ) os << cnvt::doubleToString(val) << " + ";
   //STANDARD MEAN AND STD DEFINITIONS THAT GO FOR EACH GATE, And the corresponging constraints.///
   if( rf ) {
      if( stt ) {
         stdRF = "std"+getName()+"__"+getCCC().getInputName( ni )+"To"+getCCC().getOutputName( no )+"_RF";
         stdRFcnstr = "STD__" + getName() + "__" + getCCC().getInputName( ni ) + "To" + getCCC().getOutputName( no ) + "_RF : " + getSTDRF( ni,no, opt ) + " < " + stdRF + ";";
      }
      meanRF = "mean"+getName()+"__"+getCCC().getInputName( ni )+"To"+getCCC().getOutputName( no )+"_RF";
      meanRFcnstr = "MEAN__" + getName() + "__" + getCCC().getInputName( ni ) + "To" + getCCC().getOutputName( no ) + "_RF : " + getDioRF( ni,no, opt ) + " < " + meanRF + ";";
   } else {
      if( stt ) {
         stdFR = "std"+getName()+"__"+getCCC().getInputName( ni )+"To"+getCCC().getOutputName( no )+"_FR";
         stdFRcnstr = "STD__" + getName() + "__" + getCCC().getInputName( ni ) + "To" + getCCC().getOutputName( no ) + "_FR : " + getSTDFR( ni,no, opt ) + " < " + stdFR + ";";
      }
      meanFR = "mean"+getName()+"__"+getCCC().getInputName( ni )+"To"+getCCC().getOutputName( no )+"_FR";
      meanFRcnstr = "MEAN__" + getName() + "__" + getCCC().getInputName( ni ) + "To" + getCCC().getOutputName( no ) + "_FR : " + getDioFR( ni,no, opt ) + " < " + meanFR + ";";
   }


   if( stt ) {
      double beta          = rf? getRBeta()  : getFBeta();
      const double * kappas = rf? getRKappas() : getFKappas();

      assert( beta >= 0.0 );

      if( BETA_ADDITIVE ) {
         if( beta == 0 )
            os << "( ";
         else
            os << beta << " + ( ";
      } else {
         os << beta << " * ( ";
      }

      if( KAPPA_ADDITIVE ) {
         if( rf ) {
            try {
               if( kappas[no] > 0.0 ) {
                  //k = kappas[no]*sqrt(MAX_PATH_LENGTH/path_length_table[ni][no]);
                  k = kappas[no]*sqrt( 1/path_length_table[ni][no] );
                  if( opt.isNoRiseFall() ) {
                     os << meanRF << " + " << k << " "
                        << stdRF << inEdgeVec[ni]->getTRiseName( " + " ) << ") < " << outEdgeVec[no]->getTRiseName() << ";" << endl;
                  } else {
                     os << meanRF << " + " << k << " "
                        << stdRF << inEdgeVec[ni]->getTRiseName( " + " ) << ") < " << outEdgeVec[no]->getTFallName() << ";" << endl;
                  }
                  os << stdRFcnstr << endl;
                  os << meanRFcnstr << endl;
                  rfSlopeRecord[ni][no] = 1;
               } else if( kappas[no] == 0.0 ) {
                  if( opt.isNoRiseFall() )
                     os << meanRF << inEdgeVec[ni]->getTRiseName( " + " ) << ") < " << outEdgeVec[no]->getTRiseName() << ";" << endl;
                  else
                     os << meanRF << inEdgeVec[ni]->getTRiseName( " + " ) << ") < " << outEdgeVec[no]->getTFallName() << ";" << endl;
                  os << meanRFcnstr << endl;
                  rfSlopeRecord[ni][no] = 1;
               } else
                  assert( false );
            } catch( noRefException ) {
               errorReport(
                  string( "attempt to do statistical optimization with no " )
                  + "std dio function for the input " + cc->getInputName( ni )
                  + " of ccc " + cc->getName() );
               os << ERROR_STRING;
            }
         } else {
            try {
               if( kappas[no] > 0.0 ) {
                  //k = kappas[no]*sqrt(MAX_PATH_LENGTH/path_length_table[ni][no]);
                  k = kappas[no]*sqrt( 1/path_length_table[ni][no] );
                  os << meanFR << " + " << k << " "
                     << stdFR << inEdgeVec[ni]->getTFallName( " + " ) << ") < " << outEdgeVec[no]->getTRiseName() << ";" << endl;
                  os << stdFRcnstr << endl;
                  os << meanFRcnstr << endl;
                  frSlopeRecord[ni][no] = 1;
               } else if( kappas[no] == 0.0 ) {
                  os << meanFR << inEdgeVec[ni]->getTFallName( " + " )<< ") < " << outEdgeVec[no]->getTRiseName() << ";" << endl;
                  os << meanFRcnstr << endl;
                  frSlopeRecord[ni][no] = 1;
               } else
                  assert( false );
            } catch( noRefException ) {
               errorReport(
                  string( "attempt to do statistical optimization with no " )
                  + "std dio function for the input "
                  + cc->getInputName( ni )
                  + " of ccc " + cc->getName() );
               os << ERROR_STRING;
            }
         }
      } else {
         if( rf ) {
            try {
               if( kappas[no] > 0.0 ) {
                  os << meanRF << " + " << kappas[no] << " "
                     << getVARRF( ni,no, opt )
                     << inEdgeVec[ni]->getTRiseName( " / " )
                     << inEdgeVec[ni]->getTRiseName( " + " );
                  os << meanRFcnstr << endl;
                  rfSlopeRecord[ni][no] = 1;
               } else if( kappas[no] == 0.0 ) {
                  os << meanRF << inEdgeVec[ni]->getTRiseName( " + " );
                  os << meanRFcnstr << endl;
                  rfSlopeRecord[ni][no] = 1;
               } else
                  assert( false );
            } catch( noRefException ) {
               errorReport(
                  string( "attempt to do statistical optimization with no " )
                  + "std dio function for the input " + cc->getInputName( ni )
                  + " of ccc " + cc->getName() );
               os << ERROR_STRING;
            }
         } else {
            try {
               if( kappas[no] > 0.0 ) {
                  os << meanFR << " + " << kappas[no] << " "
                     << getVARFR( ni, no,opt )
                     << inEdgeVec[ni]->getTFallName( " / " )
                     << inEdgeVec[ni]->getTFallName( " + " );
                  os << meanFRcnstr << endl;
                  frSlopeRecord[ni][no] = 1;
               } else if( kappas[no] == 0.0 ) {
                  os << meanFR << inEdgeVec[ni]->getTFallName( " + " );
                  os << meanFRcnstr << endl;
                  frSlopeRecord[ni][no] = 1;
               } else
                  assert( false );
            } catch( noRefException ) {
               errorReport(
                  string( "attempt to do statistical optimization with no " )
                  + "std dio function for the input " + cc->getInputName( ni )
                  + " of ccc " + cc->getName() );
               os << ERROR_STRING;
            }
         }
         if( rf )
            os << ") < " << outEdgeVec[no]->getTFallName() << ";"<< endl;
         else
            os << ") < " << outEdgeVec[no]->getTRiseName() << ";"<< endl;
      }
   } else {
      if( rf ) {
         try {
            //				 os << " Reached here for a rise to fall for " << endl;
            if( CorrInputnum.size() == 0 ) {
               if( opt.isNoRiseFall() ) {
                  os << meanRF << inEdgeVec[ni]->getTRiseName( " + " )
                     << " < " << outEdgeVec[no]->getTRiseName()<< ";"<< endl;
               } else {
                  os << meanRF << inEdgeVec[ni]->getTRiseName( " + " )
                     << " < " << outEdgeVec[no]->getTFallName()<< ";"<< endl;
               }
               os << meanRFcnstr << endl;
            } else {
               if( opt.isNoRiseFall() ) {
                  os << meanRF << inEdgeVec[ni]->getTRiseName_Corr( " + " , true )
                     << " < " << outEdgeVec[no]->getTRiseName()<< ";"<< endl;
               } else {
                  os << meanRF << inEdgeVec[ni]->getTRiseName_Corr( " + ", false )
                     << " < " << outEdgeVec[no]->getTFallName()<< ";"<< endl;
               }
               os << meanRFcnstr << endl;
            }
            rfSlopeRecord[ni][no] = 1;
         } catch( noRefException ) {
            cerr << "noRefException 5" << endl;
            exit( -1 );
         }
      } else {
         try {
            //			 cout << " Reached here for a fall to rise" << endl;
            if( CorrInputnum.size() == 0 ) {
               //						os << "Got it" << endl;
               os << meanFR << inEdgeVec[ni]->getTFallName( " + " )
                  << " < " << outEdgeVec[no]->getTRiseName()<< ";"<< endl;
               os << meanFRcnstr << endl;
            } else {
               //						os << "Got it 2" << endl;
               os << meanFR << inEdgeVec[ni]->getTFallName_Corr( " + " )
                  << " < " << outEdgeVec[no]->getTRiseName()<< ";"<< endl;
               os << meanFRcnstr << endl;
            }
            frSlopeRecord[ni][no] = 1;
         } catch( noRefException ) {
            cerr << "noRefException 6" << endl;
            exit( -1 );
         }
      }
   }

   return os;
}

ostream & node::delayConstraintToOstream( ostream & os, unsigned ni,
      unsigned no, opt_prob_generator & opt, bool rf,  opt_spec & osp ) {
   assert( ni < getNumberOfInputs() );

   double val = inEdgeVec[ni]->getPreValue( rf );
   const string preS = ( val!=0.0 )? cnvt::doubleToString( val ) + " + ":EMPTY_STRING;

   const unsigned M = osp.getM();
   const prob_dist & u_dist = osp.getU();
   const prob_dist & v_dist = osp.getV();

   const ProbDist uP( u_dist.getKind(),M,u_dist.getMean(),u_dist.getSTD() );
   const ProbDist vP( v_dist.getKind(),M,v_dist.getMean(),v_dist.getSTD() );

   double u,v;

   if( rf ) {
      try {
         for( unsigned i = 0; i < M; i ++ ) {
            u = uP.getSimPoint( i );
            v = vP.getSimPoint( i );
            if( u <= 0.0 ) {
               cout << cnvt::addTH( i+1 )
                    << " u is reported to be less than or equal to 0.0" << endl;
               continue;
            }
            if( v < 0.0 ) {
               cout << cnvt::addTH( i+1 )
                    << " v is reported to be less than 0.0" << endl;
               continue;
            }

            os << cnvt::doubleToString( u ) << " * ( "
               << getDioRF( ni,no, opt ) << " ) "
               << ( ( v==0.0 )? EMPTY_STRING : " + " + cnvt::doubleToString( v ) )
               << inEdgeVec[ni]->getTRiseName( i," + " )
               << " < " << outEdgeVec[no]->getTFallName( i ) << ";" << endl;
         }
      } catch( noRefException ) {
         cerr << "noRefException 7" << endl;
         exit( -1 );
      }
   } else {
      try {
         for( unsigned i = 0; i < M; i ++ ) {
            u = uP.getSimPoint( i );
            v = vP.getSimPoint( i );

            if( u <= 0.0 ) {
               cout << cnvt::addTH( i+1 )
                    << " u is reported to be less than or equal to 0.0" << endl;
               continue;
            }

            if( v < 0.0 ) {
               cout << cnvt::addTH( i+1 )
                    << " v is reported to be less than 0.0" << endl;
               continue;
            }

            os << cnvt::doubleToString( u ) << " * ( "
               << getDioFR( ni, no,opt ) << " ) "
               << ( ( v==0.0 )? EMPTY_STRING : ( " + " + cnvt::doubleToString( v ) ) )
               << inEdgeVec[ni]->getTFallName( i," + " )
               << " < " << outEdgeVec[no]->getTRiseName( i ) << ";" << endl;
         }
      } catch( noRefException ) {
         cerr << "noRefException 8" << endl;
         exit( -1 );
      }
   }

   return os;
}

void node::updateKappa
( double & kappa, double quantile1, double quantile2, double mean, double std ) {
   //there is no update needed for a VRL node
   if( isVRL() ) return;
   double prev_kappa = kappa;
   //cout << "the mean is: " << mean << endl;
   //cout << "the std is: " << std << endl;
   //cout << "the quantile is: " << quantile1 << endl;

   if( KAPPA_ADDITIVE ) {
      kappa = ( quantile1 - quantile2 - mean ) / std;

      if( KAPPA_CLIPPING ) {
         if( kappa < KAPPA_A_LWR * prev_kappa ) kappa = KAPPA_A_LWR * prev_kappa;
         if( kappa > KAPPA_A_UPR * prev_kappa ) kappa = KAPPA_A_UPR * prev_kappa;
      }

      //cout << "the kappa is: " << kappa << endl;
      assert( kappa >= 0.0 );
   } else {
      kappa = ( quantile1 - quantile2 - mean ) * quantile2 / std / std;

      if( KAPPA_CLIPPING ) {
         if( kappa < KAPPA_M_LWR * prev_kappa ) kappa = KAPPA_M_LWR * prev_kappa;
         if( kappa > KAPPA_M_UPR * prev_kappa ) kappa = KAPPA_M_UPR * prev_kappa;
      }
   }

   return;
}

void node::updateCritKappa( bool rf, unsigned num, double criticality, string & obj, double kmax, double maxCrit ) {
   //there is no update needed for a VRL node
   if( isVRL() ) return;
   assert( num < getNumberOfOutputs() );
   if( criticality < 0 || criticality > 1 ) {
      cout << "Criticality is " << criticality << endl;
      errorReport( "The criticality at the ouput of the node " + getName() + " is not between 0 and 1" );
   }
   if( obj == "stat_delay_update" ) {
      if( rf )
         RKappas[num] = Dkappa + criticality;
      else
         FKappas[num] = Dkappa + criticality;
   } else if( !EXPT )
      //The criticality of a node only increases..so that we dont go back and forth, while the
      //nodes that are permanently non-critical are also considered.
   {
      assert( maxCrit > 0 );
      double newKappa = criticality/maxCrit*kmax;
      if( newKappa < ZeroCrit )
         newKappa = 0;
      else if( newKappa < MinCrit )
         newKappa = MinCrit;
      if( rf ) {
         if( RKappas[num] < newKappa )
            RKappas[num] = newKappa;
      } else {
         if( FKappas[num] < newKappa )
            FKappas[num] = newKappa;
      }
   } else
      //do the changes in the kappa assigning algo
      //based on experiment.
   {
      if( rf ) {
         if( criticality > 0 )
            RKappas[num] = kmax;
         else
            RKappas[num] = 0;

      } else {
         if( criticality > 0 )
            FKappas[num] = kmax;
         else
            FKappas[num] = 0;
      }
   }
}


void node::updateBeta
( double & beta, double quantile, double max_quantile ) {
   double prev_beta = beta;

   if( BETA_ADDITIVE ) {
      beta = quantile - max_quantile;

      if( BETA_CLIPPING ) {
         if( beta < prev_beta * BETA_A_LWR ) beta = prev_beta * BETA_A_LWR;
         if( beta > prev_beta * BETA_A_UPR ) beta = prev_beta * BETA_A_UPR;
      }

      assert( beta >= 0.0 );
   } else {
      beta = quantile / max_quantile;

      if( BETA_CLIPPING ) {
         if( beta < prev_beta * BETA_M_LWR ) beta = prev_beta * BETA_M_LWR;
         if( beta > prev_beta * BETA_M_UPR ) beta = prev_beta * BETA_M_UPR;
      }
   }

   return;
}

string node::getRFInternalNetName( unsigned ni, unsigned no ) {
   assert( ni < getNumberOfInputs() );
   assert( no < getNumberOfOutputs() );
   return
      inEdgeVec[ni]->getMonteTRiseName() + "_PLUS_"
      + cnvt::addTH( ni+1 )+ "to" + cnvt::addTH( no+1 ) + "_RFDELAY_OF_" + getName();
}

string node::getFRInternalNetName( unsigned ni, unsigned no ) {
   assert( ni < getNumberOfInputs() );
   assert( no < getNumberOfOutputs() );

   return
      inEdgeVec[ni]->getMonteTFallName() + "_PLUS_"
      + cnvt::addTH( ni+1 )+ "to" + cnvt::addTH( no+1 ) + "_FRDELAY_OF_" + getName();
}

/**
 * Class mos MEMBER FUNCTIONS DESCRIPTION
 * */


mos::mos
( vector<string> & strings, map<string, gposy * > par_names ) {
   assert( strings.size() > 4 );

   mosName = strings.front();
   gateName = strings[2];
   drainName = strings[1];
   sourceName = strings[3];
   bodyName =strings[4];
   if( par_names.find( "W" ) != par_names.end() ) {
      widthName = par_names["W"];
   } else if( par_names.find( "w" ) != par_names.end() ) {
      widthName = par_names["w"];
   } else {
      ciropterror( "The width is not found for " + mosName );
   }
   //  cout << strings.back() << " this is the name" << endl;
   const string & npmos =  strings.back();

   if( npmos == "pmos" || npmos == "PMOS" )
      isPMos = true;
   else if ( npmos == "nmos" || npmos == "NMOS" )
      isPMos = false;
   else
      assert( false );
   isCap = false;

   return;
}

mos::mos
( const string & CapName, vector<string> & strings, const gposy * value ) {
   assert( strings.size() == 3 );

   mosName = CapName;
   gateName = strings.front();
   widthName = value;
   //  cout << strings.back() << " this is the name" << endl;
   const string & npmos =  strings.back();
   isCap = true;
   return;
}


bool mos::isGNDorVDD( string name ) const {
   return ( name=="gnd"
            || name == "GND"
            || name == "Gnd"
            || name == "vdd"
            || name == "VDD"
            || name == "Vdd"
            || name == "VSS"
            || name == "Vss" );
}

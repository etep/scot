#include <map>
#include <vector>
#include <fstream>

#include <SystemWrapper.hpp>
#include <randomc.hpp>
#include <cnvt.hpp>
#include <opt.hpp>

using namespace std;

int ciropterror( const string & s );
int errorReport( const string & s );
void errorBasedHalt();
bool errorExist();

extern string ERROR_STRING;
extern string EMPTY_STRING;

/*
 * CLASS global_parameters MEMBER FUNCTION DEFINITIONS
 */

void global_parameters::put( const string & s,const string & d ) {
   if( gvMap.find( s ) != gvMap.end() ) {
      ciropterror( "multiple global paramter assignment error: global parameter " + s + " has previously been defined." );

      return;
   }

   gvMap[s]=d;
}

string global_parameters::getValueOf( const string s ) {
   map<string, string>::iterator itr = gvMap.find( s );

   if( itr == gvMap.end() ) {
      errorReport( "global parameter '" + s + "' not defined" );
      return "0.0";
   }

   return itr->second;
}

/*
 * CLASS opt_prob_generator MEMBER FUNCTION DEFINITIONS
 */

// PUBLIC STATIC MEMBERS
double opt_prob_generator::WEIGHT_OF_OTHER_NETS_IN_OBJECTIVE( 1e-7 );
string opt_prob_generator::MAX_OF_PRIMARY_OUTPUTS_NAME( "POMAX" );
string opt_prob_generator::TOTAL_ENERGY_NAME( "E_TOTAL" );
string opt_prob_generator::CIRCUIT_ENERGY_NAME( "CircuitEnergy" );
string opt_prob_generator::LOGIC_ENERGY_NAME( "LogicEnergy" );
string opt_prob_generator::WIRE_ENERGY_NAME( "WireEnergy" );
string opt_prob_generator::INPUT_ENERGY_NAME( "InputEnergy" );
string opt_prob_generator::LOAD_ENERGY_NAME( "LoadEnergy" );
string opt_prob_generator::TOTAL_LEAKAGE_NAME( "LeakageEnergy" );
string opt_prob_generator::TOTAL_AREA_NAME( "A_TOTAL" );
string opt_prob_generator::MAX_OF_RISE_PRIMARY_OUTPUTS_NAME( "RISE_POMAX" );
string opt_prob_generator::MAX_OF_FALL_PRIMARY_OUTPUTS_NAME( "FALL_POMAX" );

bool opt_prob_generator::ERASE_GGPSOL_SOURCE_FILE = false;
bool opt_prob_generator::PRINT_INTERNAL_NET_FOR_NOMINAL_ANALYSIS = false;

// PRIVATE STATIC MEMBERS
string opt_prob_generator::CPrise	= "";
string opt_prob_generator::CPfall	= "";
string opt_prob_generator::CNrise	= "";
string opt_prob_generator::CNfall	= "";

string opt_prob_generator::CPenergy	= "";
string opt_prob_generator::CNenergy	= "";
string opt_prob_generator::MIN_SF	= "";
string opt_prob_generator::DEF_SF	= "";
string opt_prob_generator::DEF_DF	= "";
string opt_prob_generator::MIN_SL	= "";
string opt_prob_generator::MIN_RES	= "";

string opt_prob_generator::Vthn 	= "";
string opt_prob_generator::Vthp 	= "";
string opt_prob_generator::Vdd   	= "";
string opt_prob_generator::Vdd_energy   	= "";

bool opt_prob_generator::haveConstantsBeenInitialized = false;

// PRIVATE STATIC MEMBER FUNCTIONS
bool opt_prob_generator::isObjectiveStat( const string & obj ) {
   if( obj == "stat_delay_kappabeta" )	return true;
   if( obj == "stat_delay_kappas" )	return true;
   if( obj == "stat_delay_betas" )	return true;
   if( obj == "stat_delay_update" )	return true;
   if( obj == "stat_delay_CritUpdate" )	return true;
   if( obj == "delay" )	return false;
   if( obj == "energy" )	return false;
   if( obj == "area" )	return false;
   if( obj == "expected_cmax" ) return false;

   assert( false );

   return false;
}

bool opt_prob_generator::isObjectiveABs( const string & obj ) {
   if( obj == "stat_delay_kappabeta" )	return true;
   if( obj == "stat_delay_kappas" )	return true;
   if( obj == "stat_delay_betas" )	return true;
   if( obj == "stat_delay_update" )	return true;
   if( obj == "stat_delay_CritUpdate" )	return true;
   if( obj == "delay" )	return false;
   if( obj == "energy" )	return false;
   if( obj == "area" )	return false;
   if( obj == "expected_cmax" ) return false;

   assert( false );

   return false;
}

// PUBLIC MEMBER FUNCTIONS

opt_prob_generator::opt_prob_generator( const string & nm, network & nw ) : name( nm ), netw( nw ), Beta( node::Dbeta ), haveTheNetListAssigned( false ), minWidth( -1.0 ), maxWidth( -1.0 ), minVthP( -1.0 ), maxVthP( -1.0 ), minVthN( -1.0 ), maxVthN( -1.0 ),minVdd( -1.0 ), maxVdd( -1.0 ), logicDepthFactor( 1.0 ), OnlyFormulateProblem( false ), UniformKappa( false ), CriticalityKappa( false ), UseDefActFact( false ), NoLeakPow( false ), NoAllEdgeTimeConst( false ),IncludeWireEnergy( false ) {
   return;
}

opt_prob_generator::~opt_prob_generator() {
   map<string,opt_spec *>::iterator itr;

   for( itr = optSpecs.begin(); itr != optSpecs.end(); itr ++ )
      delete itr->second;
}

void opt_prob_generator::put( piinfo & pi ) {
   map<string, edge *> & edgemap = netw.getEdgeMap();
   edge & pie = netw.findOrAddEdge( pi.getName() );

   //  if( itr == edgemap.end() )
   //  {
   //    ciropterror("net name " + pi.getName() + " does not appear in " + " .CONNECT section, hence cannot be registered as " + " a primary input of the network");
   //    return;
   //  }

   pie.put( pi );
   piEdgeVec.push_back( &pie );

   return;
}

void opt_prob_generator::put( poinfo & po ) {
   map<string, edge *> & edgemap = netw.getEdgeMap();
   edge & poe = netw.findOrAddEdge( po.getName() );

   //  itr = edgemap.find( po.getName() );
   //  if( itr == edgemap.end() )
   //  {
   //    ciropterror("net name " + po.getName() + " does not appear in " + " .CONNECT section, hence cannot be registered as " + " a primary output of the network");
   //    return;
   //  }

   poe.put( po );
   poEdgeVec.push_back( & poe );

   return;
}

string & opt_prob_generator::getName() {
   return name;
}

network & opt_prob_generator::getNetwork() {
   return netw;
}

opt_spec & opt_prob_generator::getOptSpec( const string & optname ) {

   map<string, opt_spec *>::iterator it = optSpecs.find( optname );

   assert( it != optSpecs.end() );

   return *it->second;
}

ProbDist & opt_prob_generator::getProbDist( netspec & ns, monte_carlo & monte , double p ) {
   assert( p > 0.0 && p < 1.0 );
   return getProbDist( getOptSpec( ns.getOptName() ), ns.getNum(),
                       ns.getNetName(), monte, p );
}

ProbDist & opt_prob_generator::getProbDist(  opt_spec & osp, unsigned num, const string & netname, monte_carlo & monte , double p ) {
   map<string, ProbDist *> & mc
      = osp.getMonteCarlo( monte,*this,num, p );

   map<string, ProbDist *>::iterator itrm;

   assert( ( itrm = mc.find( netname ) ) != mc.end() );

   return *( itrm->second );
}

map<string,double> & opt_prob_generator::getNomAnlys( opt_dscr & aspec ) {

   map<string, opt_spec *>::iterator it = optSpecs.find( aspec.getOptName() );

   assert( it != optSpecs.end() );

   return it->second->getNomAnlys( *this, aspec.getNum() );
}

map<string,double> opt_prob_generator::getGateDios( opt_dscr & aspec ) {
   map<string, opt_spec *>::iterator itr;
   assert( ( itr = optSpecs.find( aspec.getOptName() ) ) != optSpecs.end() );
   map<string, double> gateDioMap;
   vector<double> gDs = itr->second->getGateDios( *this,aspec.getNum(), aspec.getcccName() );
   vector<string> gDsNames = itr->second->getGateDioNames();
   assert( gDs.size() == gDsNames.size() );
   for( unsigned i=0; i< gDs.size(); i++ )
      gateDioMap[gDsNames[i]] = gDs[i];

   return gateDioMap;
}

void opt_prob_generator::solve() {
   cout << "Solve 1" << endl;
   globalConstantAssignment();
   if( !UniformKappa && !CriticalityKappa ) {
      cout << "Setting the Path Length table in all nodes" << endl;
      netw.setPathLengthTables();
   }
   else if( CriticalityKappa ) {
      cout << "Recording the node criticalities" << endl;
   }
   
   map<string, opt_spec *>::iterator it;

   for( it = optSpecs.begin(); it != optSpecs.end(); it++ ) {
      
      solve( *it->second );

      errorBasedHalt();
      it->second->assertHistoryLengths();
   }

   return;
}

void opt_prob_generator::draw() {
   cout << "Drawing PDF/CDF/WIDTH_DIST/CRITICAL_DIS if needed" << endl;
   globalConstantAssignment();

   assignTheNetList();

   map<string,vector<subplot *> >::iterator itr;
   map<string,pair<unsigned,unsigned> >::iterator itrd;

   assert( drawingDim.size() == drawingList.size() );

   for( itr = drawingList.begin(), itrd = drawingDim.begin();
         itr != drawingList.end() && itrd != drawingDim.end();
         itr ++, itrd ++ ) {
      unsigned numRow = itrd->second.first;
      unsigned numCol = itrd->second.second;
      string  filename = itr->first;
      vector<subplot *> & subplotvec = itr->second;

      assert( subplotvec.size() <= numRow * numCol );

      ofstream ofs( filename.c_str(),ios::out );

      if( !ofs.is_open() ) {
         errorReport( "file open error: " + filename + " while attempting to write .m file for drawing" );
         continue;
      }

      ofs << "figure;" << endl;

      for( unsigned i = 0; i < subplotvec.size(); i ++ ) {
         ofs << "subplot(" << numRow << "," << numCol << "," << ( i+1 ) << ");"
             << endl;
         subplotvec[i]->toOstream( ofs,*this ) << endl;
      }

      ofs.close();

      if( !errorExist() ) {
         cout << "matlab script drawing plots successfully written in '"
              << filename << "'" << endl << endl;
      } else {
         cout << "matlab script drawing plots written in '"
              << filename
              << "'. However there have been some error messages generated, "
              << "hence cannot guarantee the script will be run without errors."
              << endl << endl;
      }
   }

   return;
}

void opt_prob_generator::write() {
   globalConstantAssignment();

   map<string,opt_dscr>::iterator itr;

   for( itr = writingList.begin(); itr != writingList.end(); itr ++ ) {
      string  filename = itr->first;
      opt_dscr & aSpec = itr->second;
      bool doFullAnalysis = !( aSpec.isForCCC() );
      //			 map<string,double>  & nomAnlys = getNomAnlys( aSpec );
      map<string,double> gateDios = getGateDios( aSpec );
      errorBasedHalt();

      ofstream ofs( filename.c_str(),ios::out );

      if( !ofs.is_open() ) {
         errorReport( "file open error: "+ filename + " while attempting to write result of " + "a nominal timing analysis" );
         continue;
      }

      if( PRINT_INTERNAL_NET_FOR_NOMINAL_ANALYSIS ) {
         if( doFullAnalysis )
            cnvt::toOstream( ofs, getNomAnlys( aSpec ) );
         cnvt::toOstream( ofs, gateDios );
      } else {
         if( doFullAnalysis )
            cnvt::toOstream( ofs, getNomAnlys( aSpec ),"_PLUS_" ,"DELAY_OF_" );
         //      		cnvt::toOstream( ofs, gateDios, "_PLUS_" ,"DELAY_OF_" );
         cnvt::toOstream( ofs, gateDios );
      }
      ofs.close();
      cout << "Nominal Timing Analysis result for "
           << cnvt::addTH( aSpec.getNum()+1 )
           << " optimization of '" << aSpec.getOptName()
           << "' successfully reported in file '" << filename << "'."
           << endl << endl;
   }

   map<string,vector<mvec> >::iterator mitr;

   for( mitr = mWritingList.begin(); mitr != mWritingList.end(); mitr ++ ) {
      string  filename = mitr->first;
      vector<mvec> & mvecVec = mitr->second;

      ofstream ofs( filename.c_str(),ios::out );

      if( !ofs.is_open() ) {
         errorReport( "file open error: "
                      + filename + " while attempting to write result of "
                      + "a timing analysis" );
         continue;
      }

      for( vector<mvec>::size_type i = 0; i < mvecVec.size(); i ++ )
         mvecVec[i].toOstream( ofs, *this );

      ofs.close();

      cout << "m-file '" << filename << "' successfully generated." << endl;
   }

   return;
}

void opt_prob_generator::montecarloAndStore( monte_carlo & monte, double p, opt_spec & osp ) {
   osp.assertHistoryLengths();
   map<string,ProbDist *> mc;
   map<string,double> criticality;

   assert( osp.getLengthOfOptValHistory() == osp.getNumberOfOptimizations() );

   for( unsigned i = 0; i < osp.getLengthOfOptValHistory(); i ++ ) {
      montecarlo( mc, criticality, monte, p, osp, i,0 );
      osp.store( mc, criticality, monte );
      mc.clear();
      criticality.clear();
   }

   errorBasedHalt();
   osp.assertHistoryLengths();

   return;
}

void opt_prob_generator::nominalAnalysisAndStore( opt_spec & osp, const string cccName ) {
   map<string,double> nomAnlys;
   map<string,double> criticality;
   vector<double> pathLengths;
   vector<double> pathVariances;
   vector<double> gateDios;
   vector<string> gateDioNames;

   assert( osp.getLengthOfOptValHistory() == osp.getNumberOfOptimizations() );

   for( unsigned i = 0; i < osp.getLengthOfOptValHistory(); i ++ ) {
      nominalAnalysis( nomAnlys, criticality, pathLengths, pathVariances, gateDios, gateDioNames, osp, i , cccName );
      osp.storeNomAnlys( nomAnlys, criticality, pathLengths, pathVariances, gateDios, gateDioNames );
      nomAnlys.clear();
      criticality.clear();
      pathLengths.clear();
      pathVariances.clear();
      gateDios.clear();
      gateDioNames.clear();
   }

   errorBasedHalt();
   cout << "Nominal Analysis" << endl;
   osp.assertHistoryLengths();

   return;
}

void opt_prob_generator::addConstraint( const string & cst ) {
   constraintVector.push_back( cst );
}

void opt_prob_generator::addAreaConstraint( double area ) {
   assert( area != 0.0 );
   isAreaCnst = true;
   areaCnstVal = area;
}

void opt_prob_generator::addEnergyConstraint( double energy ) {
   assert( energy != 0.0 );
   isPowrCnst = true;
   powrCnstVal = energy;
}


void opt_prob_generator::addDelayConstraint( const string & str ) {
   isDelyCnst = true;
   delyCnstVal = str;
}

void opt_prob_generator::addDelayConstraint( double delay ) {
   assert( delay != 0.0 );
   isDelyCnst = true;
   delyCnstVal = cnvt::doubleToString( delay );
}

void opt_prob_generator::addTRise( const string & net_name, double t ) {
   addT( net_name, t, true );
}

/*
 * add a constraint of the falling {net_name} being less than 't'
 */
void opt_prob_generator::addTFall( const string & net_name, double t ) {
   addT( net_name, t, false );
}

void opt_prob_generator::addT( const string & edge_name, double T, bool r ) {
   map<string, edge *> & em = netw.getEdgeMap();
   map<string, edge *>::iterator itr = em.find( edge_name );

   if( itr == em.end() ) {
      ciropterror( "net name " + edge_name + " does not appear in "
                   + " .CONNECT section" );
      return;
   }
   string a;
   if( r ) {
      a = "NDC_" + itr->second->getName() + "_RISE : "
          + itr->second->getTRiseName() + " < " + cnvt::doubleToString( T ) ;
      addConstraint
      ( a );
   } else {
      a = "NDC_" + itr->second->getName() + "_FALL : "
          + itr->second->getTFallName() + " < " + cnvt::doubleToString( T );
      addConstraint
      ( a );
   }
   return;
}

void opt_prob_generator::addDelayConstraint( const string & name, const string & str  ) {
   map<string,string>::iterator itr;

   if( ( itr = gateDelayCnstTypeI.find( name ) ) != gateDelayCnstTypeI.end() ) {
      ciropterror( "multiple gate-delay constraint error: "
                   "previously D(" + itr->first + ") < "
                   + itr->second + ";" );
      return;
   }

   gateDelayCnstTypeI[name] = str;

   return;
}

void opt_prob_generator::addDelayConstraint( const string & name, const string & rf,const string & str ) {
   assert( rf == "rf" || rf == "fr" || rf == "ff" || rf == "rr" );

   map<string,map<string,string> >::iterator itr;
   map<string,string>::iterator sitr;

   itr = gateDelayCnstTypeII.find( name );

   if( itr != gateDelayCnstTypeII.end()
         && ( sitr = itr->second.find( rf ) ) != itr->second.end() ) {
      ciropterror( "multiple gate-delay constraint error: "
                   "previously D(" + itr->first + ", " + sitr->first + " ) < "
                   + sitr->second + ";" );
      return;
   }

   gateDelayCnstTypeII[name][rf] = str;

   return;
}

void opt_prob_generator::addDelayConstraint( const string & name, double p, double val ) {
   map<string,pair<double,double> >::iterator itr;

   if( ( itr = gateDelayCnstTypeIII.find( name ) ) != gateDelayCnstTypeIII.end() ) {
      ciropterror
      ( "multiple gate-delay constraint error: "
        "previously D( " + itr->first + ", "
        + cnvt::doubleToString( itr->second.first ) + " ) < "
        + cnvt::doubleToString( itr->second.second ) + ";" );
      return;
   }

   gateDelayCnstTypeIII[name] = pair<double,double>( p,val );

   return;
}

void opt_prob_generator::addDelayConstraint( const string & name, const string & rf, double p, double val ) {
   assert( rf == "rf" || rf == "fr" || rf == "ff" || rf == "rr" );

   map<string,map<string,pair<double,double> > >
   ::iterator itr;
   map<string,pair<double,double> >::iterator litr;

   if( ( itr = gateDelayCnstTypeIV.find( name ) ) != gateDelayCnstTypeIV.end()
         && ( litr = itr->second.find( rf ) ) != itr->second.end() ) {
      ciropterror ( "multiple gate-delay constraint error: "
                    "previously D( " + itr->first + ", " + litr->first + ", "
                    + cnvt::doubleToString( litr->second.first ) + " ) < "
                    + cnvt::doubleToString( litr->second.second ) + ";" );
      return;
   }

   gateDelayCnstTypeIV[name][rf] = pair<double,double>( p,val );

   return;
}

void opt_prob_generator::setOFP() {
   OnlyFormulateProblem = true;
   return;
}

void opt_prob_generator::setUniformKappa() {
   UniformKappa = true;
   return;
}

void opt_prob_generator::setCriticalityKappa() {
   CriticalityKappa = true;
   return;
}

void opt_prob_generator::setUseDefActFact() {
   UseDefActFact = true;
   return;
}


void opt_prob_generator::setNoLeakPow() {
   NoLeakPow = true;
   return;
}

void opt_prob_generator::setNoRiseFallTiming() {
   NoRiseFallTiming = true;
   return;
}

void opt_prob_generator::setNoAllEdgeTimeConst() {
   NoAllEdgeTimeConst = true;
   return;
}

void opt_prob_generator::setIncludeWireEnergy() {
   IncludeWireEnergy = true;
   return;
}

void opt_prob_generator::setLogicDepthFactor( double ldf ) {
   assert( ldf > 0.0 );
   logicDepthFactor = ldf;
   return;
}

void opt_prob_generator::setMinVdd( double min_vdd ) {
   assert( min_vdd > 0.0 );
   minVdd = min_vdd;
   return;
}

void opt_prob_generator::setMaxVdd( double max_vdd ) {
   assert( max_vdd > 0.0 );
   maxVdd = max_vdd;
   return;
}

void opt_prob_generator::setMinVthP( double min_vth_p ) {
   assert( min_vth_p > 0.0 );
   minVthP = min_vth_p;
   return;
}


void opt_prob_generator::setMaxVthP( double max_vth_p ) {
   assert( max_vth_p > 0.0 );
   maxVthP = max_vth_p;
   return;
}


void opt_prob_generator::setMinVthN( double min_vth_n ) {
   assert( min_vth_n > 0.0 );
   minVthN = min_vth_n;
   return;
}


void opt_prob_generator::setMaxVthN( double max_vth_n ) {
   assert( max_vth_n > 0.0 );
   maxVthN = max_vth_n;
   return;
}



void opt_prob_generator::setMinWidth( double min_width ) {
   assert( min_width > 0.0 );

   minWidth = min_width;

   return;
}

void opt_prob_generator::setMaxWidth( double max_width ) {
   assert( max_width > 0.0 );

   maxWidth = max_width;

   return;
}

void opt_prob_generator::minimizeArea( const string & optname ) {
   minimizeArea( false, optname );
}

void opt_prob_generator::minimizeArea( bool noRiseFall, const string & optname ) {
   addNewOptSpec( * new opt_spec( noRiseFall, optname,"area"  ) );
   return;
}

void opt_prob_generator::minimizeEnergy( const string & optname ) {
   minimizeEnergy( false, optname );
}

void opt_prob_generator::minimizeEnergy( bool noRiseFall, const string & optname ) {
   /*
   ciropterror( "energy minimize functionality not implemented yet" );
   return;
   */
   addNewOptSpec( * new opt_spec( noRiseFall, optname, "energy" ) );
   return;
}

void opt_prob_generator::minimizeDelay( const string & optname ) {
   minimizeDelay( false, optname );
}

void opt_prob_generator::minimizeDelay( bool noRiseFall, const string & optname ) {
   addNewOptSpec( * new opt_spec( noRiseFall, optname,"delay"  ) );
   return;
}

void opt_prob_generator::minimizeDelay( const string & optname, unsigned numIter, unsigned N, double p ) {
   minimizeDelay( optname,numIter,N,p,"normal" );
}

void opt_prob_generator::minimizeDelay( const string & optname, unsigned numIter, unsigned N, double p,const string & dist ) {
   minimizeDelay( false,optname,numIter,N,p,dist );
}

void opt_prob_generator::minimizeDelay( bool noRiseFall, const string & optname, unsigned numIter, unsigned N, double p ) {
   minimizeDelay( noRiseFall,optname,numIter,N,p,"normal" );
}

void opt_prob_generator::minimizeDelay( bool noRiseFall, const string & optname, unsigned numiter, unsigned n, double p, const string & dist ) {
   addNewOptSpec( * new opt_spec( noRiseFall, optname, "stat_delay_update", numiter, n, p, dist ) );
   return;
}

void opt_prob_generator::minimizeDelay( bool noRiseFall, const string & optname, const string & kappaORbeta, double s, double e, unsigned num ) {
   assert( kappaORbeta == "kappas" || kappaORbeta == "betas" );
   
   addNewOptSpec( * new opt_spec( noRiseFall, optname,"stat_delay_"+kappaORbeta ,s,e,num ) );

   return;
}

void opt_prob_generator::minimizeDelay( const string & optname, const string & kappaORbeta, double s, double e, unsigned num ) {
   minimizeDelay( false,optname,kappaORbeta,s,e,num );
}

void opt_prob_generator::minimizeDelay( bool noRiseFall, const string & optname, const string & kappaORbeta, vector<double> & als ) {
   assert( kappaORbeta == "kappas" || kappaORbeta == "betas" );
   addNewOptSpec( * new opt_spec( noRiseFall, optname,"stat_delay_"+kappaORbeta, als ) );

   return;
}

void opt_prob_generator::minimizeDelay( const string & optname, const string & kappaORbeta,vector<double> & kappas ) {
   minimizeDelay( false,optname,kappaORbeta,kappas );
}

void opt_prob_generator::minimizeDelay( const string & optname, const string & kappaORbeta, double kappa, double beta ) {
   minimizeDelay( false,optname,kappaORbeta,kappa,beta );
}

void opt_prob_generator::minimizeDelay( bool noRiseFall, const string & optname, const string & kappaORbeta, double kappa, double beta ) {
   assert( kappaORbeta == "kappabeta" );
   addNewOptSpec(
      * new opt_spec( noRiseFall,optname, "stat_delay_"+kappaORbeta,kappa,beta ) );

   return;
}

//So far the kmax is not functional as it is set as a constant...this functionality will be
//added later.
void opt_prob_generator::minimizeDelay( const string & optname, unsigned numiter, unsigned n, double p , double kmax ) {
   assert( kmax > 0 );
   //noRiseFall is true
   addNewOptSpec( * new opt_spec( false,optname,"stat_delay_CritUpdate",numiter,n,p,"normal",kmax ) );
   return;
}


void opt_prob_generator::putDrawing( const string & mfile, vector<subplot *> & subplotList ) {
   if( isTheFilenameAlreadyRegistered( mfile ) ) {
      ciropterror( "multiple file name error: a drawing or writing with the file name '"+ mfile + "' has previously been defined" );

      return;
   }

   drawingList[mfile] = subplotList;
   drawingDim[mfile] = getRowColNumbers( subplotList.size() );

   for( unsigned i = 0; i < subplotList.size(); i ++ )
      subplotList[i]->putOptSpec( *this );

   return;
}
/*
void opt_prob_generator::putWriting
( const string & filename, const string & optname, unsigned num )
{
  if( isTheFilenameAlreadyRegistered( filename ) )
  {
    ciropterror("multiple file name error: a drawing or writing with the file name '" + filename + "' has previously been defined");

    return;
  }

  opt_dscr as(optname,num,"");

  writingList.insert(pair<string,opt_dscr>(filename,as));

  as.putOptSpec( * this );

  return;
}
*/

void opt_prob_generator::putWriting( const string & filename, const string & optname, unsigned num,const string & gateName ) {
   if( isTheFilenameAlreadyRegistered( filename )  && gateName == "" ) {
      ciropterror( "multiple file name error: a drawing or writing with the file name '" + filename + "' has previously been defined" );

      return;
   }

   opt_dscr as( optname,num,gateName );

   writingList.insert( pair<string,opt_dscr>( filename,as ) );

   as.putOptSpec( * this );

   return;
}

/*
void opt_prob_generator::putWriting
( const string & filename, const string & outfilename )
{
  if( isTheFilenameAlreadyRegistered( filename ) )
  {
    ciropterror
    ("multiple file name error: a drawing or writing with the file name '" + filename + "' has previously been defined");

    return;
  }

  opt_dscr as(outfilename,"");

  writingList.insert
    (pair<string,opt_dscr>(filename,opt_dscr(outfilename,"")));

  as.putOptSpec( * this );

  return;
}
*/

void opt_prob_generator::putWriting( const string & filename, const string & outfilename, const string & gateName ) {
   if( isTheFilenameAlreadyRegistered( filename ) && gateName == "" ) {
      ciropterror
      ( "multiple file name error: a drawing or writing with the file name '" + filename + "' has previously been defined" );

      return;
   }

   opt_dscr as( outfilename,gateName );

   writingList.insert
   ( pair<string,opt_dscr>( filename,opt_dscr( outfilename,gateName ) ) );

   as.putOptSpec( * this );

   return;
}

void opt_prob_generator::putMWriting( const string & mfilename, vector<mvec> & mvecList ) {
   if( isTheFilenameAlreadyRegistered( mfilename ) ) {
      ciropterror
      ( "multiple file name error: a drawing or writing with the file name '"+ mfilename + "' has previously been defined" );

      return;
   }

   mWritingList[mfilename] = mvecList;

   for( vector<mvec>::size_type i = 0; i < mvecList.size(); i ++ )
      mvecList[i].putOptSpec( *this );

   return;
}


void opt_prob_generator::assignTheNetList() {
   if( haveTheNetListAssigned ) return;

   map<string,opt_spec *>::iterator fitr;
   map<string,vector<subplot *> >::iterator sitr;

   for( fitr = optSpecs.begin(); fitr != optSpecs.end(); fitr ++ ) {
      for( sitr = drawingList.begin(); sitr != drawingList.end(); sitr ++ ) {
         vector<subplot *> & subpvec = sitr->second;

         for( unsigned i = 0; i < subpvec.size(); i ++ )
            fitr->second->storeTheNetList( * subpvec[i] );
      }
   }

   haveTheNetListAssigned = true;
}

void opt_prob_generator::checkLegitimacyOfReferencesToOptimizations() {
   map<string,vector<subplot *> >::iterator sitr;

   for( sitr = drawingList.begin(); sitr != drawingList.end(); sitr ++ ) {
      vector<subplot *> & subpvec = sitr->second;

      for( unsigned i = 0; i < subpvec.size(); i ++ )
         subpvec[i]->checkLegitimacyOfReferencesToOptimizations( optSpecs );
   }

   map<string, opt_dscr>::iterator aitr;

   for( aitr = writingList.begin(); aitr != writingList.end(); aitr ++ )
      aitr->second.checkLegitimacyOfReferencesToOptimizations( optSpecs );

   return;
}


int opt_prob_generator::ggpsolOutToMap( map<string,double> & optVs, const string & filename ) {
   assert( optVs.size() == 0 );

   unsigned MAX_LINELENTH = 1000;
   string::size_type sI, eI;
   ifstream ifs( filename.c_str(), ios::in );

   if( !ifs.is_open() ) {
      errorReport( "file open error: " + filename );
      return -5;
   }

   char str[MAX_LINELENTH];
   string line;

   ifs.getline( str, MAX_LINELENTH );
   line = str;

   //NOT_PRIMAL_AND_DUAL_FEASIBLE -1
   if( line == "Problem Status: not primal-dual feasible" ) return -1;

   //MEMORY_ALLOCATION_ERROR -2
   if( line == "Problem Status: memory allocation error" ) return -2;

   //GP_SOLVER_ERROR -3
   if( line ==  "Problem Status: gp solver error" ) return -3;

   //DGOPT_ERROR -4
   if( line == "Problem Status: dgopt error" ) return -4;

   do {
      ifs.getline( str, MAX_LINELENTH );
      line = str;
   } while( line.substr( 0,24 )!="Optimal Variable Values:" && !ifs.eof() );

   while( true ) {
      if( ifs.eof() ) {
         ifs.close();
         errorReport( "file format error: " +  filename );

         return -6;
      }

      ifs.getline( str, MAX_LINELENTH );
      string temp = str;

      if( str[0] == '\0' ) break;


      line = str;
      sI = line.find( ' ' );
      eI = line.rfind( ' ' );
      /*
      	 char * x = line.substr(eI+1,line.length()-eI-1).c_str();
      		int len = line.substr(eI+1,line.length()-eI-1).length();
      		cout << eI << " " << len << "are the values" << endl;
      	  cout << (int) x[len -1] << " " << (int) x[len-2] << endl;
      */
      double value = atof( line.substr( eI+1,line.length()-eI-1 ).c_str() );
      //		cout << value << " is the value" << endl;

      optVs[line.substr( 0,sI )] = value;
   }

   ifs.close();

   return 0;
}

void opt_prob_generator::addOutFileOptSpec( const string & outfilename ) {
   map<string, opt_spec *>::iterator itr
      = optSpecs.find( outfilename );

   if( itr == optSpecs.end() )
      optSpecs.insert
      ( pair<string,opt_spec*>( outfilename,new opt_spec( outfilename ) ) );

   return;
}

void opt_prob_generator::addNewOptSpec( opt_spec & os ) {
   string optname = os.getName();

   if( optSpecs.find( optname ) != optSpecs.end() ) {
      ciropterror( "multiple optimization name error: an optimization task with the same name '" + optname + "' has previously been defined." );
      delete & os;
      return;
   }

   optSpecs[optname] = & os;

   return;
}

/*
 string opt_prob_generator::getArea()
{
  string cnstr;

 map<string,node *> & nodeMap = netw.getNodeMap();

  map<string,node *>::iterator citr;
  for( citr = nodeMap.begin(); citr != nodeMap.end(); citr ++ )
    if( cnstr.length() == 0 ) cnstr += citr->second->getArea();
    else cnstr += citr->second->getArea(" + ");

  return cnstr;
}
*/

string opt_prob_generator::getArea() {
   string areastr;
   map<string, edge *> & edgemap = netw.getEdgeMap();
   map<string, edge *>::iterator itr;
   for( itr = edgemap.begin(); itr != edgemap.end(); itr ++ ) {
      //if( itr->second->isPO() ) continue; //no gate connected as load to PO.
      string  area = itr->second->getWidth();
      if( area != EMPTY_STRING )
         if( areastr == EMPTY_STRING )
            areastr = area + ")";
         else
            areastr = area + " + " + areastr;
   }
   areastr = "(" + areastr;
   return areastr;
}


/* this function is no longer used for calculating energy constraint
 * ..Its use is only for forming the objective..*/
string opt_prob_generator::getEnergy() {
   map<string, edge *> & edgemap = netw.getEdgeMap();
   map<string, edge *>::iterator itr;
   string vdd_energy = opt_prob_generator::getVdd_energy();
   string vdd_value = opt_prob_generator::getVdd();
   string pwrstr;

   for( itr = edgemap.begin(); itr != edgemap.end(); itr ++ ) {
      //if( itr->second->isPI() ) continue;
      string nameEdge = itr->second->getName();
      if( nameEdge == "GND"
            || nameEdge == "gnd"
            || nameEdge == "VDD"
            || nameEdge == "vdd"
            || nameEdge == "Vdd"
            || nameEdge == "Gnd"
            || nameEdge == "VSS"
            || nameEdge == "Vss" ) continue;

      string pwr = itr->second->getEnergy( false, getUseDefActFact() );

      if( pwr != EMPTY_STRING )
         if( pwrstr.length() == 0 )
            //if max and min vdd are specified, Vdd is a variable to be tuned!
            if( maxVdd > 0.0 && maxVdd > minVdd )
               pwrstr = vdd_energy + " ^ 2  ( \n  " + pwr + "\n";
            else //Vdd is not a variable, so use the value
               pwrstr = vdd_value + " ^ 2  ( \n  " + pwr + "\n";
         else
            pwrstr += ( " + " +  pwr + "\n" );
   }
   pwrstr += ")\n" ;
   return pwrstr;
}

/* we are not using this function as we are making the circuit energy
 * with smaller parts.
 *
string opt_prob_generator::getCircuitEnergy()
{
  map<string, edge *> & edgemap = netw.getEdgeMap();
  map<string, edge *>::iterator itr;
	string vdd_energy = opt_prob_generator::getVdd_energy();
	string vdd_value = opt_prob_generator::getVdd();
  string pwrstr;

  for( itr = edgemap.begin(); itr != edgemap.end(); itr ++ )
  {
    if( itr->second->isPI() ) continue;
		string nameEdge =itr->second->getName();
    if(nameEdge == "GND"
						|| nameEdge == "gnd"
						|| nameEdge == "VDD"
						|| nameEdge == "vdd"
						|| nameEdge == "Vdd"
						|| nameEdge == "Gnd"
						|| nameEdge == "VSS"
						|| nameEdge == "Vss") continue;

    string pwr = itr->second->getEnergy(true, getUseDefActFact());

    if( pwr != EMPTY_STRING)
        if( pwrstr == EMPTY_STRING )
						 //if max and min vdd are specified, Vdd is a variable to be tuned!
						 if(maxVdd > 0.0 && maxVdd > minVdd)
									pwrstr = vdd_energy + " ^ 2  ( \n  " + pwr + "\n";
						 else //Vdd is not a variable, so use the value
									pwrstr = vdd_value + " ^ 2  ( \n  " + pwr + "\n";
        else
						 pwrstr += ( " + " +  pwr + "\n" );
  }
	// Now iterate over nodes to see if any internal nets are to be accounted for energy
	map<string, node *> & nodemap = netw.getNodeMap();
	map<string, node *>::iterator itrN;
	for( itrN = nodemap.begin(); itrN != nodemap.end(); itrN ++ )
  {
			 string pwr = itrN->second->getCapIntNetEnergy();
			 if(pwr != EMPTY_STRING)
						pwrstr += (" + " + pwr + "\n");
	}
	pwrstr += ")\n" ;
  return pwrstr;
}
*/

string opt_prob_generator::getLogicEnergyStt() {
   cout << "Formulating the logic energy statement" << endl;
   map<string, edge *> & edgemap = netw.getEdgeMap();
   map<string, edge *>::iterator itr;
   string pwrstr;
   for( itr = edgemap.begin(); itr != edgemap.end(); itr ++ ) {
      if( itr->second->isPI() ) continue;
      string nameEdge =itr->second->getName();
      if( nameEdge == "GND"
            || nameEdge == "gnd"
            || nameEdge == "VDD"
            || nameEdge == "vdd"
            || nameEdge == "Vdd"
            || nameEdge == "Gnd"
            || nameEdge == "VSS"
            || nameEdge == "Vss" ) continue;

      if( itr->second->getEnergy( true, getUseDefActFact() ) != EMPTY_STRING )
         if( pwrstr == EMPTY_STRING )
            pwrstr = "( LogicEnergy__" + nameEdge;
         else
            pwrstr += ( " + LogicEnergy__" + nameEdge );
   }
   // Now iterate over nodes to see if any internal nets are to be accounted for energy
   map<string, node *> & nodemap = netw.getNodeMap();
   map<string, node *>::iterator itrN;
   for( itrN = nodemap.begin(); itrN != nodemap.end(); itrN ++ ) {
      string pwr = itrN->second->getCapIntNetEnergyStt();
      if( pwr != EMPTY_STRING )
         pwrstr += ( " + " + pwr );
   }
   pwrstr += ")\n" ;
   return pwrstr;
}

void opt_prob_generator::getLogicEnergy( ostream & os ) {
   cout << "Writing constraints for logic energy" << endl;
   map<string, edge *> & edgemap = netw.getEdgeMap();
   map<string, edge *>::iterator itr;
   string vdd_energy = opt_prob_generator::getVdd_energy();
   string vdd_value = opt_prob_generator::getVdd();
   string Vdd = vdd_value; //to be passed to the node for internal energy calculation.
   string pwr;
   for( itr = edgemap.begin(); itr != edgemap.end(); itr ++ ) {
      if( itr->second->isPI() ) continue;
      string nameEdge =itr->second->getName();
      if( nameEdge == "GND"
            || nameEdge == "gnd"
            || nameEdge == "VDD"
            || nameEdge == "vdd"
            || nameEdge == "Vdd"
            || nameEdge == "Gnd"
            || nameEdge == "VSS"
            || nameEdge == "Vss" ) continue;

      if( ( pwr = itr->second->getLogicEnergy( true, getUseDefActFact() ) ) != EMPTY_STRING )
         //if max and min vdd are specified, Vdd is a variable to be tuned!
         if( maxVdd > 0.0 && maxVdd > minVdd ) {
            Vdd = vdd_energy;
            os << "LogicEnergyCnst__" << itr->first << " : " << vdd_energy << " ^ 2 ( " <<  pwr << ") < " << "LogicEnergy__" << itr->first << ";" << endl;
         } else //Vdd is not a variable, so use the value
            os << "LogicEnergyCnst__" << itr->first << " : " << vdd_value << " ^ 2 ( " <<  pwr << ") < " << "LogicEnergy__" << itr->first << ";" << endl;
   }
   // Now iterate over nodes to see if any internal nets are to be accounted for energy
   map<string, node *> & nodemap = netw.getNodeMap();
   map<string, node *>::iterator itrN;
   for( itrN = nodemap.begin(); itrN != nodemap.end(); itrN ++ )
      itrN->second->getCapIntNetEnergy( os, Vdd );
}

string opt_prob_generator::getWireEnergyStt() {
   cout << "Writing wire energy statement" << endl;
   map<string, edge *> & edgemap = netw.getEdgeMap();
   map<string, edge *>::iterator itr;
   string pwrstr;
   for( itr = edgemap.begin(); itr != edgemap.end(); itr ++ ) {
      //    if( itr->second->isPI() ) continue;
      string nameEdge =itr->second->getName();
      if( nameEdge == "GND"
            || nameEdge == "gnd"
            || nameEdge == "VDD"
            || nameEdge == "vdd"
            || nameEdge == "Vdd"
            || nameEdge == "Gnd"
            || nameEdge == "VSS"
            || nameEdge == "Vss" ) continue;

      if( itr->second->getWireEnergy( getUseDefActFact() ) != EMPTY_STRING )
         if( pwrstr == EMPTY_STRING )
            pwrstr = "( WireEnergy__" + nameEdge;
         else
            pwrstr += ( " + WireEnergy__" + nameEdge );
   }
   // Now iterate over nodes to see if any internal nets are to be accounted for energy
   map<string, node *> & nodemap = netw.getNodeMap();
   map<string, node *>::iterator itrN;
   for( itrN = nodemap.begin(); itrN != nodemap.end(); itrN ++ ) {
      string pwr = itrN->second->getWireIntNetEnergyStt();
      if( pwr != EMPTY_STRING )
         pwrstr += ( " + " + pwr );
   }
   pwrstr += ")\n" ;
   return pwrstr;
}

void opt_prob_generator::getWireEnergy( ostream & os ) {
   cout << "Formulating wire energy constraints" << endl;
   map<string, edge *> & edgemap = netw.getEdgeMap();
   map<string, edge *>::iterator itr;
   string vdd_energy = opt_prob_generator::getVdd_energy();
   string vdd_value = opt_prob_generator::getVdd();
   string Vdd = vdd_value; //to be passed to the node for internal wire energy.
   string pwr;
   for( itr = edgemap.begin(); itr != edgemap.end(); itr ++ ) {
      //    if( itr->second->isPI() ) continue;
      string nameEdge =itr->second->getName();
      if( nameEdge == "GND"
            || nameEdge == "gnd"
            || nameEdge == "VDD"
            || nameEdge == "vdd"
            || nameEdge == "Vdd"
            || nameEdge == "Gnd"
            || nameEdge == "VSS"
            || nameEdge == "Vss" ) continue;

      if( ( pwr = itr->second->getWireEnergy( getUseDefActFact() ) ) != EMPTY_STRING )
         //if max and min vdd are specified, Vdd is a variable to be tuned!
         if( maxVdd > 0.0 && maxVdd > minVdd ) {
            Vdd = vdd_energy;
            os << "WireEnergyCnst__" << itr->first << " : " << vdd_energy << " ^ 2 ( " <<  pwr << ") < " << "WireEnergy__" << itr->first << ";" << endl;
         } else //Vdd is not a variable, so use the value
            os << "WireEnergyCnst__" << itr->first << " : " << vdd_value << " ^ 2 ( " <<  pwr << ") < " << "WireEnergy__" << itr->first << ";" << endl;
   }
   // Now iterate over nodes to see if any internal nets are to be accounted for energy
   map<string, node *> & nodemap = netw.getNodeMap();
   map<string, node *>::iterator itrN;
   for( itrN = nodemap.begin(); itrN != nodemap.end(); itrN ++ )
      itrN->second->getWireIntNetEnergy( os,Vdd );
}

string opt_prob_generator::getInputEnergyStt() {
   cout << "Writing input energy statement " << endl;
   map<string, edge *> & edgemap = netw.getEdgeMap();
   map<string, edge *>::iterator itr;
   string pwrstr;
   for( itr = edgemap.begin(); itr != edgemap.end(); itr ++ ) {
      if( ! itr->second->isPI() ) continue;
      string nameEdge =  itr->second->getName();
      if( nameEdge == "GND"
            || nameEdge == "gnd"
            || nameEdge == "VDD"
            || nameEdge == "vdd"
            || nameEdge == "Vdd"
            || nameEdge == "Gnd"
            || nameEdge == "VSS"
            || nameEdge == "Vss" ) continue;

      if( itr->second->getEnergy( true, getUseDefActFact() ) != EMPTY_STRING )
         if( pwrstr.length() == 0 )
            pwrstr = "( InEnergy_"+nameEdge;
         else
            pwrstr += ( " + InEnergy_" + nameEdge );
   }
   pwrstr += ")" ;
   return pwrstr;
}

/* has been replaced by individual input energy statements.
string opt_prob_generator::getInputEnergy()
{
 map<string, edge *> & edgemap = netw.getEdgeMap();
  map<string, edge *>::iterator itr;
	string vdd_energy = opt_prob_generator::getVdd_energy();
	string vdd_value = opt_prob_generator::getVdd();
  string pwrstr;

  for( itr = edgemap.begin(); itr != edgemap.end(); itr ++ )
  {
    if( ! itr->second->isPI() ) continue;
		string nameEdge =  itr->second->getName();
    if(nameEdge == "GND"
						|| nameEdge == "gnd"
						|| nameEdge == "VDD"
						|| nameEdge == "vdd"
						|| nameEdge == "Vdd"
						|| nameEdge == "Gnd"
						|| nameEdge == "VSS"
						|| nameEdge == "Vss") continue;

    string pwr = itr->second->getEnergy(false, getUseDefActFact());

    if( pwr.length() == 0 ) continue;

    if( pwrstr.length() == 0 )
				 //if max and min vdd are specified, Vdd is a variable to be tuned!
				 if(maxVdd > 0.0 && maxVdd > minVdd)
							pwrstr = vdd_energy + " ^ 2  ( \n  " + pwr + "\n";
				 else //Vdd is not a variable, so use the value
							pwrstr = vdd_value + " ^ 2  ( \n  " + pwr + "\n";
    else
				 pwrstr += ( " + " +  pwr + "\n" );
  }
	pwrstr += ")\n" ;
  return pwrstr;
}
*/

void opt_prob_generator::getInputEnergy( ostream & os ) {
   cout << "Formulating the input energy constraints" << endl;
   map<string, edge *> & edgemap = netw.getEdgeMap();
   map<string, edge *>::iterator itr;
   string vdd_energy = opt_prob_generator::getVdd_energy();
   string vdd_value = opt_prob_generator::getVdd();
   for( itr = edgemap.begin(); itr != edgemap.end(); itr ++ ) {
      if( ! itr->second->isPI() ) continue;
      string nameEdge =  itr->second->getName();
      if( nameEdge == "GND"
            || nameEdge == "gnd"
            || nameEdge == "VDD"
            || nameEdge == "vdd"
            || nameEdge == "Vdd"
            || nameEdge == "Gnd"
            || nameEdge == "VSS"
            || nameEdge == "Vss" ) continue;

      string pwr = itr->second->getLogicEnergy( false, getUseDefActFact() ); //the true or false should not matter as its a PI and so it should not be a PO.

      if( pwr != EMPTY_STRING )
         //if max and min vdd are specified, Vdd is a variable to be tuned!
         if( maxVdd > 0.0 && maxVdd > minVdd )
            os << "InEnergyConst_" << nameEdge << " : " << vdd_energy + " ^ 2 ( " << pwr << ") < " << "InEnergy_" << nameEdge << ";" << endl;
         else //Vdd is not a variable, so use the value
            os << "InEnergyConst_" << nameEdge << " : " << vdd_value + " ^ 2 ( " << pwr << ") < " << "InEnergy_" << nameEdge << ";" << endl;
   }
}

string opt_prob_generator::getLoadEnergyStt() {
   cout << "Writing the load energy statement" << endl;
   map<string, edge *> & edgemap = netw.getEdgeMap();
   map<string, edge *>::iterator itr;
   string pwrstr;
   for( itr = edgemap.begin(); itr != edgemap.end(); itr ++ ) {
      if( ! itr->second->isPO() ) continue;
      string nameEdge =  itr->second->getName();
      if( itr->second->getLoadEnergy( getUseDefActFact() ) != EMPTY_STRING )
         if( pwrstr.length() == 0 )
            pwrstr = "( LoadEnergy_" + nameEdge;
         else
            pwrstr += ( " + LoadEnergy_" + nameEdge );
   }
   pwrstr += " )\n" ;
   return pwrstr;
}

/* has been replaced by individual load energy statements
string opt_prob_generator::getLoadEnergy()
{
 map<string, edge *> & edgemap = netw.getEdgeMap();
  map<string, edge *>::iterator itr;
	string vdd_energy = opt_prob_generator::getVdd_energy();
	string vdd_value = opt_prob_generator::getVdd();
  string pwrstr;

  for( itr = edgemap.begin(); itr != edgemap.end(); itr ++ )
  {
    if( ! itr->second->isPO() ) continue;

    string pwr = itr->second->getLoadEnergy();

    if( pwr.length() == 0 ) continue;

    if( pwrstr.length() == 0 )
				 //if max and min vdd are specified, Vdd is a variable to be tuned!
				 if(maxVdd > 0.0 && maxVdd > minVdd)
							pwrstr = vdd_energy + " ^ 2  ( \n  " + pwr + "\n";
				 else //Vdd is not a variable, so use the value
							pwrstr = vdd_value + " ^ 2  ( \n  " + pwr + "\n";
    else
				 pwrstr += ( " + " +  pwr + "\n" );
  }
	pwrstr += ")\n" ;
  return pwrstr;
}
*/

void opt_prob_generator::getLoadEnergy( ostream & os ) {
   cout << "Formulating the load energy constraints" << endl;
   map<string, edge *> & edgemap = netw.getEdgeMap();
   map<string, edge *>::iterator itr;
   string vdd_energy = opt_prob_generator::getVdd_energy();
   string vdd_value = opt_prob_generator::getVdd();

   for( itr = edgemap.begin(); itr != edgemap.end(); itr ++ ) {
      if( ! itr->second->isPO() ) continue;
      string nameEdge =  itr->second->getName();

      string pwr = itr->second->getLoadEnergy( getUseDefActFact() );

      if( pwr != EMPTY_STRING )
         //if max and min vdd are specified, Vdd is a variable to be tuned!
         if( maxVdd > 0.0 && maxVdd > minVdd )
            os << "OutEnergyConst_" << nameEdge << " : " << vdd_energy + " ^ 2 ( " << pwr << ") < " << "LoadEnergy_" << nameEdge << ";" << endl;
         else //Vdd is not a variable, so use the value
            os << "OutEnergyConst_" << nameEdge << " : " << vdd_value + " ^ 2 ( " << pwr << ") < " << "LoadEnergy_" << nameEdge << ";" << endl;
   }
}


string opt_prob_generator::getLeakage( bool stat ) {
   map<string, node *> & nodemap = netw.getNodeMap();
   map<string, node *>::iterator itr;
   string vdd_energy = opt_prob_generator::getVdd_energy();
   string vdd_value = opt_prob_generator::getVdd();
   string leakstr;

   for( itr = nodemap.begin(); itr != nodemap.end(); itr ++ ) {
      if( itr->second->isCapacitor() || itr->second->isVRL() ) continue;
      for( unsigned i = 0; i < itr->second->getNumberOfOutputs(); i ++ ) {
         string leak = itr->second->getLeakPow( stat,i );

         if( leak.length() == 0 ) continue;

         if( leakstr.length() == 0 )
            if( stat )
               //if max and min vdd are specified, Vdd is a variable to be tuned!
               if( maxVdd > 0.0 && maxVdd > minVdd )
                  leakstr = "1.5*"+vdd_energy+"*(3.317 * "+vdd_energy+"^0.6812+1.758 * "+vdd_energy+"^3.416) "+ cnvt::doubleToString( logicDepthFactor ) + " " + MAX_OF_PRIMARY_OUTPUTS_NAME+"*( \n  " + leak + "\n";
               else //Vdd is not a variable, so use the value
                  leakstr = "1.5 * "+vdd_value+"*(3.317 * "+vdd_value+"^0.6812+1.758 * "+vdd_value+"^3.416 ) *"+ cnvt::doubleToString( logicDepthFactor ) + " " + MAX_OF_PRIMARY_OUTPUTS_NAME+"*( \n  " + leak + "\n";
            else
               //if max and min vdd are specified, Vdd is a variable to be tuned!
               if( maxVdd > 0.0 && maxVdd > minVdd )
                  leakstr = vdd_energy+"*(3.317 * "+vdd_energy+"^0.6812+1.758 * "+vdd_energy+"^3.416)*"+ cnvt::doubleToString( logicDepthFactor ) + " " + MAX_OF_PRIMARY_OUTPUTS_NAME+"*( \n  " + leak + "\n";
               else //Vdd is not a variable, so use the value
                  leakstr = vdd_value+"*(3.317 * "+vdd_value+"^0.6812+1.758 * "+vdd_value+"^3.416 )*"+ cnvt::doubleToString( logicDepthFactor ) + " " + MAX_OF_PRIMARY_OUTPUTS_NAME+"*( \n  " + leak + "\n";
         else
            leakstr += ( " + " +  leak + "\n" );
      }
   }
   leakstr += ")\n" ;
   return leakstr;
}




string opt_prob_generator::getMaxPODelay() {
   assert( poEdgeVec.size() > 0 );

   string maxDelay = "max( ";

   for( unsigned i = 0; i < poEdgeVec.size(); i ++ ) {
      if( i != 0 ) maxDelay += ", ";
      if( NoRiseFallTiming )
         maxDelay+=poEdgeVec[i]->getTRiseName();
      else
         maxDelay+=poEdgeVec[i]->getTRiseName()+", "+poEdgeVec[i]->getTFallName();
   }

   maxDelay += " )";

   return maxDelay;
}

string opt_prob_generator::getMaxPODelay( unsigned num ) {
   assert( poEdgeVec.size() > 0 );

   string maxDelay = "max( ";

   for( unsigned i = 0; i < poEdgeVec.size(); i ++ ) {
      if( i != 0 ) maxDelay += ", ";
      if( NoRiseFallTiming )
         maxDelay+=poEdgeVec[i]->getTRiseName( num );
      else
         maxDelay+=poEdgeVec[i]->getTRiseName( num )+", "+poEdgeVec[i]->getTFallName( num );
   }

   maxDelay += " )";

   return maxDelay;
}

void opt_prob_generator::writeGGPSOLInput( const string & filename, opt_spec & osp ) {

   globalConstantAssignment();

   string obj = osp.getObjective();

   ofstream wrt( filename.c_str(), ios::out );
   if( !wrt.is_open() ) {
      errorReport( "file open error: " + filename );
      return;
   }
   // netw.MakeVRLCnodes();
   // netw.MakeCorrelatedEdgeTable();
   // netw.CheckVRLnodes(); // no need for this anymore.
   //	netw.CheckConnectionsOfEdges();
   CheckPIandPO();
   objectiveStatementToOstream	( wrt, osp ,isObjectiveABs( obj ) );
   PICConstraintsToOstream       ( wrt );
   PITimingConstraintsToOstream  ( wrt );

   vthConstraintsToOstream    ( wrt );
   widthConstraintsToOstream  ( wrt );
   globalConstraintsToOstream ( wrt );
   vddConstraintsToOstream    ( wrt );
   recursiveConstraintsToOstream	( wrt, osp );

   gateDelayConstraintsToOstream	( wrt );
   theThreeConstraintsToOstream  ( wrt, isObjectiveABs( obj ) );
   wrt.close();
   leakPowConstraintsToOstream();
   ofstream wrt1( filename.c_str(), ios::app );
   allEdgesLessThanMaxConstraint ( wrt1 ); //used for dynamic circuits.
   wrt1.close();

   return;
}

void opt_prob_generator::globalConstantAssignment() {
   if( haveConstantsBeenInitialized ) return;

   CPrise = gP.getValueOf( "CgateP_rise" );
   CPfall = gP.getValueOf( "CgateP_fall" );
   CNrise = gP.getValueOf( "CgateN_rise" );
   CNfall = gP.getValueOf( "CgateN_fall" );

   CPenergy = gP.getValueOf( "Cgate_energy_p" );
   CNenergy = gP.getValueOf( "Cgate_energy_n" );
   //MIN_SF is used when the activity of a net is < MIN_SF
   MIN_SF = gP.getValueOf( "min_act_factor" );
   //DEF_SF is used when the activity of a net is < 0, i.e not initialized.
   DEF_SF = gP.getValueOf( "def_act_factor" );
   DEF_DF = gP.getValueOf( "def_duty_factor" );
   MIN_SL = gP.getValueOf( "min_slope_effect" );
   MIN_RES = gP.getValueOf( "min_res" );

   Vthn   = gP.getValueOf( "Vthn" );
   Vthp   = gP.getValueOf( "Vthp" );
   Vdd    = gP.getValueOf( "Vdd" );
   Vdd_energy    = gP.getValueOf( "Vdd_energy" );

   haveConstantsBeenInitialized = true;

   return;
}

void opt_prob_generator::CheckPIandPO() {
   map<string, edge *> & edgemap = netw.getEdgeMap();
   vector<edge *>::iterator itr;
   for( unsigned i = 0; i < piEdgeVec.size(); i++ ) {
      if( edgemap.find( piEdgeVec[i]->getName() ) == edgemap.end() )
         ciropterror( "net name " + piEdgeVec[i]->getName() + " does not appear in " + " .CONNECT section, hence cannot be registered as " + " a primary input of the network" );
      return;
   }
   for( unsigned i = 0; i < poEdgeVec.size(); i++ ) {
      if( edgemap.find( poEdgeVec[i]->getName() ) == edgemap.end() )
         ciropterror( "net name " + poEdgeVec[i]->getName() + " does not appear in " + " .CONNECT section, hence cannot be registered as " + " a primary output of the network" );
      return;
   }
   return;
}


ostream & opt_prob_generator::objectiveStatementToOstream( ostream & os, opt_spec & osp , bool stat ) {
   cout << "Forming the Objective function statement" << endl;
   string obj = osp.getObjective();

   if( obj == "area" ) {
      os << "minimize "  << TOTAL_AREA_NAME << ";" << endl<< endl;
      os << "area_constraint : " << getArea() << " < " << TOTAL_AREA_NAME << ";" << endl<<endl;
   } else if( obj == "energy" ) {
      os << "minimize "  << TOTAL_ENERGY_NAME <<";" << endl << endl;
      if( NoLeakPow ) {
         os << "energy_constraint : " << CIRCUIT_ENERGY_NAME << " + " << INPUT_ENERGY_NAME << " + " << LOAD_ENERGY_NAME << " < " << TOTAL_ENERGY_NAME << ";" << endl;
         //				 os << "circuit_energy : " << getLogicEnergyStt() << " < " << CIRCUIT_ENERGY_NAME << ";" << endl;
         if( IncludeWireEnergy ) {
            os << "circuit_energy : " << LOGIC_ENERGY_NAME << " + " << WIRE_ENERGY_NAME << " < " << CIRCUIT_ENERGY_NAME<< ";" << endl;
            os << "wire_energy : " << getWireEnergyStt() << " < " << WIRE_ENERGY_NAME << ";" << endl;
            getWireEnergy( os );
         } else {
            os << "circuit_energy : " << LOGIC_ENERGY_NAME << " < " << CIRCUIT_ENERGY_NAME<< ";" << endl;
         }
         os << "logic_energy : " << getLogicEnergyStt() << " < " << LOGIC_ENERGY_NAME << ";" << endl;
         getLogicEnergy( os );
         os << "input_energy : " << getInputEnergyStt() << " < " << INPUT_ENERGY_NAME << ";" << endl;
         getInputEnergy( os );
         os << "load_energy : " << getLoadEnergyStt() << " < " << LOAD_ENERGY_NAME << ";" << endl;
         getLoadEnergy( os );
      } else {
         os << "energy_constraint : " << CIRCUIT_ENERGY_NAME << " + " << INPUT_ENERGY_NAME << " + " << LOAD_ENERGY_NAME << " + " <<  TOTAL_LEAKAGE_NAME << " < " << TOTAL_ENERGY_NAME << ";" << endl;
         //				 os << "circuit_energy : " << getLogicEnergyStt() << " < " << CIRCUIT_ENERGY_NAME << ";" << endl;
         if( IncludeWireEnergy ) {
            os << "circuit_energy : " << LOGIC_ENERGY_NAME << " + " << WIRE_ENERGY_NAME << " < " << CIRCUIT_ENERGY_NAME<< ";" << endl;
            os << "wire_energy : " << getWireEnergyStt() << " < " << WIRE_ENERGY_NAME << ";" << endl;
            getWireEnergy( os );
         } else {
            os << "circuit_energy : " << LOGIC_ENERGY_NAME << " < " << CIRCUIT_ENERGY_NAME<< ";" << endl;
         }
         os << "logic_energy : " << getLogicEnergyStt() << " < " << LOGIC_ENERGY_NAME << ";" << endl;
         getLogicEnergy( os );
         os << "input_energy : " << getInputEnergyStt() << " < " << INPUT_ENERGY_NAME << ";" << endl;
         getInputEnergy( os );
         os << "load_energy : " << getLoadEnergyStt() << " < " << LOAD_ENERGY_NAME << ";" << endl;
         getLoadEnergy( os );
         os << "leakage_energy : " << getLeakage( stat ) << " < " << TOTAL_LEAKAGE_NAME << ";" << endl;
      }
   } else if( obj == "delay" ) {
      //This is to remove the degeneracy of the net timings due to the
      //max function. include it when you want to get accurate timings and
      //slack info from the optimization.
      //    os << "minimize " << MAX_OF_PRIMARY_OUTPUTS_NAME << " + " << WEIGHT_OF_OTHER_NETS_IN_OBJECTIVE << "*(" << getEdgeTimes(" + ") << ");" << endl << endl;
      os << "minimize " << MAX_OF_PRIMARY_OUTPUTS_NAME << ";" << endl << endl;
      os << getMaxPODelay() << " < " << MAX_OF_PRIMARY_OUTPUTS_NAME << ";";
      os << endl << endl;
   } else if( obj == "expected_cmax" ) {
      unsigned M = osp.getM();

      os << "minimize " << MAX_OF_PRIMARY_OUTPUTS_NAME << ";" << endl << endl;

      os << ( MAX_OF_PRIMARY_OUTPUTS_NAME + "_1" ) << endl;
      for( unsigned i = 1; i < M; i ++ ) {
         os << "+ " << ( MAX_OF_PRIMARY_OUTPUTS_NAME + "_" + cnvt::intToString( i+1 ) )
            << endl;
      }
      os << "< " << MAX_OF_PRIMARY_OUTPUTS_NAME << ";" << endl << endl;

      for( unsigned i = 0; i < M; i ++ ) {
         os << getMaxPODelay( i ) << " < "
            << ( MAX_OF_PRIMARY_OUTPUTS_NAME + "_" + cnvt::intToString( i+1 ) )
            << ";" << endl;
      }

      os << endl;
   } else if( isObjectiveStat( obj ) ) {
      //   os << "minimize " << MAX_OF_PRIMARY_OUTPUTS_NAME << " + " << WEIGHT_OF_OTHER_NETS_IN_OBJECTIVE << "*(" << getEdgeTimes(" + ") << ");" << endl << endl;
      os << "minimize " << MAX_OF_PRIMARY_OUTPUTS_NAME << ";" << endl << endl;
      os << getMaxPODelay() << " < " << MAX_OF_PRIMARY_OUTPUTS_NAME << ";";
      os << endl << endl;
   } else
      assert( false );

   return os;
}


ostream & opt_prob_generator::allEdgesLessThanMaxConstraint( ostream & os ) {
   if( NoAllEdgeTimeConst )
      cout << "Not forming AllEdgeTimings < POMAX constraint" << endl;
   else {
      cout << "Forming AllEdgeTimings < max constraint for dynamic gate inputs" << endl;
      string st;
      if( ( st = getEdgeTimes( " , " ) ) != EMPTY_STRING )
         os << "EdgeTimeConstraint : " << "max(" << st << ") < " << MAX_OF_PRIMARY_OUTPUTS_NAME << ";" << endl;
      else
         cout << "No intermediate Edges for this constraint" << endl;
   }
}



string opt_prob_generator::getEdgeTimes( const string & delim ) {
   string edgestr;
   map<string, edge *> & edgemap = netw.getEdgeMap();
   map<string, edge *>::iterator itr;
   for( itr = edgemap.begin(); itr != edgemap.end(); itr ++ ) {
      if( itr->second->isPO() ) continue; //outputs are already accounted for.
      //	  if( itr->second->isPI() ) continue; //input nets now have timing variables unlike before. The extra timing due to input slope is attributed to the input net timings.
      //if( itr->second->isGNDorVDD() ) continue; //gnd and Vdd nets are assumed as infinitely driveable inputs, just for the sake of optimization.

      if( itr->second->skipEdgeTiming() ) continue;
      if( edgestr.length() == 0 ) {
         if( isNoRiseFall() )
            edgestr = itr->second->getTRiseName();
         else
            edgestr = itr->second->getTRiseName() + delim + itr->second->getTFallName();
      } else {
         if( isNoRiseFall() )
            edgestr = itr->second->getTRiseName() + delim + edgestr;
         else
            edgestr = itr->second->getTRiseName() + delim + itr->second->getTFallName() + delim + edgestr;
      }
   }
   return edgestr;
}



ostream & opt_prob_generator::theThreeConstraintsToOstream( ostream & os, bool stat ) {
   cout << "Forming Area/Energy/Delay constraints as specified by user" << endl;
   if( isAreaCnst ) {
      os << "total_area : "  << TOTAL_AREA_NAME << " < " << areaCnstVal <<";" << endl;
      os << "area_constraint : " << getArea() << " < " << TOTAL_AREA_NAME << ";" << endl;
   }
   if( isPowrCnst ) {
      os << "total_energy : "  << TOTAL_ENERGY_NAME << " < " << powrCnstVal <<";" << endl;
      if( NoLeakPow ) {
         os << "energy_constraint : " << CIRCUIT_ENERGY_NAME << " + " << INPUT_ENERGY_NAME << " + " << LOAD_ENERGY_NAME << " < " << TOTAL_ENERGY_NAME << ";" << endl;
         //				 os << "circuit_energy : " << getLogicEnergyStt() << " < " << CIRCUIT_ENERGY_NAME << ";" << endl;
         if( IncludeWireEnergy ) {
            os << "circuit_energy : " << LOGIC_ENERGY_NAME << " + " << WIRE_ENERGY_NAME << " < " << CIRCUIT_ENERGY_NAME<< ";" << endl;
            os << "wire_energy : " << getWireEnergyStt() << " < " << WIRE_ENERGY_NAME << ";" << endl;
            getWireEnergy( os );
         } else {
            os << "circuit_energy : " << LOGIC_ENERGY_NAME << " < " << CIRCUIT_ENERGY_NAME<< ";" << endl;
         }
         os << "logic_energy : " << getLogicEnergyStt() << " < " << LOGIC_ENERGY_NAME << ";" << endl;
         getLogicEnergy( os );
         os << "input_energy : " << getInputEnergyStt() << " < " << INPUT_ENERGY_NAME << ";" << endl;
         getInputEnergy( os );
         os << "load_energy : " << getLoadEnergyStt() << " < " << LOAD_ENERGY_NAME << ";" << endl;
         getLoadEnergy( os );
      } else {
         os << "energy_constraint : " << CIRCUIT_ENERGY_NAME << " + " << INPUT_ENERGY_NAME << " + " << LOAD_ENERGY_NAME << " + " <<  TOTAL_LEAKAGE_NAME << " < " << TOTAL_ENERGY_NAME << ";" << endl;
         //				 os << "circuit_energy : " << getLogicEnergyStt() << " < " << CIRCUIT_ENERGY_NAME << ";" << endl;
         if( IncludeWireEnergy ) {
            os << "circuit_energy : " << LOGIC_ENERGY_NAME << " + " << WIRE_ENERGY_NAME << " < " << CIRCUIT_ENERGY_NAME<< ";" << endl;
            os << "wire_energy : " << getWireEnergyStt() << " < " << WIRE_ENERGY_NAME << ";" << endl;
            getWireEnergy( os );
         } else {
            os << "circuit_energy : " << LOGIC_ENERGY_NAME << " < " << CIRCUIT_ENERGY_NAME<< ";" << endl;
         }
         os << "logic_energy : " << getLogicEnergyStt() << " < " << LOGIC_ENERGY_NAME << ";" << endl;
         getLogicEnergy( os );
         os << "input_energy : " << getInputEnergyStt() << " < " << INPUT_ENERGY_NAME << ";" << endl;
         getInputEnergy( os );
         os << "load_energy : " << getLoadEnergyStt() << " < " << LOAD_ENERGY_NAME << ";" << endl;
         getLoadEnergy( os );
         os << "leakage_energy : " << getLeakage( stat ) << " < " << TOTAL_LEAKAGE_NAME << ";" << endl;
      }
   }
   if( isDelyCnst && !stat ) {
      os << "total_delay : "  << MAX_OF_PRIMARY_OUTPUTS_NAME << " < " << delyCnstVal <<";" << endl;
      os << "delay_constraint : "
         << getMaxPODelay() << " < " << MAX_OF_PRIMARY_OUTPUTS_NAME << ";" << endl;
   }
   return os;
}


void opt_prob_generator::leakPowConstraintsToOstream() {
   if( maxVthP == -1.0 && maxVthN == -1.0 && minVthP == -1.0 && minVthN == -1.0 )
      return;
   cout << "Formulating leakage energy constraints if required" << endl;
   cout << "Note: Leakage energy is highly overestimated for large CCCs" << endl;
}

ostream & opt_prob_generator::recursiveConstraintsToOstream( ostream & os,opt_spec & osp ) {
   cout << "Formulating the recursive network delay constraints" << endl;
   string obj = osp.getObjective();
   map<string, node *> & nodeMap = netw.getNodeMap();
   map<string,  node *>::iterator citr;

   if( obj == "expected_cmax" ) {
      cout << "generating expected_max delay constraints \n" << endl;
      for( citr = nodeMap.begin(); citr != nodeMap.end(); citr ++ )
         citr->second->delayConstraintToOstream( os,*this,osp );
   } else {
      //    cout << "generating normal delay constraints" << endl;
      for( citr = nodeMap.begin(); citr != nodeMap.end(); citr ++ )
         citr->second->delayConstraintToOstream( os,*this,isObjectiveStat( obj ) );
   }
   cout << "Done with the recursive Constraints" << endl;

   return os;
}

ostream & opt_prob_generator::PICConstraintsToOstream( ostream & os ) {
   cout << "Forming the input loading constraints, as specified" << endl;

   for( int i = 0; i < piEdgeVec.size(); i ++ ) {
      assert( piEdgeVec[i]->isPI() );

      os << "PICCR" << ( i+1 ) << " : "
         << piEdgeVec[i]->getRCLoad( *this ) << " < "
         << piEdgeVec[i]->getPI().capToString() << ";" << endl;

      os << "PICCF" << ( i+1 ) << " : "
         << piEdgeVec[i]->getFCLoad( *this ) << " < "
         << piEdgeVec[i]->getPI().capToString() << ";" << endl;
   }

   return os;
}

ostream & opt_prob_generator::PITimingConstraintsToOstream( ostream & os ) {
   cout << "Forming the input timing constraints, as specified" << endl;

   for( int i = 0; i < piEdgeVec.size(); i ++ ) {
      if( ! piEdgeVec[i]->isPI() ) {
         errorReport( "Creating input timing constraints for " + getName() + " which is not a PI" );
      }
      double aT;
      if( ( aT = piEdgeVec[i]->getPI().getT() ) < 0.0 ) {
         errorReport( "The arrival time of PI " + getName() + " is negative :  not allowed" );
      }

      double valR = piEdgeVec[i]->getPreValue( true );
      double valF = piEdgeVec[i]->getPreValue( false );
      if( aT == 0.0 ) {
         os << "PITCR" << ( i+1 ) << " : "
            << valR << " < "
            << piEdgeVec[i]->getTRiseName( EMPTY_STRING ) << ";" << endl;

         if( ! isNoRiseFall() ) {
            os << "PITCF" << ( i+1 ) << " : "
               << valF << " < "
               << piEdgeVec[i]->getTFallName( EMPTY_STRING ) << ";" << endl;
         }
      } else {
         os << "PITCR" << ( i+1 ) << " : "
            << valR << " + "
            << cnvt::doubleToString( aT ) << " < "
            << piEdgeVec[i]->getTRiseName( EMPTY_STRING ) << ";" << endl;

         os << "PITCF" << ( i+1 ) << " : "
            << valF << " + "
            << cnvt::doubleToString( aT ) << " < "
            << piEdgeVec[i]->getTFallName( EMPTY_STRING ) << ";" << endl;
      }
   }
   return os;
}

ostream & opt_prob_generator::globalConstraintsToOstream( ostream & os ) {
   cout << "Formulating global constraints, if any" << endl;
   for( unsigned i = 0; i < constraintVector.size(); i ++ )
      os << "GL" << ( i+1 ) << " : " << constraintVector[i] << ";" << endl;

   return os;
}


ostream & opt_prob_generator::widthConstraintsToOstream( ostream & os ) {
   cout << "Formulating width bounds, if any are specified" << endl;
   map<string, node *> & node_map = getNetwork().getNodeMap();
   map<string, node *>::iterator itr;


   if( minWidth <= 0.0 && maxWidth <= 0.0 )
      return os;


   for( itr = node_map.begin(); itr != node_map.end(); itr ++ ) {
      node & nd = * itr->second;

      //Avoid repeating the width constraints by recording them in a string.
      string minWStr= ",";
      string maxWStr = ",";
      vector<const gposy *> WNnames;
      vector<const gposy *> WPnames;
      string W;
      string Wmin;
      if( nd.isCapacitor() || nd.isVRL() )
         continue;

      unsigned num = 1;
      for( unsigned i = 0; i < nd.getNumberOfInputs(); i ++ ) {
         WNnames = nd.getCCC().getWNNameVec( i );
         for ( unsigned j = 0; j < WNnames.size(); j ++ ) {
            gposy & Winv = ( *WNnames[j] )^-1.0;
            Wmin = Winv.toString( nd.getCCC().getSymbolTable(),nd.getvName(),EMPTY_STRING,EMPTY_STRING );
            W = WNnames[j]->toString( nd.getCCC().getSymbolTable(),nd.getvName(),EMPTY_STRING,EMPTY_STRING );
            if( minWidth > 0.0 ) {
               if( minWStr.find( ","+Wmin+"," ) == string::npos ) {
                  minWStr = minWStr + Wmin + ",";
                  os << "MINWIDTH__" << nd.getName() << "__W" << num << ": " << minWidth << " " << Wmin << " <  1;" << endl;
                  num ++;
               }
            }
            if( maxWidth > 0.0 ) {
               if( maxWStr.find( ","+W+"," ) == string::npos ) {
                  maxWStr = maxWStr + W + ",";
                  os << "MAXWIDTH__" << nd.getName() << "__W" << num << ": " << W << " < " << maxWidth << ";" << endl;
                  num ++;
               }
            }
            delete & Winv;
         }

         WPnames = nd.getCCC().getWPNameVec( i );
         for ( unsigned j = 0; j < WPnames.size(); j ++ ) {
            gposy & Winv = ( *WPnames[j] )^-1.0;
            Wmin = Winv.toString( nd.getCCC().getSymbolTable(),nd.getvName(),EMPTY_STRING,EMPTY_STRING );
            W = WPnames[j]->toString( nd.getCCC().getSymbolTable(),nd.getvName(),EMPTY_STRING,EMPTY_STRING );
            if( minWidth > 0.0 ) {
               if( minWStr.find( ","+Wmin+"," ) == string::npos ) {
                  minWStr = minWStr + Wmin + ",";
                  os << "MINWIDTH__" << nd.getName() << "__W" << num << ": " << minWidth << " " << Wmin << " < 1;" << endl;
                  num ++;
               }
            }
            if( maxWidth > 0.0 ) {
               if( maxWStr.find( ","+W+"," ) == string::npos ) {
                  maxWStr = maxWStr + W + ",";
                  os << "MAXWIDTH__" << nd.getName() << "__W" << num << ": " << W << " < " << maxWidth << ";" << endl;
                  num ++;
               }
            }
            delete & Winv;
         }
      }
   }

   return os;
}

ostream & opt_prob_generator::vthConstraintsToOstream( ostream & os ) {
   cout << "Formulating gate vth bounds, if any are specified" << endl;
   map<string, node *> & node_map = getNetwork().getNodeMap();
   map<string, node *>::iterator itr;

   double invMinVthP, invMinVthN;
   if( maxVthP <= 0.0 && maxVthN <= 0.0 && minVthP <= 0.0 && minVthN <= 0.0 )
      return os;

   if( maxVthP <= 0.0 || maxVthN <= 0.0 || maxVthP <= minVthP || maxVthN <= minVthN ) {
      ciropterror( "ERROR: The max values of VthN or VthP are incorrect or less than/equal to the min values" );
      return os;
   }

   if( glbVars.find( "VthP" ) == glbVars.end() ) {
      for( itr = node_map.begin(); itr != node_map.end(); itr ++ ) {
         node & nd = * itr->second;
         if( nd.isCapacitor() || nd.isVRL() ) continue;
         if( minVthP > 0.0 ) {
            os << "MINVTHP__" << nd.getName() << ": " <<minVthP << " "<< nd.getvName() << ".VthP ^ -1 < " << "1;" << endl;
         }
         if( maxVthP >= 0.0 ) {
            os << "MAXVTHP__" << nd.getName() << ": " << nd.getvName() << ".VthP < " << maxVthP << ";" << endl;
         }
      }
   } else {
      if( minVthP > 0.0 ) {
         os << "MINVTHP__GLOBAL : " <<minVthP << " VthP ^ -1 < 1;" << endl;
      }
      if( maxVthP >= 0.0 ) {
         os << "MAXVTHP__GLOBAL : VthP < " << maxVthP << ";" << endl;
      }
   }

   if( glbVars.find( "VthN" ) == glbVars.end() ) {
      for( itr = node_map.begin(); itr != node_map.end(); itr ++ ) {
         node & nd = * itr->second;
         if( nd.isCapacitor() || nd.isVRL() ) continue;
         if( minVthN > 0.0 ) {
            os << "MINVTHN__" << nd.getName() << ": " << minVthN << " " << nd.getvName() << ".VthN ^ -1 < " << 1 << ";" << endl;
         }
         if( maxVthN > 0.0 ) {
            os << "MAXVTHN__" << nd.getName() << ": " << nd.getvName() << ".VthN  < " << maxVthN << ";" << endl;
         }
      }
   } else {
      if( minVthN > 0.0 ) {
         os << "MINVTHN__GLOBAL : " <<minVthN << " VthN ^ -1 < 1;" << endl;
      }
      if( maxVthP >= 0.0 ) {
         os << "MAXVTHN__GLOBAL : VthN < " << maxVthN << ";" << endl;
      }
   }

   return os;
}


ostream & opt_prob_generator::vddConstraintsToOstream( ostream & os ) {
   cout << "Formulating vdd bounds, if any are specified" << endl;
   if( maxVdd == -1.0 && minVdd == -1.0 )
      return os;
   if( maxVdd == 0.0 || maxVdd <= minVdd ) {
      ciropterror( "ERROR: The max value of Vdd is 0.0 or less than/equal to the min value" );
      return os;
   }
   if( minVdd > 0.0 ) {
      os << "MINVDD : " << minVdd << " Vdd^-1 < 1 ;" << endl;
   }
   os << "MAXVDD : Vdd < " << maxVdd << ";" << endl;
   return os;
}


ostream & opt_prob_generator::gateDelayConstraintsToOstream( ostream & os ) {
   cout << "Forming Gate Delay constraints for slope" << endl;

   gateDelayConstraintsTypeIandIIToOstream( os );

   return os;
}

ostream & opt_prob_generator::gateDelayConstraintsTypeIandIIToOstream( ostream & os ) {
   ostream & o = os;
   map<string,node *> & nodeMap = netw.getNodeMap();
   map<string,string> & I = gateDelayCnstTypeI;
   map<string,map<string,string> >
   & II = gateDelayCnstTypeII;

   map<string, node *>::iterator itr;
   map<string,string>::iterator itr1;
   map<string,map<string,string> >::iterator itr2;

   if( ( itr1 = I.find( "all" ) ) != I.end() ) {
      string val = itr1->second;

      for( itr = nodeMap.begin(); itr != nodeMap.end(); itr ++ )
         gateDelayConstraintsTypeIAllToOstream( os, * itr->second, val );
   }

   if( ( itr2 = II.find( "all" ) ) != II.end() ) {
      map<string,string> & val = itr2->second;

      for( itr = nodeMap.begin(); itr != nodeMap.end(); itr ++ )
         gateDelayConstraintsTypeIIAllToOstream( os, * itr->second, val );
   }
   string n ;
   for( itr1 = I.begin(); itr1 != I.end(); itr1 ++ ) {
      n = itr1->first;
      gateDelayConstraintsTypeIToOstream( os, n, itr1->second );
   }

   for( itr2 = II.begin(); itr2 != II.end(); itr2 ++ ) {
      n = itr2->first;
      gateDelayConstraintsTypeIIToOstream( os, n, itr2->second );
   }

   return os;
}


ostream & opt_prob_generator::gateDelayConstraintsTypeIAllToOstream( ostream & os, node & nd, const string & val ) {
   if( nd.isCapacitor() || nd.isVRL() ) return os;

   string  name = nd.getName();
   string meanRF;
   string meanFR;

   map<string,string> & I = gateDelayCnstTypeI;
   map<string,map<string,string> >
   & II = gateDelayCnstTypeII;

   map<string,map<string,string> >::iterator itr2;

   /*
    * "D({gate name}) < {num}" overwrites "D( all ) < {num}"
    */

   if( I.find( name ) != I.end() ) return os;

   /*
    * "D(all,rf or fr) < {num}" and "D({gate name},rf or fr) < {num}"
    * overwrite "D(all) < {num}"
    */
   if(
      !( ( ( itr2 = II.find( "all" ) ) != II.end()
           && itr2->second.find( "rf" ) != itr2->second.end() )
         ||
         ( ( itr2 = II.find( name ) ) != II.end()
           && itr2->second.find( "rf" ) != itr2->second.end() )
       )
   ) {
      for( unsigned i = 0; i < nd.getNumberOfInputs(); i ++ ) {
         if( nd.getCCC().getInputName( i ).substr( 0,3 ) == KEEPER_INPUT ) continue;
         for( unsigned j = 0; j < nd.getNumberOfOutputs(); j ++ ) {
            if( nd.getWNName( i ) != EMPTY_STRING
                  && nd.getDioRF( i, j, *this ) != EMPTY_STRING && nd.getDioRF( i, j, *this ) != ERROR_STRING ) {
               if( isNoRiseFall() )
                  os << "SL__" << nd.getName() << "__" << nd.getCCC().getInputName( i )<< "To"<< nd.getCCC().getOutputName( j )  << ": " ;
               else
                  os << "SL__" << nd.getName() << "__" << nd.getCCC().getInputName( i )<< "To"<< nd.getCCC().getOutputName( j )  << "_RF : " ;
               if( nd.getRFslopeRecord( i,j ) == 1 ) {
                  meanRF = "mean"+nd.getName()+"__"+nd.getCCC().getInputName( i ) +"To"+nd.getCCC().getOutputName( j )+"_RF";
                  os << meanRF << " < " << val << ";" << endl;
               } else
                  os << nd.getDioRF( i, j, *this ) << " < " << val << ";" << endl;
            }
         }
      }
   }

   if(
      !( ( ( itr2 = II.find( "all" ) ) != II.end()
           && itr2->second.find( "fr" ) != itr2->second.end() )
         ||
         ( ( itr2 = II.find( name ) ) != II.end()
           && itr2->second.find( "fr" ) != itr2->second.end() )
       )
   ) {
      for( unsigned i = 0; i < nd.getNumberOfInputs(); i ++ ) {
         if( nd.getCCC().getInputName( i ).substr( 0,3 ) == KEEPER_INPUT ) continue;
         for( unsigned j = 0; j < nd.getNumberOfOutputs(); j ++ ) {
            if( nd.getWPName( i ) != EMPTY_STRING
                  && nd.getDioFR( i, j, *this ) != EMPTY_STRING && nd.getDioFR( i, j, *this ) != ERROR_STRING ) {
               if( ! isNoRiseFall() ) {
                  os << "SL__" << nd.getName() << "__" << nd.getCCC().getInputName( i ) <<"To" << nd.getCCC().getOutputName( j ) << "_FR : " ;
                  if( nd.getFRslopeRecord( i,j ) == 1 ) {
                     meanFR = "mean"+nd.getName()+"__"+nd.getCCC().getInputName( i ) +"To"+nd.getCCC().getOutputName( j )+"_FR";
                     os << meanFR << " < " << val << ";" << endl;
                  } else
                     os << nd.getDioFR( i, j, *this ) << " < " << val << ";" << endl;
               }
            }
         }
      }
   }

   return os;
}

ostream & opt_prob_generator::gateDelayConstraintsTypeIToOstream( ostream & os, const string & name, const string & val ) {
   if( name == "all" ) return os;

   map<string,  node *>
   & nm = netw.getNodeMap();
   map<string,map<string,string> >
   & II = gateDelayCnstTypeII;

   map<string, node *>::iterator itr;
   map<string,map<string,string> >::iterator itr2;

   assert( ( itr = nm.find( name ) ) != nm.end() );

   node & nd = * itr->second;
   string meanRF;
   string meanFR;
   if( nd.isCapacitor() || nd.isVRL() ) return os;
   /*
    * "D({gate name}, rf or fr) < {num}" overwrites "D({gate name}) < {num}"
    */
   if( !( ( itr2 = II.find( name ) ) != II.end()
          && itr2->second.find( "rf" ) != itr2->second.end() ) ) {
      for( unsigned i = 0; i < nd.getNumberOfInputs(); i ++ ) {
         if( nd.getCCC().getInputName( i ).substr( 0,3 ) == KEEPER_INPUT ) continue;
         for( unsigned j = 0; j < nd.getNumberOfOutputs(); j ++ ) {
            if( nd.getWNName( i ) != EMPTY_STRING
                  && nd.getDioRF( i,j, *this ) != EMPTY_STRING && nd.getDioRF( i,j, *this ) != ERROR_STRING )

            {
               if( isNoRiseFall() )
                  os << "SL__" << nd.getName() << "__" << nd.getCCC().getInputName( i )<<"To"<< nd.getCCC().getOutputName( j ) << ": " ;
               else
                  os << "SL__" << nd.getName() << "__" << nd.getCCC().getInputName( i )<<"To"<< nd.getCCC().getOutputName( j ) << "_RF : " ;

               if( nd.getRFslopeRecord( i,j ) == 1 ) {
                  meanRF = "mean"+nd.getName()+"__"+nd.getCCC().getInputName( i ) +"To"+nd.getCCC().getOutputName( j )+"_RF";
                  os << meanRF << " < " << val << ";" << endl;
               } else
                  os << nd.getDioRF( i,j, *this ) << " < " << val << ";" << endl;
            }
         }
      }
   }

   if( !( ( itr2 = II.find( name ) ) != II.end()
          && itr2->second.find( "fr" ) != itr2->second.end() ) ) {
      for( unsigned i = 0; i < nd.getNumberOfInputs(); i ++ ) {
         if( nd.getCCC().getInputName( i ).substr( 0,3 ) == KEEPER_INPUT ) continue;
         for( unsigned j = 0; j < nd.getNumberOfOutputs(); j ++ ) {
            if( nd.getWPName( i ) != EMPTY_STRING
                  && nd.getDioFR( i,j, *this ) != EMPTY_STRING && nd.getDioFR( i,j, *this ) != ERROR_STRING ) {
               if( ! isNoRiseFall() ) {
                  os << "SL__" << nd.getName() << "__" << nd.getCCC().getInputName( i )<<"To" << nd.getCCC().getOutputName( j ) << "_FR : " ;
                  if( nd.getFRslopeRecord( i,j ) == 1 ) {
                     meanFR = "mean"+nd.getName()+"__"+nd.getCCC().getInputName( i ) +"To"+nd.getCCC().getOutputName( j )+"_FR";
                     os << meanFR << " < " << val << ";" << endl;
                  } else
                     os << nd.getDioFR( i,j, *this ) << " < " << val << ";" << endl;
               }
            }
         }
      }
   }

   return os;
}

ostream & opt_prob_generator::gateDelayConstraintsTypeIIAllToOstream( ostream & os, node & nd,map<string,string> & val ) {
   if( nd.isCapacitor() || nd.isVRL() ) return os;

   string  name = nd.getName();

   map<string,string>
   & I = gateDelayCnstTypeI;
   map<string,map<string,string> >
   & II = gateDelayCnstTypeII;

   map<string,map<string,string> >::iterator itr2;
   map<string,string>::iterator itr;
   string meanRF;
   string meanFR;

   /*
    * "D({gate name}) < {num}" overwrites "D(all, rf or fr ) < {num}"
    */
   if( I.find( name ) != I.end() ) return os;

   /*
    * "D({gate name}, rf or fr ) < {num}" overwrites "D(all, rf or fr ) < {num}"
    */
   for( itr = val.begin(); itr != val.end(); itr ++ ) {
      if( !( ( itr2 = II.find( name ) ) != II.end()
             && itr2->second.find( itr->first ) != itr2->second.end() ) ) {
         if( itr->first == "rf" ) {
            for( unsigned i = 0; i < nd.getNumberOfInputs(); i ++ ) {
               if( nd.getCCC().getInputName( i ).substr( 0,3 ) == KEEPER_INPUT ) continue;
               for( unsigned j = 0; j < nd.getNumberOfOutputs(); j ++ ) {
                  if( nd.getWNName( i ) != EMPTY_STRING && nd.getDioRF( i,j, *this ) != EMPTY_STRING && nd.getDioRF( i,j, *this ) != ERROR_STRING ) {
                     if( isNoRiseFall() )
                        os << "SL__" << nd.getName() << "__" << nd.getCCC().getInputName( i )<<"To"<<nd.getCCC().getOutputName( j ) << ": ";
                     else
                        os << "SL__" << nd.getName() << "__" << nd.getCCC().getInputName( i )<<"To"<<nd.getCCC().getOutputName( j ) << "_RF : ";

                     if( nd.getRFslopeRecord( i,j ) == 1 ) {
                        meanRF = "mean"+nd.getName()+"__"+nd.getCCC().getInputName( i ) +"To"+nd.getCCC().getOutputName( j )+"_RF";
                        os << meanRF << " < " << itr->second << ";" << endl;
                     } else
                        os << nd.getDioRF( i,j, *this ) << " < " << itr->second << ";" << endl;
                  }
               }
            }
         } else if( itr->first == "fr" ) {
            for( unsigned i = 0; i < nd.getNumberOfInputs(); i ++ ) {
               if( nd.getCCC().getInputName( i ).substr( 0,3 ) == KEEPER_INPUT ) continue;
               for( unsigned j = 0; j < nd.getNumberOfOutputs(); j ++ ) {
                  if( nd.getWPName( i ) != EMPTY_STRING && nd.getDioFR( i,j, *this ) != EMPTY_STRING && nd.getDioFR( i,j, *this ) != ERROR_STRING ) {
                     if( ! isNoRiseFall() ) {
                        os << "SL__" << nd.getName() << "__" << nd.getCCC().getInputName( i )<< "To"<<nd.getCCC().getOutputName( j ) << "_FR : " ;
                        if( nd.getFRslopeRecord( i,j ) == 1 ) {
                           meanFR = "mean"+nd.getName()+"__"+nd.getCCC().getInputName( i ) +"To"+nd.getCCC().getOutputName( j )+"_FR";
                           os << meanFR << " < " << itr->second << ";" << endl;
                        } else
                           os << nd.getDioFR( i,j, *this ) << " < " << itr->second << ";" << endl;
                     }
                  }
               }
            }
         } else
            assert( false );
      }
   }

   return os;
}

ostream & opt_prob_generator::gateDelayConstraintsTypeIIToOstream( ostream & os, const string & gatename, map<string,string> & val ) {
   if( gatename == "all" ) return os;

   map<string,  node *> & nm = netw.getNodeMap();
   map<string,map<string,string> >
   & II = gateDelayCnstTypeII;

   map<string,  node *>::iterator nditr;
   map<string,string>::iterator itr;
   string meanRF;
   string meanFR;
   if( ( nditr = nm.find( name ) ) == nm.end() ) {
      cout << "Forming special delay/stage constraints for " << name << endl;
      errorReport( "The node name " + name + " is not in the list of gates.\n" );
   }
   node & nd = * nditr->second;
   if( nd.isCapacitor() || nd.isVRL() ) return os;

   for( itr = val.begin(); itr != val.end(); itr ++ ) {
      if( itr->first == "rf" ) {
         for( unsigned i = 0; i < nd.getNumberOfInputs(); i ++ ) {
            if( nd.getCCC().getInputName( i ).substr( 0,3 ) == KEEPER_INPUT ) continue;
            for( unsigned j = 0; j < nd.getNumberOfOutputs(); j ++ ) {
               if( nd.getWNName( i ) != EMPTY_STRING && nd.getDioRF( i,j, *this ) != EMPTY_STRING&& nd.getDioRF( i,j, *this ) != ERROR_STRING ) {
                  if( isNoRiseFall() )
                     os << "SL__" << nd.getName() << "__" << nd.getCCC().getInputName( i )<< "To" <<nd.getCCC().getOutputName( j )  << ": " ;
                  else
                     os << "SL__" << nd.getName() << "__" << nd.getCCC().getInputName( i )<< "To" <<nd.getCCC().getOutputName( j )  << "_RF : " ;

                  if( nd.getRFslopeRecord( i,j ) == 1 ) {
                     meanRF = "mean"+nd.getName()+"__"+nd.getCCC().getInputName( i ) +"To"+nd.getCCC().getOutputName( j )+"_RF";
                     os << meanRF << " < " << itr->second << ";" << endl;
                  } else
                     os << nd.getDioRF( i,j, *this ) << " < " << itr->second << ";" << endl;
               }
            }
         }
      } else if( itr->first == "fr" ) {
         for( unsigned i = 0; i < nd.getNumberOfInputs(); i ++ ) {
            if( nd.getCCC().getInputName( i ).substr( 0,3 ) == KEEPER_INPUT ) continue;
            for( unsigned j = 0; j < nd.getNumberOfOutputs(); j ++ ) {
               if( nd.getWPName( i ) != EMPTY_STRING && nd.getDioFR( i,j, *this ) != EMPTY_STRING&& nd.getDioFR( i,j, *this ) != ERROR_STRING ) {
                  if( ! isNoRiseFall() ) {
                     os << "SL__" << nd.getName() << "__" << nd.getCCC().getInputName( i )<< "To" << nd.getCCC().getOutputName( j ) << "_FR : " ;
                     if( nd.getFRslopeRecord( i,j ) == 1 ) {
                        meanFR = "mean"+nd.getName()+"__"+nd.getCCC().getInputName( i ) +"To"+nd.getCCC().getOutputName( j )+"_FR";
                        os << meanFR << " < " << itr->second << ";" << endl;
                     } else
                        os << nd.getDioFR( i,j, *this ) << " < " << itr->second << ";" << endl;
                  }
               }
            }
         }
      } else
         assert( false );
   }

   return os;
}



// deterministic formulation
void opt_prob_generator::solve( opt_spec & optspec ) {
   cout << "Solve 2" << endl;

   if( optspec.isByOutFileName() ) {
      cout << "Returning" << endl;
      return;
   }

   string wtm = optspec.getObjective();
   cout << "Solve for " << wtm << endl;


   // clear optimal values history vector and montecarlo history vector.
   if( wtm == "area"   ) {
      solve( optspec.getName(), optspec );
      return;
   }
   if( wtm == "energy" ) {
      solve( optspec.getName(), optspec );
      return;
   }
   if( wtm == "delay"  ) {
      solve( optspec.getName(), optspec );
      return;
   }
   if( wtm == "expected_cmax" ) {
      solve( optspec.getName(), optspec );
      return;
   }
   if( wtm == "stat_delay_update"|| wtm == "stat_delay_CritUpdate"  ) {
      solve
      ( optspec.getName(),
        optspec.getNumberOfOptimizations(),
        optspec.getMonte(),
        optspec.getQuantileValue(),
        optspec.getKmax(),
        optspec );

      return;
   }

   if( wtm == "stat_delay_kappas" || wtm == "stat_delay_betas" ) {
      solve( optspec.getName(),optspec.getKappas(),optspec.getBetas(),optspec );
      return;
   }

   if( wtm == "stat_delay_kappabeta" ) {
      solve( optspec.getName(), optspec.getKappa(), optspec.getBeta(), optspec );
      return;
   }

   assert( false );
}

// deterministic formulation
void opt_prob_generator::solve( const string & nm, opt_spec & osp ) {
   
   string name;
   
   cout << "Solve 3" << endl;
   
   const char * scotHomePtr    = getenv( "SCOT_HOME_DIR" );
   const char * scotTempPtr    = getenv( "SCOT_TEMP_DIR_OPTIM_DOT_PY" );
   const string scotHome       = string( scotHomePtr );
   const string scotTemp       = string( scotTempPtr );

   if     ( osp.getObjective() == "delay"  ) {
      name = nm + "DDET";
   }
   else if( osp.getObjective() == "energy" ) {
      name = nm + "EDET";
   }
   else if( osp.getObjective() == "area"   ) {
      name = nm + "ADET";
   }
   else if( osp.getObjective() == "expected_cmax" ) {
      name = nm + "SCENARIO";
   }
   else {
      assert( false );
   }

   name = scotTemp + "/" + name;
   
   const string commandLine    = scotHome + "/ggpsolexp/bin/ggpsolexp -d " + name;
   const string outputfilename = name + ".out";
   
   map< string, double > optValues;

   writeGGPSOLInput( name, osp );

   errorBasedHalt();
   if( !OnlyFormulateProblem ) {
      SystemWrapper( commandLine );
      if( ggpsolOutToMap( optValues, outputfilename ) != 0 ) {
         errorReport( "ggpsolexp output reading error: " + outputfilename );
         return;
      }
   }
   else {
      cout << "Only Formulating the Problem in file: " << name << endl;
   }

   if( ERASE_GGPSOL_SOURCE_FILE ) {
	   
      const string rmcmd = "rm " + name;
	   SystemWrapper( rmcmd );
      // -- TODO -- remove? -- did errorReport provide any real value here? -- if( system( ( "rm "+ name ).c_str() ) != 0 ) {
      // -- TODO -- remove? -- did errorReport provide any real value here? --    errorReport( "file remove file error: " + name );
      // -- TODO -- remove? -- did errorReport provide any real value here? -- }
   }

   osp.store( optValues );

   return;
}

// iterative statistical formulation
void opt_prob_generator::solve( const string & name, unsigned numIter, const monte_carlo & monte, double p, double kmax, opt_spec & osp ) {
   cout << "Solve 4" << endl;
   string  nameroot;
   if( osp.getObjective() == "stat_delay_update" ) {
      if( kmax != 0 ) {
         cout << kmax << "  is the kmax" << endl;
         assert( false );
      }
      nameroot = name + "ISTT";
      for( unsigned i = 0; i < numIter; i ++ ) {
         solve( nameroot+cnvt::intToString( i+1 ), monte, p, kmax, osp );
      }
      return;
   } else if( osp.getObjective() == "stat_delay_CritUpdate" ) {
      assert( kmax > 0 );
      nameroot = name + "CSTT";
      for( unsigned i = 0; i < numIter; i ++ ) {
         solve( nameroot+cnvt::intToString( i+1 ), monte, p, kmax, osp );
      }
      return;
   } else {
      assert( false );
   }

}

void opt_prob_generator::solve( const string & name, const monte_carlo & monte, double p, double kmax, opt_spec & osp ) {
   cout << "Solve ISTT/CSTT" << endl;
   map<string,double> optVs;
   map<string,ProbDist *> mcVs;
   map<string,double> crtclty;

   const char * scotHomePtr   = getenv( "SCOT_HOME_DIR" );
   const string scotHome = string( scotHomePtr );
   const string commandLine    = scotHome + "/ggpsolexp/bin/ggpsolexp -d " + name;
   const string outputfilename = name + ".out";

   writeGGPSOLInput( name, osp );

   errorBasedHalt();

   if( !OnlyFormulateProblem ) {
      
      SystemWrapper( commandLine );

      if( ggpsolOutToMap( optVs, outputfilename ) != 0 ) {
         errorReport( "ggpsol output reading error: " + outputfilename );
         return;
      }
   }
   else {
      cout << "Only Formulating the Problem in file: " << name << endl;
      return;
   }

   if( ERASE_GGPSOL_SOURCE_FILE ) {
      
      const string rmcmd = "rm " + name;
	   SystemWrapper( rmcmd );
      // -- TODO -- remove -- did error report provide any value here? -- if( system( ( "rm "+name ).c_str() ) != 0 ) {
      // -- TODO -- remove -- did error report provide any value here? --    errorReport( "file remove file error: " + name );
      // -- TODO -- remove -- did error report provide any value here? -- }
   }
   osp.store ( optVs );
   montecarlo( mcVs, crtclty, monte, p, osp, osp.getLengthOfOptValHistory()-1, kmax );
   osp.store ( mcVs, crtclty, monte );
   return;
}

// uniform quantile statistical optimization
void opt_prob_generator::solve( const string & name, vector<double> & kappas, vector<double> & betas, opt_spec & osp ) {
   cout << "Solve 7" << endl;
   assert( isObjectiveABs( osp.getObjective() ) );
   assert( kappas.size() == betas.size() );


   string  nameroot = name + "USTT";

   for( unsigned i = 0; i < kappas.size(); i ++ ) {
      solve( nameroot + cnvt::intToString( i+1 ), kappas[i], betas[i], osp );
   }

   return;
}

void opt_prob_generator::solve( const string & name, double kappa, double beta, opt_spec & osp ) {
   cout << "Solve 8" << endl;
   string outputfilename = name + ".out";
   map<string,double> optVs;

   setAllKappasBetas( kappa, beta );
   writeGGPSOLInput( name, osp );

   errorBasedHalt();

   if( !OnlyFormulateProblem ) {
      const string ggpsolcmd = "ggpsol -d " + name;
      SystemWrapper( ggpsolcmd );
      if( ggpsolOutToMap( optVs, outputfilename ) != 0 ) {
         errorReport( "ggpsol output reading error: " + outputfilename );
         return;
      }
   }
   else {
      cout << "Only Formulating the Problem in file: " << name << endl;
   }
   if( ERASE_GGPSOL_SOURCE_FILE ) {
      const string rmcmd = "rm " + name;
      SystemWrapper( rmcmd );
      // -- TODO -- remove -- did error report provide any real value here? -- if( system( ( "rm "+name ).c_str() ) != 0 )
      // -- TODO -- remove -- did error report provide any real value here? --    errorReport( "file remove file error: " + name );
   }

   osp.store( optVs );

   return;
}

void opt_prob_generator::setAllKappasBetas( double kappa, double beta ) {
   Beta = beta;
   netw.setKappaBetaForAllNodes( kappa, beta );
}


void opt_prob_generator::montecarlo( map<string,ProbDist *> & monteMap, map<string,double> & criticality, const monte_carlo & monte, double p, opt_spec & osp, unsigned num , double kmax ) {
   cout << "MonteCarlo 1" << endl;
   assert( monteMap.size() == 0 );
   assert( criticality.size() == 0 );

   map<string,vector<unsigned> > netSlctVecMap;
   montecarlo( monteMap, netSlctVecMap, monte, p, osp, num );

   unsigned n = netSlctVecMap.size(); // number of nets (double if Rise Fall time is separate)
   size_t N = monte.getN(); // number of montecarlo simulations

   // assert that every vector has the same size
   map<string,vector<unsigned> >::iterator itr;
   for( itr = netSlctVecMap.begin(); itr != netSlctVecMap.end(); itr ++ ) {
      if( itr->second.size() != N )
         ciropterror( "Cannot calculate criticalities: Improper Monte Carlo for net " + itr->first );
      criticality[itr->first] = 0.0;
   }
   assert( criticality.size() == n );

   //calculate criticality information
   map<string,unsigned> netSlctMap; //specify which input number was critical for the output in a particular monte carlo run
   for( unsigned i = 0; i < N; i ++ ) {
      for( itr = netSlctVecMap.begin(); itr != netSlctVecMap.end(); itr ++ )
         netSlctMap[itr->first] = itr->second[i];

      assert( netSlctMap.size() == n );

      addOneToCriticalNets( criticality, netSlctMap );
   }
   for( unsigned i=0; i<piEdgeVec.size(); i++ ) {
      if( criticality.find( piEdgeVec[i]->getMonteTRiseName() ) == criticality.end() )
         criticality[piEdgeVec[i]->getMonteTRiseName()] = 0.0;
      if( isNoRiseFall() )
         continue;
      if( criticality.find( piEdgeVec[i]->getMonteTFallName() ) == criticality.end() )
         criticality[piEdgeVec[i]->getMonteTFallName()] = 0.0;
   }
   if( isNoRiseFall() )
      assert( criticality.size() == ( n + piEdgeVec.size() ) );
   else
      assert( criticality.size() == ( n + 2*piEdgeVec.size() ) );

   map<string,double>::iterator citr;
   double maxCrit = 0;
   for( citr = criticality.begin(); citr != criticality.end(); citr ++ ) {
      if( isNoRiseFall() )
         citr->second = citr->second / ( ( double )N ); //normalize the criticality to 1
      else
         citr->second = citr->second / ( 2*( double )N ); // the factor of 2 is to account
      //for merging rise and fall times to keep criticality of each net.Trise/Tfall to be < 1.
      //To get the net criticality, one then just needs to add Trise and Tfall criticalities
      if( citr->second > maxCrit )
         maxCrit = citr->second;
   }


   //this is done only for iterative statistical solution, in all other cases, its
   //not needed, so exit.
   if( osp.getObjective() != "stat_delay_update" && osp.getObjective() != "stat_delay_CritUpdate" )
      return;

   map<string, node *> & nodemap = netw.getNodeMap();
   map<string, node *>::iterator  nitr;
   for( nitr = nodemap.begin(); nitr != nodemap.end(); nitr ++ ) {
      node * curNode = nitr->second;
      if( curNode->isVRL() || curNode->isCapacitor() ) continue;
      for( unsigned no=0; no < curNode->getNumberOfOutputs(); no++ ) {
         string critNameRise = ( curNode->getOutEdge( no ) ).getName() + ".Trise";
         if( ( citr = criticality.find( critNameRise ) ) == criticality.end() )
            errorReport( "The rise criticality of " + critNameRise + " is not found" );
         if( citr->second < 0 || citr->second > 1 ) {
            cout << "Error: Incorrect criticality" << endl;
            cout << "Here is the list of net criticalities" << endl;
            for( citr = criticality.begin(); citr != criticality.end(); citr ++ )
               cout << citr->first << " : " << citr->second << endl;
            errorReport( "The criticality of net" + ( curNode->getOutEdge( no ) ).getName()  + "at the ouput of the node " + curNode->getName() + " is incorrect" );
         }
         curNode->updateCritKappa( false, no, citr->second, osp.getObjective(), kmax, maxCrit );
         if( isNoRiseFall() )
            continue;

         string critNameFall = ( curNode->getOutEdge( no ) ).getName() + ".Tfall";
         if( ( citr = criticality.find( critNameFall ) ) == criticality.end() )
            errorReport( "The fall criticality of " + critNameFall + " is not found" );
         if( citr->second < 0 || citr->second > 1 ) {
            cout << "Error: Incorrect criticality" << endl;
            cout << "Here is the list of net criticalities" << endl;
            for( citr = criticality.begin(); citr != criticality.end(); citr ++ )
               cout << citr->first << " : " << citr->second << endl;
            errorReport( "The criticality of net " +  ( curNode->getOutEdge( no ) ).getName()+ " which is the ouput of the node " + curNode->getName() + " is incorrect" );
         }
         curNode->updateCritKappa( true, no, citr->second,osp.getObjective(), kmax, maxCrit );
      }
   }
   return;
}

void opt_prob_generator::montecarlo( map<string,ProbDist *> & monteMap, map<string,vector<unsigned> > & netSlctVecMap, const monte_carlo & monte, double p, opt_spec & osp, unsigned num ) {
   cout << "MonteCarlo 2" << endl;
   assert( monteMap.size() == 0 );
   assert( haveTheNetListAssigned );
   assert( num < osp.getLengthOfOptValHistory() );

   map<string,double> & optVals = osp.getOptValues( num );


   // start!
   double s = clock();
   cout << "New Monte Carlo " << endl;
   netw.montecarlo( monteMap, netSlctVecMap, monte, p, optVals, isNoRiseFall() );

   // get the max of the primary outputs
   map<string,ProbDist * >::iterator ritr, fitr;
   vector<ProbDist *> poRisePDs, poFallPDs;

   for( unsigned i = 0; i < poEdgeVec.size(); i ++ ) {
      ritr = monteMap.find( poEdgeVec[i]->getTRiseName() );
      assert( ritr != monteMap.end() );
      poRisePDs.push_back( ritr->second );
      if( isNoRiseFall() )
         continue;

      fitr = monteMap.find( poEdgeVec[i]->getTFallName() );
      assert( fitr != monteMap.end() );
      poFallPDs.push_back( fitr->second );
   }

   assert( monteMap.find( MAX_OF_PRIMARY_OUTPUTS_NAME ) == monteMap.end() );
   assert( monteMap.find( MAX_OF_RISE_PRIMARY_OUTPUTS_NAME ) == monteMap.end() );

   assert( netSlctVecMap.find( MAX_OF_PRIMARY_OUTPUTS_NAME ) == netSlctVecMap.end() );
   assert( netSlctVecMap.find( MAX_OF_RISE_PRIMARY_OUTPUTS_NAME )==netSlctVecMap.end() );

   vector<unsigned> & rNSV = netSlctVecMap[MAX_OF_RISE_PRIMARY_OUTPUTS_NAME];

   ProbDist & poRiseMaxPD
      = * ( monteMap[MAX_OF_RISE_PRIMARY_OUTPUTS_NAME]
            = ProbDist::createMaxProbDist( rNSV, poRisePDs ) );

   if( isNoRiseFall() ) //In this scenario all nets are assumed to have on the Trise. This represent their timing.
      monteMap[MAX_OF_PRIMARY_OUTPUTS_NAME] = monteMap[MAX_OF_RISE_PRIMARY_OUTPUTS_NAME];
   else {
      assert( monteMap.find( MAX_OF_FALL_PRIMARY_OUTPUTS_NAME ) == monteMap.end() );
      assert( netSlctVecMap.find( MAX_OF_FALL_PRIMARY_OUTPUTS_NAME )==netSlctVecMap.end() );
      vector<unsigned> & fNSV = netSlctVecMap[MAX_OF_FALL_PRIMARY_OUTPUTS_NAME];
      ProbDist & poFallMaxPD
         = * ( monteMap[MAX_OF_FALL_PRIMARY_OUTPUTS_NAME]
               = ProbDist::createMaxProbDist( fNSV, poFallPDs ) );

      monteMap[MAX_OF_PRIMARY_OUTPUTS_NAME]
         = & ProbDist::getEqualMixture( poRiseMaxPD, poFallMaxPD );
   }
   //}

   // time it!
   double e = clock();

   double d = ( e-s )*1.0e-6;

   monte_carlo  st = monte;
   cout << st << " for"
        << " ( " << osp.getName() << ", " << cnvt::addTH( num+1 ) << " )"
        << " took " << d << cnvt::addS( " second",d ) << "." << endl;
   //  keepThoseInTheNetList( monteMap, osp );

   /*
    * special section for printing pomax of the result, if pomax distribution is asked;
    * XXX: this will later be moved to printing functionality of ciropt
    */

   map<string,ProbDist *>::iterator itr = monteMap.find( MAX_OF_PRIMARY_OUTPUTS_NAME );
   if( itr != monteMap.end() ) {
      ProbDist & pomaxpd = * itr->second;
      cout << "mean = " << pomaxpd.getMean();
      cout << ", std = " << pomaxpd.getSTD();
      cout << ", 80%quantile = " << pomaxpd.getPercentilePoint( .80 );
      cout << ", 85%quantile = " << pomaxpd.getPercentilePoint( .85 );
      cout << ", 90%quantile = " << pomaxpd.getPercentilePoint( .90 );
      cout << ", 95%quantile = " << pomaxpd.getPercentilePoint( .95 );
      cout << endl;
   }

   return;
}

void opt_prob_generator::nominalAnalysis( map<string,double> & nomAnlys, map<string,double> & criticality, vector<double> & pathLengths, vector<double> & pathVariances, vector<double> & gateDios, vector<string> & gateDioNames, opt_spec & osp, unsigned num, const string cccName ) {
   assert( nomAnlys.size() == 0 );
   assert( criticality.size() == 0 );
   assert( pathLengths.size() == 0 );
   assert( pathVariances.size() == 0 );
   assert( gateDios.size() == 0 );

   map<string,unsigned> netSelectMap;

   nominalAnalysis( nomAnlys, netSelectMap, pathLengths, pathVariances,gateDios, gateDioNames, osp, num, cccName );
   if( cccName != EMPTY_STRING )
      return; //No criticality (next step) defined for dio analysis of a gate

   unsigned n = netSelectMap.size();

   map<string,unsigned>::iterator itr;
   for( itr = netSelectMap.begin(); itr != netSelectMap.end(); itr ++ )
      criticality[itr->first] = 0.0;

   assert( criticality.size() == n );
   addOneToCriticalNets( criticality, netSelectMap );
   for( unsigned i=0; i<piEdgeVec.size(); i++ ) {
      if( criticality.find( piEdgeVec[i]->getMonteTRiseName() ) == criticality.end() )
         criticality[piEdgeVec[i]->getMonteTRiseName()] = 0.0;
      if( isNoRiseFall() )
         continue;
      if( criticality.find( piEdgeVec[i]->getMonteTFallName() ) == criticality.end() )
         criticality[piEdgeVec[i]->getMonteTFallName()] = 0.0;
   }
   if( isNoRiseFall() )
      assert( criticality.size() == ( n + piEdgeVec.size() ) );
   else
      assert( criticality.size() == ( n + 2*piEdgeVec.size() ) );

   return;
}

void opt_prob_generator::nominalAnalysis( map<string,double> & nomAnlys, map<string,unsigned> & netSelectMap, vector<double> & pathLengths, vector<double> & pathVariances, vector<double> & gateDios, vector<string> & gateDioNames, opt_spec & osp, unsigned num, const string cccName ) {
   assert( nomAnlys.size() == 0 );
   assert( netSelectMap.size() == 0 );
   assert( pathLengths.size() == 0 );
   assert( pathVariances.size() == 0 );
   assert( gateDios.size() == 0 );

   assert( haveTheNetListAssigned );

   assert( num < osp.getLengthOfOptValHistory() );
   map<string,double> & optVals = osp.getOptValues( num );

   // start!
   double s = clock();
   nominalAnalysis( nomAnlys, netSelectMap, pathLengths, pathVariances, gateDios,gateDioNames, optVals, cccName );
   // time it!
   double e = clock();

   double d = ( e-s )*1.0e-6;

   cout << "The " << cnvt::addTH( num+1 )
        << " NOMINAL ANALYSIS of " << osp.getName()
        << " took " << d << cnvt::addS( " second",d ) << "." << endl;

   return;
}

void opt_prob_generator::nominalAnalysis( map<string,double> & nomAnlys, map<string,unsigned> & nSM, vector<double> & pathLengths, vector<double> & pathVariances, vector<double> & gateDios, vector<string> & gateDioNames, map<string,double> & optVals, const string cccName ) {
   assert( nomAnlys.size() == 0 );
   assert( nSM.size() == 0 );
   assert( pathLengths.size() == 0 );
   assert( pathVariances.size() == 0 );
   assert( gateDios.size() == 0 );

   map<string,vector<double> > pathLengthMap;
   map<string,vector<double> > pathVarianceMap;
   map<string,double > gateDioMap;

   netw.nominalAnalysis( nomAnlys, nSM, pathLengthMap, pathVarianceMap, gateDioMap, optVals, cccName, isNoRiseFall() );
   //Put the gate delays in the gateDios vector
   map<string,double >::iterator gitr;
   for( gitr = gateDioMap.begin(); gitr != gateDioMap.end(); gitr++ ) {
      gateDioNames.push_back( gitr->first );
      gateDios.push_back( gitr->second );
   }
   if( cccName != EMPTY_STRING )
      return; //no need to go further as the job is done for one CCC.

   // calculate the max of the primary outputs
   map<string,double>::iterator itr;
   map<string,vector<double> >::iterator mitr;
   vector<double> poRises, poFalls;

   for( unsigned i = 0; i < poEdgeVec.size(); i ++ ) {
      string  trisename = poEdgeVec[i]->getMonteTRiseName();

      itr = nomAnlys.find( trisename );
      assert( itr != nomAnlys.end() );
      poRises.push_back( itr->second );

      mitr = pathLengthMap.find( trisename );
      assert( mitr != pathLengthMap.end() );
      pathLengths.insert<vector<double>::iterator>
      ( pathLengths.end(), mitr->second.begin(), mitr->second.end() );

      mitr = pathVarianceMap.find( trisename );
      assert( mitr != pathVarianceMap.end() );
      pathVariances.insert<vector<double>::iterator>
      ( pathVariances.end(), mitr->second.begin(), mitr->second.end() );


      if( isNoRiseFall() ) //there is only one delay per net, represented by the Trise.
         continue;

      string  tfallname = poEdgeVec[i]->getMonteTFallName();

      itr = nomAnlys.find( tfallname );
      assert( itr != nomAnlys.end() );
      poFalls.push_back( itr->second );

      mitr = pathLengthMap.find( tfallname );
      assert( mitr != pathLengthMap.end() );
      pathLengths.insert<vector<double>::iterator>
      ( pathLengths.end(), mitr->second.begin(), mitr->second.end() );

      mitr = pathVarianceMap.find( tfallname );
      assert( mitr != pathVarianceMap.end() );
      pathVariances.insert<vector<double>::iterator>
      ( pathVariances.end(), mitr->second.begin(), mitr->second.end() );
   }

   pathLengthMap.clear();
   pathVarianceMap.clear();

   assert( nomAnlys.find( MAX_OF_PRIMARY_OUTPUTS_NAME ) == nomAnlys.end() );
   assert( nomAnlys.find( MAX_OF_RISE_PRIMARY_OUTPUTS_NAME ) == nomAnlys.end() );
   assert( nomAnlys.find( MAX_OF_FALL_PRIMARY_OUTPUTS_NAME ) == nomAnlys.end() );

   assert( nSM.find( MAX_OF_PRIMARY_OUTPUTS_NAME ) == nSM.end() );
   assert( nSM.find( MAX_OF_RISE_PRIMARY_OUTPUTS_NAME ) == nSM.end() );
   assert( nSM.find( MAX_OF_FALL_PRIMARY_OUTPUTS_NAME ) == nSM.end() );
   double r = nomAnlys[MAX_OF_RISE_PRIMARY_OUTPUTS_NAME]
              = cnvt::max( nSM[MAX_OF_RISE_PRIMARY_OUTPUTS_NAME],poRises );

   if( isNoRiseFall() )
      nomAnlys[MAX_OF_PRIMARY_OUTPUTS_NAME] = nomAnlys[MAX_OF_RISE_PRIMARY_OUTPUTS_NAME];
   else {
      double f = nomAnlys[MAX_OF_FALL_PRIMARY_OUTPUTS_NAME]
                 = cnvt::max( nSM[MAX_OF_FALL_PRIMARY_OUTPUTS_NAME],poFalls );
      nomAnlys[MAX_OF_PRIMARY_OUTPUTS_NAME] = cnvt::max( r,f );
   }

   return;
}

void opt_prob_generator::addOneToCriticalNets( map<string,double> & criticality, map<string,unsigned> & netSelectMap ) {

   map<string,unsigned>::iterator it;


   it = netSelectMap.find( MAX_OF_RISE_PRIMARY_OUTPUTS_NAME );
   assert( it != netSelectMap.end() );
   
   unsigned n = it->second;
   assert( n < poEdgeVec.size() );

   poEdgeVec[n]->addOneToCriticalNets( criticality, netSelectMap, true, isNoRiseFall() ); // in the first call
   // the max of rise output comes from "rising" signals, against CMOS operation. Hence a "true" is given to bool rf so that
   // the criticality is measured in .Trise's account.

   if( isNoRiseFall() ) {
      return;
   }

   it = netSelectMap.find( MAX_OF_FALL_PRIMARY_OUTPUTS_NAME );
   assert( it != netSelectMap.end() );
   
   n = it->second;
   assert( n < poEdgeVec.size() );

   poEdgeVec[n]->addOneToCriticalNets( criticality, netSelectMap, false, false );
}

void opt_prob_generator::keepThoseInTheNetList( map<string,ProbDist *> & mc,opt_spec & osp ) {
   assert( haveTheNetListAssigned );

   map<string,ProbDist *>::iterator itr, tmpItr;
   map<string,ProbDist *> tmp = mc;

   mc.clear();

   for( itr = tmp.begin(); itr != tmp.end(); itr ++ ) {
      if( osp.isInTheNetList( itr->first ) )
         mc[itr->first] = itr->second;
      else
         delete itr->second;
   }

   return;
}

pair<unsigned,unsigned> opt_prob_generator::getRowColNumbers( unsigned num ) {
   assert( num > 0 );

   double dnum = ( double )num;

   unsigned col = ( unsigned )( sqrt( dnum ) );
   unsigned row = ( unsigned )( dnum/( double )col );

   if( col * row < num ) row ++;

   assert( row >= col );

   return pair<unsigned,unsigned>( row,col );
}

bool opt_prob_generator::isTheFilenameAlreadyRegistered( const string & filename ) {
   return ( drawingList.find( filename ) != drawingList.end()
            || writingList.find( filename ) != writingList.end()
            || mWritingList.find( filename ) != mWritingList.end() );
}

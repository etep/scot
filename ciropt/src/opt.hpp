#ifndef _SH_OPT_H_
#define _SH_OPT_H_

#include <string>
#include <vector>
#include "network.hpp"
#include "analyzer.hpp"

extern	std::map<std::string, int> glbVars;


class global_parameters;
class opt_prob_generator;

class opt_spec;
class subplot;
class opt_dscr;
class netspec;
class mvec;
 
//  The class 'global_parameters' contains global parameter values,
//  and the (only) private member of which is:
//  1) gvMap : 'std::map' of 'std::string' and 'double' pair.
// 
//  One can see this class simply a wrapper class for std::vector.
//
class global_parameters {
private:
   std::map<std::string, std::string> gvMap;

public:
   global_parameters() {}

   void put( const std::string & s,const  std::string & param );

   std::string getValueOf( std::string s );

   std::map<std::string,std::string> & getMap() {
      return gvMap;
   }

};


//  The class 'opt_prob_generator' takes the reference to a 'network'
//  as the (only) input argument of the (only) constructor
//  and has main functionalities of
//  1) formulating the optimization problem,
//  2) printing it as a GGP or a GP,
//  3) solving the opt. problem using 'mskdgopt' of mosek,
//  4) writing matlab code solving the opt. problem in a file.
// 
//  The class 'opt_prob_generator' has the following private members:
//  1) 'netw'	: reference to 'network',
//  2) 'gP'	: 'global_parameters',
//  3)*'symtab'	: 'symbol_table',
//  4) 'constraintVector'	: std::vector of std::string for constraints,
//  5)*'objectiveFunction' : std::string for the objective
//  		function of the opt. problem,
//  6)*'optProblem' : pointer to 'gen_gp' for the opt. problem,
//  7) 'haveConstantsBeenInitialized' : 'bool' indicating whether the global
// 	ants (parameters) have been initialized,
//  8) 'CPrise', 'CPfall', 'CNrise', 'CNfall', 'Vthn', 'Vthp', 'Vdd' : global constants (parameters),
//  9) 'poEdgeVec' : 'std::vector' of pointers to'edge's for primary output nets,
// 10) 'piEdgeVec' : 'std::vector' of pointers to'edge's for primary input nets,
// 11)*'doneRecursive', 'donePIC', 'doneObj' : some internal indication values.
// 
//  '*' means DEPRECATED members
//
class opt_prob_generator {
public:
   // static fields for parasitic capacitors look-up (by others)
   static std::string MAX_OF_PRIMARY_OUTPUTS_NAME;
   static std::string TOTAL_ENERGY_NAME;
   static std::string CIRCUIT_ENERGY_NAME;
   static std::string LOGIC_ENERGY_NAME;
   static std::string WIRE_ENERGY_NAME;
   static std::string INPUT_ENERGY_NAME;
   static std::string LOAD_ENERGY_NAME;
   static std::string TOTAL_LEAKAGE_NAME;
   static std::string TOTAL_AREA_NAME;
   static double WEIGHT_OF_OTHER_NETS_IN_OBJECTIVE;
   static std::string MAX_OF_RISE_PRIMARY_OUTPUTS_NAME;
   static std::string MAX_OF_FALL_PRIMARY_OUTPUTS_NAME;
   static bool ERASE_GGPSOL_SOURCE_FILE;
   static bool PRINT_INTERNAL_NET_FOR_NOMINAL_ANALYSIS;

private:
   // global parameters needed
   static std::string CPrise, CPfall, CNrise, CNfall;
   static std::string CPenergy, CNenergy, MIN_SF, DEF_SF, DEF_DF,MIN_SL, MIN_RES;
   static std::string Vthn, Vthp, Vdd, Vdd_energy;

   // indicates whether the global parameters needed have been properly
   // initialized.
   static bool haveConstantsBeenInitialized;

public:
   // getters for global constants
   static std::string getCPrise() {
      assert( haveConstantsBeenInitialized );
      return CPrise;
   }

   static std::string getCPfall() {
      assert( haveConstantsBeenInitialized );
      return CPfall;
   }

   static std::string getCNrise() {
      assert( haveConstantsBeenInitialized );
      return CNrise;
   }

   static std::string getCNfall() {
      assert( haveConstantsBeenInitialized );
      return CNfall;
   }

   static std::string getVthn() {
      assert( haveConstantsBeenInitialized );
      return Vthn;
   }

   static std::string getVthp() {
      assert( haveConstantsBeenInitialized );
      return Vthp;
   }

   static std::string getVdd() {
      assert( haveConstantsBeenInitialized );
      return Vdd;
   }

   static std::string getVdd_energy() {
      assert( haveConstantsBeenInitialized );
      return Vdd_energy;
   }

   static std::string getEnergyCP() {
      assert( haveConstantsBeenInitialized );
      return CPenergy;
   }

   static std::string getEnergyCN() {
      assert( haveConstantsBeenInitialized );
      return CNenergy;
   }

   static std::string getMinSF() {
      assert( haveConstantsBeenInitialized );
      return MIN_SF;
   }

   static std::string getDefSF() {
      assert( haveConstantsBeenInitialized );
      return DEF_SF;
   }


   static std::string getDefDF() {
      assert( haveConstantsBeenInitialized );
      return DEF_DF;
   }

   static std::string getMinSl() {
      assert( haveConstantsBeenInitialized );
      return MIN_SL;
   }

   static std::string getMinRes() {
      assert( haveConstantsBeenInitialized );
      return MIN_RES;
   }

private:
   //CHECK
   static bool isObjectiveStat( const std::string & obj );
   static bool isObjectiveABs ( const std::string & obj );

private:
   std::string name;
   network & netw;
   global_parameters gP;

   // network related private members
   // XX:
   // hence (I insist) this two members must be transferred to
   // the class 'network' eventually.
   // then some member functions of this class will need to be modified
   // and the class 'network' will need some more member functions
   // according to this change!

   std::vector< edge *> poEdgeVec; // vector for primary output nets
   std::vector< edge *> piEdgeVec; // vector for primary input nets

   // CONSTRAINT RELATED PRIVATE MEMBERS

   // tells whether there is an area constraint and the constraint value
   bool isAreaCnst;
   double areaCnstVal;

   // tells whether there is a energy constraint and the constraint value
   bool isPowrCnst;
   double powrCnstVal;

   // tells whether there is a energy constraint and the constraint value
   bool isDelyCnst;
   std::string delyCnstVal;

   // vector for global constraints
   std::vector<std::string> constraintVector;

   // (global) min/max width constraint
   double minWidth, maxWidth, minVthP, maxVthP, minVthN, maxVthN;
   double minVdd, maxVdd;
   double logicDepthFactor;
   bool OnlyFormulateProblem;

   // bool to set if uniform kappas are to be used
   bool UniformKappa;

   // bool to set if criticality is to be added to kappas
   bool CriticalityKappa;

   // bool to be set true if use of default activity factor is allowed
   // for nets with no activity factor present.
   bool UseDefActFact;

   // bool to set if the leakage is not to be included in the
   // energy expression
   bool NoLeakPow;

   // Bool to set if the net should have separate rise fall time or a simple one
   // delay per dio model. If this variable is true a simple delay is chosen.
   // Then only the falling rf (NMOS) delay is considered from the dio file.
   // The PMOS is assumed to be sized to have the same rising (fr) delay
   bool NoRiseFallTiming;

   //Flag to indicate that all edges < POMAX constraint is not needed.
   bool NoAllEdgeTimeConst;

   //Flag to indicate that there are no wires hence no wire energy to be included.
   bool IncludeWireEnergy;

   // gate_name, and RHS
   std::map<std::string,std::string> gateDelayCnstTypeI;

   // gate_name, risefall, and RHS
   std::map<std::string,std::map<std::string,std::string> > gateDelayCnstTypeII;

   // gate_name, quantile-value, and RHS
   std::map<std::string,std::pair<double,double> > gateDelayCnstTypeIII;

   // gate_name, risefall, quantile-value, and RHS
   std::map<std::string,std::map<std::string,std::pair<double,double> > >
   gateDelayCnstTypeIV;

   // OPTIMIZATION RELATED PRIVATE MEMBERS

   // map of optimization specs to their names
   // each opt_spec instance corresponds to one optimization task
   std::map<std::string, opt_spec *> optSpecs;

   // map of subplot vectors to their m-file names
   // each subplot vector corresponds to one plot
   // and each subplot instance corresponds to one subplot in the plot
   // of the vector it resides in
   std::map<std::string,std::vector<subplot *> > drawingList;
   std::map<std::string,std::pair<unsigned,unsigned> > drawingDim;

   // map of analysis specs to their nominal analysis output file names
   std::map<std::string, opt_dscr> writingList;

   std::map<std::string, std::vector<mvec> > mWritingList;

   // constant for statistical optimization
   double Beta;

   bool haveTheNetListAssigned;

public:
   // constructor
   opt_prob_generator( const std::string & name, network & nw );

   // destructor
   ~opt_prob_generator();

   // putters

   /*
    * assign global parameter values
    */
   void put( const std::string & name,const std::string & value ) {
      gP.put( name, value );
   }

   void put( const std::string & name, double value ) {
      std::string st = cnvt::doubleToString( value );
      gP.put( name, st );
   }

   /*
    * This function puts primary inputs; it searches the edge list
    * in the network with the name of the 'piinfo' class,
    * and associates {pii} with the 'edge' instance found.
    * it will report an error by calling 'errorReport'
    * and return if there is no edge with the name
    * in the edge list in the network.
    */
   void put( piinfo & pii );

   /*
    * This fuction puts primary outputs; it does similar things as
    * void put( piinfo&) does.
    */
   void put( poinfo & poi );

   // getters
   std::string & getName();
   network & getNetwork();

   opt_spec & getOptSpec( const std::string & optname );

   ProbDist & getProbDist( netspec & ns,  monte_carlo & monte , double p );

   ProbDist & getProbDist( opt_spec &, unsigned num,
                           const std::string & netname,
                           monte_carlo & monte , double p );

   //to do static timing analysis on the netlist.
   std::map<std::string,double> & getNomAnlys( opt_dscr & aspec );
   std::map<std::string,double> getGateDios( opt_dscr & aspec );

   // OPTIMIZATION
   void solve();

   // WRITE
   void write();

   // DRAW
   void draw();

   //
   // ANALYSIS
   //

   // Monte Carlo
   void montecarloAndStore( monte_carlo & monte, double p, opt_spec & osp );

   // nominal analysis
   void nominalAnalysisAndStore( opt_spec & osp, const std::string cccName );

   /*
    * ADDING constraints, optimization specs, drawing specs, etc.
    * the below member functions will be mostly called in ciropt.ypp
    * (the main bison (yacc) input file defining the whole grammar
    * of .sp files (for ciropt)
    */

   /*
    * add a global constraint by string
    */
   void addConstraint( const std::string & cst );

   void addAreaConstraint( double );

   void addEnergyConstraint( double );

   void addDelayConstraint( const std::string & str );
   void addDelayConstraint( double );

   /*
    * add a constraint of the rising {net_name} being less than 't'
    */
   void addTRise( const std::string & net_name, double t );

   /*
    * add a constraint of the falling {net_name} being less than 't'
    */
   void addTFall( const std::string & net_name, double t );
   void addT( const std::string & edge_name, double T, bool r );

   /*
    *  add slope constraints via four types of delay  constraints
    */
   void addDelayConstraint( const std::string & name, const std::string & str );

   void addDelayConstraint( const std::string & name,
                            const std::string & rf, const std::string & str );

   void addDelayConstraint( const std::string & name, double p, double val );

   void addDelayConstraint( const std::string & name,
                            const std::string & rf, double p, double val );

   /*
    * add min/max width constraint
    * XXX: it currently takes only global min_width/max_width constraint, i.e.
    * min_width/max_width constraint for all widths in the gate
    */
   void setOFP(); //only formulate the problem bool variable.
   void setUniformKappa(); //set to use uniform kappas
   void setCriticalityKappa(); //set to use uniform kappas
   void setUseDefActFact(); //set to allow the use of uniform activity factor.
   void setNoLeakPow(); //set to not include leakage in total energy
   void setNoRiseFallTiming(); //set to not have separate
   //rise fall. For example, this means just one delay for an inverter.
   bool isNoRiseFall() {
      return NoRiseFallTiming;
   }

   void setNoAllEdgeTimeConst(); //not include "all edge times < POMAX" constraint.
   void setIncludeWireEnergy(); //set to true(default) to not include the wire energy.
   //set false by presence of caps in the network.
   void setMinWidth( double min_width );
   void setMaxWidth( double max_width );
   void setMaxVthP( double max_vth_p );
   void setMinVthP( double min_vth_p );
   void setMaxVthN( double max_vth_n );
   void setMinVthN( double min_vth_n );
   void setLogicDepthFactor( double ldf );
   void setMaxVdd( double max_vdd );
   void setMinVdd( double min_vdd );
   bool getUseDefActFact() {
      return UseDefActFact;
   }

   // the client adds one optimization task
   // each time she/he calls one of the following member functions
   // of the form 'minimize**'
   void minimizeArea( const std::string & optname );
   void minimizeArea( bool noRiseFall, const std::string & optname );

   void minimizeEnergy( const std::string & optname );
   void minimizeEnergy( bool noRiseFall,const std::string & optname );

   void minimizeDelay( const std::string & optname );
   void minimizeDelay( bool noRiseFall, const std::string & optname );
   void minimizeDelay( const std::string & optname, unsigned numIter, unsigned N, double p );
   void minimizeDelay( const std::string & optname, unsigned numIter, unsigned N, double p, const std::string & dist );
   void minimizeDelay( bool noRiseFall, const std::string & optname, unsigned numIter, unsigned N, double p );
   void minimizeDelay( bool noRiseFall, const std::string & optname, unsigned numIter, unsigned N, double p, const std::string & dist );
   void minimizeDelay( const std::string & optname, const std::string & kappaORbeta, double s, double e, unsigned num );
   void minimizeDelay( bool noRiseFall, const std::string & optname, const std::string & kappaORbeta, double s, double e, unsigned num );
   void minimizeDelay( const std::string & optname, const std::string & kappaORbeta,std::vector<double> & kappas );
   void minimizeDelay( bool noRiseFall, const std::string & optname, const std::string & kappaORbeta, std::vector<double> & kappas );
   void minimizeDelay( const std::string & optname, const std::string & kappaORbeta, double kappa, double beta );
   void minimizeDelay( bool noRiseFall, const std::string & optname, const std::string & kappaORbeta, double kappa, double beta );
   void minimizeDelay( const std::string & optname, unsigned numiter, unsigned n, double p , double kmax );

   // the client adds one drawing task each time she/he calls the following member function
   void putDrawing( const std::string & mFileName,std::vector<subplot *> & subplotVector );

   /*
    * the client adds one drawing task each time she/he calls
    * the following member function
    */
   //void putWriting( const std::string & filename,const std::string & opt_name, unsigned num );

   void putWriting( const std::string & filename,const std::string & opt_name, unsigned num, const std::string & gateName );

   //void putWriting( const std::string & filename,const std::string & outFileName );

   void putWriting( const std::string & filename,const std::string & outFileName, const std::string & gateName );

   void putMWriting( const std::string & mfilename,std::vector<mvec> & mvecList );

   void assignTheNetList();

   void checkLegitimacyOfReferencesToOptimizations();


   /*
    * read the ggpsol output file with {filename} and store the values
    * in {optVs}
    *
    * return value:
    *    0, if it parses the file without errors,
    *   -1, if gppsol generated "not primal-dual feasible error",
    *   -2, if ggpsol generated "memory allocation error",
    *   -3, if gppsol generated "gp solver error",
    *   -4, if gppsol generated "dgopt error"
    *   -5, if the file with {filename} can't be opened properly,
    *   -6, if the file {filename} is not correctly formatted,
    */
   static int ggpsolOutToMap( std::map<std::string,double> & optVs,
                              const std::string & filename );

   // search {optSpec} list for a opt_spec with {out_file_name}.
   // if there exists such, just return,
   // and add one opt job to the list ({optSpecs}), otherwise.
   void addOutFileOptSpec( const std::string & outfilename );

   // add {os} to the opt_spec * vector, {optSpecs},
   // if there exists an opt_spec with the same name in the vector,
   // delete & os, sends an error message by calling errorReport()
   void addNewOptSpec( opt_spec & os );

private:
   std::string getArea();
   std::string getEnergy();
   //   std::string getCircuitEnergy();
   std::string getLogicEnergyStt();
   void getLogicEnergy( std::ostream & os );
   std::string getWireEnergyStt();
   void getWireEnergy( std::ostream & os );
   std::string getInputEnergyStt();
   //   std::string getInputEnergy();
   void getInputEnergy( std::ostream & os );
   std::string getLoadEnergyStt();
   //   std::string getLoadEnergy();
   void getLoadEnergy( std::ostream & os );
   std::string getLeakage( bool stat );
   std::string getMaxPODelay();
   std::string getMaxPODelay( unsigned i );

   /*
    * FORMING OPTIMIZATION PROBLEM
    * write a ggpsol source file with the information this class instance has
    * at the point.
    *
    * it will check whether the class instance has enough and correct
    * information in order to form an optimization problem and write it
    * in ggpsol format, but it is also possible that it generates errors
    * while forming the problem since there are some kinds of errors
    * wich are inherently impossible to detect before starting forming
    * a problem.
    *
    * it first opens a file with {filename}, checks whether it is properly
    * opened. if it is properly opened, it proceeds to write down the ggpsol
    * source input in the file, and it reports an error and return, otherwise.
    */
   void writeGGPSOLInput( const  std::string & filename, opt_spec & osp );

   /*
    * assign global constants before forming an optimization problem
    */
   void globalConstantAssignment();
   void CheckPIandPO();

   //void  MakeCorrelatedEdgeTable();
   /*
   * objective statement to {os}
   */
   std::ostream & objectiveStatementToOstream( std::ostream & os, opt_spec & osp , bool stat );
   // make max(all net times ) < POMAX ...used in dynamic logic for constraining
   // timings of the inuts to the domino nodes.

   std::ostream & allEdgesLessThanMaxConstraint( std::ostream & os );
   std::string getEdgeTimes(  const std::string & delim );

   /*
    * area constraint and energy constraint to {os}
    * if any of them is assigned
    */
   std::ostream & theThreeConstraintsToOstream( std::ostream & os, bool stat );
   void leakPowConstraintsToOstream();


   // add recursive constraints (or formula) to the opt. problem
   // traveling through each gate (node).
   std::ostream & recursiveConstraintsToOstream( std::ostream & os, opt_spec & );

   // write constraints for the primary input nets
   std::ostream & PICConstraintsToOstream      ( std::ostream & os );
   std::ostream & PITimingConstraintsToOstream ( std::ostream & os );

   // write global constraints
   // write width constraints
   // write the vth Vdd constraints
   // write the vth Vdd constraints
   std::ostream & globalConstraintsToOstream ( std::ostream & os );
   std::ostream & widthConstraintsToOstream  ( std::ostream & os );
   std::ostream & vthConstraintsToOstream    ( std::ostream & os );
   std::ostream & vddConstraintsToOstream    ( std::ostream & os );

   // gate delay constraints
   /*
    * Type I   : D( {gate name} or all ) < {pnumber} ;
    * Type II  : D( {gate name} or all, rf or fr ) < {pnumber} ;
    * Type III : D( {gate name} or all, {quantile} ) < {pnumber} ;
    * Type IV  : D( {gate name}, rf or fr, {quantile} ) < {pnumber} ;
    */
   std::ostream & gateDelayConstraintsToOstream             ( std::ostream & os );
   std::ostream & gateDelayConstraintsTypeIandIIToOstream   ( std::ostream & os );
   std::ostream & gateDelayConstraintsTypeIIIandIVToOstream ( std::ostream & os );

   std::ostream & gateDelayConstraintsTypeIAllToOstream  ( std::ostream & os, node & nd, const std::string & val ) ;
   std::ostream & gateDelayConstraintsTypeIIAllToOstream ( std::ostream & os, node & nd, std::map<std::string,std::string> & rfvals );
   std::ostream & gateDelayConstraintsTypeIToOstream     ( std::ostream & os, const std::string & gatename, const std::string & val );
   std::ostream & gateDelayConstraintsTypeIIToOstream    ( std::ostream & os, const std::string & gatename, std::map<std::string,std::string> & rfvals );

   /*
       ostream & gateDelayConstraintsTypeIIIAllToOstream
         ( ostream & os,node & nd,
    std::pair<double,double> & );

       ostream & gateDelayConstraintsTypeIIIToOstream
         ( ostream & os,std::string & gatename,
    std::pair<double,double> & );

       ostream & gateDelayConstraintsTypeIVAllToOstream
         ( ostream & os,node & nd,
    std::map<std::string,std::pair<double,double> > & rfvals );

       ostream & gateDelayConstraintsTypeIVToOstream
         ( ostream & os,std::string & gatename,
    std::map<std::string,std::pair<double,double> > & rfvals );

   	*/
   // OPTIMIZATION
   void solve( opt_spec & ops );

   // deterministic formulation AND OTHERS
   void solve( const std::string & name, opt_spec & osp );

   // iterative statistical formulation
   void solve( const std::string & name, unsigned numIter, const monte_carlo & monte, double p, double kmax, opt_spec & osp );
   void solve( const std::string & name, const monte_carlo & monte, double p, double kmax, opt_spec & osp );

   // uniform quantile statistical formulation
   void solve( const std::string & name, std::vector<double> & kappas, std::vector<double> & betas, opt_spec & osp );
   void solve( const std::string & name, double kappa, double beta, opt_spec & osp );

   void setAllKappasBetas( double kappa, double beta );

   /*          *
    * ANALYSIS *
    *          */

   // Monte Carlo
   /*
    * do Monte Carlo traveling through the network
    * (and update the \kappa and \beta for each node
    *
    * {ciriticality} is a map of net names with cirticality index,
    * which is a probability that a net be in the critical path
    * for the distribution, hence a value between 0 and 1.
    */
   void montecarlo( std::map<std::string,ProbDist *> & monteMap,
                    std::map<std::string,double> & criticality,
                    const monte_carlo & monte, double p,
                    opt_spec & osp, unsigned num , double kmax );

   /*
    * inside the above function, the below is called to
    * ACTUALLY do montecarlo with a vector of type unsinged, {selectNum},
    * to store the information of which nets have been selected
    * every time max is taken.
    */
   void montecarlo( std::map<std::string,ProbDist *> & monteMap,
                    std::map<std::string,std::vector<unsigned> > & selectVecMap,
                    const monte_carlo & monte, double p,
                    opt_spec & osp, unsigned num );

   // Nominal Timing Analysis
   /*
    * do nominal timining analysis from opt.
    * again criticality means criticality index,
    * which is a probability a net is in the critical path,
    * which for this case, 0.0 or 1.0
    */
   void nominalAnalysis( std::map<std::string,double> & nomAnlys,
                         std::map<std::string,double> & criticality,
                         std::vector<double> & pathLengths,
                         std::vector<double> & pathVariances,
                         std::vector<double> & gateDios,
                         std::vector<std::string> & gateDioNames,
                         opt_spec & osp, unsigned num, const std::string cccName );

   /*
    * do ACTUAL nominal timing analysis
    * in {selectNum},the information of which nets have been selecte
    * every time max is taken.
    */
   void nominalAnalysis( std::map<std::string,double> & nomAnlys,
                         std::map<std::string,unsigned> & netSelectMap,
                         std::vector<double> & pathLengths,
                         std::vector<double> & pathVariances,
                         std::vector<double> & gateDios,
                         std::vector<std::string> & gateDioNames,
                         opt_spec & osp, unsigned num, const std::string cccName );

   void nominalAnalysis( std::map<std::string, double > & nomAnlys,
                         std::map<std::string,unsigned> & netSelectMap,
                         std::vector<double> & pathLengths,
                         std::vector<double> & pathVariances,
                         std::vector<double> & gateDios,
                         std::vector<std::string> & gateDioNames,
                         std::map<std::string,double> & optVs, const std::string cccName );

   void addOneToCriticalNets( std::map<std::string,double> & criticality,
                              std::map<std::string,unsigned> & netSelectMap );

   void keepThoseInTheNetList( std::map<std::string,ProbDist *> & mc, opt_spec & osp );

   static std::pair<unsigned,unsigned> getRowColNumbers( unsigned num );

   bool isTheFilenameAlreadyRegistered( const std::string & filename );

};

#endif

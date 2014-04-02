#ifndef _SH_NETWORK_H_
#define _SH_NETWORK_H_

/*
 * this file has the class declarations of the seven classes;
 * network, node, edge, ccc, dio, piinfo, and poinfo.
 *
 * by Sunghee Yun at Wed Nov 19 20:28:50 PST 2003
 */



#include <map>
#include <vector>
#include <ostream>

#include "symtab.hpp"
#include "gp.hpp"
#include "probdist.hpp"
#include "cnvt.hpp"

extern std::string EMPTY_STRING;
extern std::string KEEPER_INPUT;
extern std::string DUM_KEEPER_INPUT;
//extern    std::map<std::string, int> glbVars;



#define DBNH false

class node;
class edge;
class ccc;
class dio;
class network;
class piinfo;
class poinfo;
class mos;
class opt_spec;

class monte_carlo;
class opt_prob_generator;

class noRefException {};

/*
 * The class 'network' contains lists of ccc types, nodes, and edges
 * of a circuit (or a network), where a node respresents a gate
 * and an edge represents a net.
 *
 * A client can put ccc's, nodes, edges to a 'network',
 * search a 'network' for a specific ccc or an edge by name,
 * print the whole network to the standard output.
 *
 * The (only) constructor takes no input argument and does nothing.
 * Most members and member functions can easily be understood
 * by their names.
 *
 * The class 'network' contains the following private members,
 * 1) cccMap : a 'std::map' of 'std::string's and pointers to 'ccc',
 * 2) nodeMap : a 'std::map' of 'std::string's andpointers to 'node',
 * 3) edgeMap : a 'std::map' of 'std::string's and pointers to 'edge'.
 */

class network {
    // public static member functions
public:
    static bool isGNDorVDD( const std::string & nm );

private:
    std::map<std::string, ccc *> cccMap;
    std::map<std::string, node *> nodeMap;
    std::map<std::string, edge *> edgeMap;

    //record the ccc name  and the transistors whose inputs are correlated.
    std::map<std::string, std::map<std::string, std::string> > corr_mos_info;
    //for voltage sources and resistors, record the node name and the nets across the element.
    std::map<std::string, std::map<std::string, std::string> > VRLC_info;
    std::map<std::string, std::map<std::string, gposy * > > VRLC_type;
    std::map<std::string, symbol_table> VRLC_symtab;
    //VRL stands for Voltage sources, resistors and inductors. The common property\
    //of these this is that they are transparent to signal as far as loading
    //conditions are concerned. Also its not clear what their input and output is
    //since the signal direction thru them is not clear, until the entire netlist
    //is in place. Thus first you record them in the map and then after the entire
    //netlist is read, allocate nodes to them depending on which of their ports are
    //getting driven by a gate.

    // the assignment operator and the copy constructor
    // must not be called outside of this class
    network & operator= ( network & );
    network( network & );

public:
    // constructor
    network() {
        corr_mos_info.clear();
        VRLC_info.clear();
        VRLC_type.clear();
        VRLC_symtab.clear();
        /* do nothing */
    }

    // destructor
    ~network();

    // ccc putter
    void put( ccc & c );
    void putCorrMos( const std::string & cccName,
                     std::map<std::string, std::string> & corr_mos );
    void MakeCorrelatedEdgeTable();
    /* put the info about voltage sources, resistors and Inductors
     */
    void putVRLC_info( const std::string name,const std::string net1, const std::string net2, const std::string type, gposy & val, symbol_table & st );
    void MakeVRLCnodes();
    //  void CheckVRLnodes();
    void CheckConnectionsOfEdges();
    /*
     * put a gate node in the network
     */
    void putNode( const std::string & node_name,
                  const std::string & ccc_name,
                  std::vector<std::string> & out_edge_name_list,
                  std::vector<std::string> & in_edge_name_list,
                  const std::string & var_name );

    /*
     * put a capacitor node connected to the ground in the network
     */
    void putCapNode( const std::string & node_name,
                     const std::string & e1, const std::string & e2, gposy & c ,
                     symbol_table & st, const std::string & var_name );

    /*
     * put a VRL node connected between two "edges" in the network
     */
    void putVRLNode( const std::string & VRLname,
                     const std::string & e1, const std::string & e2,
                     const std::string & type , gposy & val, symbol_table & st );


    void putCorrNets( const std::string & netname,bool risefall,
                      const std::string & corr_net,bool corr_risefall );
    /*
     * put switching factor associated with each net (edge)
     */
    void putSwitchingFactor( const std::string & netname, double sf );
    void putSwitchingFactor( const std::string & gatename, const std::string & netname, double sf );
    void putDutyFactor( const std::string & nodename, double df ,
                        const std::string & out );

    //do double long-path-search on the network and set the
    //table in each gates that shows the longest path that it is
    //included in.
    void setPathLengthTables();
    // edge finders/adders
    /*
     * search 'edgeMap' for an edge with name, 'edge_name',
     * and if there is an edge with the name, return the reference to the edge,
     * or if there is no such edge, create one with the name
     * and return the reference to the newly created edge.
     */
    edge & findOrAddEdge( const std::string & edge_name );

    // getters
    /*
     * search 'cccMap' for a ccc with name, 'ccc_name',
     * and if there is a ccc with the name, return the reference to it,
     * or if there is no such ccc, report an error and halt the program.
     */
    ccc  & findCCC ( const std::string & ccc_name );

    /*
     * search 'edgeMap' for an edge with name, 'edge_name',
     * and if there is an edge with the name, return the reference to it,
     * or if there is no such edge, report an error and halt the program.
     */
    edge & findEdge( const std::string & edge_name );

    /*
     * print the network to the standard output
     */
    void print();

    /*
     * return thereference to 'nodeMap',
     * hence the client cannot modify the 'nodeMap'.
     */
    std::map<std::string, node *> & getNodeMap() {
        return nodeMap;
    }

    /*
     * return thereference to 'edgeMap',
     * hence the client cannot modify the 'edgeMap'.
     */
    std::map<std::string, edge *>   &  getEdgeMap() {
        return edgeMap;
    }

    std::map<std::string, ccc *>   &   getCCCMap() {
        return cccMap;
    }

    // ANALYSIS

    /*
     * do Monte Carlo traveling through the network
     * and update the constant at the same time
     */
    void montecarlo( std::map<std::string,ProbDist *> & mc,
                     std::map<std::string,std::vector<unsigned> > & netSelectMap,
                     const monte_carlo & monte, double p,
                     std::map<std::string,double> & optVs, bool noTriseTfall );

    // montecalro for independent case
    void montecarlo( std::map<std::string,ProbDist *> & mc,
                     std::map<std::string,std::vector<unsigned> > & netSelectMap,
                     const std::string & dist, unsigned N, double p,
                     std::map<std::string,double> & optVs, bool noTriseTfall );

    // montecarlo for dependent case
    void montecarlo( std::map<std::string,ProbDist *> & mc,
                     std::map<std::string,std::vector<unsigned> > & netSelectMap,
                     std::map<std::string,ProbDist *> & gatePDmap,
                     const monte_carlo & monte, double p,
                     std::map<std::string,double> & optVs, bool noTriseTfall );


    /*
     * nominal analysis
     */
    void nominalAnalysis( std::map<std::string,double> & nomAnlys,
                          std::map<std::string,unsigned> & netSelectVec,
                          std::map<std::string,std::vector<double> > & pathLengthMap,
                          std::map<std::string,std::vector<double> > & pathVarianceMap,
                          std::map<std::string,double > & gateDioMap,
                          std::map<std::string,double> & optVs, const std::string cccName, bool noTriseTfall );

    /*
     * set the two constants indentically to 'kappa' and 'beta'
     * for every gate/node
     */
    void setKappaBetaForAllNodes( double kappa, double beta );
};

/*
 * The class 'ccc' contains the following private members:
 * 1) name: ccc name,
 * 2) numInputs: # of inputs
 * 3) inputNameVec: vector of input net names,
 * 3) outputNameVec: vector of output net names,
 * 4) symtab: 'symbol_table' for its delay functions,
 * 5) dioVec: vector of delay functions from each input to output,
 * 6) wpVec: vector of WP names (actually gposys)
 * 7) wnVec: vector of WN names (actually gposys)
 */
class ccc {
private:
    static std::string FCLOAD_NAME, RCLOAD_NAME;

    std::string name;
    std::vector<std::string> inputNameVec;
    std::vector<std::string> outputNameVec;
    std::vector<std::string> intNetNameVec;
    unsigned numInputs;
    unsigned numOutputs;
    unsigned numIntNets;
    symbol_table symtab;



    // this is a vector indexed by the inputs. Since all the inputs
    // may not be going to all the outputs the mean and std. dev. of the
    // dios are stored as a vector of map. The map contains the output name
    // and the associted mean dio or std. dev. dio.
    std::vector<std::map<std::string,dio *> >dioVec, stdVec;
    std::vector<mos> devices;

    // wpVec[i] means the name of p-mos width
    // connected to ith input and similarly for wnVec[i]
    std::vector<std::vector<const gposy *> > wpVec, wnVec;

    // parasitic capacitor associated with each output of the ccc
    std::map<std::string, gposy *> parCap;
    std::map<std::string, gposy *> wireCap;
    std::map<std::string, gposy *> leakPowPmean;
    std::map<std::string, gposy *> leakPowNmean;
    std::map<std::string, gposy *> leakPowPstat;
    std::map<std::string, gposy *> leakPowNstat;

    // the assignment operator and the copy constructor
    // must never be called outside of this class.
    ccc & operator = ( ccc & );
    ccc( ccc & );

public:
    // constructor
    /*
     * ccc constructor takes three input arguments;
     * the name of the ccc instatnce, vector of input names,
     * and the vector of mos.
     */
    ccc( const std::string & name,
         std::vector<std::string> & outputnames,
         std::vector<std::string> & inputnames );


    /* DEPRECATED
     * ccc constructor takes three input arguments, which are the name
     * of the ccc instatnce, the vector of input names, and the vector
     * of ({width name},{input name}) pairs.
     */
    /*
    ccc(std::string & name,
        std::vector<std::string> & inputNames,
        std::vector<std::pair<std::string,std::string> > & ssVec );
         */

    // destructor
    ~ccc();
    void putMosList( std::vector<mos> & mos_list );
    void putInternalNets();

    // putters
    /*
     * put delay function from the input named 'inputName' to the output
     * if there is no such inputName in the ccc,
     * generates an error messages by calling dioerror().
     */
    void putMeanDio( dio & d,const std::string & inputName,
                     const std::string & outputName );
    void putSTDDio ( dio & d,const std::string & inputName,
                     const std::string & outputName );
    void putParCap( gposy * parcap ,const std::string & netName );
    void putWireCap( gposy * wirecap ,const std::string & netName );
    void putLeakPowNmean( gposy * leakpow ,const std::string & outputName );
    void putLeakPowPmean( gposy * leakpow,const std::string & outputName );
    void putLeakPowNstat( gposy * leakpow,const std::string & outputName );
    void putLeakPowPstat( gposy * leakpow,const std::string & outputName );

    // getters
    /*
     * get 'symbol_table' of the class.
     * note that the member function
     * is not declared as const, nor is its return type, hence
     * the client have a permission to modify it, thus
     * the client must be extra-careful after calling this function
     * and being passed to the reference to the 'symbol_table'.
     */
    symbol_table & getSymbolTable() {
        return symtab;
    }

    /*
     * get the name of the ccc
     */
    std::string & getName() {
        return name;
    }

    //get the device info from the device name.
    int get_gate_num( const std::string & mos_name );
    bool isPMOS( const std::string & mos_name );


    /*
    * get number of inputs of the ccc
    */
    unsigned getNumberOfInputs() {
        return numInputs;
    }

    unsigned getNumberOfOutputs() {
        return numOutputs;
    }

    unsigned getNumberOfIntNets() {
        return numIntNets;
    }
    /*
     * get {num}th input name of the ccc
     */
    std::string & getInputName( unsigned num );
    std::string & getOutputName( unsigned num );
    std::string & getIntNetName( unsigned num );

    /*
     * get index of symbol of in 'symtab'
     */
    int getSymbolIndex( const std::string & s );

    /*
     * get the symbol in 'symtab' with index 'i'
     */
    const std::string & getSymbol( unsigned i );

    /*
     * get the string of addition PMOS width name/s connected to 'num'th input
     */
    std::string getWPName( unsigned num );

    /*
     * get the string of addition of NMOS width name/s connected to 'num'th input
     */
    std::string getWNName( unsigned num );

    /* get the vector containing the mos names that the num'th input
     * goes to.
     * */
    std::vector<const gposy *> getWPNameVec ( unsigned num );
    std::vector<const gposy *> getWNNameVec ( unsigned num );

    // to check if the dio is defined for a particular in-out pair.
    bool isMeanDioAssigned( unsigned ni, unsigned no );
    bool isStdDioAssigned( unsigned ni, unsigned no );

    // get cap for energy calculation
    bool isParCapAssigned( unsigned num ) ;
    bool isMeanLeakPowAssigned( unsigned num ) ;
    bool isStatLeakPowAssigned( unsigned num ) ;


    std::string getLeakPowNnom( node & nd,  unsigned num ) ;
    std::string getLeakPowPnom( node & nd,  unsigned num ) ;
    std::string getLeakPowNstat( node & nd, unsigned num ) ;
    std::string getLeakPowPstat( node & nd, unsigned num ) ;
    std::string getParCap( node & nd, unsigned num ) ;
    std::string getParCap( node & nd, std::string intNet ) ;
    std::string getWireCap( node & nd, std::string intNet ) ;

    /*
     * get the delay function from {num}th input to the output
     * of the gate named {gatename} with load capacitor term
     * ccc::FCLOAD_NAME and ccc::RCLOAD_NAME replaced by 'fload' or 'rload'
     * for  RF or FR respectively.
     * the returned string is always parenthesized, hence, e.g., the client
     * can multiply a number to the returned string reliably
     * expecting that the number will be multiplied to the whole string.
     */

    // XXX: eventually the below 6 functions will become helper functions
    // as in the case of the below double return-valued functions,
    // meaning that these functions will eventually become private ones

    std::string getDioRF( unsigned ni, unsigned no,
                          const std::string & gatename,const std::string & fload ) ;

    std::string getDioFR( unsigned ni, unsigned no,
                          const std::string & gatename,const std::string & rload ) ;

    std::string getSTDRF( unsigned ni, unsigned no,
                          const std::string & gatename,const std::string & fload )
    throw( noRefException );

    std::string getSTDFR( unsigned ni, unsigned no,
                          const std::string & gatename,const std::string & rload )
    throw( noRefException );

    std::string getVARRF( unsigned ni, unsigned no,
                          const std::string & gatename,const std::string & fload )
    throw( noRefException );

    std::string getVARFR( unsigned ni, unsigned no,
                          const std::string & gatename,const std::string & rload )
    throw( noRefException );

    double meanDioRF( unsigned ni, unsigned no,
                      const std::string & gatename,
                      std::map<std::string,double> & optVs,
                      const edge & outedge ) ;

    double meanDioFR( unsigned ni, unsigned no,
                      const std::string & gatename,
                      std::map<std::string,double> & optVs,
                      const edge & outedge ) ;

    double stdDioRF( unsigned ni, unsigned no,
                     const std::string & gatename,
                     std::map<std::string,double> & optVs,
                     const edge & outedge ) ;

    double stdDioFR( unsigned ni, unsigned no,
                     const std::string & gatename,
                     std::map<std::string,double> & optVs,
                     const edge & outedge ) ;

    double varDioRF( unsigned ni, unsigned no,
                     const std::string & gatename,
                     std::map<std::string,double> & optVs,
                     const edge & outedge ) ;

    double varDioFR( unsigned ni, unsigned no,
                     const std::string & gatename,
                     std::map<std::string,double> & optVs,
                     const edge & outedge ) ;

private:
    // helper functions
    double meanDioRF
    ( unsigned ni, unsigned no,const std::string & gName,
      std::map<std::string,double> & optVs, double fload ) ;

    double meanDioFR
    ( unsigned ni, unsigned no,const std::string & gName,
      std::map<std::string,double> & optVs, double rload ) ;

    double stdDioRF
    ( unsigned ni, unsigned no,const std::string & gName,
      std::map<std::string,double> & optVs, double fload ) ;

    double stdDioFR
    ( unsigned ni, unsigned no,const std::string & gName,
      std::map<std::string,double> & optVs, double rload ) ;

    double varDioRF
    ( unsigned ni, unsigned no,const std::string & gName,
      std::map<std::string,double> & optVs, double fload ) {
        double a = stdDioRF( ni, no, gName,optVs,fload );
        return a*a;
    }

    double varDioFR
    ( unsigned ni, unsigned no,const std::string & gName,
      std::map<std::string,double> & optVs, double rload ) {
        double a = stdDioFR( ni,no,gName,optVs,rload );
        return a*a;
    }

public:
    /*
     * get expression of sum of all the widths in delay expression
     * except load terms
     */
    std::string getArea( const std::string & nodename );

    // print
    void print() ;
    void print( bool varName ) ;

private:
    void assignWidthToInput( mos & ms );
    void checkWidthAssignment(); //const;

    /* DEPRECATED
    void assignWidthToInput
      (std::string & widthName,std::string & inputName );
      */

    unsigned getInputNumber( const std::string & inputName );

};

/*
 * The class 'dio' is simply a collection of four kinds of delay functions,
 * 'dioRF', 'dioFR', 'dioRR', and 'dioFF'.
 * 'dioRF' represents delay of rise to fall,
 * 'dioFR' represents delay of fall to rise,
 * 'dioRR' represents delay of rise to rise,
 * and 'dioFF' represents delay of fall to fall.
 * Especially the last two delay function types are for transition gates.
 * The class contains the pointers to the four delay functions,
 * each of which is an instance of the class 'gposy'.
 *
 * The class 'dio' has the following private members,
 * 1) dioRF : a pointer to the delay function from rise to fall,
 * 2) dioFR : a pointer to the delay function from fall to rise,
 * 3) dioRR : a pointer to the delay function from rise to rise,
 * 4) dioFF : a pointer to the delay function from fall to fall.
 *
 */
class dio {
private:
    gposy * dioRF;
    gposy * dioFR;
    gposy * dioRR;
    gposy * dioFF;

    // the assignment operator and the copy constructor
    // must never be called outside of this class.
    dio & operator = ( dio & );
    dio( dio & );

public:
    /*
     * constructor takes no argument.
     * simply initializes all pointers to NULL.
     */
    dio() {
        dioRF = dioFR = dioRR = dioFF = ( gposy * )NULL;
    }

    ~dio() {
        delete dioRF, dioFR, dioRR, dioFF;
    }

    /*
     * take the reference to the delay function, which is an instance
     * of 'gposy', then simply assign the pointer to it
     * to corresponding dio pointer.
     * check also that the corresponding dio pointer is empty
     * (in order to prevent the situation where the client tried to
     * assign a delay function of the same kind more than once).
     */
    void putRF( gposy & gp );
    void putFR( gposy & gp );
    void putRR( gposy & gp );
    void putFF( gposy & gp );

    /*
     * get each delay function.
     * since the member function is declared as 'const',
     * and so is the return reference type,
     * the client cannot make a change to it.
     */
    gposy & getRF()throw( noRefException );
    gposy & getFR()throw( noRefException );

    // print functions
    void print();
    void print( symbol_table & symtab );
};

/*
 * The class 'edge' represents a net in a circuit, which is
 * an output of a gate or a primary input net or a primay output net
 * of the circuit or the ground or the vdd.
 *
 * The class 'edge' contains the following private members:
 * 1) 'name'      : 'std::string' for the name,
 * 2) 'tRiseName' : 'std::string' for the name of the net in the optimization
 *         problem as a rising net,
 * 3) 'tFallName' : 'std::string' for the name of the net in the optimization
 *         problem as a falling net,
 * 4) 'inNumVec'  : 'std::vector' of 'int' for the input number through which
 *                 the net is going into the gates, hence the size of the
 *                 vector is the number of gates the net is connected as an
 *                 input net to them,
 * 5) 'fanoutNodeVec' : 'std::vector' of pointers to'node's for the
 *                 gates the net is going into as an input net,
 * 6) 'driverNode'   : pointer to'node' for the gate from which the net
 *                 comes out of,
 * 7) 'pi'        : pointer to'piinfo'; this points to 'pi' when the net
 *                 is a primary input net of the circuit, and it is NULL
 *                 otherwise.
 * 8) 'pi'        : pointer to'poinfo'; this points to 'po' when the net
 *                 is a primary output net of the circuit, and it is NULL
 *                 otherwise.
 */
class edge {
private:
    std::string name;
    std::string tRiseName;
    std::string tFallName;
    std::string tName;

    double switching_factor;

    std::vector<unsigned> inNumVec;
    std::vector< node *> fanoutNodeVec;
    double numOfMonteCarloVisits; //to record the number of visits during the monte carlo. This way, we can eliminate the Probdist for this edge and save memory.
    //edges corresponding to rise time of this edge and their rise or fall..
    std::vector<std::string> CorrEdgeTrise;
    std::vector<std::string> RiseFallTrise;


    //edges corresponding to fall time of this edge and their rise or fall..
    std::vector<std::string> CorrEdgeTfall;
    std::vector<std::string> RiseFallTfall;

    node * driverNode;

    piinfo * pi;
    poinfo * po;

    // the assignment operator and the copy constructor
    // must not be called outside of this class
    edge & operator = ( edge & );
    edge( edge & );

public:
    // constructor
    /*
     * constructor simply taking a string for its name
     */
    edge( const std::string & nm );

    // destructor
    ~edge();

    // putters
    /*
     * pass the reference to 'piinfo' in case that the net is a primary input
     */
    void put( piinfo & );

    /*
     * pass the reference to 'poinfo' in case that the net is a primary output
     */
    void put( poinfo & );

    /*
     * pass the reference to'node' as the gate for which the net is
     * the output net.
     */
    /* Output node is the gate for which this net is the output!!
     */
    void putOutNode( node & on );

    /*
     * pass the reference to'node' with 'int' for which the net
     * is the 'i'th input net.
     */
    /* Input node is a list of gates for which this is a fanin net
     * */
    void putInNode( node & in, unsigned i );

    //insert in the list of edges all the edges whose rise/fall (specified)
    //timings are correlated with this edge's rise/fall time
    void putCorrNetTrise( const std::string & in,bool rf );
    void putCorrNetTfall( const std::string & in,bool rf );

    void print_corr_edges();

    std::string getCorrNets( const std::string & risefall );


    void putSwitchingFactor( double sf );

    // getters
    const std::string & getName() const {
        return name;
    }

    unsigned getNumFanoutNodes() const {
        return fanoutNodeVec.size();
    }

    unsigned getFO() {
        assert( !isPO() );
        return getNumFanoutNodes();
    }

    node & getFanoutNode( unsigned i );

    unsigned getInCCCNum( unsigned i ) { //return the number of the input of the ccc that is the i th ccc in this edge's fanout.
        assert( i < inNumVec.size() );
        return inNumVec[i];
    }

    node * getDriverNode() const {
        return driverNode;
    }

    void resetNumOfMCVisits() {
        numOfMonteCarloVisits =0;
    }

    bool isPI() const {
        return ( pi!=( piinfo * )NULL );
    }

    bool isPO() const {
        return ( po!=( poinfo * )NULL );
    }

    bool skipEdgeTiming();

    bool isGNDorVDD() const;

    piinfo & getPI() {
        assert( pi!=( piinfo * )NULL );
        return *pi;
    }


    poinfo & getPO() {
        assert( po!=( poinfo * )NULL );
        return *po;
    }

    // this value needs to be added to the delay from the primary inputs
    // since this will return 0.0 when this net is not a primary net,
    // the client can safely add this value in any case,
    // (actually must add this value to correctly calculate the delay).

    double getPreValue( bool rf );

    // get energy
    std::string getEnergy( bool noLoad, bool UseDefActFact );
    std::string getLogicEnergy( bool noLoad, bool UseDefActFact );
    std::string getWireEnergy( bool UseDefActFact );
    std::string getLoadEnergy( bool UseDefActFact );

    double getSwitchingFactor();

    /*
     * get the rise/fall load capacitances
     */
    std::string getRCLoad( opt_prob_generator & opt );
    std::string getFCLoad( opt_prob_generator & opt );

    // get capacitor for energy calculation
    std::string getEnergyCLoad( bool noLoad );
    std::string getParCap();
    std::string getWireCap();
    std::string getWidth();

    double valRCLoad( std::map<std::string,double> & optVs )const;

    double valFCLoad( std::map<std::string,double> & optVs )const;

    /*
     * get the net name as a rising net possibly after concatenating to 'pre'
     */
    std::string getNetTRiseName() {
        return getTRiseName();
    }

    std::string getTRiseName() {
        return getTRiseName( EMPTY_STRING );
    }

    std::string getTRiseName( unsigned num ) {
        return getTRiseName( num,EMPTY_STRING );
    }

    std::string getTRiseName( unsigned num,const std::string & pre );

    std::string getTRiseName( const std::string & pre );
    std::string getTRiseName_Corr( const std::string & pre, bool isNoRiseFall ); //The bool is true when rise fall delay is not considered separately
    //so the correlated nets are both rise, as all the nets are appended with .Trise in the NoRiseFallTiming scenario.


    std::string getTFallName_Corr( const std::string & pre );

    std::string getMonteTRiseName() {
        return getMonteTRiseName( EMPTY_STRING );
    }

    std::string getMonteTRiseName( const std::string & pre );

    /*
     * get the net name as a falling net possibly after concatenating to 'pre'
     */
    std::string getNetTFallName() {
        return getTFallName();
    }

    std::string getTFallName() {
        return getTFallName( EMPTY_STRING );
    }

    std::string getTFallName( unsigned num ) {
        return getTFallName( num,EMPTY_STRING );
    }

    std::string getTFallName( unsigned num,const std::string & pre );

    std::string getTFallName( const std::string & pre );

    std::string getMonteTFallName() {
        return getMonteTFallName( EMPTY_STRING );
    }

    std::string getMonteTFallName( const std::string & pre );

    // print
    void print();

    // ANALYSIS

    /*
     * do Monte Carlo for the edge and update the constants for nodes.
     * if the edge is a primary input, it simply assigns a constant distribution,
     * and assigns max of the inputs plus delays, otherwise.
     */
    void montecarlo( std::map<std::string,ProbDist *> & mc,
                     std::map<std::string,std::vector<unsigned> > & netSelectMap,
                     const std::string & dist, unsigned N, double p,
                     std::map<std::string,double> & optV, bool noTriseTfalls );

    void montecarlo( std::map<std::string,ProbDist *> & mc,
                     std::map<std::string,std::vector<unsigned> > & netSelectMap,
                     std::map<std::string,ProbDist *> & gatePDMap,
                     const monte_carlo & monte, double p,
                     std::map<std::string,double> & optVs, bool noTriseTfall );

    void nominalAnalysis( std::map<std::string,double> & nomAnlys,
                          std::map<std::string,unsigned> & netSelectVec,
                          std::map<std::string,std::vector<double> > & pathLengthMap,
                          std::map<std::string,std::vector<double> > & pathVarianceMap,
                          std::map<std::string,double > & gateDioMap,
                          std::map<std::string,double> & optVs, bool noTriseTfall );

    /*  void addOneToCriticalNets( std::map<std::string,double> & criticality,
         std::map<std::string,unsigned> & netSelectMap )
      {
              return addOneToCriticalNets( criticality, netSelectMap, true );
     }
    */
    void addOneToCriticalNets( std::map<std::string,double> & criticality,
                               std::map<std::string,unsigned> & netSelectMap, bool rf, bool noTriseTfall );

};

/*
 * The class 'piinfo' contains name, input capacitance constraint,
 * rising slope, falling slope, and the starting time.
 */
class piinfo {
    std::string name;
    gposy * cap;
    symbol_table * symtab;

    std::string capString;
    double slope_r, slope_f, t;
    bool nameB, capB, slopeB, tB;

public:
    // constructor
    piinfo() {
        nameB = capB = slopeB = tB = false;
    }

    // destructor
    ~piinfo() {
        delete cap;
        delete symtab;
    }

    // putters
    void put( const std::string & s,const std::string & nm );
    void put( const std::string & nm, double );
    void put( const std::string & nm, double, double );
    void put( const std::string & nm,gposy & cp,symbol_table & st );

    // getters
    bool hasProperlyInitiated() {
        return ( nameB && capB && slopeB && tB );
    }

    const std::string & getName() const {
        assert( nameB );
        return name;
    };

    std::string & capToString() {
        assert( capB );
        return capString;
    }

    double getT() {
        assert( tB );
        return t;
    }

    double getRSlope() {
        assert( slopeB );
        return slope_r;
    }

    double getFSlope() {
        assert( slopeB );
        return slope_f;
    }

};

/*
 * The class 'poinfo' contains name and output load capacitance.
 */
class poinfo {
    std::string name;

    gposy * cap;
    symbol_table * symtab;

    std::string capString;
    bool nameB, capB;

public:
    // constructor
    poinfo() {
        nameB = capB = false;
    }

    // destructor
    ~poinfo() {
        delete cap;
        delete symtab;
    }

    // putters
    void put( const std::string & _NAME_,const std::string & name );
    void put( const std::string & _C_,gposy & cap,symbol_table & );

    // getters
    bool hasProperlyInitiated() {
        return ( nameB && capB );
    }

    std::string & getName() {
        assert( nameB );
        return name;
    }

    std::string & capToString() {
        assert( capB );
        return capString;
    }

    double getCap( std::map<std::string,double> & vls );

};

/*
 * The class 'node' represents a gate or a capacitor to the ground
 * in a circuit considered. It also has also basic functionalities
 * via its public member functions, most of * which are getters.
 * Instead the number of input arguments of constructors
 * is quite large, hence can take all the information to construct
 * a 'node' instance (e.g. as opposed to the class 'ccc').
 *
 * The class 'node' has the following private members,
 * 1) 'name'      : 'std::string' for the name,
 * 2) 'inEdgeVec' : 'std::vector' of pointers to 'edge' for input nets,
 * 3) 'outEdge'   : reference to 'edge' for output net,
 * 4) 'cc'        : pointer to 'ccc' for the ccc type of the 'node',
 * 5) 'type'      : string indicating what the 'node' type is CVRLG,
 * 6) 'cap'       : 'std::string' for capacitor
 *                  in case that 'type == "c"'
 */
class node {
private:

    static const bool KAPPA_ADDITIVE, KAPPA_CLIPPING;
    static const bool BETA_ADDITIVE, BETA_CLIPPING;
    static const double KAPPA_M_LWR, KAPPA_M_UPR, KAPPA_A_LWR, KAPPA_A_UPR;
    static const double BETA_M_LWR, BETA_M_UPR, BETA_A_LWR, BETA_A_UPR;


    //name is the unique name of the node used to identify it and
    //its recursive delay constaints
    //
    //vname is the name prefixed to its variables, its usually
    //same as the name, but sometimes when user wants to deUniquify
    //this node then the vname is not unique and is used to
    //prefix the variables.
    std::string name;
    std::string vname;
    std::vector<edge *> inEdgeVec;
    std::vector<edge *> outEdgeVec;
    std::map<std::string, double> capIntNetAFmap;
    std::map<std::string, double> wireIntNetAFmap;
    ccc  * cc;

    // type of node : capacitor (c), voltage source(v), gate(g), resistor(r), inductor(i)
    std::string type;
    std::vector<unsigned int> CorrInputnum;
    const gposy * VRLCval;
    bool couple; //only for capacitor nodes .. to indicate that coupling is on.
    // the following symbol table is used to store the
    // variable names of the gposy cap is this node happens to be
    // a capacitor, other wise this symbol table is never used.
    symbol_table symtab;
    std::vector<double> duty_factor; //the fraction of time a particular output is high

    /*
     * for STAT OPT
     */
    double RBeta;
    double FBeta;

    double * RKappas;
    double * FKappas;
    //table for recording the length of the longest path
    //that goes thru a given pair of input output.
    //The first index is input and the second is
    //output. This is useful in finding the right kappa to be
    //used for statistical optimization.
    std::vector<std::vector <double> > path_length_table;

    //store the latest local criticality values of the inputs w.r.t different outputs.
    //This will be used to get the edge criticalities during montecarlo.
    //The strings are outputs and the vectors are the relative criticality values of
    //different inputs. All criticalies for every output sum to 1.
    //Not Implemented yet
    std::map<std::string, std::vector<double> > InputCritMapFR;
    std::map<std::string, std::vector<double> > InputCritMapRF;


    //DEPRECATED
    //double critRise;
    //double critFall;

    //These arrays are basically used to record if this node had a delay
    //from input i to output j, so that while formulating the slope constraints
    //we can use the same variable instead of having to put the entire expression of
    //dio < slope_value;
    std::vector<std::vector <unsigned> > rfSlopeRecord;
    std::vector<std::vector <unsigned> > frSlopeRecord;

    // the assignment operator and the copy constructor
    // must not be called outside of this class
    node & operator = ( node & );
    node( node & );

public:
    static const std::string BETWEEN_NETS;

    static const double Dbeta;
    static const double Dkappa;
    static const double InitKappa;
    static const double MaxKappa;
    static const double MinCrit;
    static const double ZeroCrit;

public:
    // constructor for 'gate' node
    // constructor for 'cap' node
    // constructor for coupled 'cap' node
    // constructor for 'v r l' node
    node( const std::string & node_name, std::vector<edge *> & output_edge_list, ccc & ccc_type, std::vector<edge *> & input_edge_list, const std::string & var_name );
    node( const std::string & node_name, edge & output_net, gposy & c, symbol_table & st, const std::string & vnm );
    node( const std::string & nm, edge & ie1, edge & ie2, gposy & c , symbol_table & st, const std::string & vnm, bool couple );
    node( const std::string & VRLname, edge & in, edge & out, const std::string  type, gposy & val , symbol_table & st );

    // destructor
    ~node();

    // setters
    void setCoupling();
    /*DEPRECATED : now nodes can have multiple outputs
    void setCritRise(double cr)
    {
          critRise = cr;
          return;
    }
    void setCritFall(double cf)
    {
          critFall = cf;
          return;
    }
    */
    // getters
    const std::string & getName() const {
        return name;
    }

    const std::string & getvName() const {
        return vname;
    }

    ccc & getCCC() {
        if( cc == ( ccc * )NULL ) {
            std::cout <<"Failed to get the CCC of the node " << getName() << std::endl;
            std::cout <<"Returning (ccc *) NULL" << std::endl;
            //exit(-1);
        }
        return *cc;
    }


    /*
     * get the number of inputs
     * if the client calls this function for a capacitor node
     * this function generates an error message saying so
     * by calling reportReport.
     */
    double getDutyFactor( unsigned no );
    unsigned getNumberOfInputs();
    unsigned getNumberOfOutputs();
    void putCorrInputnum( unsigned int num );
    void putDutyFactor( double df ,const std::string & out ); //assign duty factor to the
    void put_path_length( unsigned ni, unsigned no, double pl );
    //output of the gate (for leakage energy purposes)
    /*
     * tell whether the node is a cap. node or not (i.e. gate node).
     */
    bool isCapacitor() {
        //             cout <<" the type of "<< getName() <<" is "<< type << endl;
        if( type == "c" ) {
            return true;
        }
        else {
            return false;
        }
    }

    bool isVRL() {
        //             cout <<" the type of "<< getName() <<" is "<< type << endl;
        if( type == "v" || type == "r" || type == "l" ) {
            return true;
        }
        else {
            return false;
        }
    }


    std::string getType() {
        return type;
    }

    std::vector<edge *> & getInEdgeVec() {
        return inEdgeVec;
    }

    std::vector<edge *> & getOutEdgeVec() {
        return outEdgeVec;
    }

    std::map<std::string, double>  getCapIntNetAFmap() {
        return capIntNetAFmap;
    }

    std::map<std::string, double>  getWireIntNetAFmap() {
        return wireIntNetAFmap;
    }

    void putIntNetAF( std::string netName, double af );


    unsigned getRFslopeRecord( unsigned i, unsigned j ) {
        assert( i<getNumberOfInputs()  && j < getNumberOfOutputs() );
        return rfSlopeRecord[i][j];
    }

    unsigned getFRslopeRecord( unsigned i, unsigned j ) {
        assert( i<getNumberOfInputs()  && j < getNumberOfOutputs() );
        return frSlopeRecord[i][j];
    }

    edge & getInEdge( unsigned num );
    edge & getOutEdge( unsigned num );


    std::string getWNName( unsigned num );
    std::string getWPName( unsigned num );

    /* get the internal net caps energy */
    std::string getCapIntNetEnergyStt();
    std::string getWireIntNetEnergyStt();
    //  std::string getCapIntNetEnergy();
    void getCapIntNetEnergy( std::ostream & os, std::string Vdd );
    void getWireIntNetEnergy( std::ostream & os, std::string Vdd );

    /*
     * get the rise/fall load capacitance
     */
    std::string getRCLoad( unsigned num,opt_prob_generator & ) ;
    std::string getFCLoad( unsigned num,opt_prob_generator & ) ;

    double valRCLoad( unsigned num, std::map<std::string,double> & ) ;
    double valFCLoad( unsigned num, std::map<std::string,double> & ) ;

    // get cap for energy calculation
    std::string getEnergyCLoad( unsigned num ) ; //for a capacitor anynumber will do.
    std::string getParCap( unsigned num );
    std::string getLeakPow( bool stat, unsigned num );

    /*
     * get area of widths; simply sums over all widths
     */
    std::string getArea();
    //Sometimes you want to add area of this to something else
    //so we have a prefix attched to the area expression.
    //This prefix is generally something like +.*,_ etc.
    std::string getArea( const std::string & pre );

    /*
     * get beta and kappas
     */
    double getRBeta();
    double getFBeta();

    double * getRKappas();
    double * getFKappas();
    double getPathLength( unsigned ni, unsigned no );

    /*
     * get Dio functions
     */

    std::string getDioRF( unsigned ni, unsigned no,opt_prob_generator & opt ) ;

    std::string getDioFR( unsigned ni, unsigned no,opt_prob_generator & opt ) ;

    std::string getSTDRF( unsigned ni, unsigned no,opt_prob_generator & opt )
    throw( noRefException );

    std::string getSTDFR( unsigned ni, unsigned no,opt_prob_generator & opt )
    throw( noRefException );

    std::string getVARRF( unsigned ni, unsigned no,opt_prob_generator & opt )
    throw( noRefException );

    std::string getVARFR( unsigned ni, unsigned no,opt_prob_generator & opt )
    throw( noRefException );

    double meanDioRF( unsigned ni, unsigned no,std::map<std::string,double> & optVs )
    ;

    double meanDioFR( unsigned ni, unsigned no,std::map<std::string,double> & optVs )
    ;

    double stdDioRF( unsigned ni, unsigned no,std::map<std::string,double> & optVs )
    ;

    double stdDioFR( unsigned ni, unsigned no,std::map<std::string,double> & optVs )
    ;

    double varDioRF( unsigned ni, unsigned no,std::map<std::string,double> & optVs )
    ;

    double varDioFR( unsigned ni, unsigned no,std::map<std::string,double> & optVs )
    ;

    void print();

    // OPTIMIZATION & MONTECARLO

    /*
     * set kappas identically to {kappa} and set beta to {beta}
     */
    void setKappaBeta( double kappa, double beta );

    // ANALYSIS

    /*
     * do montecarlo for each input net first, then,
     */
    void montecarlo( std::map<std::string,ProbDist *> & mc,
                     std::map<std::string,std::vector<unsigned> > & netSelectMap,
                     const std::string & dist, unsigned N, double p,
                     std::map<std::string,double> & optVs, bool noTriseTfall );

    void montecarlo( std::map<std::string,ProbDist *> & mc,
                     std::map<std::string,std::vector<unsigned> > & netSelectMap,
                     std::map<std::string,ProbDist *> & gatePDMap,
                     const monte_carlo & monte, double p,
                     std::map<std::string,double> & optVs, bool noTriseTfall );

    // nominal analysis
    void nominalAnalysis( std::map<std::string,double> & nomAnlys,
                          std::map<std::string,unsigned> & netSelectVec,
                          std::map<std::string,std::vector<double> > & pathLengthMap,
                          std::map<std::string,std::vector<double> > & pathVarianceMap,
                          std::map<std::string,double > & gateDioMap,
                          std::map<std::string,double> & optVs, bool noTriseTfall );

    void dioAnalysis( std::map<std::string,double > & gateDioMap,
                      std::map<std::string,double> & optVs, bool noTriseTfall );


    void addOneToCriticalNets( std::map<std::string,double> & criticality,
                               std::map<std::string,unsigned> & netSelectMap,
                               unsigned n, bool rf, bool noTriseTfall );

    bool skipDelayExpression( unsigned num );
    std::ostream & delayConstraintToOstream ( std::ostream & os,opt_prob_generator & opt, bool stt );

    std::ostream & delayConstraintToOstream( std::ostream & os,opt_prob_generator & opt,opt_spec & osp );

    void updateKappa( double & kappa, double quantile1, double quantile2, double mean, double std );

    //update the kappas based on criticality.
    void updateCritKappa( bool rf, unsigned num,double criticality, std::string & obj, double kmax, double maxCrit );

    void updateBeta( double & beta, double quantile, double max_quantile );


private:
    std::ostream & rfDelayConstraintToOstream( std::ostream & os, unsigned ni, unsigned no, opt_prob_generator & opt, bool stt ) {
        return delayConstraintToOstream( os,ni,no,opt,true,stt );
    }

    std::ostream & frDelayConstraintToOstream( std::ostream & os, unsigned ni, unsigned no,opt_prob_generator & opt, bool stt ) {
        return delayConstraintToOstream( os,ni,no,opt,false,stt );
    }

    std::ostream & delayConstraintToOstream( std::ostream & os, unsigned ni,unsigned no,opt_prob_generator & opt, bool rf ) {
        return delayConstraintToOstream( os,ni,no,opt,rf,false );
    }

    std::ostream & delayConstraintToOstream( std::ostream & os, unsigned ni, unsigned no, opt_prob_generator & opt, bool rf, bool stt );
    std::ostream & delayConstraintToOstream( std::ostream & os, unsigned ni, unsigned no, opt_prob_generator & opt, bool rf, opt_spec & );

    std::string getRFInternalNetName( unsigned ni, unsigned no );
    std::string getFRInternalNetName( unsigned ni, unsigned no );

};

class mos {
private:
    bool isPMos;
    bool isCap;
    std::string mosName;
    std::string gateName;
    std::string drainName;
    std::string sourceName;
    std::string bodyName;
    const gposy * widthName;

public:
    // constructors
    mos( std::vector<std::string> & strings,
         std::map<std::string, gposy *>  par_names );
    mos( const std::string & CapName, std::vector<std::string> & strings,
         const gposy * value );

    bool isGNDorVDD( std::string name ) const;
    const std::string & getMosName() const {
        return mosName;
    }

    bool isPMOS() const {
        return isPMos;
    }

    bool isNMOS() const {
        return !isPMos;
    }

    bool isCAP() const {
        return isCap;
    }

    const std::string & getGateName() const {
        return gateName;
    }

    const std::string & getDrainName() const {
        return drainName;
    }

    const std::string & getSourceName() const {
        return sourceName;
    }

    const std::string & getBodyName() const {
        return bodyName;
    }

    const gposy * getWidthName() const {
        return widthName;
    }
};

#endif

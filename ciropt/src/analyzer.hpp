#ifndef _SH_ANAL_H_
#define _SH_ANAL_H_

#include <string>
#include <vector>
#include <set>

#include "cnvt.hpp"
#include "opt.hpp"

extern double DEFAULT_QUANTILE;


class opt_dscr;
class monte_carlo;
class mvec;
class opt_spec;
// opt_spec stores the various specifications given
// by the user in the .OPTIMIZE section.
class netspec;
class subplot;
class prob_dist;

class opt_dscr {
private:
    bool byOutFileName;
    bool isCCCnameSpecified; // This is to find out if only one CCC needs to be analysed or what.
    std::string cccName;
    std::string name;

    unsigned num;

public:
    opt_dscr( const std::string & nm, unsigned nn, const std::string & cccName )
        : name( nm ), num( nn ), cccName( cccName ),
          byOutFileName( cnvt::getFileExtension( nm ) == "out" ),
          isCCCnameSpecified( cccName != "" ) {}

    opt_dscr( const std::string & optname, const std::string & cccName )
        : name( optname ), num( 0 ), cccName( cccName ),
          byOutFileName( cnvt::getFileExtension( optname ) == "out" ),
          isCCCnameSpecified( cccName != "" ) {}


    opt_dscr() {}

    // getters
    unsigned getNameLength() {
        return name.length();
    }

    std::string & getOptName() {
        assert( getNameLength() > 0 );
        return name;
    }

    unsigned getNum() {
        assert( getNameLength() > 0 );
        return num;
    }

    bool isByOutFileName() {
        assert( getNameLength() > 0 );
        return byOutFileName;
    }

    bool isForCCC() {
        return isCCCnameSpecified;
    }


    std::string & getOutFileName() {
        assert( getNameLength() > 0 );
        assert( byOutFileName );
        return name;
    }

    std::string & getcccName() {
        if( isCCCnameSpecified ) {
            return cccName;
        }
        return EMPTY_STRING;
    }

    std::string getLegend() {
        return cnvt::addTH( getNum()+1 ) + " of " + getOptName();
    }

    // others
    void putOptSpec( opt_prob_generator & opg );
    void checkLegitimacyOfReferencesToOptimizations( std::map<std::string, opt_spec *> & ops );
};



class monte_carlo {
private:
    std::string dist;
    std::string dependency;
    double depFactor;
    size_t N;

public:
    monte_carlo( const std::string & dt, size_t n )
        : dist( dt ), N( n ), depFactor( 0.0 ) {}

    monte_carlo( const std::string & dt, size_t n, const std::string & dep, double depF )
        : dist( dt ), N( n ), dependency( dep ), depFactor( depF ) {}

    monte_carlo() {}

    // getters
    bool isEmpty() const {
        return ( dist.length() == 0 );
    }

    bool isIndependent() const {
        return ( dependency.size() == 0 );
    }

    const std::string & getDistribution() const {
        assert( !isEmpty() );
        return dist;
    }

    const std::string & getDependencyType() const {
        assert( !isEmpty() );
        return dependency;
    }

    double getDependencyFactor() const {
        assert( !isEmpty() );
        return depFactor;
    }

    size_t getN() const {
        assert( dist.length() > 0 );
        return N;
    }

    // less than operator
    bool operator < ( const monte_carlo & monte )const  ;
};

std::ostream & operator<< ( std::ostream &, monte_carlo & );

class mvec {
private:
    std::string kind;
    std::string name;
    opt_dscr optDscr;
    monte_carlo monte;

public:
    mvec( const std::string & kd, const std::string & nm, opt_dscr & od );
    mvec( const std::string & kd, const std::string & nm, opt_dscr & od,
          monte_carlo & mt );
    mvec() {}

    // getters
    std::string & getKind() {
        assert( kind.length() > 0 );
        return kind;
    }

    std::string & getName() {
        assert( kind.length() > 0 );
        return name;
    }

    opt_dscr & getOptDscr() {
        assert( kind.length() > 0 );
        return optDscr;
    }

    monte_carlo & getMonte() {
        assert( kind.length() > 0 );
        return monte;
    }

    void putOptSpec( opt_prob_generator & opg ) {
        getOptDscr().putOptSpec( opg );
    }

    std::ostream & toOstream( std::ostream & os, opt_prob_generator & opg ) ;
};

class prob_dist {
private:
    std::string kind;
    double first, second;
    double mean, std, var;

public:
    prob_dist();
    prob_dist( const std::string & kind, double first, double second );

    bool isEmpty() const {
        return ( kind.length() == 0 );
    }

    const std::string & getKind() const {
        return kind;
    }

    double getMean() const {
        return mean;
    }

    double getSTD() const {
        return std;
    }

    double getVAR() const {
        return var;
    }
};

class opt_spec {
private:
    static  bool SHORT_LEGEND;

    bool byOutFileName;

    // opt_spec by opt specs
    std::string name;
    std::string objective;

    unsigned numberOfOpts;
    std::string distribution;
    unsigned N; //# of monte carlo iterations
    double p; //the quantile value
    double kmax; //the maximum value of kappa (used for criticality based iterative statistical soln)

    double sval, eval;
    unsigned numval;
    double kappa, beta;
    std::vector<double> kappas, betas;

    bool noRiseFall;

    prob_dist u_dist, v_dist;
    unsigned M;

    // optimal values history: for storage of results of synthesis and analysis optimal values
    // a vector of maps:
    // each map: variable name => optimal value
    std::vector< std::map< std::string, double> > optVHist;

    // Monte Carlo
    std::map<monte_carlo,std::vector<std::map<std::string,ProbDist *> > > monteHist;

    // statistical criticality indices (from Monte Carlo)
    std::map<monte_carlo,std::vector<std::map<std::string,double> > >
    statCrtcHist;

    // nominal timing analysis
    std::vector<std::map<std::string,double> > nomAnlysHist;
    std::vector<std::vector<double> > pathLengthsHist;
    std::vector<std::vector<double> > pathVariancesHist;
    std::vector<std::vector<double> > gateDiosHist;
    std::vector<std::string> gateDioNames;

    // deterministic criticality indices (0or1) (from nominal timining analysis)
    std::vector<std::map<std::string,double> > detCritHist;

    std::set<std::string> drawingNetList;

public:
    opt_spec( bool nrf, const std::string & nm, const std::string & wtm )
        : name( nm ), objective( wtm ), noRiseFall( nrf ), numberOfOpts( 1 ),
          byOutFileName( false ) {
        checkItIsNotOutFileName( name );
    }

    opt_spec( bool nrf, const std::string & nm,const  std::string & wtm,
              unsigned numiter, unsigned num, double pp, const std::string & dist )
        : name( nm ), objective( wtm ),
          numberOfOpts( numiter ), N( num ), p( pp ),
          distribution( dist ), noRiseFall( nrf ),
          byOutFileName( false ), kmax( 0 ) {
        checkItIsNotOutFileName( name );
    }

    opt_spec( bool nrf, const std::string & nm,const  std::string & wtm,
              unsigned numiter, unsigned num, double pp, const std::string & dist, double k )
        : name( nm ), objective( wtm ),
          numberOfOpts( numiter ), N( num ), p( pp ),
          distribution( dist ), noRiseFall( nrf ),
          byOutFileName( false ), kmax( k ) {
        checkItIsNotOutFileName( name );
    }

    opt_spec( bool nrf, const std::string & nm, const std::string & wtm,
              double s, double e, unsigned num );

    opt_spec( bool rnf, const std::string & nm, const std::string & wtm,
              std::vector<double> & vals );

    opt_spec( bool nrf, const std::string & nm, const std::string & wtm,
              double alp, double bt )
        : name( nm ), objective( wtm ), kappa( alp ), beta( bt ),
          noRiseFall( nrf ), numberOfOpts( 1 ),
          byOutFileName( false ) {
        checkItIsNotOutFileName( name );
    }

    opt_spec( bool nrf, const std::string & nm,
              const prob_dist & u,const  prob_dist & v, unsigned M );

    opt_spec( const std::string & outfilename );

    ~opt_spec();

    // getters
    std::string & getOptName() {
        return getName();
    }

    std::string & getName() {
        return name;
    }

    bool isNoRiseFall() {
        return noRiseFall;
    }

    bool isByOutFileName() {
        return byOutFileName;
    }

    std::string & getObjective() {
        return objective;
    }

    unsigned getLengthOfOptValHistory() {
        return optVHist.size();
    }

    unsigned getNumberOfOptimizations() {
        return numberOfOpts;
    }

    unsigned getNumberOfNomAnlys() {
        unsigned n = nomAnlysHist.size();
        assert( n == pathLengthsHist.size() );
        assert( n == pathVariancesHist.size() );
        assert( n == detCritHist.size() );

        return n;
    }

    unsigned getM() {
        assert( objective == "expected_cmax" );
        return M;
    }

    prob_dist & getU() {
        assert( objective == "expected_cmax" );
        return u_dist;
    }

    prob_dist & getV() {
        assert( objective == "expected_cmax" );
        return v_dist;
    }

    // std::string & getDistribution() {
    //    assert( objective == "stat_delay" ); return distribution;
    // }

    // unsigned getNumberOfMonteCarlos() {
    //    assert( objective == "stat_delay" ); return N;
    // }

    monte_carlo getMonte() {
        assert( objective == "stat_delay_update" || objective == "stat_delay_CritUpdate" );
        return monte_carlo( distribution,N );
    }

    double getQuantileValue() {
        assert( objective == "stat_delay_update" || objective == "stat_delay_CritUpdate" );
        return p;
    }

    double getKmax() {
        assert( objective == "stat_delay_update" || objective == "stat_delay_CritUpdate" );

        if( objective == "stat_delay_CritUpdate" ) {
            return kmax;
        }
        else {
            return 0;
        }
    }

    double getStartingValue() {
        return sval;
    }

    double getEndingValue() {
        return eval;
    }

    unsigned getNumberOfValues() {
        return numval;
    }

    double getKappa() {
        return kappa;
    }

    double getBeta() {
        return beta;
    }

    std::vector<double> & getKappas() {
        return kappas;
    }

    std::vector<double> & getBetas() {
        return betas;
    }

    double getNumber( unsigned num ) ;

    std::string getLegend( std::string & netname ) {
        return getLegend( netname, 0 );
    }

    std::string getLegend( std::string & netname, unsigned num ) ;

    double getOptimalValue( std::string & name, unsigned num ) ;

    std::vector<std::map<std::string,double> > & getOptValuesHistory() {
        return optVHist;
    }

    std::map< std::string,double > & getOptValues( unsigned num ) {
        assert( num < optVHist.size() );
        return optVHist[ num ];
    }

    std::vector<double> getOptWidthVector( size_t num ) ;

    // ANALYSIS
    std::map< std::string, ProbDist *> & getMonteCarlo ( monte_carlo & monte,opt_prob_generator & op, unsigned num, double p );
    std::map< std::string, double    > & getCrtcMap    ( monte_carlo & monte,opt_prob_generator & op, size_t   num, double p );

    std::map<std::string,double> & getNomAnlys( opt_prob_generator & op, unsigned num );
    std::vector<double> & getPathLengths   ( opt_prob_generator & op, unsigned num );
    std::vector<double> & getPathVariances ( opt_prob_generator & op, unsigned num );

    std::vector<double> & getGateDios( opt_prob_generator & op, unsigned num, const std::string cccName );

    std::vector<std::string> & getGateDioNames() {
        return gateDioNames;
    }

    std::map<std::string,double> & getDetCrtcMap( opt_prob_generator & op, size_t num );

    // storers
    void store( std::map< std::string, double > & ov ) {

        optVHist.push_back( ov );
    }

    void store( std::map< std::string, ProbDist * > & mc, std::map<std::string,double> & criticality, const monte_carlo & monte ) {
        monteHist[monte].push_back( mc );
        std::vector<std::map<std::string,ProbDist *> > res =  monteHist[monte];
        statCrtcHist[monte].push_back( criticality );
    }

    void store( std::map< std::string, double > & ov, std::map<std::string,ProbDist *> & mc, std::map<std::string,double> & criticality, monte_carlo & monte ) {
        store( ov );
        store( mc, criticality, monte );
    }

    void storeNomAnlys( std::map<std::string,double> & nomAnlys,
                        std::map<std::string,double> & criticality,
                        std::vector<double> & pathLengths,
                        std::vector<double> & pathVariances,
                        std::vector<double> & gateDios,
                        std::vector<std::string> & gDN ) {
        nomAnlysHist.push_back( nomAnlys );
        detCritHist.push_back( criticality );
        pathLengthsHist.push_back( pathLengths );
        pathVariancesHist.push_back( pathVariances );
        gateDiosHist.push_back( gateDios );
        gateDioNames = gDN;
    }

    void assertHistoryLengths() ;

    void storeTheNetList( subplot & sp );

    bool isInTheNetList( const std::string & netName ) ;

    void checkLegitimacyOfReferencesToOptimizations( netspec & ns ) ;

private:
    static void checkItIsOutFileName( const std::string & optname );
    static void checkItIsNotOutFileName( const std::string & optname );
    static bool isWidthName( const std::string & name );
};

class netspec {
private:
    opt_dscr optDscr;
    std::string net_name;

public:
    // constructor
    netspec( const std::string & optname, const std::string & netname )
        : optDscr( optname,"" ), net_name( netname ) {
        assert( getNetNameLength() > 0 );
    }

    netspec( const std::string & optname, unsigned nn, const std::string & netname )
        : optDscr( optname,nn,"" ), net_name( netname ) {
        assert( getNetNameLength() > 0 );
    }

    netspec() {}

    // getters
    bool isByOutFileName() {
        assert( getNetNameLength() > 0 );
        return getOptDscr().isByOutFileName();
    }

    std::string & getOptName() {
        assert( getNetNameLength() > 0 );
        return getOptDscr().getOptName();
    }

    std::string & getNetName() {
        assert( getNetNameLength() > 0 );
        return net_name;
    }

    unsigned getNum() {
        assert( getNetNameLength() > 0 );
        return getOptDscr().getNum();
    }

    opt_dscr & getOptDscr() {
        assert( getNetNameLength() > 0 );
        return optDscr;
    }

    unsigned getNetNameLength() {
        return net_name.length();
    }

    std::string getLegend( opt_prob_generator & opt ) ;

    void checkLegitimacyOfReferencesToOptimizations
    ( std::map<std::string, opt_spec *> & optSps ) ;

    void putOptSpec( opt_prob_generator & opg ) ;
};


class subplot {
private:
    static std::vector<std::string> colors;
    static std::vector<std::string> styles;
    static std::vector<std::string> mrkers;
    static bool initDone;

    static void initializeColorsStyles();
    static bool PLOT_OPTIMAL_VALUES;

    static double WIDTH_DIST_SMOOTH_FACTOR;

    std::string kind;

    // if kind is "values", then we use the following fields
    netspec netSpec;
    std::vector<double> quantiles;

    // if kind is "pdf" or "cdf", we use the netspec vector below
    std::vector<netspec> netspecs;
    //this is the quantile
    double p;

    // if kind is "WIDTHDIST" or "CRCALDIST", we use opt_dscr vector
    std::vector<opt_dscr> optDscrList;

    // common fiels for both cases
    monte_carlo monte;

    std::string xlabel;
    std::string ylabel;
    std::string title;

public:
    // constructors
    subplot( const std::string & cdf_or_pdf, monte_carlo & mt, double p,
             std::vector<netspec> & netspecVec );

    subplot( const std::string & cdf_or_pdf, monte_carlo & mt,
             std::vector<netspec> & netspecVec );

    subplot( const std::string & values, netspec & nspec,
             monte_carlo & mt, std::vector<double> & qts );

    subplot ( const std::string & kind,
              std::vector<opt_dscr> & optDscrList );

    subplot ( const std::string & kind, monte_carlo & mt,
              std::vector<opt_dscr> & optDscrList );


    // DEPRECATED
    subplot( const std::string & cdf_or_pdf,
             const std::string & dist, unsigned N, double p,
             std::vector<netspec> & netspecVec );

    subplot( const std::string & cdf_or_pdf,
             const std::string & dist, unsigned N,
             std::vector<netspec> & netspecVec );

    subplot( const std::string & values,
             const std::string & optname, const std::string & netname,
             const std::string & dist, unsigned N, std::vector<double> & qts );

    subplot ( const std::string & kind,
              const std::string & dist, unsigned N,
              std::vector<opt_dscr> & optDscrList );

    // getters
    std::vector<netspec> & getNetSpecs() {
        assert( getKind() == "pdf" || getKind() == "cdf" );
        return netspecs;
    }

    std::string & getKind() {
        return kind;
    }

    netspec & getNetSpec() {
        assert( getKind() == "values" );
        return netSpec;
    }

    std::string & getOptName() {
        assert( getKind() == "values" );
        return netSpec.getOptName();
    }

    std::string & getNetName() {
        assert( getKind() == "values" );
        return netSpec.getNetName();
    }

    monte_carlo & getMonte() {
        std::string & kd = getKind();
        assert( kd=="pdf" || kd=="cdf" || kd=="values" || kd=="CRCALDIST" );
        return monte;
    }

    /* DEPRECATED
    std::string & getDistribution()
    { assert( getKind() == "pdf" || getKind() == "cdf"
    || getKind() == "values" || getKind() == "CRCALDIST" );
         return distribution; }
       */

    /* DEPRECATED
    unsigned getN()
    { assert( getKind() == "pdf" || getKind() == "cdf"
    || getKind() == "values" || getKind() == "CRCALDIST" );
         return N; }
       */

    // to ostream
    std::ostream & toOstream( std::ostream & os, opt_prob_generator & opt ) ;

    // checkers
    void checkLegitimacyOfReferencesToOptimizations( std::map<std::string, opt_spec *> & optSps );

    // others
    void putOptSpec( opt_prob_generator & opg ) ;
};

#endif

#include "randomc.hpp"
#include "analyzer.hpp"

int ciropterror( const std::string & s );
int errorReport( const std::string & s );
void errorBasedHalt();
bool errorExist();

//
// CLASS opt_dscr MEMBER FUNCTION DEFINITIONS
//
void opt_dscr::checkLegitimacyOfReferencesToOptimizations( std::map<std::string, opt_spec *> & optSps ) {
   
   assert( getNameLength() > 0 );

   std::map<std::string, opt_spec *>::iterator itr;

   for( itr = optSps.begin(); itr != optSps.end(); itr ++ ) {
      opt_spec & optSp = * itr->second;
      unsigned n = optSp.getNumberOfOptimizations();

      if( optSp.getName() == getOptName() ) {
         if( getNum() >= n ) {
            errorReport(
               "illegal attempt to access "
               + cnvt::addTH( getNum()+1 ) + " optimization "
               + "of " + getOptName()
               + ", which has only " + cnvt::toString( n )
               + cnvt::addS( " optimization", ( int )n ) + "."
            );
         }

         return;
      }
   }
   
   errorReport( "optimization name error: no optimization with name '" + getOptName() + "'" );

   return;
}

void opt_dscr::putOptSpec( opt_prob_generator & opg ) {
   assert( getNameLength() > 0 );

   if( !isByOutFileName() ) return;

   opg.addOutFileOptSpec( getOptName() );

   return;
}

//
// CLASS monte_carlo MEMBER FUNCTION DEFINITIONS
//
bool monte_carlo::operator < ( const monte_carlo & monte ) const {
   return ( getDependencyType()  <  monte.getDependencyType()
            ||    getDependencyType()   == monte.getDependencyType()
            && getDependencyFactor() <  monte.getDependencyFactor()
            ||    getDependencyType()   == monte.getDependencyType()
            && getDependencyFactor() ==  monte.getDependencyFactor()
            && getDistribution()     <  monte.getDistribution()
            ||    getDependencyType()   == monte.getDependencyType()
            && getDependencyFactor() ==  monte.getDependencyFactor()
            && getDistribution()     == monte.getDistribution()
            && getN()                <  monte.getN() );
}

ostream & operator << ( ostream & os, monte_carlo	& monte ) {
   os << monte.getN() << " MONTECARLOs with " << monte.getDistribution();

   if( !monte.isIndependent() ) {
      os << ", " << monte.getDependencyType() << " with factor " << monte.getDependencyFactor();
   }

   return os;
}

//
// CLASS mvec MEMBER FUNCTION DEFINITIONS
//
mvec::mvec( const std::string & kd, const std::string & nm, opt_dscr & od ) : kind( kd ), name( nm ), optDscr( od ) {
   assert( kd == "widths" || kd == "criticalities" || kd == "pathlengths" || kd == "pathvariances" || kd == "gate_dios" );
   return;
}

mvec::mvec( const std::string & kd, const std::string & nm, opt_dscr & od, monte_carlo & mt ) : kind( kd ), name( nm ), optDscr( od ), monte( mt ) {
   assert( kd == "criticalities" );
   return;
}

ostream & mvec::toOstream( ostream & os, opt_prob_generator & opg ) {
   
   opt_dscr    & oD = getOptDscr();
   monte_carlo & mC = getMonte();

   opt_spec & oS = opg.getOptSpec( oD.getOptName() );

   if     ( getKind() == "widths" ) {
      
      std::vector<double> widths = oS.getOptWidthVector( oD.getNum() );
      cnvt::doubleVectorToMatlabOstream( os,widths,getName() );
   }
   else if( getKind() == "criticalities" && mC.isEmpty() ) {
      
      std::vector<double> crical = cnvt::mapToDoubleVector( oS.getDetCrtcMap( opg, oD.getNum() ) );
      cnvt::doubleVectorToMatlabOstream( os,crical,getName() );
   }
   else if( getKind() == "criticalities" && !mC.isEmpty() ) {
      
      std::vector<double> crical = cnvt::mapToDoubleVector( oS.getCrtcMap( mC, opg, oD.getNum(), DEFAULT_QUANTILE ) );
      cnvt::doubleVectorToMatlabOstream( os,crical,getName() );
   }
   else if( getKind() == "pathlengths" ) {
      
      std::vector<double> pLs = oS.getPathLengths( opg, oD.getNum() );
      cnvt::doubleVectorToMatlabOstream( os,pLs,getName() );
   }
   else if( getKind() == "pathvariances" ) {
      
      std::vector<double> pVs = oS.getPathVariances( opg, oD.getNum() );
      cnvt::doubleVectorToMatlabOstream( os,pVs,getName() );
   }
   else if( getKind() == "gate_dios" ) {
      
      std::vector<double> gDs = oS.getGateDios( opg, oD.getNum(), EMPTY_STRING );
      cnvt::doubleVectorToMatlabOstream( os,gDs,getName() );
   }
   else {
      assert( false );
   }

   return os;
}

//
// CLASS opt_spec MEMBER FUNCTION DEFINITIONS
//
bool opt_spec::SHORT_LEGEND = true;

opt_spec::opt_spec( bool nrf, const std::string & nm, const std::string & wtm, double s, double e, unsigned num ) : name( nm ), objective( wtm ), sval( s ), eval( e ), numval( num ), noRiseFall( nrf ), numberOfOpts( num ), byOutFileName( false ) {

   checkItIsNotOutFileName( name );

   double intv = e - s;

   assert( num > 1 );

   if( wtm == "stat_delay_kappas" ) {
      for( unsigned i = 0; i < num; i ++ ) {
         kappas.push_back( s + intv * ( ( double )i ) / ( ( double )( num-1 ) ) );
         betas.push_back( 0.0 );
      }
   }
   else if( wtm == "stat_delay_betas" ) {

      for( unsigned i = 0; i < num; i ++ ) {
         betas.push_back( s + intv * ( ( double )i ) / ( ( double )( num-1 ) ) );
         kappas.push_back( 0.0 );
      }
   }
   else {
      assert( false );
   }
}

opt_spec::opt_spec( bool rnf, const std::string & nm, const std::string & wtm, std::vector<double> & vals ) : name( nm ), objective( wtm ), noRiseFall( rnf ), numberOfOpts( vals.size() ), byOutFileName( false ) {

   checkItIsNotOutFileName( name );

   if( wtm == "stat_delay_kappas" ) {

      for( unsigned i = 0; i < vals.size(); i ++ ) {
         kappas.push_back( vals[i] );
         betas.push_back( 0.0 );
      }
   }
   else if( wtm == "stat_delay_betas" ) {
      
      for( unsigned i = 0; i < vals.size(); i ++ ) {
         betas.push_back( vals[i] );
         kappas.push_back( 0.0 );
      }
   }
   else {
      assert( false );
   }
}

opt_spec::opt_spec( bool nrf, const std::string & nm, const prob_dist & u,const  prob_dist & v, unsigned mm ) : byOutFileName( false ), noRiseFall( nrf ), name( nm ), objective( "expected_cmax" ), u_dist( u ), v_dist( v ), M( mm ), numberOfOpts( 1 ) {
   
   checkItIsNotOutFileName( name );
}

opt_spec::opt_spec( const std::string & outfilename ) : byOutFileName( true ), name( outfilename ), numberOfOpts( 1 ), noRiseFall( false ) {

   checkItIsOutFileName( name );

   std::map<std::string,double> optVs;

   if( opt_prob_generator::ggpsolOutToMap( optVs, getName() ) != 0 ) {
      errorReport( "ggpsol output reading error: " + getName() );
      errorBasedHalt();
   }

   optVHist.push_back( optVs );
   //  noRiseFall = opt_prob_generator::isDOSIMPLE( optVs );
}

opt_spec::~opt_spec() {
   
   std::map<monte_carlo,std::vector<std::map<std::string,ProbDist *> > >::iterator it;

   for( it = monteHist.begin(); it != monteHist.end(); it++ ) {

      std::vector<std::map<std::string,ProbDist *> > & mcHistory = it->second;

      for( unsigned i = 0; i < mcHistory.size(); i++ ) {
         
         std::map<std::string,ProbDist *>::iterator lit;
         
         for( lit = mcHistory[i].begin(); lit != mcHistory[i].end(); lit++ ) {
            delete lit->second;
         }
      }
   }
}

double opt_spec::getNumber( unsigned num ) {
   assert( num < getNumberOfOptimizations() );

   if     ( objective == "stat_delay_kappas"     ) {
      assert( kappas.size() == getNumberOfOptimizations() );
      return kappas[num];
   }
   else if( objective == "stat_delay_betas"      ) {
      assert( betas.size() == getNumberOfOptimizations() );
      return kappas[num];
   }
   else if( objective == "stat_delay_update"     ) {
      return ( double )num;
   }
   else if( objective == "stat_delay_CritUpdate" ) {
      return ( double )num;
   }
   assert( false );
   return 0.0;
}

std::string opt_spec::getLegend( std::string & netname, unsigned num ) {

   assert( num < getNumberOfOptimizations() );

   if( isByOutFileName() ) return getName();

   if     ( objective == "stat_delay_kappas"     ) {
      assert( kappas.size() == getNumberOfOptimizations() );
      return ( SHORT_LEGEND? "":( netname + " of " + getName() + " when " ) )
             + "kappa = " + cnvt::doubleToString( kappas[num] );
   }
   else if( objective == "stat_delay_betas"      ) {
      assert( kappas.size() == getNumberOfOptimizations() );
      return ( SHORT_LEGEND? "":( netname + " of " + getName() + " when " ) )
             + "beta = " + cnvt::doubleToString( betas[num] );
   }
   else if( objective == "stat_delay_kappabeta"  ) {
      return ( SHORT_LEGEND? "":( netname + " of " + getName() + " when " ) )
             + "alppa = " + cnvt::doubleToString( kappa )
             + ", beta = " + cnvt::doubleToString( beta );
   }
   else if( objective == "delay" || objective == "power" || objective == "area" ) {
      return ( SHORT_LEGEND? "":( netname+" of " ) ) + getName();
   }
   else if( objective == "stat_delay_update"     ) {
      return cnvt::intToString( ( int )num );
   }
   else if( objective == "stat_delay_CritUpdate" ) {
      return cnvt::intToString( ( int )num );
   }
   else if( objective == "expected_cmax"         ) {
      return ( SHORT_LEGEND? netname:netname + " of scenario-based opt." );
   }

   assert( false );

   return "";
}

double opt_spec::getOptimalValue( std::string & name, unsigned num ) {
   
   assert( getNumberOfOptimizations() == getLengthOfOptValHistory() );
   assert( num < getNumberOfOptimizations() );

   std::map< std::string,double>::iterator it = optVHist[num].find( name );

   if( it == optVHist[num].end() ) {

      errorReport ( "illegal attempt to obtain the optimal value for " + name + " in " + cnvt::addTH( num+1 ) + " optimization of " + getName() + "." );
      return 0.0;
   }

   return it->second;
}

std::vector<double> opt_spec::getOptWidthVector( size_t num ) {

   std::vector<double> vec;

   std::map<std::string,double> & optVs = getOptValues( num );

   std::map<std::string,double>::iterator it;

   for( it = optVs.begin(); it != optVs.end(); it++ ) {
      if( isWidthName( it->first ) ) vec.push_back( it->second );
   }

   cout << "# of optimal widths in opt(" << getOptName() << "," << ( num+1 ) << ") is " << vec.size() << endl;

   return vec;
}

std::map<std::string, ProbDist*> & opt_spec::getMonteCarlo( monte_carlo & monte, opt_prob_generator & opg, unsigned num , double p ) {

   unsigned n = getNumberOfOptimizations();

   assert( getLengthOfOptValHistory() == n );

   std::map<monte_carlo,std::vector<std::map<std::string,ProbDist *> > >::iterator itr;

   assert( num < n );

   if( ( itr = monteHist.find( monte ) ) == monteHist.end() ) {
      opg.montecarloAndStore( monte, p, *this );
   }

   assert( ( itr = monteHist.find( monte ) ) != monteHist.end() );

   std::vector<std::map<std::string,ProbDist *> > & mcHistory = itr->second;

   assert( mcHistory.size() == n );

   return mcHistory[num];
}

std::map<std::string,double> & opt_spec::getCrtcMap( monte_carlo & monte, opt_prob_generator & opg, size_t num, double p ) {

   unsigned n = getNumberOfOptimizations();

   assert( getLengthOfOptValHistory() == n );
   assert( num < n );

   std::map<monte_carlo,std::vector<std::map<std::string,ProbDist *> > >::iterator itr;

   std::map<monte_carlo,std::vector<std::map<std::string,double> > >::iterator citr;

   if( ( itr = monteHist.find( monte ) ) == monteHist.end() ) {
      opg.montecarloAndStore( monte, p, *this );
   }

   assert( ( itr = monteHist.find( monte ) )   != monteHist.end() );
   assert( ( citr=statCrtcHist.find( monte ) ) != statCrtcHist.end() );

   std::vector<std::map<std::string,double> > & crtcHist = citr->second;

   assert( crtcHist.size() == n );

   return crtcHist[num];
}

std::map<std::string,double> & opt_spec::getNomAnlys ( opt_prob_generator & opg, unsigned num ) {
   
   unsigned n = getNumberOfOptimizations();

   assert( num < n );
   assert( getLengthOfOptValHistory() == n );
   assert( getNumberOfNomAnlys() == 0 || getNumberOfNomAnlys() == n );

   if( getNumberOfNomAnlys() == 0 ) {
      opg.nominalAnalysisAndStore( *this, EMPTY_STRING );
   }

   assert( getNumberOfNomAnlys() == n );

   return nomAnlysHist[num];
}

std::vector<double> & opt_spec::getPathLengths( opt_prob_generator & opg, unsigned num ) {
   unsigned n = getNumberOfOptimizations();

   assert( num < n );
   assert( getLengthOfOptValHistory() == n );
   assert( getNumberOfNomAnlys() == 0 || getNumberOfNomAnlys() == n );

   if( getNumberOfNomAnlys() == 0 ) {
      opg.nominalAnalysisAndStore( *this, EMPTY_STRING );
   }

   assert( getNumberOfNomAnlys() == n );
   assert( pathLengthsHist.size() == n );

   return pathLengthsHist[num];
}

std::vector<double> & opt_spec::getPathVariances( opt_prob_generator & opg, unsigned num ) {
   
   unsigned n = getNumberOfOptimizations();

   assert( num < n );
   assert( getLengthOfOptValHistory() == n );
   assert( getNumberOfNomAnlys() == 0 || getNumberOfNomAnlys() == n );

   if( getNumberOfNomAnlys() == 0 ) {
      opg.nominalAnalysisAndStore( *this, EMPTY_STRING );
   }

   assert( getNumberOfNomAnlys() == n );
   assert( pathVariancesHist.size() == n );

   return pathVariancesHist[num];
}

std::vector<double> & opt_spec::getGateDios( opt_prob_generator & opg, unsigned num, const std::string cccName ) {

   unsigned n = getNumberOfOptimizations();

   assert( num < n );
   assert( getLengthOfOptValHistory() == n );
   assert( getNumberOfNomAnlys() == 0 || getNumberOfNomAnlys() == n );

   if( getNumberOfNomAnlys() == 0 ) {
      opg.nominalAnalysisAndStore( *this, cccName );
   }

   assert( getNumberOfNomAnlys() == n );
   assert( gateDiosHist.size() == n );

   return gateDiosHist[num];
}

std::map<std::string,double> & opt_spec::getDetCrtcMap( opt_prob_generator & opg, size_t num ) {

   unsigned n = getNumberOfOptimizations();

   assert( num < n );
   assert( detCritHist.size() == 0 || detCritHist.size() == n );

   if( detCritHist.size() == 0 ) {
      opg.nominalAnalysisAndStore( *this, EMPTY_STRING );
   }

   assert( detCritHist.size() == n );

   return detCritHist[num];
}

void opt_spec::assertHistoryLengths() {
   
   unsigned n = getNumberOfOptimizations();

   assert( n == getLengthOfOptValHistory() );

   assert( getNumberOfNomAnlys() == 0 || getNumberOfNomAnlys() == n );

   std::map<monte_carlo,std::vector<std::map<std::string,ProbDist *> > >::iterator itr;

   for( itr = monteHist.begin(); itr != monteHist.end(); itr++ ) {
      assert( n == itr->second.size() );
   }

   return;
}

void opt_spec::storeTheNetList( subplot & subplot ) {
   if( subplot.getKind() == "values" ) {
      if( getName() == subplot.getOptName() )
         drawingNetList.insert( subplot.getNetName() );

      return;
   }

   if( subplot.getKind() == "pdf" || subplot.getKind() == "cdf" ) {
      std::vector<netspec> & netspecvec = subplot.getNetSpecs();
      for( unsigned i = 0; i < netspecvec.size(); i ++ ) {
         if( getName() == netspecvec[i].getOptName() )
            drawingNetList.insert( netspecvec[i].getNetName() );
      }

      return;
   }

   if( subplot.getKind() == "WIDTHDIST" || subplot.getKind() == "CRCALDIST" ) {
      return;
   }

   assert( false );
}

bool opt_spec::isInTheNetList( const std::string & netName ) {
   return ( drawingNetList.find( netName ) != drawingNetList.end() );
}

void opt_spec::checkItIsOutFileName( const std::string & name ) {

   if( cnvt::getFileExtension( name ) == "out" ) { return; }

   errorReport( name + " does not have .out extension" );
}

void opt_spec::checkItIsNotOutFileName( const std::string & name ) {

   if( cnvt::getFileExtension( name ) != "out" ) { return; }

   ciropterror( "optimization name must not have .out extension: " + name );
}

bool opt_spec::isWidthName( const std::string & name ) {
   return ( name.find( "WN" ) < name.length() || name.find( "WP" ) < name.length() );
}


//
// CLASS netspec MEMBER FUNCTION DEFINITIONS
//
std::string netspec::getLegend( opt_prob_generator & opt ) {
   assert( getNetNameLength() > 0 );
   return opt.getOptSpec( getOptName() ).getLegend( getNetName(),getNum() );
}

void netspec::checkLegitimacyOfReferencesToOptimizations( std::map<std::string, opt_spec *> & optSps ) {

   assert( getNetNameLength() > 0 );

   std::map<std::string, opt_spec *>::iterator itr;

   for( itr = optSps.begin(); itr != optSps.end(); itr++ ) {
      opt_spec & optSp = * itr->second;
      unsigned n = optSp.getNumberOfOptimizations();

      if( optSp.getName() == getOptName() ) {
         if( getNum() >= n )
            errorReport(
               "illegal attempt to access a net name in "
               + cnvt::addTH( getNum()+1 ) + " optimization "
               + "of " + getOptName()
               + ", which has only " + cnvt::toString( n )
               + cnvt::addS( " optimization", ( int )n ) + "."
            );

         return;
      }
   }

   errorReport( "optimization name error: no optimization with name '" + getOptName() + "'" );
}

void netspec::putOptSpec( opt_prob_generator & opg ) {
   assert( getNetNameLength() > 0 );

   if( !isByOutFileName() ) return;

   opg.addOutFileOptSpec( getOptName() );
}

//
// CLASS subplot MEMBER FUNCTION DEFINITIONS
//
double subplot::WIDTH_DIST_SMOOTH_FACTOR = 0.1;

std::vector<std::string> subplot::colors;
std::vector<std::string> subplot::styles;
std::vector<std::string> subplot::mrkers;

bool subplot::initDone = false;
bool subplot::PLOT_OPTIMAL_VALUES = true;

void subplot::initializeColorsStyles() {
   if( initDone ) return;

   colors.push_back( "b" );
   colors.push_back( "r" );
   colors.push_back( "k" );
   colors.push_back( "m" );
   colors.push_back( "y" );
   colors.push_back( "c" );
   colors.push_back( "g" );

   styles.push_back( "-" );
   styles.push_back( "--" );
   styles.push_back( "-." );
   styles.push_back( ":" );

   mrkers.push_back( "+" );
   mrkers.push_back( "o" );
   mrkers.push_back( "*" );
   mrkers.push_back( "." );
   mrkers.push_back( "x" );
   mrkers.push_back( "s" );
   mrkers.push_back( "d" );
   mrkers.push_back( "^" );
   mrkers.push_back( "v" );
   mrkers.push_back( ">" );
   mrkers.push_back( "<" );
   mrkers.push_back( "p" );
   mrkers.push_back( "h" );

   initDone = true;

   return;
}

subplot::subplot( const std::string & cdfpdf, monte_carlo & mt, double pp, std::vector<netspec> & netspecV ) : kind( cdfpdf ), monte( mt ), p( pp ), netspecs( netspecV ), xlabel( "x" ), ylabel( "y" ), title( cdfpdf ) {

   assert( getKind() == "cdf" || getKind() == "pdf" );
   assert( p > 0.0 && p < 1.0 );
   assert( netspecs.size() > 0 );

   initializeColorsStyles();
}

subplot::subplot( const std::string & cdfpdf, monte_carlo & mt, std::vector<netspec> & netspecV ) : kind( cdfpdf ), monte( mt ), p( DEFAULT_QUANTILE ), netspecs( netspecV ), xlabel( "x" ), ylabel( "y" ), title( cdfpdf ) {

   assert( getKind() == "cdf" || getKind() == "pdf" );
   assert( netspecV.size() > 0 );

   initializeColorsStyles();
}

subplot::subplot( const std::string & values, netspec & nspec, monte_carlo & mt, std::vector<double> & qts ) : kind( values ), monte( mt ), netSpec( nspec ), quantiles( qts ), xlabel( "x" ), ylabel( "y" ), title( "t" ) {

   assert( getKind() == "values" );
   assert( quantiles.size() > 0 );

   for( unsigned i = 0; i < quantiles.size(); i ++ )
      assert( quantiles[i] > 0.0 && quantiles[i] < 1.0 );

   initializeColorsStyles();
}

subplot::subplot( const std::string & kd, std::vector<opt_dscr> & ODL ) : kind( kd ), optDscrList( ODL ), xlabel( "x" ), ylabel( "y" ), title( kd ) {

   assert( getKind() == "WIDTHDIST" );
}

subplot::subplot( const std::string & kd, monte_carlo & mt, std::vector<opt_dscr> & ODL ) : kind( kd ), optDscrList( ODL ), monte( mt ), xlabel( "x" ), ylabel( "y" ), title( kd ) {

   assert( getKind() == "CRCALDIST" );
}

subplot::subplot( const std::string & cdfpdf, const std::string & dist, unsigned NN, double pp, std::vector<netspec> & netspecV ) : kind( cdfpdf ), monte( dist,NN ), p( pp ), netspecs( netspecV ), xlabel( "x" ), ylabel( "y" ), title( cdfpdf ) {

   assert( getKind() == "cdf" || getKind() == "pdf" );
   assert( p > 0.0 && p < 1.0 );
   assert( netspecs.size() > 0 );

   initializeColorsStyles();
}

subplot::subplot( const std::string & cdfpdf, const std::string & dist, unsigned NN, std::vector<netspec> & netspecV ) : kind( cdfpdf ), monte( dist,NN ), p( DEFAULT_QUANTILE ), netspecs( netspecV ), xlabel( "x" ), ylabel( "y" ), title( cdfpdf ) {

   assert( getKind() == "cdf" || getKind() == "pdf" );
   assert( netspecV.size() > 0 );

   initializeColorsStyles();
}

subplot::subplot( const std::string & values, const std::string & on, const std::string & nn, const std::string & dist, unsigned NN, std::vector<double> & qts ) : kind( values ), monte( dist,NN ), netSpec( on,nn ), quantiles( qts ), xlabel( "x" ), ylabel( "y" ), title( "t" ) {

   assert( getKind() == "values" );
   assert( quantiles.size() > 0 );

   for( unsigned i = 0; i < quantiles.size(); i ++ ) {
      assert( quantiles[i] > 0.0 && quantiles[i] < 1.0 );
   }

   initializeColorsStyles();
}

subplot::subplot( const std::string & kd, const std::string & dist, unsigned NN, std::vector<opt_dscr> & ODL ) : kind( kd ), optDscrList( ODL ), monte( dist,NN ), xlabel( "x" ), ylabel( "y" ), title( kd ), p( DEFAULT_QUANTILE ) {

   assert( getKind() == "CRCALDIST" );
}

ostream & subplot::toOstream( ostream & os, opt_prob_generator & opt ) {

   os << "hold on;" << endl;

   if( getKind() == "pdf" || getKind() == "cdf" ) {
      unsigned colorNum, styleNum;
      for( unsigned i = 0; i < netspecs.size(); i ++ ) {
         ProbDist & pd = opt.getProbDist( netspecs[i],getMonte(),p );
         pd.xMatVecToOstream( os << "x" << i << " = [" ) << "];" << endl;

         if( getKind() == "pdf" ) {
            pd.pdfMatVecToOstream( os << "y" << i << " = [" ) << "];" << endl;
         }
         else {
            pd.cdfMatVecToOstream( os << "y" << i << " = [" ) << "];" << endl;
         }
      }

      os << "plot( ..." << endl;
      for( unsigned i = 0; i < netspecs.size(); i ++ ) {
         colorNum = i - ( ( unsigned )( i/colors.size() ) )*colors.size();
         styleNum = i - ( ( unsigned )( i/styles.size() ) )*styles.size();

         os << "  x" << i << ",y" << i << ",'";
         os << colors[colorNum] << styles[styleNum];

         if( i != netspecs.size() - 1 ) os << "', ..." << endl;
         else os << "');" << endl << endl;
      }

      os << "legend(";
      for( unsigned i = 0; i < netspecs.size(); i ++ ) {
         os << "'" << netspecs[i].getLegend( opt ) << "'";

         if( i != netspecs.size() - 1 )
            os << ",";
      }
      os << ");" << endl;

      if( xlabel.length() != 0 ) os << "xlabel('" << xlabel << "');" << endl;
      if( ylabel.length() != 0 ) os << "ylabel('" << ylabel << "');" << endl;
      if( title.length() != 0 ) os << "title('" << title << "');" << endl;

      if( p != 0.0 ) {
         for( unsigned i = 0; i < netspecs.size(); i ++ ) {
            colorNum = i - ( ( unsigned )( i/colors.size() ) )*colors.size();
            styleNum = i - ( ( unsigned )( i/styles.size() ) )*styles.size();

            ProbDist & pd = opt.getProbDist( netspecs[i],getMonte(),p );
            double qp = pd.getPercentilePoint( p );
            os << "line( " << qp << " * [1 1], [ 0, ";
            os << ( ( kind=="pdf" )?pd.getPDF( qp ):p ) << "],";
            os << "'Color','" << colors[colorNum];
            os << "','LineStyle','" << styles[styleNum] << "');" << endl << endl;
         }
      }
   }
   else if( getKind() == "values" ) {
      // store
      std::vector<double> points;
      opt_spec & osp = opt.getOptSpec( getOptName() );
      std::string netname = getNetName();

      for( unsigned i = 0; i < quantiles.size(); i ++ ) {
         points.clear();
         for( unsigned j = 0; j < osp.getNumberOfOptimizations(); j ++ ) {
            
            ProbDist & pd = opt.getProbDist( osp,j,netname,getMonte(),quantiles[i] );
            points.push_back( pd.getPercentilePoint( quantiles[i] ) );
         }

         cnvt::doubleVectorToOstream( os << "monte" << i << " = [", points );
         os << "];" << endl;
      }

      std::vector<double> nums;
      points.clear();
      for( unsigned j = 0; j < osp.getNumberOfOptimizations(); j ++ ) {
         
         points.push_back( osp.getOptimalValue( netname,j ) );
         nums.push_back( osp.getNumber( j ) );
      }

      cnvt::doubleVectorToOstream( os << "optvals = [", points ) << "];" << endl;
      cnvt::doubleVectorToOstream( os << "nums = [", nums ) << "];" << endl;

      // plot
      if( PLOT_OPTIMAL_VALUES ) {
         os << "plot( nums, optvals, '" << colors[0] << styles[0] << mrkers[0] << "', ..." << endl;
      }
      else {
         os << "plot( ..." << endl;
      }

      for( unsigned i = 0; i < quantiles.size(); i ++ ) {
         unsigned j = PLOT_OPTIMAL_VALUES? ( i+1 ):i;
         unsigned colorNum = j - ( ( unsigned )( j/colors.size() ) )*colors.size();
         unsigned styleNum = j - ( ( unsigned )( j/styles.size() ) )*styles.size();
         unsigned mrkerNum = j - ( ( unsigned )( j/mrkers.size() ) )*mrkers.size();

         os << "  nums, monte" << i << ",'";
         os << colors[colorNum] << styles[styleNum] << mrkers[mrkerNum];

         if( i != quantiles.size() - 1 ) os << "', ..." << endl;
         else os << "');" << endl << endl;
      }

      // legend
      if( PLOT_OPTIMAL_VALUES ) {
         os << "legend( 'opt_value', ";
      }
      else {
         os << "legend( ";
      }

      for( unsigned i = 0; i < quantiles.size(); i ++ ) {
         os << ( ( i==0 )? "'":",'" ) << quantiles[i] << "'";
      }
      os << ");" << endl;

      if( xlabel.length() != 0 ) os << "xlabel('" << xlabel << "');" << endl;
      if( ylabel.length() != 0 ) os << "ylabel('" << ylabel << "');" << endl;
      if( title.length() != 0 )  os << "title('" << title << "');" << endl;

   }
   else if( getKind() == "WIDTHDIST" ) {
      
      assert( optDscrList.size() > 0 );

      std::vector<std::string> xnames;
      std::vector<std::string> ynames;
      std::vector<std::string> legends;

      for( std::vector<opt_dscr>::size_type i = 0; i < optDscrList.size(); i ++ ) {
         std::string & name = optDscrList[i].getOptName();
         unsigned num = optDscrList[i].getNum();

         std::string xname = "widths" + cnvt::intToString( i+1 );
         std::string yname = "pdf" + cnvt::intToString( i+1 );

         std::vector<double>  wv
            = opt.getOptSpec( name ).getOptWidthVector( num );

         std::vector<double> x, y;

         ProbDist::doubleVectorToSmoothedPDF( x,y,wv,WIDTH_DIST_SMOOTH_FACTOR );

         cnvt::doubleVectorToMatlabOstream( os, x, xname );
         cnvt::doubleVectorToMatlabOstream( os, y, yname );

         xnames.push_back( xname );
         ynames.push_back( yname );
         legends.push_back( optDscrList[i].getLegend() );
      }

      cnvt::matlabPlotToOstream( os, xnames, ynames );
      cnvt::matlabLegendToOstream( os, legends );

      os << "xlabel('" << xlabel << "');" << endl;
      os << "ylabel('" << ylabel << "');" << endl;
      os << "title('" << title << "');" << endl;
   }
   else if( getKind() == "CRCALDIST" ) {
      
      assert( optDscrList.size() > 0 );

      std::vector<std::string> xnames;
      std::vector<std::string> ynames;
      std::vector<std::string> legends;

      for( std::vector<opt_dscr>::size_type i = 0; i < optDscrList.size(); i ++ ) {
         std::string & name = optDscrList[i].getOptName();
         unsigned num = optDscrList[i].getNum();

         std::string xname = "crcls" + cnvt::intToString( i+1 );
         std::string yname = "pdf" + cnvt::intToString( i+1 );

         std::map<std::string,double> & cm
            = opt.getOptSpec( name ).getCrtcMap( getMonte(),opt,num, p );

         std::map<std::string,double>::iterator cmItr = cm.begin();
         os << "% Net Criticalities by name and in a vector" << endl;
         os << "netcrit = [" << endl;

         for ( cmItr = cm.begin(); cmItr != cm.end(); cmItr++ ) {
            os << "%" << cmItr->first << "\t\t" << cmItr->second << endl;
            os << cmItr->second << ";" << endl;
         }
         os << "];" << endl;

         std::vector<double> x, y;

         ProbDist::doubleVectorToSmoothedPDF( x,y, cnvt::mapToDoubleVector( cm ), WIDTH_DIST_SMOOTH_FACTOR );

         cnvt::doubleVectorToMatlabOstream( os, x, xname );
         cnvt::doubleVectorToMatlabOstream( os, y, yname );

         xnames.push_back( xname );
         ynames.push_back( yname );
         legends.push_back( optDscrList[i].getLegend() );
      }

      cnvt::matlabPlotToOstream( os, xnames, ynames );
      cnvt::matlabLegendToOstream( os, legends );

      os << "xlabel('" << xlabel << "');" << endl;
      os << "ylabel('" << ylabel << "');" << endl;
      os << "title('" << title << "');" << endl;
   }
   else {
      assert( false );
   }

   os << "hold off;" << endl;

   return os;
}

void subplot::checkLegitimacyOfReferencesToOptimizations( std::map<std::string, opt_spec *> & optSps ) {

   if( getKind() == "values" ) {
      getNetSpec().checkLegitimacyOfReferencesToOptimizations( optSps );
      return;
   }
   else if( getKind() == "pdf" || getKind() == "cdf" ) {
      for( unsigned i = 0; i < getNetSpecs().size(); i ++ ) {
         getNetSpecs()[i].checkLegitimacyOfReferencesToOptimizations( optSps );
      }
   }
   else if( getKind() == "WIDTHDIST" || getKind() == "CRCALDIST" ) {
      for( std::vector<opt_dscr>::size_type i = 0; i < optDscrList.size(); i ++ ) {
         optDscrList[i].checkLegitimacyOfReferencesToOptimizations( optSps );
      }
   }
   else {
      assert( false );
   }
}

void subplot::putOptSpec( opt_prob_generator & opg ) {

   if( getKind() == "values" ) {
      getNetSpec().putOptSpec( opg );
   }
   else if( getKind() == "pdf" || getKind() == "cdf" ) {
      for( size_t i = 0; i < getNetSpecs().size(); i ++ )
         getNetSpecs()[i].putOptSpec( opg );
   }
   else if( getKind() == "WIDTHDIST" || getKind() == "CRCALDIST" ) {
      for( std::vector<opt_dscr>::size_type i = 0; i < optDscrList.size(); i ++ )
         optDscrList[i].putOptSpec( opg );
   }
   else {
      assert( false );
   }
}

// MEMBER FUNCTIONS of CLASS prob_dist

prob_dist::prob_dist() : kind( "normal" ), first( 0.0 ), second( 0.0 ), mean( 0.0 ), std( 0.0 ), var( 0.0 ) {}

prob_dist::prob_dist( const std::string & kd, double fst, double snd )  : kind( kd ), first( fst ), second ( snd ) {

   if ( kind == "normal" ) {
      mean = fst;
      var = snd;
      std = sqrt( var );
   }
   else if ( kind == "uniform" ) {
      mean = ( fst+snd )/2;
      var = ( fst-snd )*( fst-snd )/12;
      std = sqrt( var );
   }
   else {
      assert( false );
   }
}

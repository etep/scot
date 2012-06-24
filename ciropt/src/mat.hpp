#ifndef _SH_MAT_H_
#define _SH_MAT_H_

#include <ostream.h>
#include <vector>

class Mat {
public:
   static ostream & arrayToOstream ( ostream & os, const double *, unsigned n,  const std::string & name );
   static ostream & vecToOstream   ( ostream & os, const std::vector<double> &, const std::string & name );
};
#endif

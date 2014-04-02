#ifndef _SH_MAT_H_
#define _SH_MAT_H_

#include <vector>
#include <ostream>

class Mat {
public:
    static std::ostream & arrayToOstream ( std::ostream & os, const double *, unsigned n,  const std::string & name );
    static std::ostream & vecToOstream   ( std::ostream & os, const std::vector<double> &, const std::string & name );
};
#endif

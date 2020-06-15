#ifndef OBERON_R_RBITFINEX_H
#define OBERON_R_RBITFINEX_H


#include "rcoinbase.h"
#include <Rcpp.h>
using Rcpp::DataFrame;


namespace oberon { namespace core { class Reconstructor; }  } 

namespace oberon {

namespace R {

class RBitfinex : public RStore {
  public:
    Rcpp::DataFrame run(const Rcpp::DataFrame & quotes, const Rcpp::DataFrame & trades, bool extract_only = false);

};

} // namespace oberon::R

} // namespace oberon
#endif

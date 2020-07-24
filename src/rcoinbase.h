#ifndef OBERON_R_RCOINBASE_H
#define OBERON_R_RCOINBASE_H


#include <Rcpp.h>
using Rcpp::List;

#include <Rcpp.h>
using Rcpp::DataFrame;

#include "rcommon.h"

namespace oberon { namespace core { class Reconstructor; }  } 

namespace oberon {

namespace R {

class RCoinbase : public RStore {
  public:
    Rcpp::List run(const Rcpp::DataFrame & data, bool extract_only = false);

};

} // namespace oberon::R

} // namespace oberon
#endif

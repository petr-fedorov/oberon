#ifndef OBERON_R_RBITSTAMP_H
#define OBERON_R_RBITSTAMP_H


#include "rcommon.h"
#include <Rcpp.h>
using Rcpp::List;

#include <Rcpp.h>
using Rcpp::DataFrame;


namespace oberon { namespace core { class Reconstructor; }  } 

namespace oberon {

namespace R {

class RBitstamp : public RStore {
  public:
    Rcpp::List run(const Rcpp::DataFrame & quotes, const Rcpp::DataFrame & trades, bool extract_only = false);

};

} // namespace oberon::R

} // namespace oberon
#endif

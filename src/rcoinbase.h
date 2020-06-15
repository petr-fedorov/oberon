#ifndef OBERON_R_RCOINBASE_H
#define OBERON_R_RCOINBASE_H


#include <vector>

#include "reconstructor.h"
#include <Rcpp.h>
using Rcpp::DataFrame;


namespace oberon { namespace core { class Event; }  } 
namespace oberon { namespace core { class Reconstructor; }  } 

namespace oberon {

namespace R {

class RStore : public oberon::core::Store {
  public:
    virtual void transmit( std::unique_ptr<oberon::core::Event> && event);


  private:
    Rcpp::StringVector order_id_;

    vector<double> timestamp_;

    vector<int32_t> event_no_;

    vector<int32_t> state_;

    vector<oberon::core::Price> price_;

    vector<oberon::core::Volume> volume_;

    vector<oberon::core::Volume> delta_volume_;

    vector<int32_t> trade_id_;

    Rcpp::StringVector taker_order_id_;

    vector<double> local_timestamp_;


  public:
    Rcpp::DataFrame toDataframe();

};
class RCoinbase : public RStore {
  public:
    DataFrame run(const Rcpp::DataFrame & data, bool extract_only = false);

};

} // namespace oberon::R

} // namespace oberon
#endif

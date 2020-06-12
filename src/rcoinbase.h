#ifndef OBERON_R_RCOINBASE_H
#define OBERON_R_RCOINBASE_H


#include <Rcpp.h>
using Rcpp::DataFrame;

#include "reconstructor.h"
#include <vector>

#include <string>
using namespace std;

namespace oberon { namespace core { class Reconstructor; }  } 
namespace oberon { namespace core { class Event; }  } 

namespace oberon {

namespace R {

class RCoinbase : public oberon::core::Store {
  public:
    DataFrame run(const Rcpp::DataFrame & data, bool extract_only = false);

    virtual void transmit( std::unique_ptr<oberon::core::Event> && event);


  private:
    vector<string> order_id_;

    vector<double> timestamp_;

    vector<int32_t> event_no_;

    vector<int32_t> state_;

    vector<oberon::core::Price> price_;

    vector<oberon::core::Volume> volume_;

    vector<oberon::core::Volume> delta_volume_;

    vector<int32_t> trade_id_;

    vector<string> taker_order_id_;

    vector<double> local_timestamp_;

};

} // namespace oberon::R

} // namespace oberon
#endif

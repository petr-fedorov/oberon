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
    DataFrame run(const Rcpp::DataFrame & data);

    virtual void transmit( std::unique_ptr<oberon::core::Event> && event);


  private:
    vector<string> order_id_;

    vector<double> timestamp_;

    vector<oberon::core::EventNo> event_no_;

    vector<oberon::core::Price> price_;

    vector<oberon::core::Volume> volume_;

    vector<string> trade_id_;

    vector<oberon::core::EventType> event_type_;

};

} // namespace oberon::R

} // namespace oberon
#endif

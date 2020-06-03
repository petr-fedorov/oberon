
#include "rcoinbase.h"
#include "reconstructor.h"
#include <boost/uuid/uuid_io.hpp>


#include "date.h"
namespace oberon {

namespace R {

DataFrame RCoinbase::run(const Rcpp::DataFrame & data) {
  using oberon::core::Reconstructor;
  using oberon::core::Timestamp;
  using oberon::core::Duration;
  using namespace date;
  Rcpp::CharacterVector type = data["_type"];
  Rcpp::NumericVector timestamp = data["_time"];
  Rcpp::CharacterVector order_id = data["_order_id"];
  auto coinbase = Reconstructor::create(oberon::core::kCoinbase, "BTCUSD",this, 2.0);
  oberon::core::Timestamp current_timestamp = Timestamp();
  for(size_t i = 0; i < type.size(); ++i) {
    Timestamp next_timestamp = Timestamp(Duration(lround(timestamp[i]*1000000)));
    if(current_timestamp != next_timestamp) {
      current_timestamp = next_timestamp;
      //Rcpp::Rcout << format("%FT%TZ\n",current_timestamp);
      using boost::property_tree::ptree;
      ptree tree;
      tree.put("type", "elapsed");
      tree.put("time",format("%FT%TZ\n",current_timestamp));
      coinbase->process(tree);
    }
    if(type[i] == "open") {
      using boost::property_tree::ptree ;
      ptree tree;
      tree.put("type", "open");
      tree.put("time",format("%FT%TZ\n",current_timestamp));
      tree.put("order_id", order_id[i]);
      coinbase->process(tree);
    }
  }
  return Rcpp::DataFrame::create(Rcpp::Named("order_id")=order_id_,
      Rcpp::Named("timestamp")=timestamp_);
}

void RCoinbase::transmit( std::unique_ptr<oberon::core::Event> && event) {
  if(event) {
    order_id_.push_back(to_string(event->getOrderId()));
    timestamp_.push_back(event->getTimestamp().time_since_epoch().count()/1000000.0);
    event_no_.push_back(event->getEventNo());
    price_.push_back(event->getPrice());
    volume_.push_back(event->getVolume());
    trade_id_.push_back(to_string(event->getTradeId()));
    event_type_.push_back(event->getEventType());
  }
}


} // namespace oberon::R

} // namespace oberon

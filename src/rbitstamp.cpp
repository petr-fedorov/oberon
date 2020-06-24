
#include "rbitstamp.h"
#include "reconstructor.h"

#include "date.h"
namespace oberon {

namespace R {

Rcpp::DataFrame RBitstamp::run(const Rcpp::DataFrame & quotes, const Rcpp::DataFrame & trades, bool extract_only) {
  using oberon::core::Duration;
  using oberon::core::Reconstructor;
  using oberon::core::Timestamp;
  using namespace date;
  Rcpp::NumericVector order_id = quotes["order_id"];
  Rcpp::NumericVector amount = quotes["amount"];
  Rcpp::CharacterVector event = quotes["event"];
  Rcpp::CharacterVector order_type = quotes["order_type"];
  Rcpp::NumericVector quote_timestamp = quotes["microtimestamp"];
  Rcpp::NumericVector quote_local = quotes["local_timestamp"];
  Rcpp::NumericVector price = quotes["price"];
  
  Rcpp::NumericVector trade_id = trades["bitstamp_trade_id"];
  Rcpp::NumericVector trade_amount = trades["amount"];
  Rcpp::NumericVector trade_price = trades["price"];
  Rcpp::CharacterVector trade_type = trades["trade_type"];
  Rcpp::NumericVector trade_timestamp = trades["trade_timestamp"];
  Rcpp::NumericVector buy_order_id = trades["buy_order_id"];
  Rcpp::NumericVector sell_order_id = trades["sell_order_id"];
  Rcpp::NumericVector trade_local = trades["local_timestamp"];
  
  auto bitstamp = Reconstructor::create(oberon::core::kBitstamp, "ETHUSD", this,
                                        2.0, extract_only);
  oberon::core::Timestamp current_timestamp = Timestamp();
  size_t q{0}, t{0};
  try {
    while (q < quote_local.size() || t < trade_local.size()) {
  
      if (q < quote_timestamp.size() &&
          (t >= trade_local.size() || quote_timestamp[q] <= trade_timestamp[t])) {
        Timestamp next_timestamp =
            Timestamp(Duration(lround(quote_timestamp[q] * 1000000)));
        if (current_timestamp != next_timestamp) {
          using boost::property_tree::ptree;
          ptree tree;
          current_timestamp = next_timestamp;
          tree.put("event", "elapsed");
          tree.put("timestamp", format("%FT%TZ\n", current_timestamp));
          bitstamp->process(tree);
        }
        using boost::property_tree::ptree;
        ptree tree;
        tree.put("microtimestamp", format("%FT%TZ\n", current_timestamp));
        tree.put("local_timestamp",
                 format("%FT%TZ\n",
                        Timestamp(Duration(lround(quote_local[q] * 1000000)))));
        tree.put("event", event[q]);
        tree.put("id", order_id[q]);
        tree.put("price", price[q]);
        tree.put("amount", amount[q]);
        tree.put("order_type", order_type[q]);
        bitstamp->process(tree);
        ++q;
      } else if (t < trade_timestamp.size() &&
                 (q >= quote_timestamp.size() ||
                  trade_timestamp[t] < quote_timestamp[q])) {
        Timestamp next_timestamp =
            Timestamp(Duration(lround(trade_timestamp[t] * 1000000)));
        if (current_timestamp != next_timestamp) {
          using boost::property_tree::ptree;
          ptree tree;
          current_timestamp = next_timestamp;
          tree.put("event", "elapsed");
          tree.put("timestamp", format("%FT%TZ\n", current_timestamp));
          bitstamp->process(tree);
        }
        using boost::property_tree::ptree;
        ptree tree;
        tree.put("trade_timestamp", format("%FT%TZ\n", current_timestamp));
        tree.put("local_timestamp",
                 format("%FT%TZ\n",
                        Timestamp(Duration(lround(trade_local[t] * 1000000)))));
        tree.put("event", "match");
        tree.put("price", trade_price[t]);
        tree.put("amount", trade_amount[t]);
        tree.put("trade_id", trade_id[t]);
        if (trade_type[t] == "buy") {
          tree.put("maker_order_id", sell_order_id[t]);
          tree.put("taker_order_id", buy_order_id[t]);
          tree.put("order_type", "sell");
        } else {
          tree.put("maker_order_id", buy_order_id[t]);
          tree.put("taker_order_id", sell_order_id[t]);
          tree.put("order_type", "buy");
        }
        bitstamp->process(tree);
        ++t;
      }
    }
  } catch (const std::logic_error &e) {
    std::cout << "Exception while processing " << e.what() << std::endl;
  };
  
  // Final elapsed for cleanup
  using boost::property_tree::ptree;
  ptree tree;
  tree.put("event", "elapsed");
  tree.put("timestamp",
           format("%FT%TZ\n", current_timestamp + std::chrono::seconds{2}));
  bitstamp->process(tree);
  return toDataframe();
}


} // namespace oberon::R

} // namespace oberon


#include "rbitfinex.h"
#include "reconstructor.h"

#include "date.h"
namespace oberon {

namespace R {

Rcpp::DataFrame RBitfinex::run(const Rcpp::DataFrame & quotes, const Rcpp::DataFrame & trades, bool extract_only) {
  using oberon::core::Duration;
  using oberon::core::Reconstructor;
  using oberon::core::Timestamp;
  using namespace date;
  Rcpp::NumericVector order_id = quotes["order_id"];
  Rcpp::NumericVector amount = quotes["amount"];
  Rcpp::NumericVector quote_timestamp = quotes["exchange_timestamp"];
  Rcpp::NumericVector quote_local = quotes["local_timestamp"];
  Rcpp::NumericVector price = quotes["price"];
  
  Rcpp::NumericVector trade_id = trades["trade_id"];
  Rcpp::NumericVector trade_amount = trades["qty"];
  Rcpp::NumericVector trade_price = trades["price"];
  Rcpp::NumericVector trade_timestamp = trades["exchange_timestamp"];
  Rcpp::NumericVector trade_local = trades["local_timestamp"];
  
  auto bitfinex = Reconstructor::create(oberon::core::kBitfinex, "ETHUSD", this,
                                        2.0, extract_only);
  oberon::core::Timestamp current_timestamp = Timestamp();
  size_t q{0}, t{0};
  while (q < quote_local.size() || t < trade_local.size()) {
  
    if (q < quote_local.size() && (t >= trade_local.size() || quote_local[q] <= trade_local[t])) {
      Timestamp next_timestamp =
          Timestamp(Duration(lround(quote_timestamp[q] * 1000000)));
      if (current_timestamp != next_timestamp) {
        using boost::property_tree::ptree;
        ptree tree;
        current_timestamp = next_timestamp;
        tree.put("event", "elapsed");
        tree.put("timestamp", format("%FT%TZ\n", current_timestamp));
        bitfinex->process(tree);
      }
      using boost::property_tree::ptree;
      ptree tree;
      if (price[q])
        tree.put("event", "order_changed");
      else
        tree.put("event", "order_deleted");
      tree.put("exchange_timestamp", format("%FT%TZ\n", current_timestamp));
      // std::cout << "\r" << tree.get<string>("exchange_timestamp") << std::flush;
      tree.put("local_timestamp",
               format("%FT%TZ\n",
                      Timestamp(Duration(lround(quote_local[q] * 1000000)))));
      tree.put("order_id", order_id[q]);
      tree.put("price", price[q]);
      tree.put("amount", amount[q]);
      bitfinex->process(tree);
      ++q;
    } else if (t < trade_local.size() && (q >= quote_local.size() || trade_local[t] < quote_local[q])) {
      Timestamp next_timestamp =
          Timestamp(Duration(lround(trade_timestamp[t] * 1000000)));
      if (current_timestamp != next_timestamp) {
        using boost::property_tree::ptree;
        ptree tree;
        current_timestamp = next_timestamp;
        tree.put("event", "elapsed");
        tree.put("timestamp", format("%FT%TZ\n", current_timestamp));
        bitfinex->process(tree);
      }
      using boost::property_tree::ptree;
      ptree tree;
      tree.put("event", "match");
      tree.put("mts", format("%FT%TZ\n", current_timestamp));
      tree.put("local_timestamp",
               format("%FT%TZ\n",
                      Timestamp(Duration(lround(trade_local[t] * 1000000)))));
      tree.put("event", "match");
      tree.put("price", trade_price[t]);
      tree.put("amount", trade_amount[t]);
      tree.put("id", trade_id[t]);
      bitfinex->process(tree);
      ++t;
    }
  }
  
  // Final elapsed for cleanup
  using boost::property_tree::ptree;
  ptree tree;
  tree.put("event", "elapsed");
  tree.put("timestamp",
           format("%FT%TZ\n", current_timestamp + std::chrono::seconds{2}));
  bitfinex->process(tree);
  return toDataframe();
}


} // namespace oberon::R

} // namespace oberon

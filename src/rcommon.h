#ifndef OBERON_R_RCOMMON_H
#define OBERON_R_RCOMMON_H


#include "reconstructor.h"
#include <vector>

#include <map>

#include <Rcpp.h>
using Rcpp::List;

#include <string>
using namespace std;
#include <Rcpp.h>
using Rcpp::DataFrame;


#include "date.h"
#include <chrono>
#include <boost/uuid/string_generator.hpp>
namespace oberon { namespace core { class Event; }  } 

namespace oberon {

namespace R {

class RDepthImage : public oberon::core::Store {
  public:
    RDepthImage(const oberon::core::Price & min_price, const oberon::core::Price & max_price, double tick_size, double start, double end);

    using Column = std::vector<double>;


  private:
    map<oberon::core::OrderId, std::unique_ptr<oberon::core::Event>> events_;


  public:
    Rcpp::List toList();


  private:
    double tick_size_;

    oberon::core::Price min_price_;

    oberon::core::Price max_price_;

    vector<double> x_;

    vector<string> x_lab_;

    vector<double> y_;

    vector<std::shared_ptr<Column>> z_;


  public:
    virtual void transmit( std::unique_ptr<oberon::core::Event> && event);

    template<class Period>
    inline Rcpp::List run(const Rcpp::DataFrame & events);

    std::shared_ptr<Column> column();


  private:
    double start_;

    double end_;

};
template<class Period>
inline Rcpp::List RDepthImage::run(const Rcpp::DataFrame & events) {
  using namespace std;
  using namespace std::chrono;
  using namespace date;
  using Duration = duration<int64_t, Period>;
  using Timestamp = time_point<system_clock, Duration>;
  Rcpp::CharacterVector maker_order_id = events["maker_order_id"];
  Rcpp::IntegerVector event_no = events["event_no"];
  Rcpp::NumericVector timestamp = events["timestamp"];
  Rcpp::IntegerVector state = events["state"];
  Rcpp::NumericVector price = events["price"];
  Rcpp::NumericVector volume = events["volume"];
  Rcpp::IntegerVector trade_id = events["state"];
  Rcpp::NumericVector delta_volume = events["delta_volume"];
  Rcpp::CharacterVector taker_order_id = events["taker_order_id"];
  Rcpp::NumericVector local_timestamp = events["local_timestamp"];
  Rcpp::LogicalVector is_deleted = events["is_deleted"];
  
  Timestamp current_timestamp =
      Timestamp(ceil<Duration>(duration<double>{timestamp[0]}));
  Timestamp start =
      Timestamp(ceil<Duration>(duration<double>{start_}));
  Timestamp end =
      Timestamp(ceil<Duration>(duration<double>{end_}));
  // std::cout << "Initial " << format("%FT%TZ\n", current_timestamp);
  for (size_t i = 0; i < maker_order_id.size(); ++i) {
    Timestamp next_timestamp =
        Timestamp(ceil<Duration>(duration<double>{timestamp[i]}));
    if (next_timestamp > current_timestamp) {
      // std::cout << i << "Next " << format("%FT%TZ", next_timestamp) << " current " << format("%FT%TZ\n", current_timestamp);
      auto col = column();
      while (next_timestamp > current_timestamp) {
        if (current_timestamp >= start && current_timestamp <= end) {
          z_.push_back(col);
          x_.push_back(duration_cast<duration<double>>(
                           current_timestamp.time_since_epoch())
                           .count());
        }
        current_timestamp += Duration {1};
        // std::cout << format("%FT%TZ\n", current_timestamp);
      }
    }
    boost::uuids::string_generator gen;
    boost::uuids::uuid taker;
    try {
      taker = gen(std::string{taker_order_id[i]});
    } catch (...) {
      taker = oberon::core::Event::kNaOrderId;
    }
    // moid is supposed to be used in gdb's breakpoint condition
    // std::string moid{maker_order_id[i]};
    // std::cout << "i=" << i << " " << moid << "\n";
    transmit(oberon::core::Event::create(
        gen(maker_order_id[i]),
        oberon::core::Timestamp(round<oberon::core::Timestamp::duration>(duration<double>(timestamp[i]))),
        oberon::core::Timestamp(round<oberon::core::Timestamp::duration>(duration<double>(local_timestamp[i]))),
        static_cast<oberon::core::EventNo>(event_no[i]),static_cast<oberon::core::Price>(price[i]),
        static_cast<oberon::core::Volume>(volume[i]), static_cast<oberon::core::Volume>(delta_volume[i]),
        static_cast<oberon::core::OrderState>(state[i]),
        static_cast<oberon::core::TradeId>(Rcpp::IntegerVector::is_na(trade_id[i])
                                 ? oberon::core::Event::kNaTradeId
                                 : trade_id[i]),
        taker, false));
  }
  z_.push_back(column());
  x_.push_back(
      duration_cast<duration<double>>(current_timestamp.time_since_epoch())
          .count());
  x_.push_back(duration_cast<duration<double>>(
                   (current_timestamp + Duration{1}).time_since_epoch())
                   .count());
  return toList();
}

class RStore : public oberon::core::Store {
  public:
    virtual void transmit( std::unique_ptr<oberon::core::Event> && event);


  private:
    vector<string> order_id_;

    vector<double> timestamp_;

    vector<int32_t> event_no_;

    vector<int32_t> state_;

    vector<double> price_;

    vector<oberon::core::Volume> volume_;

    vector<oberon::core::Volume> delta_volume_;

    vector<int32_t> trade_id_;

    vector<string> taker_order_id_;

    vector<double> local_timestamp_;

    vector<bool> is_deleted_;


  public:
    Rcpp::List toDataframe();


  private:
    vector<double> next_timestamp_;

    map<oberon::core::OrderId, std::size_t> previous_;

};

} // namespace oberon::R

} // namespace oberon
#endif


#include "rcommon.h"
#include <boost/uuid/string_generator.hpp>

#include <boost/uuid/uuid_io.hpp>


namespace oberon {

namespace R {

RDepthImage::RDepthImage(const oberon::core::Price & min_price, const oberon::core::Price & max_price, double tick_size, double start, double end): min_price_ {min_price.alignDown(tick_size)}, max_price_ {max_price.alignUp(tick_size)}, tick_size_ {tick_size}, start_{start}, end_{end} {
  for (double price = min_price_; price <= max_price_+tick_size_; price += tick_size_)
    y_.push_back(price);
}

Rcpp::List RDepthImage::toList() {
  // std::cout << "z.size() = " << z_.size() << " y_.size()=" << y_.size() << std::endl;
  // vector<double> z(z_.size() * y_.size());
  Rcpp::NumericVector z(z_.size() * y_.size());
  
  auto it = z.begin();
  for (auto i = 0; i < z_.size(); ++i)
    it = copy(z_[i]->begin(), z_[i]->end(), it);
  z.attr("dim") = Rcpp::Dimension(y_.size(), z_.size());
  return Rcpp::List::create(Rcpp::Named("x") = x_,Rcpp::Named("x_lab") = x_lab_, Rcpp::Named("y") = y_,
                            Rcpp::Named("z") = Rcpp::transpose(Rcpp::as<Rcpp::NumericMatrix>(z)));
}

void RDepthImage::transmit( std::unique_ptr<oberon::core::Event> && event) {
  // std::cout << event->toString();
  if (event->state()== oberon::core::OrderState::kActive) {
    // std::cout << " - saved\n";
    events_[event->orderId()] = std::move(event);
  }
  else {
    // std::cout << " - erased\n";
    events_.erase(event->orderId());
  }
}

std::shared_ptr<RDepthImage::Column> RDepthImage::column() {
  auto res = std::make_shared<Column>(y_.size());
  for (auto it = events_.begin(); it != events_.end(); ++it) {
    long loc;
    if (it->second->volume() > 0)
      loc = std::lround((it->second->price().alignDown(tick_size_) - min_price_) /
                        tick_size_);
    else
      loc = std::lround((it->second->price().alignUp(tick_size_) - min_price_) /
                        tick_size_);
  
    if (loc >= 0 && loc < y_.size())
      (*res)[loc] += it->second->volume();
  }
  /*res->at(0) = 2;
  res->at(y_.size()/4) = 1;
  res->at(y_.size()*3/4) = -1;
  res->at(y_.size()-1) = -2;
  */
  return res;
}

void RStore::transmit( std::unique_ptr<oberon::core::Event> && event) {
  using namespace oberon::core;
  using std::chrono::duration_cast;
  using std::chrono::duration;
  using boost::uuids::to_string;
  if (event) {
    if (event->orderId() != Event::kNaOrderId)
      if (event->orderId() != boost::uuids::nil_uuid())
        order_id_.push_back(to_string(event->orderId()));
      else
        order_id_.push_back("");
    else
      order_id_.push_back("NA");
    timestamp_.push_back(duration_cast<duration<double>>(event->timestamp().time_since_epoch()).count());
    try {
      next_timestamp_[previous_.at(event->orderId())] = timestamp_.back();
    } catch (const std::out_of_range &) {
    }
    if (event->state() != OrderState::kFinished) {
      next_timestamp_.push_back(31556984400.0);
      previous_[event->orderId()] = timestamp_.size() - 1;
    }
    else {
      next_timestamp_.push_back(timestamp_.back());
      previous_.erase(event->orderId());
    }
    local_timestamp_.push_back(duration_cast<duration<double>>(event->localTimestamp().time_since_epoch()).count());
    if (event->eventNo() != Event::kNaEventNo)
      event_no_.push_back(event->eventNo());
    else
      event_no_.push_back(NA_INTEGER);
    price_.push_back(event->price());
    volume_.push_back(event->volume());
    delta_volume_.push_back(event->deltaVolume());
    is_deleted_.push_back(event->isDeleted());
    if (event->tradeId() != Event::kNaTradeId)
      trade_id_.push_back(event->tradeId());
    else
      trade_id_.push_back(NA_INTEGER);
    if (event->takerOrderId() != Event::kNaOrderId)
      if (event->takerOrderId() != boost::uuids::nil_uuid())
        taker_order_id_.push_back(to_string(event->takerOrderId()));
      else
        taker_order_id_.push_back("");
    else
      taker_order_id_.push_back("NA");
    state_.push_back(event->state());
  }
}

Rcpp::List RStore::toDataframe() {
  Rcpp::List res = Rcpp::List::create(
      Rcpp::Named("maker_order_id") = order_id_,
      Rcpp::Named("event_no") = event_no_, Rcpp::Named("timestamp") = timestamp_,
      Rcpp::Named("state") = state_, Rcpp::Named("price") = price_,
      Rcpp::Named("volume") = volume_, Rcpp::Named("next_timestamp") = next_timestamp_, Rcpp::Named("trade_id") = trade_id_,
      Rcpp::Named("delta_volume") = delta_volume_,
      Rcpp::Named("taker_order_id") = taker_order_id_,
      Rcpp::Named("local_timestamp") = local_timestamp_,
      Rcpp::Named("is_deleted") = is_deleted_);
  res.attr("class") = Rcpp::CharacterVector::create("data.table", "data.frame");
  return res;
}


} // namespace oberon::R

} // namespace oberon

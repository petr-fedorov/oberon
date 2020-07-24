#include "rcoinbase.h"
#include "rbitstamp.h"
#include "rbitfinex.h"
#include "rcommon.h"
#include <Rcpp.h>
#include <iostream>
#include <chrono>
#include <ratio>

using namespace Rcpp;
// [[Rcpp::export]]
List reconstructCoinbase(DataFrame source, LogicalVector extract_only) {

  // std::streambuf *coutbuf = std::cout.rdbuf();
  std::cout.rdbuf(Rcpp::Rcout.rdbuf());
  oberon::R::RCoinbase r;
  return r.run(source, extract_only[0]);
}

// [[Rcpp::export]]
List reconstructBitstamp(DataFrame quotes, DataFrame trades, LogicalVector extract_only) {

  // std::streambuf *coutbuf = std::cout.rdbuf();
  std::cout.rdbuf(Rcpp::Rcout.rdbuf());
  oberon::R::RBitstamp r;
  return r.run(quotes, trades, extract_only[0]);
}

// [[Rcpp::export]]
List reconstructBitfinex(DataFrame quotes, DataFrame trades, LogicalVector extract_only) {

  // std::streambuf *coutbuf = std::cout.rdbuf();
  std::cout.rdbuf(Rcpp::Rcout.rdbuf());
  oberon::R::RBitfinex r;
  return r.run(quotes, trades, extract_only[0]);
}

// [[Rcpp::export]]
List events2image(DataFrame events, NumericVector min_price, NumericVector max_price, NumericVector tick_size, NumericVector start, NumericVector end, CharacterVector sampling_period) {

  // std::streambuf *coutbuf = std::cout.rdbuf();
  std::cout.rdbuf(Rcpp::Rcout.rdbuf());
  oberon::R::RDepthImage r {min_price[0], max_price[0], tick_size[0], start[0], end[0]};
  return r.run<std::deci>(events);
}

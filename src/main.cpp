#include "rcoinbase.h"
#include "rbitstamp.h"
#include <Rcpp.h>
#include <iostream>

using namespace Rcpp;
// [[Rcpp::export]]
DataFrame reconstructCoinbase(DataFrame source, LogicalVector extract_only) {

  // std::streambuf *coutbuf = std::cout.rdbuf();
  std::cout.rdbuf(Rcpp::Rcout.rdbuf());
  oberon::R::RCoinbase r;
  return r.run(source, extract_only[0]);
}

// [[Rcpp::export]]
DataFrame reconstructBitstamp(DataFrame quotes, DataFrame trades, LogicalVector extract_only) {

  // std::streambuf *coutbuf = std::cout.rdbuf();
  std::cout.rdbuf(Rcpp::Rcout.rdbuf());
  oberon::R::RBitstamp r;
  return r.run(quotes, trades, extract_only[0]);
}

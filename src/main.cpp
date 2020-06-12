#include "rcoinbase.h"
#include <Rcpp.h>
#include <iostream>

using namespace Rcpp;
// [[Rcpp::export]]
DataFrame reconstructOrderBook(DataFrame source, LogicalVector extract_only) {

  // std::streambuf *coutbuf = std::cout.rdbuf();
  std::cout.rdbuf(Rcpp::Rcout.rdbuf());
  oberon::R::RCoinbase r;
  return r.run(source, extract_only[0]);
}

// Generated by using Rcpp::compileAttributes() -> do not edit by hand
// Generator token: 10BE3573-1514-4C36-9D1C-5A225CD40393

#include <Rcpp.h>

using namespace Rcpp;

// reconstructOrderBook
DataFrame reconstructOrderBook(DataFrame source, LogicalVector extract_only);
RcppExport SEXP _oberon_reconstructOrderBook(SEXP sourceSEXP, SEXP extract_onlySEXP) {
BEGIN_RCPP
    Rcpp::RObject rcpp_result_gen;
    Rcpp::RNGScope rcpp_rngScope_gen;
    Rcpp::traits::input_parameter< DataFrame >::type source(sourceSEXP);
    Rcpp::traits::input_parameter< LogicalVector >::type extract_only(extract_onlySEXP);
    rcpp_result_gen = Rcpp::wrap(reconstructOrderBook(source, extract_only));
    return rcpp_result_gen;
END_RCPP
}

static const R_CallMethodDef CallEntries[] = {
    {"_oberon_reconstructOrderBook", (DL_FUNC) &_oberon_reconstructOrderBook, 2},
    {NULL, NULL, 0}
};

RcppExport void R_init_oberon(DllInfo *dll) {
    R_registerRoutines(dll, NULL, CallEntries, NULL, NULL);
    R_useDynamicSymbols(dll, FALSE);
}

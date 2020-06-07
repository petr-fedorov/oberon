#' @useDynLib oberon, .registration = TRUE
#' @import data.table

.dummy <- function() {}

#' @export
reconstruct <- function(source) {
  output <- reconstructOrderBook(source)
  setDT(output)
  output[, timestamp := as.POSIXct(timestamp, origin="1970-01-01")]
  output
}


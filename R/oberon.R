#' @useDynLib oberon, .registration = TRUE
#' @import data.table

.dummy <- function() {}

#' @export
reconstruct <- function(source, extract.only=FALSE) {
  output <- reconstructOrderBook(source, extract.only)
  setDT(output)
  output[, c("timestamp", "local_timestamp") := .(as.POSIXct(timestamp, origin="1970-01-01"), as.POSIXct(local_timestamp, origin="1970-01-01"))]
  output
}


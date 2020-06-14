#' @useDynLib oberon, .registration = TRUE
#' @import data.table

.dummy <- function() {}

#' @export
reconstruct <- function(exchange=c("Coinbase", "Bitstamp"), source1, source2=NULL,  extract.only=FALSE) {
  exchange <- match.arg(exchange)
  output <- switch(exchange,
                   Coinbase=reconstructCoinbase(source1, extract.only),
                   Bitstamp=reconstructBitstamp(source1, source2, extract.only))
  setDT(output)
  output[, c("timestamp", "local_timestamp") := .(as.POSIXct(timestamp, origin="1970-01-01"), as.POSIXct(local_timestamp, origin="1970-01-01"))]
  output
}

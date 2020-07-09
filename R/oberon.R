#' @useDynLib oberon, .registration = TRUE
#' @import data.table

.dummy <- function() {}

#' @export
reconstruct <- function(exchange=c("Coinbase", "Bitstamp", "Bitfinex"), source1, source2=NULL,  extract.only=FALSE) {
  exchange <- match.arg(exchange)
  output <- switch(exchange,
                   Coinbase=reconstructCoinbase(source1, extract.only),
                   Bitstamp=reconstructBitstamp(source1[order(microtimestamp, local_timestamp), ], source2[order(trade_timestamp, local_timestamp), ], extract.only),
                   Bitfinex=reconstructBitfinex(source1, source2, extract.only))
  setDT(output)
  output[, c("timestamp", "local_timestamp") := .(as.POSIXct(timestamp, origin="1970-01-01"), as.POSIXct(local_timestamp, origin="1970-01-01"))]
  output[maker_order_id == "NA", maker_order_id := NA_character_]
  output[taker_order_id == "NA", taker_order_id := NA_character_]
  output
}

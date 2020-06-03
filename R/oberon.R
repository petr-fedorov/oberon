#' @useDynLib oberon, .registration = TRUE

.dummy <- function() {}

#' @export
reconstruct <- function(source) {
  reconstructOrderBook(source)
}


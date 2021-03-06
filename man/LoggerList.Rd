% Generated by roxygen2: do not edit by hand
% Please edit documentation in R/RcppExports.R
\name{LoggerList}
\alias{LoggerList}
\title{Logger list class to collect all loggers}
\format{\code{\link{S4}} object.}
\description{
This class is meant to define all logger which should be used to track the
progress of the algorithm.
}
\section{Usage}{

\preformatted{
LoggerList$new()
}
}

\section{Details}{


  This class is a wrapper around the pure \code{C++} implementation. To see
  the functionality of the \code{C++} class visit
  \url{https://schalkdaniel.github.io/compboost/cpp_man/html/classloggerlist_1_1_logger_list.html}.
}

\section{Fields}{

  This class doesn't contain public fields.
}

\section{Methods}{

\describe{
\item{\code{clearRegisteredLogger()}}{Removes all registered logger
  from the list. The used logger are not deleted, just removed from the
  map.}
\item{\code{getNamesOfRegisteredLogger()}}{Returns the registered logger
  names as character vector.}
\item{\code{getNumberOfRegisteredLogger()}}{Returns the number of registered
  logger as integer.}
\item{\code{printRegisteredLogger()}}{Prints all registered logger.}
\item{\code{registerLogger(logger)}}{Includes a new \code{logger}
  into the logger list with the \code{logger.id} as key.}
}
}

\examples{
# Define logger:
log.iters = LoggerIteration$new("iteration", TRUE, 100)
log.time = LoggerTime$new("time", FALSE, 20, "minutes")

# Create logger list:
logger.list = LoggerList$new()

# Register new loggeR:
logger.list$registerLogger(log.iters)
logger.list$registerLogger(log.time)

# Print registered logger:
logger.list$printRegisteredLogger()

# Remove all logger:
logger.list$clearRegisteredLogger()

# Get number of registered logger:
logger.list$getNumberOfRegisteredLogger()

}

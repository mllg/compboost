% Generated by roxygen2: do not edit by hand
% Please edit documentation in R/RcppExports.R
\name{LoggerInbagRisk}
\alias{LoggerInbagRisk}
\title{Logger class to log the inbag risk}
\format{\code{\link{S4}} object.}
\description{
This class logs the inbag risk for a specific loss function. It is also
possible to use custom losses to log performance measures. For details
see the use case or extending compboost vignette.
}
\section{Usage}{

\preformatted{
LoggerInbagRisk$new(logger_id, use_as_stopper, used_loss, eps_for_break)
}
}

\section{Arguments}{

\describe{
\item{\code{logger_id} [\code{character(1)}]}{
  Unique identifier of the logger.
}
\item{\code{use_as_stopper} [\code{logical(1)}]}{
  Boolean to indicate if the logger should also be used as stopper.
}
\item{\code{used_loss} [\code{Loss} object]}{
  The loss used to calculate the empirical risk by taking the mean of the
  returned defined loss within the loss object.
}
\item{\code{eps_for_break} [\code{numeric(1)}]}{
  This argument is used if the loss is also used as stopper. If the relative
  improvement of the logged inbag risk falls above this boundary the stopper
  returns \code{TRUE}.
}
}
}

\section{Details}{


This logger computes the risk for the given training data
\eqn{\mathcal{D} = \{(x^{(i)},\ y^{(i)})\ |\ i \in \{1, \dots, n\}\}}
and stores it into a vector. The empirical risk \eqn{\mathcal{R}} for
iteration \eqn{m} is calculated by:
\deqn{
  \mathcal{R}_\mathrm{emp}^{[m]} = \frac{1}{n}\sum\limits_{i = 1}^n L(y^{(i)}, \hat{f}^{[m]}(x^{(i)}))
}

\strong{Note:}
\itemize{
  \item
    If \eqn{m=0} than \eqn{\hat{f}} is just the offset.

  \item
    The implementation to calculate \eqn{\mathcal{R}_\mathrm{emp}^{[m]}} is
    done in two steps:
      \enumerate{
       \item
         Calculate vector \code{risk_temp} of losses for every observation for
         given response \eqn{y^{(i)}} and prediction \eqn{\hat{f}^{[m]}(x^{(i)})}.

       \item
         Average over \code{risk_temp}.
     }
   }
   This procedure ensures, that it is possible to e.g. use the AUC or any
   arbitrary performance measure for risk logging. This gives just one
   value for \code{risk_temp} and therefore the average equals the loss
   function. If this is just a value (like for the AUC) then the value is
   returned.

  This class is a wrapper around the pure \code{C++} implementation. To see
  the functionality of the \code{C++} class visit
  \url{https://schalkdaniel.github.io/compboost/cpp_man/html/classlogger_1_1_inbag_risk_logger.html}.
}

\section{Fields}{

  This class doesn't contain public fields.
}

\section{Methods}{

\describe{
  \item{\code{summarizeLogger()}}{Summarize the logger object.}
}
}

\examples{
# Used loss:
log.bin = LossBinomial$new()

# Define logger:
log.inbag.risk = LoggerInbagRisk$new("inbag", FALSE, log.bin, 0.05)

# Summarize logger:
log.inbag.risk$summarizeLogger()

}

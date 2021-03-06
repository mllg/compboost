% Generated by roxygen2: do not edit by hand
% Please edit documentation in R/RcppExports.R
\name{Compboost_internal}
\alias{Compboost_internal}
\title{Main Compboost Class}
\format{\code{\link{S4}} object.}
\description{
This class collects all parts such as the factory list or the used logger
and passes them to \code{C++}. On the \code{C++} side is then the main
algorithm.
}
\section{Usage}{

\preformatted{
Compboost$new(response, learning_rate, stop_if_all_stopper_fulfilled,
  factory_list, loss, logger_list, optimizer)
}
}

\section{Arguments}{

\describe{
\item{\code{response} [\code{numeric}]}{
  Vector of the true values which should be modeled.
}
\item{\code{learning_rate} [\code{numeric(1)}]}{
  The learning rate which is used to shrink the parameter in each iteration.
}
\item{\code{stop_if_all_stopper_fulfilled} [\code{logical(1)}]}{
  Boolean to indicate which stopping strategy is used. If \code{TRUE} then
  the algorithm stops if all registered logger stopper are fulfilled.
}
\item{\code{factory_list} [\code{BlearnerFactoryList} object]}{
  List of base-learner factories from which one base-learner is selected
  in each iteration by using the
}
\item{\code{loss} [\code{Loss} object]}{
  The loss which should be used to calculate the pseudo residuals in each
  iteration.
}
\item{\code{logger_list} [\code{LoggerList} object]}{
  The list with all registered logger which are used to track the algorithm.
}
\item{\code{optimizer} [\code{Optimizer} object]}{
  The optimizer which is used to select in each iteration one good
  base-learner.
}
}
}

\section{Details}{


  This class is a wrapper around the pure \code{C++} implementation. To see
  the functionality of the \code{C++} class visit
  \url{https://schalkdaniel.github.io/compboost/cpp_man/html/classcboost_1_1_compboost.html}.
}

\section{Fields}{

  This class doesn't contain public fields.
}

\section{Methods}{

\describe{
\item{\code{train(trace)}}{Initial training of the model. The integer
  argument \code{trace} indicates if the logger progress should be printed
  or not and if so trace indicates which iterations should be printed.}
\item{\code{continueTraining(trace, logger_list)}}{Continue the training
  by using an additional \code{logger_list}. The retraining is stopped if
  the first logger says that the algorithm should be stopped.}
\item{\code{getPrediction()}}{Get the inbag prediction which is done during
  the fitting process.}
\item{\code{getSelectedBaselearner()}}{Returns a character vector of how
  the base-learner are selected.}
\item{\code{getLoggerData()}}{Returns a list of all logged data. If the
  algorithm is retrained, then the list contains for each training one
  element.}
\item{\code{getEstimatedParameter()}}{Returns a list with the estimated
  parameter for base-learner which was selected at least once.}
\item{\code{getParameterAtIteration(k)}}{Calculates the prediction at the
  iteration \code{k}.}
\item{\code{getParameterMatrix()}}{Calculates a matrix where row \code{i}
  includes the parameter at iteration \code{i}. There are as many rows
  as done iterations.}
\item{\code{isTrained()}}{This function returns just a boolean value which
  indicates if the initial training was already done.}
\item{\code{predict(newdata)}}{Prediction on new data organized within a
  list of source data objects. It is important that the names of the source
  data objects matches those one that were used to define the factories.}
\item{\code{predictAtIteration(newdata, k)}}{Prediction on new data by using
  another iteration \code{k}.}
\item{\code{setToIteration(k)}}{Set the whole model to another iteration
  \code{k}. After calling this function all other elements such as the
  parameters or the prediction are calculated corresponding to \code{k}.}
\item{\code{summarizeCompboost()}}{Summarize the \code{Compboost} object.}
}
}

\examples{

# Some data:
df = mtcars
df$mpg.cat = ifelse(df$mpg > 20, 1, -1)

# # Create new variable to check the polynomial base-learner with degree 2:
# df$hp2 = df[["hp"]]^2

# Data for the baselearner are matrices:
X.hp = as.matrix(df[["hp"]])
X.wt = as.matrix(df[["wt"]])

# Target variable:
y = df[["mpg.cat"]]

data.source.hp = InMemoryData$new(X.hp, "hp")
data.source.wt = InMemoryData$new(X.wt, "wt")

data.target.hp1 = InMemoryData$new()
data.target.hp2 = InMemoryData$new()
data.target.wt1 = InMemoryData$new()
data.target.wt2 = InMemoryData$new()

# List for oob logging:
oob.data = list(data.source.hp, data.source.wt)

# List to test prediction on newdata:
test.data = oob.data

# Factories:
linear.factory.hp = BaselearnerPolynomial$new(data.source.hp, data.target.hp1, 
  list(degree = 1, intercept = TRUE))
linear.factory.wt = BaselearnerPolynomial$new(data.source.wt, data.target.wt1, 
  list(degree = 1, intercept = TRUE))
quadratic.factory.hp = BaselearnerPolynomial$new(data.source.hp, data.target.hp2, 
  list(degree = 2, intercept = TRUE))
spline.factory.wt = BaselearnerPSpline$new(data.source.wt, data.target.wt2, 
  list(degree = 3, n.knots = 10, penalty = 2, differences = 2))

# Create new factory list:
factory.list = BlearnerFactoryList$new()

# Register factories:
factory.list$registerFactory(linear.factory.hp)
factory.list$registerFactory(linear.factory.wt)
factory.list$registerFactory(quadratic.factory.hp)
factory.list$registerFactory(spline.factory.wt)

# Define loss:
loss.bin = LossBinomial$new()

# Define optimizer:
optimizer = OptimizerCoordinateDescent$new()

## Logger

# Define logger. We want just the iterations as stopper but also track the
# time, inbag risk and oob risk:
log.iterations  = LoggerIteration$new(" iteration.logger", TRUE, 500)
log.time        = LoggerTime$new("time.logger", FALSE, 500, "microseconds")
log.inbag       = LoggerInbagRisk$new("inbag.binomial", FALSE, loss.bin, 0.05)
log.oob         = LoggerOobRisk$new("oob.binomial", FALSE, loss.bin, 0.05, oob.data, y)

# Define new logger list:
logger.list = LoggerList$new()

# Register the logger:
logger.list$registerLogger(log.iterations)
logger.list$registerLogger(log.time)
logger.list$registerLogger(log.inbag)
logger.list$registerLogger(log.oob)

# Run compboost:
# --------------

# Initialize object:
cboost = Compboost_internal$new(
  response      = y,
  learning_rate = 0.05,
  stop_if_all_stopper_fulfilled = FALSE,
  factory_list = factory.list,
  loss         = loss.bin,
  logger_list  = logger.list,
  optimizer    = optimizer
)

# Train the model (we want to print the trace):
cboost$train(trace = 50)
cboost

# Get estimated parameter:
cboost$getEstimatedParameter()

# Get trace of selected base-learner:
cboost$getSelectedBaselearner()

# Set to iteration 200:
cboost$setToIteration(200)

# Get new parameter values:
cboost$getEstimatedParameter()

}

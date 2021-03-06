# ============================================================================ #
#                                                                              #
#                   Benchmarking Compboost vs. Mboost                          #
#                                                                              #
# ============================================================================ #

# Setup:
# ---------------------------------------------------

source("benchmark/memory/defs.R")
source("benchmark/memory/algorithms.R")

bm.dir = "benchmark/memory/benchmark_files"

# Create frame for Benchmark:
# ---------------------------------------------------

if (my.setting$overwrite || (! dir.exists(bm.dir))) {
  if (dir.exists(bm.dir)) { 
    unlink(bm.dir, recursive = TRUE) 
  } else {
    regis = makeExperimentRegistry(
      file.dir = bm.dir,
      packages = my.setting$packages,
      seed     = round(1000 * pi)
    )
  }
} else {
  regis = loadRegistry(bm.dir)
}

# Define data and algorithm for benchmark:
# ---------------------------------------------------

# Function to simulate data:

# data is static part which remains the same while arguments in fun are
# dynamic and can be changed in addExperiment
addProblem(name = "my.data", data = list(noise = 0), fun = function (data, job, n, p) {

  noise = data$noise
  vars = p + noise

  #create beta distributed correlations
  corrs = rbeta(n = (vars * (vars - 1))/2, shape1 = 1, shape2 = 8)
  corrs = sample(c(-1, 1), size = length(corrs), replace = TRUE) * corrs

  sigma = matrix(1, nrow = vars, ncol = vars)
  sigma[upper.tri(sigma)] = corrs
  sigma[lower.tri(sigma)] = t(sigma)[lower.tri(sigma)]

  data = as.data.frame(rmvnorm(n = n, sigma = sigma, method = "svd"))

  betas = runif(p + 1, min = -2, max = 2)
  data$y = rnorm(n = n, mean = as.matrix(cbind(1, data[,1:p])) %*% betas)

  #return (list(data = data, betas = betas))
  return (list(data = data))
})


# Function including the algorithm, depending on the parameter for the
# benchmark:

# instance are the object generated by addProblem, data is again the same
# as in addProblem
addAlgorithm("compboost", fun = benchmarkCompboost)
addAlgorithm("mboost.fast", fun = benchmarkMboostFast)
addAlgorithm("mboost", fun = benchmarkMboost)

# Increase number dimension of data:
# ----------------------------------
addExperiments(
  # Registry file:
  reg = regis,

  # Test different data sizes:
  prob.design = list(
    my.data = expand.grid(
      n = c(500, 5000, 50000),
      p = c(100, 2000)
    )
  ),

  # Fix number of iterations:
  algo.designs = list(
    compboost = data.frame(
      iters   = c(1000, 10000),
      learner = c("spline", "linear"),
      stringsAsFactors = FALSE
    ),
    mboost = data.frame(
      iters   = c(1000, 10000),
      learner = c("spline", "linear"),
      stringsAsFactors = FALSE
    ),
    mboost.fast = data.frame(
      iters   = c(1000, 10000),
      learner = c("spline", "linear"),
      stringsAsFactors = FALSE
    )
  ),
  # Number of replications:
  repls = 1
)

submitJobs(findNotDone())
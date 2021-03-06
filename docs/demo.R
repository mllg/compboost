library(compboost)


# Quickstart with wrapper functions
# --------------------------------------------

cboost = boostSplines(data = iris, target = "Petal.Length", loss = LossQuadratic$new())
cboost

cboost = boostSplines(data = iris, target = "Petal.Length", loss = LossQuadratic$new(),
  n.knots = 4, penalty = 4)


# Explicitely defining elements
# --------------------------------------------

# Define Compboost object:
cboost = Compboost$new(data = iris, target = "Petal.Length", 
  loss = LossQuadratic$new())
cboost

# Add base-learner:
cboost$addBaselearner(feature = "Petal.Width", id = "spline", bl.factory = BaselearnerPSpline, 
  degree = 3, n.knots = 10, penalty = 2, differences = 2)

cboost$addBaselearner(feature = c("Sepal.Length", "Sepal.Width"), id = "2dim_linear", 
  bl.factory = BaselearnerPolynomial, degree = 1, intercept = TRUE)

cboost$addBaselearner(feature = "Species", id = "categorical", bl.factory = BaselearnerPolynomial, 
  degree = 1, intercept = FALSE)

# Print registered base-learner:
cboost$getBaselearnerNames()

# Train 1000 iterations:
cboost$train(1000)


# Acess elements of a fitted model
# --------------------------------------------

# Get vector of selected base-learner
selected = cboost$getSelectedBaselearner()
selected[1:10]

table(selected)

# Get vector of inbag risk:
risk = cboost$getInbagRisk()
risk[1:10]

# Train 500 additional iterations:
cboost$train(1500)

# Visualize feature:
gg1 = cboost$plot("Petal.Width_spline")
gg1

library(ggplot2)
library(ggthemes)

gg2 = cboost$plot("Petal.Width_spline", iters = c(5, 100, 500, 1000, 1500)) +
  labs(title = "Effect of Petal Width", subtitle = "Additive contribution to linear predictor") +
  theme_tufte() + 
  scale_color_brewer(palette = "Spectral")
gg2

ggsave(filename = "plot1.png", plot = gg1, width = 7, height = 4.5)
ggsave(filename = "plot2.png", plot = gg2, width = 7, height = 4.5)

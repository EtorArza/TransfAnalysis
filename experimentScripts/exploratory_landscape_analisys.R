# # https://github.com/kerschke/flacco
# # Run first time:
# install.packages("flacco", dependencies = TRUE)

library(flacco)


get_features_problem <- function(problem_index) {


  f_continuous <- function(x) {
    x_str <- paste(x, collapse = ",")
    cmd <- paste(c("./main.out -evaluate-continuous-problem", problem_index, x_str), collapse = " ")
    res <- system(cmd, intern = TRUE)
    return(as.double(res))
  }



  ## (1) Create some example-data
  X = createInitialSample(n.obs = 500, dim = 20)
  y = apply(X, 1, f_continuous)

  ## (2) Compute the feature object
  feat.object = createFeatureObject(X = X, y = y)

  # ## (3) Have a look at feat.object
  # print(feat.object)

  # ## (4) Check, which feature sets are available
  # listAvailableFeatureSets()

  ## (5) Calculate a specific feature set, e.g. the ELA meta model
  featureSet = calculateFeatureSet(feat.object, set = "ela_meta")

  return(as.double(featureSet))

}


for (i in 1:12) {
  print(paste("Working on problem",i))
  writeLines(paste(formatC(get_features_problem(i), format = "f", digits = 36), collapse = ","), paste("experimentResults/problem_analisys/exploratory_landscape_analisys/ELA_continuous_",i,".txt", collapse=""))
}






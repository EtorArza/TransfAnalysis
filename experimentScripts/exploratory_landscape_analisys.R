# # https://github.com/kerschke/flacco
# # Run first time:
# install.packages("flacco", dependencies = TRUE)

library(flacco)


get_features_problem <- function(objective_function) {

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

# 12 continuous problems
lines = c()
for (i in 1:12) {
  f_continuous <- function(x) {
    x_str <- paste(x, collapse = ",")
    cmd <- paste(c("./main.out -evaluate-continuous-problem", i, x_str), collapse = " ")
    res <- system(cmd, intern = TRUE)
    return(as.double(res))
  }

  print(paste("Working on problem",i))
  lines = c(lines,paste(paste("_",i,"_,",sep="",collapse=""),paste(formatC(get_features_problem(i), format = "f", digits = 36), collapse = ",", sep=",")))
}
writeLines(lines, "experimentResults/problem_analisys/continuous12_ELA.txt")


# generated continuous problems

updateParameterInFile <- function(filePath, newNLO) {
  fileContent <- readLines(filePath)
  fileContent[7] <- as.character(newNLO)
  writeLines(fileContent, filePath)
}


lines = c()
for (nlo in c(1,4,8,16,32,64)) {
  f_continuous <- function(x) {

    updateParameterInFile("src/experiments/real/real_func_src/jani_ronkkonen_problem_generator/quad_function.dat", nlo)


    x_str <- paste(x, collapse = ",")

    cmd <- paste(c("./main.out -evaluate-continuous-problem", 0, x_str), collapse = " ")
    res <- system(cmd, intern = TRUE)
    return(as.double(res))
  }

  print(paste("Working on generated rokonen problem with NLO=",nlo))
  lines = c(lines,paste(paste("NLO_",nlo,",",sep="",collapse=""),paste(formatC(get_features_problem(i), format = "f", digits = 36), collapse = ",", sep=",")))
}
writeLines(lines, "experimentResults/problem_analisys/rokkonen_ELA.txt")




cmake_minimum_required(VERSION 3.9)
project(algorithm)

set(CMAKE_CXX_STANDARD 11)

link_directories(/Users/kosuke/Library/R/3.3/library/Rcpp/libs)
add_executable(lda main.cpp lda_base.cpp lda_base.h lda_bernoulli.cpp lda_bernoulli.h)

include_directories(/Users/kosuke/Library/R/3.3/library/Rcpp/include)
include_directories(/Library/Frameworks/R.framework/Versions/3.3/Resources/include)
include_directories(/Library/Frameworks/R.framework/Versions/3.3/Resources/library/RcppArmadillo/include)
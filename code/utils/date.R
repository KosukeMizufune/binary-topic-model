library(stringr)

get_monthly <- function(data){
  month <- c("January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December")
    if (nchar(str_which(data, month)) == 1){
        return (str_c(str_sub(data, start=-4), '-0',  na.omit(as.character(str_which(data, month))), '-01'))
    }
    else{
        return (str_c(str_sub(data, start=-4), '-',  na.omit(as.character(str_which(data, month))), '-01'))
    }
}


get_year <- function(data){
    return (str_c(str_sub(data, start = -4), "-01-01"))
}

library(lda)
library(ggplot2)


loglik_LDA <- function(data.lda, data.vocab, K, iter=2000, alpha=0.1, eta=0.1){
	set.seed(123)
	result <- lda.collapsed.gibbs.sampler(data.lda,
                                       K,
                                       data.vocab,
                                       iter,  # 繰り返し数
                                       alpha, # ディリクレ過程のハイパーパラメータα
                                       eta, # ディリクレ過程のハイパーパラメータη
                                       compute.log.likelihood=TRUE)
	loglik = length(result$log.likelihood[2,(iter/2+1):iter]) / sum(1/result$log.likelihood[2,(iter/2+1):iter])
	return(loglik)
}


gg_loglik <- function(result, K){
	p = ggplot(data.frame(result), aes(x = K, y = result))
	p = p + geom_line()
	p = p + xlab("トピック数") + ylab("周辺尤度")
	p = p + theme_bw()
    p <- p + theme(text = element_text(family = "YuGo-Medium"),
            plot.title = element_text(size = 20, face = "bold", hjust = 0.5),
            axis.title.x = element_text(size = 12),
            axis.title.y = element_text(size = 12),
            axis.text.x = element_text(size = 12),
            axis.text.y = element_text(size = 12))
   p
	return(p)
}


get_topic_param <- function(doc_topic) {
	D <- ncol(doc_topic)
	K <- nrow(doc_topic)
	theta <- matrix(0, K, D)
	for(d in 1:D){
		for(k in 1:K){
			theta[k, d] <- (doc_topic[k, d] + 0.1) / (sum(doc_topic[,d]) + K * 0.1)
		}
	}
	return(theta)
}

wide2long_with_index <- function(mcmc, num_iter){
    gg_mcmc <- data.frame(mcmc) %>% tidyr::gather(key, value)
    gg_mcmc_index <- data.frame(index=rep(1:num_iter), gg_mcmc)
    return (gg_mcmc_index)
}

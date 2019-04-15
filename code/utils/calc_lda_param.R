calc_phi <- function(corpus, topic, num_topics, num_vocab, alpha){
    Nkv <- matrix(0, num_topics, num_vocab)
    for (n in 1:nrow(topic)){
        word_index  <- corpus[n, 1]
        topic_index <- (1:num_topics)[max(topic[n, ]) == topic[n, ]][1]

        Nkv[topic_index, word_index] <- Nkv[topic_index, word_index] + 1
    }
    Nk <- rowSums(Nkv)
    Est_Ph <- (Nkv + alpha) /
        (matrix(rep(Nk, num_vocab), num_topics, num_vocab) + num_vocab * alpha)
    return (Est_Ph)
}

calc_theta <- function(corpus, topic, num_topics, num_doc, beta){
    Ndk <- matrix(0, num_doc, num_topics)
    for (n in 1:nrow(topic)){
        doc_index   <- corpus[n, 2]
        topic_index <- (1:num_topics)[max(topic[n, ]) == topic[n, ]][1]

        Ndk[doc_index, topic_index] <- Ndk[doc_index, topic_index] + 1
    }
    Nd <- rowSums(Ndk)
    Est_Th <- (Ndk + beta) /
        (matrix(rep(Nd, num_topics), num_doc, num_topics) + num_topics * beta)
    return (Est_Th)
}

calc_psi <- function(corpus, topic, num_topics, gamma){
    Nkg <- rep(0, num_topics)
    Nk  <- rep(0, num_topics)
    for (n in 1:nrow(topic)){
        tag_index   <- corpus[n, 4]
        topic_index <- (1:num_topics)[max(topic[n, ]) == topic[n, ]][1]

        if (tag_index == 1){
            Nkg[topic_index] <- Nkg[topic_index] + 1
        }

        Nk[topic_index] <- Nk[topic_index] + 1
    }
    Est_Gh <- (Nkg + gamma) / (Nk + 2 * gamma)
    return (Est_Gh)
}

est_z <- function(topic, num_topics){
    Est_Z <- numeric(nrow(topic))
    for (n in 1:nrow(topic)){
        topic_index <- (1:num_topics)[max(topic[n, ]) == topic[n, ]][1]
        Est_Z[n] <- topic_index
    }
    return (Est_Z)
}

calc_topwords <- function(Est_Ph, vocab, num_topics, num_topwords){
    colnames(Est_Ph) <- vocab
    topwords <- matrix(0, num_topwords, num_topics)
    for (k in 1:num_topics){
        topwords[, k] <-  names(sort(Est_Ph[k, ], decreasing = T)[1:num_topwords])
    }
    return (topwords)
}

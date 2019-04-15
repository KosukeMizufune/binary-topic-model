hold_out_deter <- function(word_index, doc_index, Est_phi, Est_psi){
    tilde_psi      <- sapply(word_index, choose_tilde_psi, Est_phi=Est_phi, Est_psi=Est_psi)
    sentiment      <- pred_y(doc_index, tilde_psi)
    return (sentiment)
}

hold_out_prob <- function(word_index, doc_index, Est_phi, Est_psi){
    tilde_psi      <- sapply(word_index, calc_tilde_psi, Est_phi=Est_phi, Est_psi=Est_psi)
    sentiment      <- pred_y(doc_index, tilde_psi)
    return (sentiment)
}


pred_y <- function(doc_index, tilde_psi){
    min_d <- min(doc_index)
    max_d <- max(doc_index)
    y_d   <- sapply(min_d:max_d, pred_y_iter, doc_index=doc_index, tilde_psi=tilde_psi)
    return (y_d)
}


pred_y_iter <- function(d, doc_index, tilde_psi){
    tilde_psi_d      <- tilde_psi[doc_index == d]
    mean_tilde_psi_d <- mean(tilde_psi_d)
    y_d              <- classify(mean_tilde_psi_d)
    return (y_d)
}


choose_tilde_psi <- function(word_index, Est_phi, Est_psi){
    phi_v <- Est_phi[, word_index]
    phi_v_regular <- phi_v / sum(phi_v)
    return (Est_psi[phi_v_regular == max(phi_v_regular)][1])
}


calc_tilde_psi <- function(word_index, Est_phi, Est_psi){
    phi_v <- Est_phi[, word_index]
    phi_v_regular <- phi_v / sum(phi_v)
    return (phi_v_regular %*% Est_psi)
}

classify <- function(prob){
    if (prob > 0.5){
        return (1)
    }
    else {
        return (0)
    }
}


pred_y_lexicon <- function(d, doc_index, polarity){
    polarity_d <- polarity[doc_index == d]

    # コーパスがない場合
    if (length(polarity_d) == 0){
        return ("negative")
    }
    count <- table(polarity_d)
    max_polarity <- names(count[count == max(count)])

    # posとnegの数が同じ場合
    if (length(max_polarity) > 1){
        return ("negative")
    }
    return (max_polarity)
}


calc_accuracy <- function(y_d, test_y){
    return (table(y_d == test_y)[2] / length(y_d))
}

calc_auc <- function(y_d, test_y){
    pred     <- prediction(y_d, test_y)
    auc.temp <- performance(pred, "auc")
    auc   <- unlist(auc.temp@y.values)
    return (auc)
}


replace_y <- function(x, key){
    if (x == key) {
        return (0)
    }
    return (1)
}

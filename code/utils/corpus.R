feature_select <- function(text, min_count) {
    bow        <- lexicalize(text)
    word_count <- word.counts(bow$documents)
    vocab      <- bow$vocab[word_count >= min_count]
    return (vocab)
}


get_word_index <- function(text, vocab){
    doc   <- lexicalize(text, vocab = vocab)
    word_index_list <- sapply(doc, function(x){
        return (x[1, ])
    })

    return (word_index_list)
}

get_doc_index <- function(word_index_list){
    num_word_doc <- sapply(word_index_list, length)
    doc_index    <- as.vector(NULL)

    for (i in 1:length(word_index_list)){
        doc_index <- append(doc_index, rep(i, num_word_doc[i]))
    }
    return (doc_index)
}

get_tag_index <- function(word_index_list, y){
    num_word_doc <- sapply(word_index_list, length)
    tag_index    <- as.vector(NULL)

    for (i in 1:length(word_index_list)){
        if (y[i] == "yes"){
            tag_index <- append(tag_index, rep(1, num_word_doc[i]))
        }
        else{
            tag_index <- append(tag_index, rep(0, num_word_doc[i]))
        }
    }
    return (tag_index)
}

create_simple_corpus <- function(text, vocab){
    word_index_list <- get_word_index(text, vocab)
    doc_index       <- get_doc_index(word_index_list)
    word_index      <- unlist(word_index_list)

    corpus          <- cbind(word_index, doc_index)
    corpus[, 1]     <- corpus[, 1] + 1 # lexicalizeは0からのインデックスなので1を足してあげる
    # [Todo]: rcpp側の実装で上のコードが必要なくなるようにしてあげる
    corpus          <- cbind(corpus, 1) # tmp-topic assignments
    return (corpus)
}

create_tags_corpus <- function(text, vocab, y){
    word_index_list <- get_word_index(text, vocab)
    doc_index       <- get_doc_index(word_index_list)
    tag_index       <- get_tag_index(word_index_list, y)
    word_index      <- unlist(word_index_list)

    corpus <- cbind(word_index, doc_index, 1, tag_index)
    corpus[, 1] <- corpus[, 1] + 1 # lexicalizeは0からのインデックスなので1を足してあげる
    # [Todo]: rcpp側の実装で上のコードが必要なくなるようにしてあげる
    return (corpus)
}

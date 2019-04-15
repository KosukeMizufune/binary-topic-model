//
// Created by 水船公輔 on 2018/05/13.
//
//
#define ARMA_size_docONT_PRINT_ERRORS
#include <RcppArmadillo.h>
using namespace arma;
using namespace Rcpp;
#include "lda_base.h"

LdaBase::LdaBase(mat& corpus, int size_topic, vec& alpha, vec& beta,
                  int num_total_sampling, int num_burnin_sampling, int IsRecLikelihood)
        :corpus (corpus),
         size_topic (size_topic),
         IsRecLikelihood (IsRecLikelihood),
         num_burnin_sampling (num_burnin_sampling),
         num_total_sampling (num_total_sampling),

         alpha (alpha),
         beta (beta),

         rec_topic (zeros(corpus.n_rows, size_topic)),
         rec_likelihood (zeros(num_total_sampling)),
         phi (zeros(size_topic)),
         theta (zeros(size_topic)),

         size_corpus (corpus.n_rows),
         size_vocab (max(corpus.col(0))),
         size_doc (max(corpus.col(1))),


         word_index (corpus.col(0) - 1),
         doc_index (corpus.col(1) - 1),
         topic_assignment (corpus.col(2) - 1)
{
    count_variable(size_topic);
}


List LdaBase::lda_cgs() {
    for (int si = 0; si < num_total_sampling; si++){
        Rcout << si << endl;
        for (int n = 0; n < size_corpus; n++){
            int wi    = word_index(n);
            int di    = doc_index(n);
            int old_t = topic_assignment(n);


            subtract(old_t, di, wi);
            int new_t     = generate_topic(wi, di);
            update(n, new_t, di, wi);

            save_param(n, si, new_t);
        }
    }
    List sampling_param = List::create(
            Named("topic")      = rec_topic,
            Named("likelihood") = rec_likelihood);
    return sampling_param;
}

int LdaBase::generate_topic(int wi, int di) {
    phi     = calc_phi(wi);
    theta   = calc_theta(di);
    vec tmp_topic_prob = phi % theta;
    vec topic_prob     = tmp_topic_prob / sum(tmp_topic_prob);
    int new_t          = topic_allocate(topic_prob);
    return new_t;
}

vec LdaBase::calc_phi(int wi) {
    vec numerator   = num_word_topic_vocab.col(wi) + alpha(0);
    vec denominator = num_word_topic + size_vocab * alpha(0);
    return numerator / denominator;
}


vec LdaBase::calc_theta(int di) {
    vec numerator      = trans(num_word_doc_topic.row(di)) + beta(0);
    double denominator = num_word_doc(di) + size_topic * beta(0);
    return numerator / denominator;
}

int LdaBase::topic_allocate(vec &topic_prob) {
    vec uniform = runif(1);
    vec cum_prob = zeros(1);
    int new_topic = 0;

    for (int k = 0; k < size_topic; k++){
        cum_prob(0) = cum_prob(0) + topic_prob(k);
        if (uniform(0) < cum_prob(0)){
            new_topic = k;
            break;
        }
    }
    return new_topic;
}


void LdaBase::subtract(int old_t, int di, int wi) {
    num_word_topic_vocab(old_t, wi) -= 1;
    num_word_topic(old_t)           -= 1;
    num_word_doc_topic(di, old_t)   -= 1;
}


void LdaBase::update(int n, int new_t, int di, int wi) {
    num_word_topic_vocab(new_t, wi) += 1;
    num_word_topic(new_t)           += 1;
    num_word_doc_topic(di, new_t)   += 1;
    topic_assignment(n)              = new_t;
}

void LdaBase::save_param(int n, int si, int new_t){
    if (si >= num_burnin_sampling){
        rec_topic(n, new_t) += 1;
    }

    // [TODO]: phiはこの定義で良いのか？
    if (si >= num_burnin_sampling and IsRecLikelihood == 1){
        rec_likelihood(si) += log(phi(new_t));
    }
}


void LdaBase::count_variable(int size_topic) {
    num_word_doc         = zeros(size_doc);
    num_word_topic_vocab = zeros(size_topic, size_vocab);
    num_word_topic       = zeros(size_topic);
    num_word_doc_topic   = zeros(size_doc, size_topic);

    for (int n=0; n < size_corpus; n++){
        num_word_topic_vocab(topic_assignment(n), word_index(n)) += 1;
        num_word_doc_topic(doc_index(n), topic_assignment(n))    += 1;
    }

    for (int d=0; d < size_doc; d++){
        num_word_doc(d) += sum(num_word_doc_topic.row(d));
    }

    for (int k=0; k < size_topic; k++){
        num_word_topic(k) += sum(num_word_topic_vocab.row(k));
    }
}


// [[Rcpp::plugins("cpp11")]]
// [[Rcpp::depends(RcppArmadillo)]]
// [[Rcpp::export]]
List LDAcgs(mat& corpus,
             int size_topic,
             vec& alpha,
             vec& beta,
             int num_valid_sampling,
             int num_burnin_sampling,
             int IsLilihoodRec) {
    LdaBase lda(corpus, size_topic, alpha, beta, num_valid_sampling, num_burnin_sampling, IsLilihoodRec);
    return lda.lda_cgs();
}
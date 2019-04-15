//
// Created by 水船公輔 on 2018/05/15.
//

#define ARMA_size_docONT_PRINT_ERRORS
#include <RcppArmadillo.h>
using namespace arma;
using namespace Rcpp;
#include "lda_bernoulli.h"

LdaBernoulli::LdaBernoulli(mat& corpus,
                           int size_topic,
                           vec& alpha,
                           vec& beta,
                           vec& gamma,
                           int num_total_sampling,
                           int num_burnin_sampling,
                           int IsRecLikelihood,
                           int IsRecWLikelihood,
                           int IsRecYLikelihood)
        :LdaBase(corpus, size_topic, alpha, beta, num_total_sampling, num_burnin_sampling, IsRecLikelihood),
         IsRecWLikelihood (IsRecWLikelihood),
         IsRecYLikelihood (IsRecYLikelihood),
         label (corpus.col(3)),
         gamma (gamma),
         psi (zeros(size_topic)),
         rec_wlikelihood (zeros(num_total_sampling)),
         rec_ylikelihood (zeros(num_total_sampling)),
         y_missing (corpus.col(4)),
         rec_y (zeros(size_corpus, 2)),
         rec_phi (zeros(size_vocab, size_topic)),
         rec_psi (zeros(size_topic))
{
    count_variable(size_topic);
}


List LdaBernoulli::lda_cgs() {
    for (int si = 0; si < num_total_sampling; si++){
        for (int n = 0; n < size_corpus; n++){
            int wi    = word_index(n);
            int di    = doc_index(n);
            int old_t = topic_assignment(n);

            subtract(n, old_t, di, wi);
            int new_t = generate_topic(wi, di, n);
            update(n, new_t, di, wi);


            // predict missing polarity
            if (y_missing(n) == 0){
                int yi = label(n);
                num_word_polarity(new_t, yi) -= 1;

                vec numerator = trans(num_word_polarity.row(new_t)) + gamma(0);
                double denominator = sum(numerator);
                vec y_prob = numerator / denominator;

                int new_y = polarity_allocate(y_prob);

                num_word_polarity(new_t, new_y) += 1;
                label(n) = new_y;
            }

            save_param(n, si, new_t);
        }
        if (si >= num_burnin_sampling){
            for (int k=0; k < size_topic; k++){
                rec_phi.col(k) += calc_phi_k(k);
            }
            rec_psi += calc_psi_all();
        }
    }
    rec_phi /= (num_total_sampling - num_burnin_sampling);
    rec_psi /= (num_total_sampling - num_burnin_sampling);
    List sampling_param = List::create(
            Named("topic")        = rec_topic,
            Named("likelihood")   = rec_likelihood,
            Named("y")            = rec_y,
            Named("w_likelihood") = rec_wlikelihood,
            Named("y_likelihood") = rec_ylikelihood,
            Named("phi")          = rec_phi,
            Named("psi")          = rec_psi
    );
    return sampling_param;
}


int LdaBernoulli::polarity_allocate(vec& y_prob) {
    vec uniform = runif(1);
    vec cum_prob = zeros(1);
    int new_y = 0;

    for (int p = 0; p < size_topic; p++){
        cum_prob(0) = cum_prob(0) + y_prob(p);
        if (uniform(0) < cum_prob(0)){
            new_y = p;
            break;
        }
    }
    return new_y;
}


int LdaBernoulli::generate_topic(int wi, int di, int n){
    phi     = calc_phi(wi);
    theta   = calc_theta(di);
    psi     = calc_psi(n);
    vec tmp_topic_prob = phi % theta % psi;
    vec topic_prob     = tmp_topic_prob / sum(tmp_topic_prob);
    int new_t          = topic_allocate(topic_prob);
    return new_t;
}


vec LdaBernoulli::calc_psi(int n) {
    vec numerator = num_word_polarity.col(label(n)) + gamma(0);
    vec denominator = num_word_topic + 2 * gamma(0);
    return numerator / denominator;
}


void LdaBernoulli::subtract(int n, int old_t, int di, int wi) {
    num_word_topic_vocab(old_t, wi) -= 1;
    num_word_topic(old_t)           -= 1;
    num_word_doc_topic(di, old_t)   -= 1;
    num_word_polarity(old_t, label(n)) -= 1;
}


void LdaBernoulli::update(int n, int new_t, int di, int wi) {
    num_word_topic_vocab(new_t, wi) += 1;
    num_word_topic(new_t)           += 1;
    num_word_doc_topic(di, new_t)   += 1;
    topic_assignment(n)              = new_t;
    num_word_polarity(new_t, label(n)) += 1;

}


void LdaBernoulli::save_param(int n, int si, int new_t){
    if (si >= num_burnin_sampling) {
        rec_topic(n, new_t) += 1;
        rec_y(n, label(n)) += 1;
    }

    // [TODO]: phiはこの定義で良いのか？
    if (si >= num_burnin_sampling and IsRecLikelihood == 1){
        rec_likelihood(si) += log(phi(new_t) * psi(new_t));
    }

    if (si >= num_burnin_sampling and IsRecWLikelihood == 1){
        rec_wlikelihood(si) += log(phi(new_t));
    }

    if (si >= num_burnin_sampling and IsRecYLikelihood == 1){
        rec_ylikelihood(si) += log(psi(new_t));
    }
}


int LdaBernoulli::topic_allocate(vec &topic_prob) {
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


void LdaBernoulli::count_variable(int size_topic) {
    num_word_doc         = zeros(size_doc);
    num_word_topic_vocab = zeros(size_topic, size_vocab);
    num_word_topic       = zeros(size_topic);
    num_word_doc_topic   = zeros(size_doc, size_topic);
    num_word_polarity    = zeros(size_topic, 2);

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

    for (int n = 0; n < size_corpus; n++) {
        num_word_polarity(topic_assignment(n), label(n)) += 1;
    }
}

vec LdaBernoulli::calc_phi_k(int k) {
    return (num_word_topic_vocab.row(k) + alpha(0)).t() / (num_word_topic(k) + size_vocab * alpha(0));
}

vec LdaBernoulli::calc_psi_all() {
    return (num_word_polarity.col(1) + gamma(0)) / (num_word_topic + 2 * gamma(0));
}


// [[Rcpp::plugins("cpp11")]]
// [[Rcpp::depends(RcppArmadillo)]]
// [[Rcpp::export]]
List LDAcgs(mat& corpus,
            int size_topic,
            vec& alpha,
            vec& beta,
            vec& gamma,
            int num_total_sampling,
            int num_burnin_sampling,
            int IsRecLikelihood,
            int IsWRecLikelihood,
            int IsYRecLikelihood)
{
    LdaBernoulli lda(corpus,
                            size_topic,
                            alpha,
                            beta,
                            gamma,
                            num_total_sampling,
                            num_burnin_sampling,
                            IsRecLikelihood,
                            IsWRecLikelihood,
                            IsYRecLikelihood
    );
    return lda.lda_cgs();
}

//
// Created by 水船公輔 on 2018/05/15.
//

#ifndef ALGORITHM_LDA_BERNOULLI_H
#define ALGORITHM_LDA_BERNOULLI_H

#include "lda_base.h"

class LdaBernoulli: public LdaBase {
public:
    LdaBernoulli(mat& corpus,
                 int size_topic,
                 vec& alpha,
                 vec& beta,
                 vec& gamma,
                 int num_total_sampling,
                 int num_burnin_sampling,
                 int IsRecLikelihood,
                 int IsRecWLikelihood,
                 int IsRecYLikelihood
);
    List lda_cgs();

protected:
    int IsRecWLikelihood;
    int IsRecYLikelihood;
    vec label;
    mat num_word_polarity;
    vec gamma;
    vec psi;
    vec rec_wlikelihood;
    vec rec_ylikelihood;
    void count_variable(int size_topic);
    void subtract(int n, int old_t, int di, int wi);
    void update(int n, int new_t, int di, int wi);
    vec calc_psi(int n);
    int generate_topic(int wi, int di, int n);
    int topic_allocate(vec &topic_generate_prob);
    mat y_missing;
    mat rec_y;
    int polarity_allocate(vec& y_prob);
    virtual void save_param(int n, int si, int new_t);
    vec calc_phi_k(int k);
    vec calc_psi_all();
    mat rec_phi;
    vec rec_psi;
};


#endif //ALGORITHM_LDA_BERNOULLI_H

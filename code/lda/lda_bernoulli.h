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
    mat y_missing;
    vec label;
    int IsRecWLikelihood;
    int IsRecYLikelihood;

    vec gamma;

    mat num_word_polarity;

    vec rec_wlikelihood;
    vec rec_ylikelihood;
    mat rec_y;

    vec psi;

    vec calc_psi(int n);

private:
    int topic_allocate(vec &topic_generate_prob);
    int generate_topic(int wi, int di, int n);
    void subtract(int n, int old_t, int di, int wi);
    void update(int n, int new_t, int di, int wi);
    void count_variable(int size_topic);
    void save_param(int n, int si, int new_t);

    int polarity_allocate(vec& y_prob);
};


#endif //ALGORITHM_LDA_BERNOULLI_H

//
// Created by 水船公輔 on 2018/05/13.
//

#ifndef ALGORITHM_LDA_BASE_H
#define ALGORITHM_LDA_BASE_H


using namespace arma;
using namespace Rcpp;


class LdaBase {
public:
    LdaBase(mat& corpus, int size_topic, vec& alpha, vec& beta,
            int num_total_sampling, int num_burnin_sampling, int IsRecLikelihood);
    virtual List lda_cgs();

protected:
    mat corpus;
    int size_topic;
    int IsRecLikelihood;
    int num_burnin_sampling;
    int num_total_sampling;

    vec alpha;
    vec beta;

    mat rec_topic;
    vec rec_likelihood;

    vec phi;
    vec theta;

    mat num_word_topic_vocab;
    mat num_word_doc_topic;
    vec num_word_topic;
    vec num_word_doc;

    int size_corpus;
    int size_vocab;
    int size_doc;

    vec word_index;
    vec doc_index;
    vec topic_assignment;

    vec calc_phi(int wi);
    vec calc_theta(int di);

private:
    virtual int topic_allocate(vec &topic_generate_prob);
    virtual int generate_topic(int wi, int di);
    virtual void subtract(int old_t, int di, int wi);
    virtual void update(int n, int new_t, int di, int wi);
    virtual void save_param(int n, int si, int new_t);
    virtual void count_variable(int size_topic);
};



#endif //ALGORITHM_LDA_BASE_H

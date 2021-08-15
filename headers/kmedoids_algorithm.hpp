#ifndef KMEDOIDS_ALGORITHM_H_
#define KMEDOIDS_ALGORITHM_H_

#include "log_helper.hpp"

#include <carma>
#include <armadillo>
#include <vector>
#include <fstream>
#include <iostream>
#include <chrono>
#include <omp.h>


typedef std::tuple<size_t, size_t> key_t_bpam;

struct key_hash : public std::unary_function<key_t_bpam, double>
{
 std::size_t operator()(const key_t_bpam& k) const;
};

namespace km {
  /**
   *  \brief Class implementation for running KMedoids methods.
   *
   *  KMedoids class. Creates a KMedoids object that can be used to find the medoids
   *  for a particular set of input data.
   *
   *  @param nMedoids Number of medoids/clusters to create
   *  @param algorithm Algorithm used to find medoids; options are "BanditPAM" for
   *  the "Bandit-PAM" algorithm, or "naive" to use the naive method
   *  @param verbosity Verbosity of the algorithm, 0 will have no log file
   *  emitted, 1 will emit a log file
   *  @param maxIter The maximum number of iterations the algorithm runs for
   *  @param buildConfidence Constant that affects the sensitivity of build confidence bounds
   *  @param swapConfidence Constant that affects the sensitiviy of swap confidence bounds
   *  @param logFilename The name of the output log file
   */
  class KMedoids {
    public:
      KMedoids(size_t n_medoids = 5, const std::string& algorithm = "BanditPAM", size_t verbosity = 0, size_t max_iter = 1000,
              size_t buildConfidence =  1000, size_t swapConfidence = 10000, std::string logFilename = "KMedoidsLogfile");
      
      ~KMedoids();

      void fit(const arma::mat& inputData, const std::string& loss, std::string module, std::string dist_mat);

      // std::map is a RB tree, should use unordered_map
      std::unordered_map<key_t_bpam, double, key_hash> cache;

      // The functions below are "get" functions for read-only attributes

      arma::rowvec getMedoidsFinal();

      arma::rowvec getMedoidsBuild();

      arma::rowvec getLabels();

      size_t getSteps();

      // The functions below are get/set functions for attributes

      size_t getNMedoids();

      void setNMedoids(size_t new_num);

      std::string getAlgorithm();

      void setAlgorithm(const std::string& new_alg); // pass by ref

      size_t getVerbosity();

      void setVerbosity(size_t new_ver);

      size_t getMaxIter();

      void setMaxIter(size_t new_max);

      size_t getbuildConfidence();

      void setbuildConfidence(size_t new_buildConfidence);

      size_t getswapConfidence();

      void setswapConfidence(size_t new_swapConfidence);

      std::string getLogfileName();

      void setLogFilename(const std::string& new_lname);

      void setLossFn(std::string loss);

      
    private:
      // The functions below are PAM's constituent functions
      void fit_bpam(const arma::mat& inputData);

      void fit_naive(const arma::mat& inputData); // pass by ref? (and above)

      void build_naive(const arma::mat& data, arma::rowvec& medoidIndices);

      void swap_naive(const arma::mat& data, arma::rowvec& medoidIndices, arma::rowvec& assignments);

      void build(
        const arma::mat& data,
        arma::rowvec& medoidIndices,
        arma::mat& medoids
      );

      void build_sigma(
        const arma::mat& data,
        arma::rowvec& best_distances,
        arma::rowvec& sigma,
        arma::uword batch_size,
        bool use_absolute
      );

      arma::rowvec build_target(
        const arma::mat& data,
        arma::uvec& target,
        size_t batch_size,
        arma::rowvec& best_distances,
        bool use_absolute
      );

      void swap(
        const arma::mat& data,
        arma::rowvec& medoidIndices,
        arma::mat& medoids,
        arma::rowvec& assignments
      );

      void calc_best_distances_swap(
        const arma::mat& data,
        arma::rowvec& medoidIndices,
        arma::rowvec& best_distances,
        arma::rowvec& second_distances,
        arma::rowvec& assignments
      );

      arma::vec swap_target(
        const arma::mat& data,
        arma::rowvec& medoidIndices,
        arma::uvec& targets,
        size_t batch_size,
        arma::rowvec& best_distances,
        arma::rowvec& second_best_distances,
        arma::rowvec& assignments
      );

      void swap_sigma(
        const arma::mat& data,
        arma::mat& sigma,
        size_t batch_size,
        arma::rowvec& best_distances,
        arma::rowvec& second_best_distances,
        arma::rowvec& assignments
      );

      void sigma_log(arma::mat& sigma);

      double calc_loss(const arma::mat& data, arma::rowvec& medoidIndices);

      // Loss functions
      double wrappedLossFn(const arma::mat& data, size_t i, size_t j, bool use_cache);

      size_t lp;
      double LP(const arma::mat& data, size_t i, size_t j) const;

      double LINF(const arma::mat& data, size_t i, size_t j) const;

      double cos(const arma::mat& data, size_t i, size_t j) const;

      double manhattan(const arma::mat& data, size_t i, size_t j) const;

      void checkAlgorithm(const std::string& algorithm);

      // Constructor params
      size_t n_medoids; ///< number of medoids identified for a given dataset

      std::string algorithm; ///< options: "naive" and "BanditPAM"

      size_t max_iter; ///< maximum number of iterations during KMedoids::fit

      // Properties of the KMedoids instance
      arma::mat data; ///< input data used during KMedoids::fit

      arma::rowvec labels; ///< assignments of each datapoint to its medoid

      arma::rowvec medoid_indices_build; ///< medoids at the end of build step

      arma::rowvec medoid_indices_final; ///< medoids at the end of the swap step

      double (KMedoids::*lossFn)(const arma::mat& data, size_t i, size_t j) const; ///< loss function used during KMedoids::fit

      void (KMedoids::*fitFn)(const arma::mat& inputData); ///< function used for finding medoids (from algorithm)

      LogHelper logHelper; ///< helper object for making formatted logs

      size_t steps; ///< number of actual swap iterations taken by the algorithm

      // Hyperparameters
      size_t buildConfidence; ///< constant that affects the sensitivity of build confidence bounds

      size_t swapConfidence; ///< constant that affects the sensitiviy of swap confidence bounds

      const double precision = 0.001; ///< bound for double comparison precision

      const size_t batchSize = 100; ///< batch size for computation steps

      size_t verbosity; ///< determines whether KMedoids::fit outputs a logfile

      std::string logFilename; ///< name of the logfile output (verbosity permitting)
  };
}

  #endif // KMEDOIDS_ALGORITHM_H_

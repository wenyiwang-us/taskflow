#include <stdlib.h>
#include <math.h>
#include <string>
#include <cassert>
#include <iostream>
#include <chrono>
#include <fstream>
#include <memory>
#include <random>
#include "../taskflow/taskflow.hpp"
#include "../taskflow/algorithm/for_each.hpp"
// #include <CLI11.hpp>

//Precision to use for calculations

typedef struct OptionData_ {
  float s;          // spot price
  float strike;     // strike price
  float r;          // risk-free interest rate
  float divq;       // dividend rate
  float v;          // volatility
  float t;          // time to maturity or option expiration in years
                     //     (1yr = 1.0, 6mos = 0.5, 3mos = 0.25, ..., etc)
  char OptionType;   // Option type.  "P"=PUT, "C"=CALL
  float divs;       // dividend vals (not used in this test)
  float DGrefval;   // DerivaGem Reference Value
} OptionData;

OptionData *optdata;
float *prices;
int numOptions;
int NUM_RUNS;
int*    otype;
float* sptprice;
float* strike;
float* rate;
float* volatility;
float* otime;
int numError;
float* BUFFER;
int* BUFFER2;

////////////////////////////////////////////////////////////////////////////////
// Cumulative Normal Distribution Function
// See Hull, Section 11.8, P.243-244
////////////////////////////////////////////////////////////////////////////////
#define inv_sqrt_2xPI 0.39894228040143270286

inline float CNDF( float InputX ) {
    int sign;

    float OutputX;
    float xInput;
    float xNPrimeofX;
    float expValues;
    float xK2;
    float xK2_2, xK2_3;
    float xK2_4, xK2_5;
    float xLocal, xLocal_1;
    float xLocal_2, xLocal_3;

    // Check for negative value of InputX
    if (InputX < 0.0) {
      InputX = -InputX;
      sign = 1;
    }
    else {
      sign = 0;
    }

    xInput = InputX;

    // Compute NPrimeX term common to both four & six decimal accuracy calcs
    expValues = std::exp(-0.5f * InputX * InputX);
    xNPrimeofX = expValues;
    xNPrimeofX = xNPrimeofX * inv_sqrt_2xPI;

    xK2 = 0.2316419 * xInput;
    xK2 = 1.0 + xK2;
    xK2 = 1.0 / xK2;
    xK2_2 = xK2 * xK2;
    xK2_3 = xK2_2 * xK2;
    xK2_4 = xK2_3 * xK2;
    xK2_5 = xK2_4 * xK2;

    xLocal_1 = xK2 * 0.319381530;
    xLocal_2 = xK2_2 * (-0.356563782);
    xLocal_3 = xK2_3 * 1.781477937;
    xLocal_2 = xLocal_2 + xLocal_3;
    xLocal_3 = xK2_4 * (-1.821255978);
    xLocal_2 = xLocal_2 + xLocal_3;
    xLocal_3 = xK2_5 * 1.330274429;
    xLocal_2 = xLocal_2 + xLocal_3;

    xLocal_1 = xLocal_2 + xLocal_1;
    xLocal   = xLocal_1 * xNPrimeofX;
    xLocal   = 1.0 - xLocal;

    OutputX  = xLocal;

    if (sign) {
        OutputX = 1.0 - OutputX;
    }

    return OutputX;
}



inline float BlkSchlsEqEuroNoDiv(
  float sptprice, float strike, float rate,
  float volatility, float time, int otype, float) {

  float OptionPrice;

  // local private working variables for the calculation
  //float xStockPrice;  // These two variables are not used
  //float xStrikePrice;
  float xRiskFreeRate;
  float xVolatility;
  float xTime;
  float xSqrtTime;

  float logValues;
  float xLogTerm;
  float xD1;
  float xD2;
  float xPowerTerm;
  float xDen;
  float d1;
  float d2;
  float FutureValueX;
  float NofXd1;
  float NofXd2;
  float NegNofXd1;
  float NegNofXd2;

  //xStockPrice = sptprice;
  //xStrikePrice = strike;
  xRiskFreeRate = rate;
  xVolatility = volatility;

  xTime = time;
  xSqrtTime = std::sqrt(xTime);

  logValues = std::log(sptprice/strike);

  xLogTerm = logValues;

  xPowerTerm = xVolatility * xVolatility;
  xPowerTerm = xPowerTerm * 0.5;

  xD1 = xRiskFreeRate + xPowerTerm;
  xD1 = xD1 * xTime;
  xD1 = xD1 + xLogTerm;

  xDen = xVolatility * xSqrtTime;
  xD1 = xD1 / xDen;
  xD2 = xD1 -  xDen;

  d1 = xD1;
  d2 = xD2;

  NofXd1 = CNDF( d1 );
  NofXd2 = CNDF( d2 );

  FutureValueX = strike * ( std::exp( -(rate)*(time) ) );
  if (otype == 0) {
    OptionPrice = (sptprice * NofXd1) - (FutureValueX * NofXd2);
  }
  else {
    NegNofXd1 = (1.0 - NofXd1);
    NegNofXd2 = (1.0 - NofXd2);
    OptionPrice = (FutureValueX * NegNofXd2) - (sptprice * NegNofXd1);
  }

  return OptionPrice;
}


inline void check_error(unsigned i, float price) {
  float priceDelta = optdata[i].DGrefval - price;
  if(std::fabs(priceDelta) >= 1e-4 ){
    printf("Error on %d. Computed=%.5f, Ref=%.5f, Delta=%.5f\n",
        i, price, optdata[i].DGrefval, priceDelta);
    numError ++;
  }
}


// Sequential version
inline void bs_seq(float *seq_prices) {

  int i, j;
  float price;

  for (j=0; j<NUM_RUNS; j++) {
    for (i=0; i<numOptions; i++) {
      /* Calling main function to calculate option value based on
       * Black & Scholes's equation.
       */
      price = BlkSchlsEqEuroNoDiv( sptprice[i], strike[i],
          rate[i], volatility[i], otime[i],
          otype[i], 0);
      seq_prices[i] = price;

#ifdef ERR_CHK
      check_error(i, seq_prices[i]);
#endif
    }
  }
}

inline void destroy_options() {
  free(optdata);
  free(prices);
  free(BUFFER);
  free(BUFFER2);
}

// Read input option data from file
inline void generate_options(size_t num_options) {

  numOptions = num_options == 0 ? 1 : num_options;
  NUM_RUNS = numOptions;

  // Allocate spaces for the option data
  optdata = static_cast<OptionData*>(malloc(numOptions*sizeof(OptionData)));
  prices = static_cast<float*>(malloc(numOptions*sizeof(float)));

  for(int i=0; i<numOptions; ++i) {
    optdata[i].s = ::rand() % 200;
    optdata[i].strike = ::rand() % 200;
    optdata[i].r = 0.1f;
    optdata[i].divq = 0.0f;
    optdata[i].v = ::rand() % 100 / 100.0f;
    optdata[i].t = ::rand() % 100 / 100.0f;
    optdata[i].OptionType = ::rand() % 2 ? 'P' : 'C';
    optdata[i].divs = 0.0f;
    optdata[i].DGrefval = ::rand() % 20;
  }

  const int PAD {256};
  const int LINESIZE {64};

  BUFFER = static_cast<float *>(malloc(5 * numOptions * sizeof(float) + PAD));
  sptprice = reinterpret_cast<float *>(((unsigned long long)BUFFER + PAD) & ~(LINESIZE - 1));
  strike = sptprice + numOptions;
  rate = strike + numOptions;
  volatility = rate + numOptions;
  otime = volatility + numOptions;

  BUFFER2 = static_cast<int *>(malloc(numOptions * sizeof(float) + PAD));
  //otype = (int *) (((unsigned long long)BUFFER2 + PAD) & ~(LINESIZE - 1));
  otype = reinterpret_cast<int *>(((unsigned long long)BUFFER2 + PAD) & ~(LINESIZE - 1));

  for(auto i=0; i<numOptions; i++) {
    otype[i]      = (optdata[i].OptionType == 'P') ? 1 : 0;
    sptprice[i]   = optdata[i].s;
    strike[i]     = optdata[i].strike;
    rate[i]       = optdata[i].r;
    volatility[i] = optdata[i].v;
    otime[i]      = optdata[i].t;
  }
}

std::chrono::microseconds measure_time_taskflow(unsigned);
// std::chrono::microseconds measure_time_tbb(unsigned);
// std::chrono::microseconds measure_time_omp(unsigned);

//==============================================================================
// Taskflow implementation
//==============================================================================

void bs_taskflow(unsigned num_threads) {

  static tf::Executor executor(num_threads);
  tf::Taskflow taskflow;

  auto init = taskflow.placeholder();

  auto loop = taskflow.for_each_index(
    0, numOptions, 1, [&](int i) {
    auto price = BlkSchlsEqEuroNoDiv(
      sptprice[i], strike[i],
      rate[i], volatility[i], otime[i],
      otype[i], 0
    );

    prices[i] = price;
#ifdef ERR_CHK
    check_error(i, price);
#endif
  }, tf::StaticPartitioner());

  auto cond = taskflow.emplace([i=0] () mutable{
    return ++i == NUM_RUNS ? -1 : 0;
  });

  init.precede(loop);
  loop.precede(cond);
  cond.precede(loop);

  executor.run(taskflow).wait();
}


std::chrono::microseconds measure_time_taskflow(unsigned num_threads) {
  auto beg = std::chrono::high_resolution_clock::now();
  bs_taskflow(num_threads);
  auto end = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::microseconds>(end - beg);
}




// // extern global variables
// OptionData *optdata = nullptr;
// float *prices = nullptr;
// int numOptions = 0;
// int NUM_RUNS = 1;
// int*    otype = nullptr;
// float* sptprice = nullptr;
// float* strike = nullptr;
// float* rate = nullptr;
// float* volatility = nullptr;
// float* otime = nullptr;
// int numError = 0;
// float* BUFFER = nullptr;
// int* BUFFER2 = nullptr;

void black_scholes(
  const std::string& model,
  const unsigned num_threads,
  const unsigned num_rounds
) {

  std::cout << std::setw(12) << "size"
            << std::setw(12) << "threads"
            << std::setw(12) << "runtime"
            << std::endl;

  // for(size_t N = 1000; N<=10000; N+=1000) {
    size_t N = 10000;
    generate_options(N);

    double runtime {0.0};

    for(unsigned j=0; j<num_rounds; ++j) {
      if(model == "tf") {
        runtime += measure_time_taskflow(num_threads).count();
      }
      // else if(model == "tbb") {
      //   runtime += measure_time_tbb(num_threads).count();
      // }
    //   else if(model == "omp") {
    //     runtime += measure_time_omp(num_threads).count();
    //   }
      else assert(false);
    }

    destroy_options();

    std::cout << std::setw(12) << N
              << std::setw(12) << num_threads
              << std::setw(12) << runtime / num_rounds / 1e3
              << std::endl;
  // }
}


int main (int argc, char *argv[]) {

  unsigned num_threads {1};
  unsigned num_rounds {1}; 
  bool cmp_seq {false};
  std::string model = "tf";

  for(int i=1; i<argc; i++) {
    std::string arg = argv[i];
    if(arg == "-t" || arg == "--num_threads") {
      if(i + 1 < argc) {
        num_threads = std::stoi(argv[++i]);
      }
    }
    else if(arg == "-r" || arg == "--num_rounds") {
      if(i + 1 < argc) {
        num_rounds = std::stoi(argv[++i]); 
      }
    }
    else if(arg == "-s" || arg == "--seq") {
      cmp_seq = true;
    }
    else if(arg == "-m" || arg == "--model") {
      if(i + 1 < argc) {
        model = argv[++i];
        if(model != "tf" && model != "xtf") {
          std::cerr << "Error: model must be tf or xtf" << std::endl;
          exit(1);
        }
      }
    }
    else {
      std::cerr << "Usage: " << argv[0] << " [-t threads] [-r rounds] [-s] [-m model]" << std::endl;
      exit(1);
    }
  }

  std::cout << "model=" << model << ' '
            << "num_threads=" << num_threads << ' '
            << "num_rounds=" << num_rounds << ' '
            << std::endl;

  black_scholes(model, num_threads, num_rounds);

  // Compare with sequential version to check correctness
  //if(cmp_seq) {
  //  auto seq_prices = static_cast<FPTYPE*>(malloc(numOptions*sizeof(FPTYPE)));
  //  bs_seq(seq_prices);
  //  for(auto i=0; i<numOptions; i++) {
  //    assert(std::fabs(seq_prices[i] - prices[i]) < 0.01);
  //  }
  //  std::cout << "Results are consistent with the sequential version\n";
  //  free(seq_prices);
  //}

#ifdef ERR_CHK
    printf("Num Errors: %d\n", numError);
#endif

  return 0;
}




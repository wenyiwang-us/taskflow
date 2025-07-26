// Modified by W. W., Thread Scaling
#include "fibonacci.hpp"
#include <CLI11.hpp>

void bench_fibonacci(
  const std::string& model,
  const unsigned num_threads,
  const unsigned num_rounds,
  const unsigned num_fibonacci
  ) {

  std::cout << std::setw(12) << "size"
            << std::setw(12) << "threads"
            << std::setw(12) << "runtime"
            << std::endl;

  // for(size_t f=1; f<=num_fibonacci; ++f) {

    double runtime {0.0};

    for(unsigned j = 0; j < num_rounds; ++j) {
      if(model == "tf") {
        runtime += measure_time_taskflow(num_threads, num_fibonacci).count();
      }
      else if(model == "omp") {
        runtime += measure_time_omp(num_threads, num_fibonacci).count();
      }
      // else if(model == "tbb") {
      //   runtime += measure_time_tbb(num_threads, num_fibonacci).count();
      // }
      
      else assert(false);
    }

    std::cout << std::setw(12) << num_fibonacci
              << std::setw(12) << num_threads
              << std::setw(12) << runtime / num_rounds / 1e3
              << std::endl;
  // }
}

int main(int argc, char* argv[]) {

  CLI::App app{"Fibonacci"};

  unsigned num_threads {1};
  app.add_option("-t,--num_threads", num_threads, "number of threads (default=1)");
  
  unsigned num_fibonacci {35};
  app.add_option("-i,--input", num_fibonacci, "max number of fibonacci (default=35)");

  unsigned num_rounds {1};
  app.add_option("-r,--num_rounds", num_rounds, "number of rounds (default=1)");

  std::string model = "tf";
  app.add_option("-m,--model", model, "model name std|omp|tf|tbb (default=tf)")
     ->check([] (const std::string& m) {
        if(m != "omp" && m != "tf") {
          return "model name should be \"omp\" or \"tf\"";
        }else if(m == "tbb") {
          return "tbb is not supported";
        }
        return "";
     });

  CLI11_PARSE(app, argc, argv);

  std::cout << "model="       << model << ' '
            << "num_threads=" << num_threads << ' '
            << "num_rounds="  << num_rounds << ' '
            << "fibonacci="   << num_fibonacci << ' '
            << std::endl;

  bench_fibonacci(model, num_threads, num_rounds, num_fibonacci);

  return 0;
}



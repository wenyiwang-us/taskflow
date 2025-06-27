#include <algorithm> // for std::max
#include <cassert>
#include <cstdio>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <thread>
#include <random>
#include <cmath>
#include <atomic>
#include <string>
#include <cstdlib>
#include "../taskflow/taskflow.hpp"

inline void func(std::atomic<size_t>& counter) { 
  counter.fetch_add(1, std::memory_order_relaxed); 
}

// async_task computing
void async_task_taskflow(unsigned num_threads, size_t num_tasks) {

  static tf::Executor executor(num_threads);

  std::atomic<size_t> counter(0);

  for(size_t i=0; i<num_tasks; i++) {
    executor.silent_async([&] { func(counter); });
  }

  executor.wait_for_all();
  
  if(counter.load(std::memory_order_relaxed) != num_tasks) {
    throw std::runtime_error("incorrect result");
  }
}

std::chrono::microseconds measure_time_taskflow(unsigned num_threads, size_t num_tasks) {
  auto beg = std::chrono::high_resolution_clock::now();
  async_task_taskflow(num_threads, num_tasks);
  auto end = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::microseconds>(end - beg);
}

void print_usage(const char* program_name) {
  std::cout << "Usage: " << program_name << " [options]\n"
            << "Options:\n"
            << "  -t, --num_threads <num>    number of threads (default=1)\n"
            << "  -r, --num_rounds <num>     number of rounds (default=1)\n"
            << "  -m, --model <name>         model name std|omp|tf|tbb (default=tf)\n"
            << "  -h, --help                 show this help message\n";
}

void bench_async_task(
  const std::string& model,
  const unsigned num_threads,
  const unsigned num_rounds
  ) {

  std::cout << std::setw(12) << "size"
            << std::setw(12) << "threads"
            << std::setw(12) << "runtime"
            << std::endl;
  int S = 2097152;
  // for(int S=1; S<=2097152; S<<=1) {

    double runtime {0.0};

    for(unsigned j=0; j<num_rounds; ++j) {
      if(model == "tf") {
        runtime += measure_time_taskflow(num_threads, S).count();
      }
    //   else if(model == "std") {
    //     runtime += measure_time_std(num_threads, S).count();
    //   }
    //   else if(model == "omp") {
    //     runtime += measure_time_omp(num_threads, S).count();
    //   }
      // else if(model == "tbb") {
      //   runtime += measure_time_tbb(num_threads, S).count();
      // }
      else assert(false);
    }

    std::cout << std::setw(12) << S
              << std::setw(12) << num_threads
              << std::setw(12) << runtime / num_rounds / 1e3
              << std::endl;
  // }
}

int main(int argc, char* argv[]) {

  unsigned num_threads = 1;
  unsigned num_rounds = 1;
  std::string model = "tf";

  // Parse command line arguments
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    
    if (arg == "-h" || arg == "--help") {
      print_usage(argv[0]);
      return 0;
    }
    else if (arg == "-t" || arg == "--num_threads") {
      if (i + 1 < argc) {
        num_threads = std::stoul(argv[++i]);
      } else {
        std::cerr << "Error: " << arg << " requires a value\n";
        print_usage(argv[0]);
        return 1;
      }
    }
    else if (arg == "-r" || arg == "--num_rounds") {
      if (i + 1 < argc) {
        num_rounds = std::stoul(argv[++i]);
      } else {
        std::cerr << "Error: " << arg << " requires a value\n";
        print_usage(argv[0]);
        return 1;
      }
    }
    else if (arg == "-m" || arg == "--model") {
      if (i + 1 < argc) {
        model = argv[++i];
        if (model != "std" && model != "omp" && model != "tf" && model != "tbb") {
          std::cerr << "Error: model name should be \"std\", \"omp\", \"tbb\", or \"tf\"\n";
          print_usage(argv[0]);
          return 1;
        }
      } else {
        std::cerr << "Error: " << arg << " requires a value\n";
        print_usage(argv[0]);
        return 1;
      }
    }
    else {
      std::cerr << "Error: unknown option " << arg << "\n";
      print_usage(argv[0]);
      return 1;
    }
  }

  std::cout << "model=" << model << ' '
            << "num_threads=" << num_threads << ' '
            << "num_rounds=" << num_rounds << ' '
            << std::endl;

  bench_async_task(model, num_threads, num_rounds);

  return 0;
}



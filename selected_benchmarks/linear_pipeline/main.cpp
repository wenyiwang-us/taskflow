#include "linear_pipeline.hpp"
#include <CLI11.hpp>

int main(int argc, char* argv[]) {

  CLI::App app{"Parallel Pipeline"};

  unsigned num_threads {1};
  app.add_option("-t,--num_threads", num_threads, "number of threads (default=1)");

  unsigned num_rounds {1};
  app.add_option("-r,--num_rounds", num_rounds, "number of rounds (default=1)");

  std::string model = "tf";
  app.add_option("-m,--model", model, "model name tbb|omp|tf (default=tf)")
     ->check([] (const std::string& m) {
        if(m != "tbb" && m != "omp" && m != "tf") {
          return "model name should be \"tbb\", \"omp\", or \"tf\"";
        }
        return "";
     });

  std::string input = "8:ssssssss";
  app.add_option("-i,--input", input, "input format: num_lines:pipes (default=8:ssssssss)")
      ->check([input] (const std::string& p) {
        size_t colon_pos = p.find(':');
        if (colon_pos == std::string::npos) {
          return "input format should be num_lines:pipes (e.g., 8:ssssssss)";
        }
        std::string pipes = p.substr(colon_pos + 1);
        if (pipes[0] == 'p') {
          return "the first pipe should be \"s\" (serial)";
        }
        else if (pipes.size() > 16) {
          return "no more than 16 pipes";
        }
        else if (pipes.size() == 0) {
          return "at least one pipe is required";
        }
        return "";
      });

  CLI11_PARSE(app, argc, argv);

  // Parse input string to extract num_lines and pipes
  size_t colon_pos = input.find(':');
  unsigned num_lines = std::stoul(input.substr(0, colon_pos));
  std::string pipes = input.substr(colon_pos + 1);

  std::cout << "model="       << model       << ' '
            << "num_threads=" << num_threads << ' '
            << "num_rounds="  << num_rounds  << ' '
            << "num_lines="   << num_lines   << ' '
            << "pipes="       << pipes       << ' '
            << std::endl;

  std::cout << std::setw(12) << "size"
            << std::setw(12) << "threads"
            << std::setw(12) << "Runtime"
            << '\n';

  size_t  log_length = 23;

  // for(size_t i = 1; i <= log_length; ++i) {

    // size_t L = 1 << i;

    size_t L = 1 << log_length;

    double runtime {0.0};

    for(unsigned j = 0; j < num_rounds; ++j) {
      if(model == "tf") {
        runtime += measure_time_taskflow(pipes, num_lines, num_threads, L).count();
      }
      // else if(model == "tbb") {
      //   runtime += measure_time_tbb(pipes, num_lines, num_threads, L).count();
      // }
      else if(model == "omp") {
        runtime += measure_time_omp(pipes, num_lines, num_threads, L).count();
      }
    }

    std::cout << std::setw(12) << L
              << std::setw(12) << num_threads
              << std::setw(12) << runtime / num_rounds / 1e3
              << std::endl;

    /*if (model == "tf") {
      std::ofstream outputfile;
      outputfile.open("./tf_time.csv", std::ofstream::app);
      outputfile << num_threads << ','
                 << num_lines   << ','
                 << pipes       << ','
                 << L           << ','
                 << runtime / num_rounds / 1e3 << '\n';

      outputfile.close();
    }
    else if (model == "tbb") {
      std::ofstream outputfile;
      outputfile.open("./tbb_time.csv", std::ofstream::app);
      outputfile << num_threads << ','
                 << num_lines   << ','
                 << pipes       << ','
                 << L           << ','
                 << runtime / num_rounds / 1e3 << '\n';

      outputfile.close();
    }*/
  // }
}

#include "levelgraph.hpp"
#include <CLI11.hpp>

int main(int argc, char* argv[]) {

  CLI::App app{"Graph Pipeline"};

  unsigned num_threads {1};
  app.add_option("-t,--num_threads", num_threads, "number of threads (default=1)");

  unsigned num_rounds {1};
  app.add_option("-r,--num_rounds", num_rounds, "number of rounds (default=1)");

  std::string model = "tf";
  app.add_option("-m,--model", model, "model name tbb|omp|tf|ff (default=tf)")
     ->check([] (const std::string& m) {
        if(m != "tbb" && m != "omp" && m != "tf" && m !="gold" && m != "ff") {
          return "model name should be \"tbb\", \"omp\", or \"tf\", or \"ff\"";
        }
        return "";
     });

  std::string input = "8:8";
  app.add_option("-i,--input", input, "input format: num_lines:pipes (default=8:8)");

  CLI11_PARSE(app, argc, argv);

  // Parse input string to extract num_lines and pipes
  size_t colon_pos = input.find(':');
  if (colon_pos == std::string::npos) {
    throw std::runtime_error("input format should be num_lines:pipes (e.g., 8:8)");
  }
  
  unsigned num_lines = std::stoul(input.substr(0, colon_pos));
  size_t pipes = std::stoul(input.substr(colon_pos + 1));

  if(pipes == 0 || pipes > 16) {
    throw std::runtime_error("can only support 1-16 pipes");
  }

  std::cout << "model="       << model       << ' '
            << "num_threads=" << num_threads << ' '
            << "num_rounds="  << num_rounds  << ' '
            << "num_lines="   << num_lines   << ' '
            << "pipes="       << pipes       << ' '
            << std::endl;

  std::cout << std::setw(12) << "|V|+|E|"
            << std::setw(12) << "threads"
            << std::setw(12) << "Runtime"
             << '\n';

  // for(int i = 1; i <= 451; i += 15) {
    int i = 451;

    double runtime {0.0};

    LevelGraph graph(i, i);

    for(unsigned j = 0; j < num_rounds; ++j) {
      if(model == "tf") {
        runtime += measure_time_taskflow(graph, pipes, num_lines, num_threads).count();
      }
      // else if(model == "tbb") {
      //   runtime += measure_time_tbb(graph, pipes, num_lines, num_threads).count();
      // }
      else if(model == "omp") {
        runtime += measure_time_omp(graph, pipes, num_lines, num_threads).count();
      }
      else if(model == "gold") {
        runtime += measure_time_gold(graph, pipes).count();
      }
      //else if(model == "ff") {
      //  runtime += measure_time_fastflow(graph, pipes).count();
      //}
      graph.clear_graph();
    }

    std::cout << std::setw(12) << graph.graph_size()
              << std::setw(12) << num_threads
              << std::setw(12) << runtime / num_rounds / 1e3
              << std::endl;
  // }
}

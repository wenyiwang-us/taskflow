#!/usr/bin/env python3
import os
import subprocess
import sys
import argparse
import csv
import itertools
import random
from typing import Dict, List, Any

sanity_check_inputs = {
    'wavefront': [1024, 2048],
    'graph_traversal': [451, 900],
    'binary_tree': [15, 20],
    'linear_chain': [20, 23],
    'matrix_multiplication': [1024, 2048],
    'black_scholes': [1000, 2000],
    'mandelbrot': [100, 200],
    'reduce_sum': [100000000, 200000000],
    'scan': [100000000, 200000000],
    'sort': [10000000, 20000000],
    'for_each': [10000000, 20000000],
    'primes': [1000000, 2000000],
}

small_inputs = {
    'binary_tree': [24],
    'black_scholes': [4000],
    'for_each': [10000000],
    'graph_traversal': [900],
    'linear_chain': [23],
    'mandelbrot': [500],
    'matrix_multiplication': [1024],
    'primes': [1000000],
    'reduce_sum': [8*1000000000],
    'scan': [1000000000],
    'sort': [100000000],
    'wavefront': [4096],
}

adjusted_inputs_7_28 = {
    'binary_tree': [23],
    'black_scholes': [4000], # remain the same
    'for_each' : [int(2e8)], 
    'graph_traversal': [2000],
    'linear_chain': [23], # remain the same
    'mandelbrot': [500],
    'matrix_multiplication': [2048],
    'primes' : [int(3e6)],
    'reduce_sum': [int(4e9)],
    'scan': [int(4e9)],
    'sort': [int(4e8)],
    'wavefront': [16384],
}

adjusted_inputs_7_28_1 = {
    'binary_tree': [23],
    'black_scholes': [4000], # remain the same
    'for_each' : [int(2e9)], 
    'graph_traversal': [2000],
    'linear_chain': [23], # remain the same
    'mandelbrot': [5000],
    'matrix_multiplication': [4096],
    'primes' : [int(2e8)],
    'reduce_sum': [int(8e9)],
    'scan': [int(2e9)],
    'sort': [int(3e8)],
    'wavefront': [16384],
}

# benchmark_inputs = small_inputs
# benchmark_inputs = adjusted_inputs_7_28
benchmark_inputs = adjusted_inputs_7_28_1


def calculate_total_experiments(args, benchmark_inputs):
    """Calculate the total number of experiments to be run"""
    total = 0
    
    for benchmark in args.benchmarks:
        if benchmark not in benchmark_inputs:
            continue
            
        inputs = benchmark_inputs[benchmark]
        
        for method in args.methods:
            if method == 'tf':
                # For tf: benchmark × inputs × threads × rounds
                total += len(inputs) * len(args.threads) * args.rounds
            elif method == 'xtf':
                # For xtf: benchmark × inputs × threads × nvtms × nsteals × nwaits × rounds
                total += len(inputs) * len(args.threads) * len(args.nvtms) * len(args.nsteals) * len(args.nwaits) * args.rounds
    
    return total

def main():
    parser = argparse.ArgumentParser(description='Run complete set of experiments with different parameter combinations')
    
    parser.add_argument(
        '-n', '--name',
        type=str,
        help='Name of this run (used for CSV filename)',
        required=True
    )
    
    parser.add_argument(
        '-b', '--benchmarks',
        nargs='+',
        help='list of benchmark names',
        choices=['wavefront', 
                 'graph_traversal', 
                 'binary_tree', 
                 'linear_chain', 
                 'matrix_multiplication',
                 'black_scholes',
                 'mandelbrot',
                 'reduce_sum',
                 'scan',
                 'sort',
                 'for_each',
                 'async_task',
                 'mnist',
                 'fibonacci',
                 'nqueens',
                 'integrate',
                 'primes',
                 'skynet'],
        required=True
    )
    
    parser.add_argument(
        '-t', '--threads',
        type=int,
        nargs='+',
        help='list of thread counts',
        required=True
    )
    
    parser.add_argument(
        '-r', '--rounds',
        type=int,
        help='number of rounds to run each combination',
        default=1
    )
    
    parser.add_argument(
        '-m', '--methods',
        nargs='+',
        help='list of methods to run',
        choices=['tf', 'xtf'],
        default=['tf', 'xtf']
    )
    
    parser.add_argument(
        '--nvtms',
        type=int,
        nargs='+',
        help='list of nvtms values for xtf experiments',
        default=[1, 2, 4]
    )
    
    parser.add_argument(
        '--nsteals',
        type=int,
        nargs='+',
        help='list of nsteals values for xtf experiments',
        default=[1, 2, 4, 32]
    )
    
    parser.add_argument(
        '--nwaits',
        type=int,
        nargs='+',
        help='list of nwaits values for xtf experiments',
        default=[1000, 10000, 100000]
    )
    
    parser.add_argument(
        '--plot',
        action='store_true',
        help='plot the results using plot.py after completion',
        default=False
    )
    
    parser.add_argument(
        '--output-dir',
        type=str,
        help='directory to save CSV files (default: output/csv)',
        default='output/csv'
    )
    
    parser.add_argument(
        '--shuffle',
        action='store_true',
        help='shuffle experiment combinations for better load distribution',
        default=False
    )
    
    args = parser.parse_args()
    
    
    print(f"Run name: {args.name}")
    print(f"Benchmarks: {args.benchmarks}")
    print(f"Thread counts: {args.threads}")
    print(f"Rounds: {args.rounds}")
    print(f"Methods: {args.methods}")
    print(f"Output directory: {args.output_dir}")
    print(f"Shuffle combinations: {args.shuffle}")
    
    if 'xtf' in args.methods:
        print(f"NVTMs: {args.nvtms}")
        print(f"NSteals: {args.nsteals}")
        print(f"NWaits: {args.nwaits}")
    
    # Calculate total number of experiments
    total_experiments = calculate_total_experiments(args, benchmark_inputs)
    print(f"\nTotal experiments to run: {total_experiments}")
    print("=" * 50)
    
    # Run all experiments
    all_results = run_experiments(args, benchmark_inputs)
    
    # Save results to CSV
    save_results_to_csv(all_results, args.name, args.output_dir)
    
    # Plot if requested
    if args.plot:
        try:
            import plot
            plot.plot_results(f"{args.name}.csv")
            print(f"Plot saved for {args.name}.csv")
        except ImportError:
            print("plot.py not found, skipping plotting")
        except Exception as e:
            print(f"Error during plotting: {e}")

def run_experiments(args, benchmark_inputs):
    """Run all experiments and collect results"""
    all_results = []
    
    for benchmark in args.benchmarks:
        if benchmark not in benchmark_inputs:
            print(f"Warning: No input arguments defined for benchmark '{benchmark}', skipping")
            continue
            
        inputs = benchmark_inputs[benchmark]
        
        for method in args.methods:
            if method == 'tf':
                # For tf method, create all combinations using product
                tf_combinations = list(itertools.product(
                    [benchmark],      # benchmark
                    inputs,           # input values
                    args.threads,     # thread counts
                    range(1, args.rounds + 1)  # rounds
                ))
                
                # Shuffle the combinations for better load distribution
                if args.shuffle:
                    random.shuffle(tf_combinations)
                
                for benchmark_name, input_val, nthreads, round_num in tf_combinations:
                    result = run_tf_experiment(benchmark_name, input_val, nthreads, round_num, args.name)
                    if result:
                        all_results.append(result)
                        
            elif method == 'xtf':
                # For xtf method, create all combinations using product
                xtf_combinations = list(itertools.product(
                    [benchmark],      # benchmark
                    inputs,           # input values
                    args.threads,     # thread counts
                    args.nvtms,       # nvtms values
                    args.nsteals,     # nsteals values
                    args.nwaits,      # nwaits values
                    range(1, args.rounds + 1)  # rounds
                ))
                
                # Shuffle the combinations for better load distribution
                if args.shuffle:
                    random.shuffle(xtf_combinations)
                
                for benchmark_name, input_val, nthreads, nvtms, nsteals, nwaits, round_num in xtf_combinations:
                    result = run_xtf_experiment(
                        benchmark_name, input_val, nthreads, 
                        nvtms, nsteals, nwaits, round_num, args.name
                    )
                    if result:
                        all_results.append(result)
    
    return all_results

def run_tf_experiment(benchmark, input_val, nthreads, round_num, run_name):
    """Run a single tf experiment"""
    exe = f'../build/selected_benchmarks/bench_{benchmark}'
    
    print(f"Running tf experiment: {benchmark} with input={input_val}, threads={nthreads}, round={round_num}")
    
    # Run the benchmark executable
    cmd = [exe, '-m', 'tf', '-t', str(nthreads), '-r', '1', '-i', str(input_val)]
    
    try:
        with open('tmp.txt', 'w') as f:
            subprocess.run(cmd, stdout=f, stderr=subprocess.PIPE, check=True)
        
        # Parse results
        with open('tmp.txt', 'r') as f:
            # Skip header lines
            f.readline()  # Skip first header
            f.readline()  # Skip second header
            
            for line in f:
                tokens = line.strip().split()
                if len(tokens) == 3:
                    size = int(tokens[0])
                    num_threads = int(tokens[1])
                    runtime_ms = float(tokens[2])
                    
                    return {
                        'name': run_name,
                        'benchmark': benchmark,
                        'method': 'tf',
                        'input': input_val,
                        'nthreads': num_threads,
                        'round': round_num,
                        'runtime(ms)': runtime_ms
                    }
    
    except subprocess.CalledProcessError as e:
        print(f"Error running tf experiment: {e}")
        return None
    except Exception as e:
        print(f"Error parsing tf results: {e}")
        return None
    
    return None

def run_xtf_experiment(benchmark, input_val, nthreads, nvtms, nsteals, nwaits, round_num, run_name):
    """Run a single xtf experiment"""
    exe = f'../build/selected_benchmarks/xtf_{benchmark}'
    
    print(f"Running xtf experiment: {benchmark} with input={input_val}, threads={nthreads}, "
          f"nvtms={nvtms}, nsteals={nsteals}, nwaits={nwaits}, round={round_num}")
    
    # Set environment variables for xtf
    env = os.environ.copy()
    env['TF_NVTMS'] = str(nvtms)
    env['TF_NSTEALS'] = str(nsteals)
    env['TF_NWAITS'] = str(nwaits)
    
    # Run the benchmark executable
    cmd = [exe, '-m', 'tf', '-t', str(nthreads), '-r', '1', '-i', str(input_val)]
    
    try:
        with open('tmp.txt', 'w') as f:
            subprocess.run(cmd, stdout=f, stderr=subprocess.PIPE, check=True, env=env)
        
        # Parse results
        with open('tmp.txt', 'r') as f:
            # Skip header lines
            f.readline()  # Skip first header
            f.readline()  # Skip second header
            
            for line in f:
                tokens = line.strip().split()
                if len(tokens) == 3:
                    size = int(tokens[0])
                    num_threads = int(tokens[1])
                    runtime_ms = float(tokens[2])
                    
                    return {
                        'name': run_name,
                        'benchmark': benchmark,
                        'method': 'xtf',
                        'input': input_val,
                        'nthreads': num_threads,
                        'round': round_num,
                        'nvtms': nvtms,
                        'nsteals': nsteals,
                        'nwaits': nwaits,
                        'runtime(ms)': runtime_ms
                    }
    
    except subprocess.CalledProcessError as e:
        print(f"Error running xtf experiment: {e}")
        return None
    except Exception as e:
        print(f"Error parsing xtf results: {e}")
        return None
    
    return None

def save_results_to_csv(results, run_name, output_dir):
    """Save results to CSV file"""
    if not results:
        print("No results to save")
        return
    
    # Create output directory if it doesn't exist
    os.makedirs(output_dir, exist_ok=True)
    
    csv_filename = os.path.join(output_dir, f"{run_name}.csv")
    
    # Determine headers based on the first result
    if results:
        headers = list(results[0].keys())
    else:
        headers = []
    
    with open(csv_filename, 'w', newline='') as csvfile:
        writer = csv.DictWriter(csvfile, fieldnames=headers)
        writer.writeheader()
        
        for result in results:
            writer.writerow(result)
    
    print(f"Results saved to {csv_filename}")
    print(f"Total experiments completed: {len(results)}")

if __name__ == "__main__":
    main() 

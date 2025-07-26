#!/usr/bin/env python3
import os
import subprocess
import sys
import math
import argparse
import matplotlib.pyplot as plot
import statistics as stat
import numpy as np
import csv
import pandas as pd
###########################################################
# main function
###########################################################
def main():

  # example usage
  # -b wavefront graph_traversal -t 1 2 3 -m tbb omp tf

  parser = argparse.ArgumentParser(description='regression')

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
             # 'hetero_traversal',
             'fibonacci',
             'nqueens',
             'integrate',
             'primes',
             'skynet'],

    required=True
  )


  parser.add_argument(
    '-m','--methods', 
    nargs='+', 
    help='list of tasking methods', 
    default=['tf', 'xtf', 'tbb', 'omp'],
    choices=['tf', 'xtf', 'tbb', 'omp']
  )

  parser.add_argument(
    '-t', '--threads', 
    type=int,
    nargs='+',
    help='list of the number of threads',
    required=True
  )

  parser.add_argument(
    '-r', '--num_rounds',
    type=int,
    help='number of rounds to average',
    default=1
  )

  parser.add_argument(
    '-p', '--plot',
    action='store_true',
    help='create plot from CSV data (requires plot.py)',
    default=False
  )

  parser.add_argument(
    '-o', '--output',
    type=str,
    help='file name to save the plot result',
    default="result.png"
  )

  parser.add_argument(
    '-c', '--csv',
    type=str,
    help='CSV folder path (CSV files will be saved as path/name.csv)',
    required=True
  )

  parser.add_argument(
    '-n', '--name',
    type=str,
    help='Name of this run (used for CSV filename and name column)',
    required=True
  )

  parser.add_argument(
    '-a', '--append',
    type=bool,
    help='append to existing CSV file or create new one',
    default=False
  )
  
  # parse the arguments
  args = parser.parse_args()
  
  print('benchmarks: ', args.benchmarks)
  print('threads:', args.threads)
  print('methods:', args.methods)
  print('num_rounds:', args.num_rounds)
  print('plot:', args.plot)
  print('csv folder:', args.csv)
  print('run name:', args.name)
  print('append:', args.append)

  # Run benchmarks and collect data
  all_results = run_all_benchmarks(args.benchmarks, args.methods, args.threads, args.num_rounds, args.name)
  
  # Save results to CSV
  save_to_csv(all_results, args.csv, args.name, args.append)
  
  # Create plot if requested
  if args.plot:
    print("Plotting functionality has been moved to plot.py")

def run_all_benchmarks(benchmarks, methods, threads, num_rounds, run_name):
    """Run all benchmarks and collect results"""
    all_results = []
    
    for benchmark in benchmarks:
        for method in methods:
            for thread in threads:
                results = run_benchmark(benchmark, method, thread, num_rounds, run_name)
                all_results.extend(results)
    
    return all_results

def run_benchmark(benchmark, method, thread, num_rounds, run_name):
    """Run a single benchmark and return results"""
    if method == 'xtf':
        exe = f'../build/selected_benchmarks/xtf_{benchmark}'
        method_name = 'tf'
    else:
        exe = f'../build/selected_benchmarks/bench_{benchmark}'
        method_name = method

    results = []
    print(f'{exe} -m {method_name} -t {thread} -r {num_rounds}')
    
    # Run the benchmark executable
    cmd = [exe, '-m', method_name, '-t', str(thread), '-r', str(num_rounds)]
    
    with open('tmp.txt', 'w') as f:
        subprocess.call(cmd, stdout=f)
   
    with open('tmp.txt', 'r') as f:
        # first two lines are header
        f.readline()
        f.readline()
        for line in f:
            token = line.split()
            assert len(token) == 3, "thread_scaling: output line must have exactly three numbers"
            size = int(token[0])
            nthreads = int(token[1])
            exec_time_ms = float(token[2])
            
            # Add result for each round
            for round_num in range(1, num_rounds + 1):
                results.append({
                    'name': run_name,
                    'benchmark': benchmark,
                    'method': method,
                    'nthreads': nthreads,
                    'round': round_num,
                    'size': size,
                    'exec_time_ms': exec_time_ms
                })
    
    return results

def save_to_csv(results, csv_folder, run_name, append=False):
    """Save results to CSV file"""
    # Ensure the folder exists
    os.makedirs(csv_folder, exist_ok=True)
    
    csv_filename = os.path.join(csv_folder, f"{run_name}.csv")
    
    # Define CSV headers
    headers = ['name', 'benchmark', 'method', 'nthreads', 'round', 'size', 'exec_time_ms']
    
    # Check if file exists and append mode
    file_exists = os.path.exists(csv_filename)
    mode = 'a' if append and file_exists else 'w'
    
    with open(csv_filename, mode, newline='') as csvfile:
        writer = csv.DictWriter(csvfile, fieldnames=headers)
        
        # Write header only if creating new file or not appending
        if mode == 'w' or not append:
            writer.writeheader()
        
        # Write results
        for result in results:
            writer.writerow(result)
    
    print(f"Results saved to {csv_filename}")



# Legacy function for backward compatibility
def run(benchmark, method, threads, num_rounds):
    """Legacy function - kept for backward compatibility"""
    if method == 'xtf':
        exe = f'../build/selected_benchmarks/xtf_{benchmark}'
        method = 'tf'
    else:
        exe = f'../build/selected_benchmarks/bench_{benchmark}'

    X = []
    Y = [] 
    for thread in threads:
        print(f'{exe} -m {method} -t {thread} -r {num_rounds}')
        # Run the benchmark executable
        cmd = [exe, '-m', method, '-t', str(thread), '-r', str(num_rounds)]
        
        with open('tmp.txt', 'w') as f:
            subprocess.call(cmd, stdout=f)
   
        
        with open('tmp.txt', 'r') as f:
            # first two lines are header
            f.readline()
            f.readline()
            for line in f:
                token = line.split()
                assert len(token) == 3, "thread_scaling: output line must have exactly three numbers"
                X.append(int(token[1]))
                Y.append(float(token[2]))
        
    return X, Y

if __name__ == "__main__":
  main()

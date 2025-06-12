#!/usr/bin/env python3
import os
import subprocess
import sys
import math
import argparse
import matplotlib.pyplot as plot
import statistics as stat
import numpy as np
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
             'hetero_traversal',
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
    type=bool,
    help='show the plot or not',
    default=False
  )

  parser.add_argument(
    '-o', '--output',
    type=str,
    help='file name to save the plot result',
    default="result.png"
  )
  
  # parse the arguments
  args = parser.parse_args()
  
  print('benchmarks: ', args.benchmarks)
  print('threads:', args.threads)
  print('methods:', args.methods)
  print('num_rounds:', args.num_rounds)
  print('plot:', args.plot)

  # Create the plot
  # Create subplot for each benchmark
  num_benchmarks = len(args.benchmarks)
  rows = math.ceil(num_benchmarks / 2)  # 2 columns
  cols = 1
  fig, axes = plot.subplots(rows, cols, figsize=(12, 5*rows))
  # Ensure axes is always a list of axis objects
  if num_benchmarks == 1:
    axes = np.array([axes])
  axes = axes.flatten()

  # Plot each benchmark
  for idx, benchmark in enumerate(args.benchmarks):
    ax = axes[idx]
    ax.set_title(benchmark)
    ax.set_xlabel('Number of Threads')
    ax.set_ylabel('Runtime (ms)')
    
    for method in args.methods:
      X, Y = run(benchmark, method, args.threads, args.num_rounds)
      print(method, X, Y)
      if method == 'tf':
        marker = ''
        color = 'b'
      elif method == 'xtf':
        marker = 'x'
        color = 'r'
      elif method == 'omp':
        marker = '+'
        color = 'g'
      else:  # tbb
        marker = '.'
        color = 'orange'
    
      ax.plot(X, Y, label=method, marker=marker, color=color)
      ax.legend()

  # Remove empty subplots
  for idx in range(num_benchmarks, len(axes)):
    fig.delaxes(axes[idx])

  plot.tight_layout()
  plot.savefig(args.output)

  if args.plot:
    plot.show()

  plot.close(fig)
  
def run(benchmark, method, threads, num_rounds):


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

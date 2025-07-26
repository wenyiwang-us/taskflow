#!/usr/bin/env python3
import os
import argparse
import matplotlib.pyplot as plot
import pandas as pd
import math
import numpy as np
import sys

def main():
    parser = argparse.ArgumentParser(description='Plot thread scaling results from CSV files')
    
    parser.add_argument(
        'csv_files',
        nargs='+',
        help='List of CSV files to process (at least one required)',
        default=['./thread_scaling_results/xtf.csv',]
    )
    
    parser.add_argument(
        '-b', '--benchmarks',
        nargs='+',
        help='List of benchmarks to plot (if not specified, plot all available)',
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
                 'skynet']
    )
    
    parser.add_argument(
        '-m', '--methods',
        nargs='+',
        help='List of methods to plot (if not specified, plot all available)',
        choices=['tf', 'xtf', 'tbb', 'omp']
    )
    
    parser.add_argument(
        '-o', '--output',
        type=str,
        help='Output filename for the plot',
        default='comparison_plot.png'
    )
    
    parser.add_argument(
        '--show_plot',
        action='store_true',
        help='Show the plot interactively'
    )
    
    parser.add_argument(
        '--dpi',
        type=int,
        help='DPI for the output image',
        default=300
    )
    
    args = parser.parse_args()
    
    print(f'CSV files: {args.csv_files}')
    print(f'Output file: {args.output}')
    
    # Load and combine data from all CSV files
    all_data = load_csv_data(args.csv_files)
    
    if all_data.empty:
        print("No data found in the specified CSV files!")
        return
    
    # Validate that there are no multiple sizes for the same benchmark/nthreads combination
    validate_data_consistency(all_data)
    
    # Get available benchmarks and methods if not specified
    available_benchmarks = all_data['benchmark'].unique()
    available_methods = all_data['method'].unique()
    
    benchmarks_to_plot = args.benchmarks if args.benchmarks else available_benchmarks
    methods_to_plot = args.methods if args.methods else available_methods
    
    print(f'Available benchmarks: {available_benchmarks}')
    print(f'Available methods: {available_methods}')
    print(f'Plotting benchmarks: {benchmarks_to_plot}')
    print(f'Plotting methods: {methods_to_plot}')
    
    # Create the comparison plot
    create_comparison_plot(all_data, benchmarks_to_plot, methods_to_plot, args.output, args.dpi)
    
    if args.show_plot:
        plot.show()

def load_csv_data(csv_files):
    """Load data from multiple CSV files and combine them"""
    all_data = []
    
    for csv_file in csv_files:
        if not os.path.exists(csv_file):
            print(f"Warning: CSV file {csv_file} not found, skipping...")
            continue
            
        try:
            df = pd.read_csv(csv_file)
            # Extract prefix from filename for identification (fallback if 'name' column doesn't exist)
            prefix = os.path.splitext(os.path.basename(csv_file))[0]
            if 'name' in df.columns:
                df['prefix_name'] = df['name']
            else:
                # Handle legacy format with filename_prefix column
                if 'filename_prefix' in df.columns:
                    df['prefix_name'] = df['filename_prefix']
                else:
                    df['prefix_name'] = prefix
            all_data.append(df)
            print(f"Loaded data from {csv_file}: {len(df)} rows")
        except Exception as e:
            print(f"Error loading {csv_file}: {e}")
            continue
    
    if all_data:
        return pd.concat(all_data, ignore_index=True)
    else:
        return pd.DataFrame()

def validate_data_consistency(data):
    """Validate that there are no multiple sizes for the same benchmark/nthreads combination"""
    # Group by benchmark, method, nthreads and check for multiple sizes
    size_check = data.groupby(['benchmark', 'method', 'nthreads'])['size'].nunique()
    problematic_combinations = size_check[size_check > 1]
    
    if not problematic_combinations.empty:
        print("ERROR: Multiple sizes found for the same benchmark/method/nthreads combination:")
        for (benchmark, method, nthreads), count in problematic_combinations.items():
            sizes = data[(data['benchmark'] == benchmark) & 
                        (data['method'] == method) & 
                        (data['nthreads'] == nthreads)]['size'].unique()
            print(f"  Benchmark: {benchmark}, Method: {method}, Threads: {nthreads}")
            print(f"    Sizes found: {sizes}")
        print("\nPlease fix the data by ensuring consistent sizes for each benchmark/method/nthreads combination.")
        sys.exit(1)

def create_comparison_plot(data, benchmarks, methods, output_filename, dpi=300):
    """Create comparison plot from CSV data"""
    
    # Filter data for specified benchmarks and methods
    filtered_data = data[
        (data['benchmark'].isin(benchmarks)) & 
        (data['method'].isin(methods))
    ]
    
    if filtered_data.empty:
        print("No data matches the specified benchmarks and methods!")
        return
    
    # Create subplots for each benchmark
    num_benchmarks = len(benchmarks)
    rows = math.ceil(num_benchmarks)
    cols = 1
    fig, axes = plot.subplots(rows, cols, figsize=(12, 5*rows), dpi=dpi)
    
    # Ensure axes is always a list of axis objects
    if num_benchmarks == 1:
        axes = np.array([axes])
    axes = axes.flatten()
    
    # Define colors and markers for different methods
    colors = ['b', 'r', 'g', 'orange', 'purple', 'brown', 'pink', 'gray', 'olive', 'cyan']
    markers = ['o', 's', '^', 'D', 'v', '<', '>', 'p', '*', 'h']
    
    # Plot each benchmark
    for idx, benchmark in enumerate(benchmarks):
        ax = axes[idx]
        ax.set_title(f'{benchmark}', fontsize=14, fontweight='bold')
        ax.set_xlabel('Number of Threads', fontsize=12)
        ax.set_ylabel('Runtime (ms)', fontsize=12)
        ax.grid(True, alpha=0.3)
        
        # Filter data for this benchmark
        benchmark_data = filtered_data[filtered_data['benchmark'] == benchmark]
        
        # Plot each method
        for method_idx, method in enumerate(methods):
            method_data = benchmark_data[benchmark_data['method'] == method]
            
            if not method_data.empty:
                # Group by (prefix_name, benchmark, method, nthreads, size) and calculate mean/std
                grouped_data = method_data.groupby(['prefix_name', 'benchmark', 'method', 'nthreads', 'size'])['exec_time_ms'].agg(['mean', 'std', 'count']).reset_index()
                
                # Further group by nthreads to get overall mean/std across all prefixes and sizes
                final_grouped = grouped_data.groupby('nthreads')['mean'].agg(['mean', 'std']).reset_index()
                X = final_grouped['nthreads'].tolist()
                Y = final_grouped['mean'].tolist()
                Y_err = final_grouped['std'].tolist()
                
                # Choose color and marker
                color = colors[method_idx % len(colors)]
                marker = markers[method_idx % len(markers)]
                
                # Create label with method name
                label = method
                
                # Plot with error bars
                if len(Y_err) > 0 and any(err > 0 for err in Y_err):
                    ax.errorbar(X, Y, yerr=Y_err, label=label, marker=marker, 
                              color=color, capsize=3, capthick=1, linewidth=2, markersize=6)
                else:
                    ax.plot(X, Y, label=label, marker=marker, color=color, 
                           linewidth=2, markersize=6)
        
        ax.legend()
        ax.tick_params(axis='both', which='major', labelsize=10)
    
    # Remove empty subplots
    for idx in range(num_benchmarks, len(axes)):
        fig.delaxes(axes[idx])
    
    plot.tight_layout()
    plot.savefig(output_filename, dpi=dpi, bbox_inches='tight')
    plot.close(fig)
    print(f"Comparison plot saved to {output_filename}")

def create_summary_statistics(data, benchmarks, methods):
    """Create summary statistics for the comparison"""
    print("\n=== Summary Statistics ===")
    
    for benchmark in benchmarks:
        print(f"\nBenchmark: {benchmark}")
        benchmark_data = data[data['benchmark'] == benchmark]
        
        for method in methods:
            method_data = benchmark_data[benchmark_data['method'] == method]
            
            if not method_data.empty:
                print(f"  Method: {method}")
                
                # Group by prefix, nthreads, and size
                summary = method_data.groupby(['prefix_name', 'nthreads', 'size'])['exec_time_ms'].agg(['mean', 'std', 'count']).reset_index()
                
                # Further group by nthreads
                final_summary = summary.groupby('nthreads')['mean'].agg(['mean', 'std']).reset_index()
                
                for _, row in final_summary.iterrows():
                    print(f"    {row['nthreads']} threads: {row['mean']:.2f} ± {row['std']:.2f} ms")

if __name__ == "__main__":
    main() 
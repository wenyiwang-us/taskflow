#!/usr/bin/env python3
"""
Performance scaling plot script for TaskFlow benchmarks.
Plots runtime vs thread count for different methods (tf, xtf) across benchmarks.
For xtf, shows only the best performing work-stealing parameter combinations.
"""

import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import argparse
import os
from pathlib import Path

plt.style.use('paper.mplstyle')

SEQ_BENCH = [
    'black_scholes',
    'for_each',
    'primes',
    'matrix_multiplication',
    'reduce_sum',
    'sort',
    'mandelbrot',
    'scan',
    'wavefront',
    'graph_traversal',
    'binary_tree',
    'linear_chain'
]

SELECTED_BENCH = [
    'for_each',
    'matrix_multiplication',
    'graph_traversal',
    'binary_tree',
]

def load_and_process_data(csv_paths, methods=['tf', 'xtf']):
    """
    Load CSV files and process the data according to the requirements.
    
    Args:
        csv_paths: List of CSV file paths
        methods: List of methods to include in the analysis
    
    Returns:
        Processed DataFrame with aggregated results
    """
    # Load and concatenate all CSV files
    dfs = []
    for csv_path in csv_paths:
        if os.path.exists(csv_path):
            print(f"Loading CSV file: {csv_path}")
            df = pd.read_csv(csv_path)
            print(f"  - Shape: {df.shape}")
            print(f"  - Benchmarks: {sorted(df['benchmark'].unique())}")
            print(f"  - Methods: {sorted(df['method'].unique())}")
            dfs.append(df)
        else:
            print(f"Warning: CSV file {csv_path} not found")
    
    if not dfs:
        raise ValueError("No valid CSV files found")
    
    print(f"Total CSV files loaded: {len(dfs)}")
    
    # Concatenate all dataframes
    combined_df = pd.concat(dfs, ignore_index=True)
    
    # Set all NaN to 0
    combined_df = combined_df.fillna(0)
    
    # Filter for specified methods
    combined_df = combined_df[combined_df['method'].isin(methods)]
    print(f"After filtering for methods {methods}: {combined_df.shape}")
    
    # Group by parameters and compute mean and std for runtime
    group_cols = ['name', 'benchmark', 'method', 'input', 'nthreads']
    if 'nvtms' in combined_df.columns:
        group_cols.extend(['nvtms', 'nsteals', 'nwaits'])
    
    agg_df = combined_df.groupby(group_cols).agg({
        'runtime(ms)': ['mean', 'std']
    }).reset_index()
    
    # Flatten column names
    agg_df.columns = [col[0] if col[1] == '' else f"{col[0]}_{col[1]}" for col in agg_df.columns]
    
    # For xtf method, find the best performing configuration for each benchmark/thread combination
    if 'xtf' in methods:
        # Get xtf data
        xtf_data = agg_df[agg_df['method'] == 'xtf'].copy()
        
        # Find best configuration for each benchmark/thread combination
        best_xtf = xtf_data.loc[xtf_data.groupby(['benchmark', 'nthreads'])['runtime(ms)_mean'].idxmin()].copy()
        
        # Create parameter string for annotation
        best_xtf['params'] = best_xtf.apply(
            lambda row: f"({int(row['nvtms'])},{int(row['nsteals'])},{int(row['nwaits'])})", axis=1
        )
        
        # Get tf data
        tf_data = agg_df[agg_df['method'] == 'tf'].copy()
        
        # Combine tf and best xtf data
        final_df = pd.concat([tf_data, best_xtf], ignore_index=True)
    else:
        final_df = agg_df
    
    return final_df

def create_performance_plot(data, output_file='performance_scaling.png'):
    """
    Create performance scaling plots for each benchmark.
    
    Args:
        data: Processed DataFrame with performance data
        output_file: Output file path for the plot
    """
    # Get selected benchmarks
    benchmarks = SELECTED_BENCH
    
    # Calculate subplot layout - 2x2 grid for 4 benchmarks
    n_benchmarks = len(benchmarks)
    n_cols = 2
    n_rows = 2
    fig, axes = plt.subplots(n_rows, n_cols, figsize=(8, 5))
    
   
    axes = axes.flatten()  # Use column-major (Fortran-style) order
    
    # Colors for different methods
    colors = {'tf': 'blue', 'xtf': 'red'}
    markers = {'tf': 'o', 'xtf': 's'}
    
    for idx, benchmark in enumerate(benchmarks):
        if idx >= len(axes):  # Safety check
            break
        ax = axes[idx]
        
        # Get data for this benchmark
        benchmark_data = data[data['benchmark'] == benchmark]
        
        # Plot each method
        for method in benchmark_data['method'].unique():
            method_data = benchmark_data[benchmark_data['method'] == method]
            
            # Sort by thread count
            method_data = method_data.sort_values('nthreads')
            
            # Plot line with error bars
            ax.errorbar(
                method_data['nthreads'], 
                method_data['runtime(ms)_mean'],
                yerr=method_data['runtime(ms)_std'],
                label=method.upper(),
                color=colors[method],
                marker=markers[method],
                markersize=6,
                linewidth=2,
                capsize=5,
                capthick=2
            )
            
            # Add parameter annotations for xtf
            if method == 'xtf' and 'params' in method_data.columns:
                for _, row in method_data.iterrows():
                    ax.annotate(
                        row['params'],
                        xy=(row['nthreads'], row['runtime(ms)_mean']),
                        xytext=(5, 5),
                        textcoords='offset points',
                        fontsize=8,
                        bbox=dict(boxstyle='round,pad=0.3', facecolor='yellow', alpha=0.7),
                        arrowprops=dict(arrowstyle='->', connectionstyle='arc3,rad=0')
                    )
        
        # Customize subplot
        ax.set_xlabel('Number of Threads')
        ax.set_ylabel('Runtime (ms)')
        ax.set_title(f'{benchmark.replace("_", " ").title()}')
        ax.legend()
        ax.grid(True, alpha=0.3)
        
        # Set x-axis to show actual thread counts
        ax.set_xticks(sorted(benchmark_data['nthreads'].unique()))
    
    # Hide unused subplots
    for idx in range(len(benchmarks), len(axes)):
        axes[idx].set_visible(False)
    
    # Adjust layout
    plt.tight_layout()
    
    # Save plot
    plt.savefig(output_file, dpi=300, bbox_inches='tight')
    print(f"Plot saved to {output_file}")
    
    # Show plot
    plt.show()

def main():
    parser = argparse.ArgumentParser(description='Create performance scaling plots for TaskFlow benchmarks')
    parser.add_argument('csv_files', nargs='+', help='CSV files to process')
    parser.add_argument('--methods', nargs='+', default=['tf', 'xtf'], 
                       help='Methods to include in the analysis (default: tf xtf)')
    parser.add_argument('--output', default='performance_scaling.png',
                       help='Output file path (default: performance_scaling.pdf)')
    
    args = parser.parse_args()
    
    try:
        # Process data
        print("Loading and processing data...")
        processed_data = load_and_process_data(args.csv_files, args.methods)
        
        print(f"Processed data shape: {processed_data.shape}")
        print(f"Benchmarks found: {sorted(processed_data['benchmark'].unique())}")
        print(f"Methods found: {sorted(processed_data['method'].unique())}")
        
        # Create plot
        print("Creating performance plot...")
        create_performance_plot(processed_data, args.output)
        
    except Exception as e:
        print(f"Error: {e}")
        return 1
    
    return 0

if __name__ == "__main__":
    exit(main())

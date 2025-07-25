import polars as pl
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
import numpy as np
import sys
import os

def load_and_parse_data(csv_path):
    """Load and parse the worker stats CSV data using Polars"""
    df = pl.read_csv(csv_path)
    return df

def get_contrast_text_color(hex_color):
    """Return 'white' or 'black' depending on bar color brightness for contrast."""
    hex_color = hex_color.lstrip('#')
    r, g, b = tuple(int(hex_color[i:i+2], 16) for i in (0, 2, 4))
    brightness = (r*299 + g*587 + b*114) / 1000
    return 'black' if brightness > 160 else 'white'

def create_task_execution_locality_chart(ax, df):
    """Create horizontal bar chart for task execution locality"""
    # Prepare data for execution locality
    worker_ids = [f"Worker {row['worker_id']}" for row in df.iter_rows(named=True)]
    
    # Extract execution data
    self_exec = [row['nexec_from_self'] for row in df.iter_rows(named=True)]
    remote_exec = [row['nexec_from_remote'] for row in df.iter_rows(named=True)]
    executor_exec = [row['nexec_from_executor'] for row in df.iter_rows(named=True)]
    
    # Create horizontal stacked bar chart
    y_pos = np.arange(len(worker_ids))
    width = 0.8
    
    # Plot bars
    bars1 = ax.barh(y_pos, self_exec, width, label='Self', color='#1f77b4', alpha=0.8)
    bars2 = ax.barh(y_pos, remote_exec, width, left=self_exec, label='Remote', color='#ff7f0e', alpha=0.8)
    bars3 = ax.barh(y_pos, executor_exec, width, left=np.array(self_exec) + np.array(remote_exec), 
                    label='Executor', color='#2ca02c', alpha=0.8)
    
    # Customize the plot
    ax.set_yticks(y_pos)
    ax.set_yticklabels(worker_ids)
    ax.set_xlabel('Number of Tasks')
    ax.set_title('Task Execution Locality')
    # Deduplicate legend entries
    handles, labels = ax.get_legend_handles_labels()
    by_label = dict(zip(labels, handles))
    ax.legend(by_label.values(), by_label.keys())
    ax.grid(axis='x', alpha=0.3)
    
    # Add value labels on bars
    bar_colors = {'Self': '#1f77b4', 'Remote': '#ff7f0e', 'Executor': '#2ca02c'}
    for i, (s, r, e) in enumerate(zip(self_exec, remote_exec, executor_exec)):
        if s > 0:
            ax.text(s/2, i, f'{s:,}', ha='center', va='center', fontsize=8,
                   color=get_contrast_text_color(bar_colors['Self']), fontweight='bold',
                   bbox=dict(boxstyle='round,pad=0.2', facecolor=bar_colors['Self'], alpha=0.8, edgecolor='black'))
        if r > 0:
            ax.text(s + r/2, i, f'{r:,}', ha='center', va='center', fontsize=8,
                   color=get_contrast_text_color(bar_colors['Remote']), fontweight='bold',
                   bbox=dict(boxstyle='round,pad=0.2', facecolor=bar_colors['Remote'], alpha=0.8, edgecolor='black'))
        if e > 0:
            ax.text(s + r + e/2, i, f'{e:,}', ha='center', va='center', fontsize=8,
                   color=get_contrast_text_color(bar_colors['Executor']), fontweight='bold',
                   bbox=dict(boxstyle='round,pad=0.2', facecolor=bar_colors['Executor'], alpha=0.8, edgecolor='black'))

def create_task_queue_operations_chart_old_pushed(ax, df):
    """Create horizontal bar chart for task push operations (old format)"""
    # Prepare data for push operations
    worker_ids = [f"Worker {row['worker_id']}" for row in df.iter_rows(named=True)]
    
    # Extract push operation data
    pushed_self = [row['ntasks_pushed_self'] for row in df.iter_rows(named=True)]
    pushed_remote = [row['ntasks_pushed_remote'] for row in df.iter_rows(named=True)]
    not_pushed = [row['ntasks_not_pushed'] for row in df.iter_rows(named=True)]
    
    # Create horizontal stacked bar chart
    y_pos = np.arange(len(worker_ids))
    width = 0.8
    
    # Calculate total width needed
    max_total = max([ps + pr + np for ps, pr, np in zip(pushed_self, pushed_remote, not_pushed)])
    
    # Plot bars
    ax.barh(y_pos, pushed_self, width, left=[0]*len(worker_ids), label='Pushed Self', color='#1f77b4', alpha=0.8)
    ax.barh(y_pos, pushed_remote, width, left=pushed_self, label='Pushed Remote', color='#ff7f0e', alpha=0.8)
    ax.barh(y_pos, not_pushed, width, left=[ps + pr for ps, pr in zip(pushed_self, pushed_remote)], 
            label='Not Pushed', color='#9467bd', alpha=0.8)
    
    # Add value labels on bars
    bar_colors = {'Pushed Self': '#1f77b4', 'Pushed Remote': '#ff7f0e', 'Not Pushed': '#9467bd'}
    for i, (ps, pr, not_p) in enumerate(zip(pushed_self, pushed_remote, not_pushed)):
        if ps > 0:
            ax.text(ps/2, i, f'{ps:,}', ha='center', va='center', fontsize=8,
                   color=get_contrast_text_color(bar_colors['Pushed Self']), fontweight='bold',
                   bbox=dict(boxstyle='round,pad=0.2', facecolor=bar_colors['Pushed Self'], alpha=0.8, edgecolor='black'))
        if pr > 0:
            ax.text(ps + pr/2, i, f'{pr:,}', ha='center', va='center', fontsize=8,
                   color=get_contrast_text_color(bar_colors['Pushed Remote']), fontweight='bold',
                   bbox=dict(boxstyle='round,pad=0.2', facecolor=bar_colors['Pushed Remote'], alpha=0.8, edgecolor='black'))
        if not_p > 0:
            ax.text(ps + pr + not_p/2, i, f'{not_p:,}', ha='center', va='center', fontsize=8,
                   color=get_contrast_text_color(bar_colors['Not Pushed']), fontweight='bold',
                   bbox=dict(boxstyle='round,pad=0.2', facecolor=bar_colors['Not Pushed'], alpha=0.8, edgecolor='black'))
    
    # Customize the plot
    ax.set_yticks(y_pos)
    ax.set_yticklabels(worker_ids)
    ax.set_xlabel('Number of Tasks')
    ax.set_title('Task Push Operations')
    # Deduplicate legend entries
    handles, labels = ax.get_legend_handles_labels()
    by_label = dict(zip(labels, handles))
    ax.legend(by_label.values(), by_label.keys())
    ax.grid(axis='x', alpha=0.3)
    
    # Set x-axis limit to show full bars
    ax.set_xlim(0, max_total * 1.05)  # Add 5% padding

def create_task_queue_operations_chart_old_popped(ax, df):
    """Create horizontal bar chart for task pop operations (old format)"""
    # Prepare data for pop operations
    worker_ids = [f"Worker {row['worker_id']}" for row in df.iter_rows(named=True)]
    
    # Extract pop operation data
    popped_self = [row['ntasks_popped_self'] for row in df.iter_rows(named=True)]
    popped_remote = [row['ntasks_popped_remote'] for row in df.iter_rows(named=True)]
    
    # Create horizontal stacked bar chart
    y_pos = np.arange(len(worker_ids))
    width = 0.8
    
    # Plot bars
    ax.barh(y_pos, popped_self, width, left=[0]*len(worker_ids), label='Popped Self', color='#2ca02c', alpha=0.8)
    ax.barh(y_pos, popped_remote, width, left=popped_self, label='Popped Remote', color='#d62728', alpha=0.8)
    
    # Add value labels on bars
    bar_colors = {'Popped Self': '#2ca02c', 'Popped Remote': '#d62728'}
    for i, (ps, pr) in enumerate(zip(popped_self, popped_remote)):
        if ps > 0:
            ax.text(ps/2, i, f'{ps:,}', ha='center', va='center', fontsize=8,
                   color=get_contrast_text_color(bar_colors['Popped Self']), fontweight='bold',
                   bbox=dict(boxstyle='round,pad=0.2', facecolor=bar_colors['Popped Self'], alpha=0.8, edgecolor='black'))
        if pr > 0:
            ax.text(ps + pr/2, i, f'{pr:,}', ha='center', va='center', fontsize=8,
                   color=get_contrast_text_color(bar_colors['Popped Remote']), fontweight='bold',
                   bbox=dict(boxstyle='round,pad=0.2', facecolor=bar_colors['Popped Remote'], alpha=0.8, edgecolor='black'))
    
    # Customize the plot
    ax.set_yticks(y_pos)
    ax.set_yticklabels(worker_ids)
    ax.set_xlabel('Number of Tasks')
    ax.set_title('Task Pop Operations')
    # Deduplicate legend entries
    handles, labels = ax.get_legend_handles_labels()
    by_label = dict(zip(labels, handles))
    ax.legend(by_label.values(), by_label.keys())
    ax.grid(axis='x', alpha=0.3)

def create_task_queue_operations_chart_old_failed(ax, df):
    """Create horizontal bar chart for failed pop operations (old format)"""
    # Prepare data for failed pop operations
    worker_ids = [f"Worker {row['worker_id']}" for row in df.iter_rows(named=True)]
    
    # Extract failed pop operation data
    not_popped = [row['ntasks_not_popped'] for row in df.iter_rows(named=True)]
    
    # Create horizontal bar chart
    y_pos = np.arange(len(worker_ids))
    width = 0.8
    
    # Plot bars
    ax.barh(y_pos, not_popped, width, label='Not Popped', color='#8c564b', alpha=0.8)
    
    # Add value labels on bars
    bar_color = '#8c564b'
    for i, not_popped_val in enumerate(not_popped):
        if not_popped_val > 0:
            ax.text(not_popped_val/2, i, f'{not_popped_val:,}', ha='center', va='center', fontsize=8,
                   color=get_contrast_text_color(bar_color), fontweight='bold',
                   bbox=dict(boxstyle='round,pad=0.2', facecolor=bar_color, alpha=0.8, edgecolor='black'))
    
    # Customize the plot
    ax.set_yticks(y_pos)
    ax.set_yticklabels(worker_ids)
    ax.set_xlabel('Number of Tasks')
    ax.set_title('Failed Pop Operations')
    # Deduplicate legend entries
    handles, labels = ax.get_legend_handles_labels()
    by_label = dict(zip(labels, handles))
    ax.legend(by_label.values(), by_label.keys())
    ax.grid(axis='x', alpha=0.3)

def create_task_queue_operations_chart_new(ax, df):
    """Create horizontal bar chart for popped and stolen operations (new format)"""
    # Prepare data for queue operations
    worker_ids = [f"Worker {row['worker_id']}" for row in df.iter_rows(named=True)]
    
    # Extract queue operation data
    popped_wsq = [row['ntasks_popped_wsq'] for row in df.iter_rows(named=True)]
    stolen_remote = [row['ntasks_stolen_remote'] for row in df.iter_rows(named=True)]
    
    # Create horizontal stacked bar chart
    y_pos = np.arange(len(worker_ids))
    width = 0.8
    
    # Plot bars
    ax.barh(y_pos, popped_wsq, width, left=[0]*len(worker_ids), label='Popped WSQ', color='#2ca02c', alpha=0.8)
    ax.barh(y_pos, stolen_remote, width, left=popped_wsq, label='Stolen Remote', color='#d62728', alpha=0.8)
    
    # Add value labels on bars
    bar_colors = {'Popped WSQ': '#2ca02c', 'Stolen Remote': '#d62728'}
    for i, (pop, s) in enumerate(zip(popped_wsq, stolen_remote)):
        if pop > 0:
            ax.text(pop/2, i, f'{pop:,}', ha='center', va='center', fontsize=8,
                   color=get_contrast_text_color(bar_colors['Popped WSQ']), fontweight='bold',
                   bbox=dict(boxstyle='round,pad=0.2', facecolor=bar_colors['Popped WSQ'], alpha=0.8, edgecolor='black'))
        if s > 0:
            ax.text(pop + s/2, i, f'{s:,}', ha='center', va='center', fontsize=8,
                   color=get_contrast_text_color(bar_colors['Stolen Remote']), fontweight='bold',
                   bbox=dict(boxstyle='round,pad=0.2', facecolor=bar_colors['Stolen Remote'], alpha=0.8, edgecolor='black'))
    
    # Calculate total width needed
    max_total = max([pop + s for pop, s in zip(popped_wsq, stolen_remote)])
    
    # Customize the plot
    ax.set_yticks(y_pos)
    ax.set_yticklabels(worker_ids)
    ax.set_xlabel('Number of Tasks')
    ax.set_title('Popped and Stolen Operations')
    # Deduplicate legend entries
    handles, labels = ax.get_legend_handles_labels()
    by_label = dict(zip(labels, handles))
    ax.legend(by_label.values(), by_label.keys(), bbox_to_anchor=(1.05, 1), loc='upper left')
    ax.grid(axis='x', alpha=0.3)
    
    # Set x-axis limit to show full bars
    ax.set_xlim(0, max_total * 1.05)  # Add 5% padding

def create_load_balancing_chart(ax, df):
    """Create centered bar chart for load balancing statistics"""
    # Prepare data for load balancing
    worker_ids = [f"Worker {row['worker_id']}" for row in df.iter_rows(named=True)]
    
    # Extract load balancing data
    requests_attempted = [row['nrequests_attempted'] for row in df.iter_rows(named=True)]
    requests_sent = [row['nrequests_sent'] for row in df.iter_rows(named=True)]
    handled_attempted = [row['nhandled_attempted'] for row in df.iter_rows(named=True)]
    handled_stolen = [row['nhandled_stolen'] for row in df.iter_rows(named=True)]
    handled_not_stolen = [row['nhandled_not_stolen'] for row in df.iter_rows(named=True)]
    
    # Create horizontal bar chart with centered zero
    y_pos = np.arange(len(worker_ids))
    width = 0.6
    
    # Left side (requests) - negative values
    ax.barh(y_pos, [-r for r in requests_attempted], width, label='Requests Attempted', 
            color='#1f77b4', alpha=0.8)
    ax.barh(y_pos, [-r for r in requests_sent], width, left=[-r for r in requests_attempted], 
            label='Requests Sent', color='#ff7f0e', alpha=0.8)
    
    # Right side (handled) - positive values
    ax.barh(y_pos, handled_attempted, width, left=[0]*len(worker_ids), 
            label='Handled Attempted', color='#2ca02c', alpha=0.8)
    ax.barh(y_pos, handled_stolen, width, left=handled_attempted, 
            label='Handled Stolen', color='#d62728', alpha=0.8)
    ax.barh(y_pos, handled_not_stolen, width, 
            left=[h + s for h, s in zip(handled_attempted, handled_stolen)], 
            label='Handled Not Stolen', color='#9467bd', alpha=0.8)
    
    # Add value labels on the bars
    bar_colors = {
        'Requests Attempted': '#1f77b4',
        'Requests Sent': '#ff7f0e',
        'Handled Attempted': '#2ca02c',
        'Handled Stolen': '#d62728',
        'Handled Not Stolen': '#9467bd'
    }
    # Calculate scaling to make both sides symmetric
    # Find the maximum total bar width on both sides
    max_left = max([ra + rs for ra, rs in zip(requests_attempted, requests_sent)])  # Total left bar width
    max_right = max([h + s + n for h, s, n in zip(handled_attempted, handled_stolen, handled_not_stolen)])  # Total right bar width
    
    # Determine which side is longer and calculate scaling factor
    if max_left > max_right:
        # Left side is longer, scale right side up
        scale_factor = max_left / max_right if max_right > 0 else 1
        # Scale the right side data
        handled_attempted_scaled = [h * scale_factor for h in handled_attempted]
        handled_stolen_scaled = [s * scale_factor for s in handled_stolen]
        handled_not_stolen_scaled = [n * scale_factor for n in handled_not_stolen]
        # Keep left side as is
        requests_attempted_scaled = requests_attempted
        requests_sent_scaled = requests_sent
        max_display = max_left
    else:
        # Right side is longer, scale left side up
        scale_factor = max_right / max_left if max_left > 0 else 1
        # Scale the left side data
        requests_attempted_scaled = [r * scale_factor for r in requests_attempted]
        requests_sent_scaled = [r * scale_factor for r in requests_sent]
        # Keep right side as is
        handled_attempted_scaled = handled_attempted
        handled_stolen_scaled = handled_stolen
        handled_not_stolen_scaled = handled_not_stolen
        max_display = max_right
    
    # Recreate the bars with scaled data
    # Left side (requests) - negative values
    ax.barh(y_pos, [-r for r in requests_attempted_scaled], width, label='Requests Attempted', 
            color='#1f77b4', alpha=0.8)
    ax.barh(y_pos, [-r for r in requests_sent_scaled], width, left=[-r for r in requests_attempted_scaled], 
            label='Requests Sent', color='#ff7f0e', alpha=0.8)
    
    # Right side (handled) - positive values
    ax.barh(y_pos, handled_attempted_scaled, width, left=[0]*len(worker_ids), 
            label='Handled Attempted', color='#2ca02c', alpha=0.8)
    ax.barh(y_pos, handled_stolen_scaled, width, left=handled_attempted_scaled, 
            label='Handled Stolen', color='#d62728', alpha=0.8)
    ax.barh(y_pos, handled_not_stolen_scaled, width, 
            left=[h + s for h, s in zip(handled_attempted_scaled, handled_stolen_scaled)], 
            label='Handled Not Stolen', color='#9467bd', alpha=0.8)
    
    # Add value labels on the bars (show original values, not scaled)
    for i, (ra, rs, ha, hs, hns) in enumerate(zip(requests_attempted, requests_sent, 
                                                   handled_attempted, handled_stolen, handled_not_stolen)):
        # Left side labels (requests) - use scaled positions but original values
        if ra > 0:
            scaled_ra = requests_attempted_scaled[i]
            ax.text(-scaled_ra/2, i, f'{ra:,}', ha='center', va='center', fontsize=8, 
                   color=get_contrast_text_color(bar_colors['Requests Attempted']), fontweight='bold',
                   bbox=dict(boxstyle='round,pad=0.2', facecolor=bar_colors['Requests Attempted'], alpha=0.8, edgecolor='black'))
        if rs > 0:
            scaled_ra = requests_attempted_scaled[i]
            scaled_rs = requests_sent_scaled[i]
            ax.text(-scaled_ra - scaled_rs/2, i, f'{rs:,}', ha='center', va='center', fontsize=8, 
                   color=get_contrast_text_color(bar_colors['Requests Sent']), fontweight='bold',
                   bbox=dict(boxstyle='round,pad=0.2', facecolor=bar_colors['Requests Sent'], alpha=0.8, edgecolor='black'))
        # Right side labels (handled) - use scaled positions but original values
        right_offset = 0
        if ha > 0:
            scaled_ha = handled_attempted_scaled[i]
            ax.text(right_offset + scaled_ha/2, i, f'{ha:,}', ha='center', va='center', fontsize=8, 
                   color=get_contrast_text_color(bar_colors['Handled Attempted']), fontweight='bold',
                   bbox=dict(boxstyle='round,pad=0.2', facecolor=bar_colors['Handled Attempted'], alpha=0.8, edgecolor='black'))
            right_offset += scaled_ha
        if hs > 0:
            scaled_hs = handled_stolen_scaled[i]
            ax.text(right_offset + scaled_hs/2, i, f'{hs:,}', ha='center', va='center', fontsize=8, 
                   color=get_contrast_text_color(bar_colors['Handled Stolen']), fontweight='bold',
                   bbox=dict(boxstyle='round,pad=0.2', facecolor=bar_colors['Handled Stolen'], alpha=0.8, edgecolor='black'))
            right_offset += scaled_hs
        if hns > 0:
            scaled_hns = handled_not_stolen_scaled[i]
            ax.text(right_offset + scaled_hns/2, i, f'{hns:,}', ha='center', va='center', fontsize=8, 
                   color=get_contrast_text_color(bar_colors['Handled Not Stolen']), fontweight='bold',
                   bbox=dict(boxstyle='round,pad=0.2', facecolor=bar_colors['Handled Not Stolen'], alpha=0.8, edgecolor='black'))
            right_offset += scaled_hns
    
    # Set symmetric x-axis limits with some padding
    ax.set_xlim(-max_display * 1.1, max_display * 1.1)
    
    # Deduplicate legend entries
    handles, labels = ax.get_legend_handles_labels()
    by_label = dict(zip(labels, handles))
    ax.legend(by_label.values(), by_label.keys(), bbox_to_anchor=(1.05, 1), loc='upper left')
    
    # Customize the plot
    ax.set_yticks(y_pos)
    ax.set_yticklabels(worker_ids)
    ax.set_xlabel('Number of Requests/Handled')
    ax.set_title('Load Balancing Statistics')
    ax.grid(axis='x', alpha=0.3)
    
    # Add vertical line at x=0
    ax.axvline(x=0, color='black', linestyle='--', linewidth=2)
    
    # Add text labels for the zero line
    ax.text(0.02, 0.98, 'Requests', transform=ax.transAxes, fontsize=10, 
            verticalalignment='top', horizontalalignment='left', 
            bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.5))
    ax.text(0.98, 0.98, 'Handled', transform=ax.transAxes, fontsize=10, 
            verticalalignment='top', horizontalalignment='right', 
            bbox=dict(boxstyle='round', facecolor='lightgreen', alpha=0.5))

def create_work_stealing_chart(ax, df):
    """Create horizontal bar chart for failed operations (new format)"""
    # Prepare data for failed operations
    worker_ids = [f"Worker {row['worker_id']}" for row in df.iter_rows(named=True)]
    
    # Extract failed operation data
    not_pushed_wsq = [row['ntasks_not_pushed_wsq'] for row in df.iter_rows(named=True)]
    not_popped_wsq = [row['ntasks_not_popped_wsq'] for row in df.iter_rows(named=True)]
    not_stolen_remote = [row['ntasks_not_stolen_remote'] for row in df.iter_rows(named=True)]
    
    # Create horizontal stacked bar chart
    y_pos = np.arange(len(worker_ids))
    width = 0.8
    
    # Plot bars
    ax.barh(y_pos, not_pushed_wsq, width, left=[0]*len(worker_ids), 
            label='Not Pushed WSQ', color='#9467bd', alpha=0.8)
    ax.barh(y_pos, not_popped_wsq, width, left=not_pushed_wsq, 
            label='Not Popped WSQ', color='#8c564b', alpha=0.8)
    ax.barh(y_pos, not_stolen_remote, width, left=[not_p + npo for not_p, npo in zip(not_pushed_wsq, not_popped_wsq)], 
            label='Not Stolen Remote', color='#e377c2', alpha=0.8)
    
    # Add value labels on bars
    bar_colors = {'Not Pushed WSQ': '#9467bd', 'Not Popped WSQ': '#8c564b', 'Not Stolen Remote': '#e377c2'}
    for i, (not_p, npo, ns) in enumerate(zip(not_pushed_wsq, not_popped_wsq, not_stolen_remote)):
        if not_p > 0:
            ax.text(not_p/2, i, f'{not_p:,}', ha='center', va='center', fontsize=8,
                   color=get_contrast_text_color(bar_colors['Not Pushed WSQ']), fontweight='bold',
                   bbox=dict(boxstyle='round,pad=0.2', facecolor=bar_colors['Not Pushed WSQ'], alpha=0.8, edgecolor='black'))
        if npo > 0:
            ax.text(not_p + npo/2, i, f'{npo:,}', ha='center', va='center', fontsize=8,
                   color=get_contrast_text_color(bar_colors['Not Popped WSQ']), fontweight='bold',
                   bbox=dict(boxstyle='round,pad=0.2', facecolor=bar_colors['Not Popped WSQ'], alpha=0.8, edgecolor='black'))
        if ns > 0:
            ax.text(not_p + npo + ns/2, i, f'{ns:,}', ha='center', va='center', fontsize=8,
                   color=get_contrast_text_color(bar_colors['Not Stolen Remote']), fontweight='bold',
                   bbox=dict(boxstyle='round,pad=0.2', facecolor=bar_colors['Not Stolen Remote'], alpha=0.8, edgecolor='black'))
    
    # Calculate total width needed
    max_total = max([not_p + npo + ns for not_p, npo, ns in zip(not_pushed_wsq, not_popped_wsq, not_stolen_remote)])
    
    # Customize the plot
    ax.set_yticks(y_pos)
    ax.set_yticklabels(worker_ids)
    ax.set_xlabel('Number of Tasks')
    ax.set_title('Failed Operations')
    # Deduplicate legend entries
    handles, labels = ax.get_legend_handles_labels()
    by_label = dict(zip(labels, handles))
    ax.legend(by_label.values(), by_label.keys())
    ax.grid(axis='x', alpha=0.3)
    
    # Set x-axis limit to show full bars
    ax.set_xlim(0, max_total * 1.05)  # Add 5% padding

def detect_csv_format(df):
    """Detect the CSV format based on column headers"""
    columns = df.columns
    
    # Check for new format indicators
    if 'ntasks_popped_wsq' in columns and 'ntasks_stolen_remote' in columns:
        return 'new'
    # Check for old format indicators
    elif 'ntasks_pushed_self' in columns and 'nrequests_attempted' in columns:
        return 'old'
    else:
        # Default to old format if uncertain
        return 'old'

def create_combined_visualization(csv_path):
    """Create a combined visualization with all three subplots"""
    # Load data
    df = load_and_parse_data(csv_path)
    
    # Detect CSV format
    format_type = detect_csv_format(df)
    print(f"Detected CSV format: {format_type}")
    
    if format_type == 'new':
        # Create figure with 3 subplots for new format
        fig, (ax1, ax2, ax3) = plt.subplots(3, 1, figsize=(14, 16))
        
        # Create each subplot
        create_task_execution_locality_chart(ax1, df)
        create_task_queue_operations_chart_new(ax2, df)
        create_work_stealing_chart(ax3, df)
        
    else:  # old format
        # Create figure with 5 subplots for old format
        fig, (ax1, ax2, ax3, ax4, ax5) = plt.subplots(5, 1, figsize=(18, 20))
        
        # Create each subplot
        create_task_execution_locality_chart(ax1, df)
        create_task_queue_operations_chart_old_pushed(ax2, df)
        create_task_queue_operations_chart_old_popped(ax3, df)
        create_task_queue_operations_chart_old_failed(ax4, df)
        create_load_balancing_chart(ax5, df)
    
    # Adjust layout to prevent overlap
    plt.tight_layout()
    
    return fig

def main():
    """Main function to run the visualization"""
    # Get command line arguments
    if len(sys.argv) > 1:
        output_filename = sys.argv[1]
        # Remove extension if provided and add .png
        base_name = os.path.splitext(output_filename)[0]
        output_filename = f"charts/{base_name}.png"
    else:
        output_filename = "worker_stats_visualization.png"
    
    # Get CSV path from second optional argument
    if len(sys.argv) > 2:
        csv_folder = sys.argv[2]
        csv_path = os.path.join(csv_folder, "worker_stats.csv")
    else:
        print("No CSV folder provided, using default path")
        csv_path = "tf_stats/worker_stats.csv"  # Default path
    
    try:
        # Set matplotlib style for better appearance
        plt.style.use('default')
        
        # Create combined visualization
        fig = create_combined_visualization(csv_path)
        
        # Save as PNG with the specified filename
        fig.savefig(output_filename, dpi=300, bbox_inches='tight', 
                   format='png', facecolor='white', edgecolor='none')
        print(f"Visualization saved as '{output_filename}'")
        
        # Show the plot
        # plt.show()
        
    except Exception as e:
        print(f"Error creating visualization: {e}")
        import traceback
        traceback.print_exc()

if __name__ == "__main__":
    main()

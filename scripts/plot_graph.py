#!/usr/bin/env python

import argparse
import json
import math
import matplotlib.pyplot
import os
import re
import sys

dpi = 163
width = 3840
height = 2160

bar_label_fontsize = 9
legend_fontsize = 20
ticks_fontsize = 20
title_fontsize = 28
xlabel_fontsize = 20
ylabel_fontsize = 28

bars_width = 0.90

# Mapping from a parse benchmark name to the associated human readable parser name.
parse_benchmark_parser_names = { \
    'BM_ParseHapply' : 'hapPLY', \
    'BM_ParseMiniply' : 'miniply', \
    'BM_ParseMshPly' : 'msh_ply', \
    'BM_ParseNanoPly' : 'nanoply', \
    'BM_ParsePlywoot' : 'PLYwoot', \
    'BM_ParsePlyLib' : 'plylib', \
    'BM_ParseRPly' : 'RPly', \
    'BM_ParseTinyply' : 'tinyply 2.3'
}

# Mapping from a write benchmark name to the associated human readable parser name.
write_benchmark_parser_names = { \
    'BM_WriteHapply' : 'hapPLY', \
    'BM_WriteMshPly' : 'msh_ply', \
    'BM_WriteNanoPly' : 'nanoply', \
    'BM_WritePlywoot' : 'PLYwoot', \
    'BM_WriteRPly' : 'RPly', \
    'BM_WriteTinyply' : 'tinyply 2.3' \
}

def legend_without_duplicate_labels(ax, **kwargs):
    handles, labels = ax.get_legend_handles_labels()
    unique = [(h, l) for i, (h, l) in enumerate(zip(handles, labels)) if l not in labels[:i]]
    ax.legend(*zip(*unique), **kwargs)

def machine_info_text_box(ax):
    for line in open('/proc/cpuinfo', 'r').readlines():
        m = re.match('^model name.*:(.*)$', line)
        if m:
            cpu = m.group(1).strip()
    for line in open('/proc/meminfo', 'r').readlines():
        m = re.match('^MemTotal.*:(.*)$', line)
        if m:
            memory = m.group(1).strip()

    sysname = os.uname().sysname

    text_length = 1.9 * max(len(cpu), len(memory), len(sysname))
    fontsize = legend_fontsize * 0.9

    # Draw two different text boxes, to be able to control alignment.
    heading_text = matplotlib.offsetbox.AnchoredText(('CPU:\nMemory:\nOS:{:%d}' % text_length).format(' '), prop={'fontweight': 'demibold', 'fontsize': fontsize * 0.8}, borderpad=0.8, loc='upper center')
    heading_text.patch.set_boxstyle('round')
    heading_text.patch.set_facecolor('blanchedalmond')
    heading_text.patch.set_edgecolor('orange')

    info_text = matplotlib.offsetbox.AnchoredText('%s\n%s\n%s' % (cpu, memory, sysname), prop={'fontsize': fontsize * 0.8, 'ha': 'right'}, borderpad=0.8, loc='upper center')
    info_text.patch.set_alpha(0.0)

    ax.add_artist(heading_text)
    ax.add_artist(info_text)

def render_graph(benchmarks, output_png_file, benchmark_metric, benchmark_parser_names, title, ylabel, metrics_reversed=False):
    # Mapping from a model name to a tuple of the parser name and the time
    # required to parse the model by that parser.
    metrics_by_model = {}

    # Create a mapping, from PLY file to parser benchmark results.
    for benchmark in benchmarks:
        # Extract the model name, and the parser name.
        benchmark_name, _, model_name = benchmark['name'].partition('/')

        # Strip off quotes from the model name, and split put the model format type
        # on a separate line.
        model_name = '\n('.join(model_name.strip('"').split(' ('))

        if not model_name in metrics_by_model:
            metrics_by_model[model_name] = []

        if benchmark_name in benchmark_parser_names:
            parser_name = benchmark_parser_names[benchmark_name]
            metrics_by_model[model_name].append((parser_name, float('NaN') if 'error_occurred' in benchmark else benchmark[benchmark_metric]))

    # List of 3D model names, sorted on the format type first, model name second.
    model_names = sorted(list([model_name for model_name, metrics in metrics_by_model.items() if metrics]), key=lambda n: tuple(reversed(n.split('\n'))))

    fig, ax = matplotlib.pyplot.subplots(figsize=(width / dpi, height / dpi))

    prop_cycle = matplotlib.pyplot.rcParams['axes.prop_cycle']
    colors = prop_cycle.by_key()['color']

    # Map a parser name on a color.
    parser_color = {parser_name: colors[idx] for idx, parser_name in enumerate(benchmark_parser_names.values())}

    bar_width = bars_width / len(benchmark_parser_names)
    bar_offset = (1.0 - bars_width) * 0.5 - 0.5 * bars_width

    for idx, model_name in enumerate(model_names):
        # Sort metrics for the current model.
        model_metrics = sorted(metrics_by_model[model_name], key=lambda t : (float('+inf') if math.isnan(t[1]) else t[1]) * (-1 if metrics_reversed else 1))

        parser_names = [parser_name for parser_name, _ in model_metrics]
        metrics = [metric for _, metric in model_metrics]
        x_offsets = [idx + bar_offset + i * bar_width for i in range(len(metrics))]

        rect = ax.bar(x_offsets, metrics, bar_width, color=[parser_color[parser_name] for parser_name in parser_names], label=parser_names)
        ax.bar_label(rect, fmt='%.2f', padding=3, fontsize=bar_label_fontsize, rotation=70)

    ax.set_title(title, fontsize=title_fontsize)
    ax.set_xticks(list(range(len(model_names))), labels=model_names)
    ax.set_ylabel(ylabel, fontsize=ylabel_fontsize)
    ax.tick_params(axis='both', labelsize=ticks_fontsize)
    ax.tick_params(axis='x', length=0)

    legend_without_duplicate_labels(ax, fontsize=legend_fontsize)

    fig.tight_layout()

    matplotlib.pyplot.savefig(output_png_file, format='png', dpi=dpi)

def render_parse_cpu_time_graph(benchmarks, output_png_file):
    time_unit = None if not benchmarks else benchmarks[0]['time_unit']
    render_graph(benchmarks,
                 output_png_file,
                 'cpu_time',
                 parse_benchmark_parser_names,
                 'Average CPU time parsing triangle mesh models [ms] (lower is better)',
                 'CPU time [%s]' % time_unit
    )

def render_write_cpu_time_graph(benchmarks, output_png_file):
    time_unit = None if not benchmarks else benchmarks[0]['time_unit']
    render_graph(benchmarks,
                 output_png_file,
                 'cpu_time',
                 write_benchmark_parser_names,
                 'Average CPU time writing random mesh data [ms] (lower is better)',
                 'CPU time [%s]' % time_unit
    )

def render_parse_transfer_speed_graph(benchmarks, output_png_file):
    for benchmark in benchmarks:
        benchmark['mib_per_second'] = float('NaN') if 'error_occurred' in benchmark else benchmark['bytes_per_second'] / (1024 * 1024)

    render_graph(benchmarks,
                 output_png_file,
                 'mib_per_second',
                 parse_benchmark_parser_names,
                 'Data transfer speeds parsing various models [MiB/s] (higher is better)',
                 'Read performance [MiB/s]',
                 metrics_reversed=True
    )

def render_write_transfer_speed_graph(benchmarks, output_png_file):
    for benchmark in benchmarks:
        benchmark['mib_per_second'] = float('NaN') if 'error_occurred' in benchmark else benchmark['bytes_per_second'] / (1024 * 1024)

    render_graph(benchmarks,
                 output_png_file,
                 'mib_per_second',
                 write_benchmark_parser_names,
                 'Data transfer speeds writing uniform triangle meshes [MiB/s] (higher is better)',
                 'Write performance [MiB/s]',
                 metrics_reversed=True
    )

if __name__ == '__main__':
    parser = argparse.ArgumentParser(
            prog='plot_graph.py',
            description='Plots various graphs given the JSON output generated by PLYbench.')

    graph_type_choices = ['parse_cpu_time', 'write_cpu_time', 'parse_transfer_speed', 'write_transfer_speed']

    parser.add_argument('-i', '--input',
                        help='input JSON file generated by PLYbench, in case this is not specified, stdin is used instead')
    parser.add_argument('-o', '--output',
                        help='output PNG file containing the requested graph, in case this is not specified, outputs to stdout instead')
    parser.add_argument('-t', '--type',
                        help='graph type, may be any one of [%s], default: %s' % (','.join(graph_type_choices), graph_type_choices[0]),
                        default=graph_type_choices[0],
                        choices=graph_type_choices)

    args = parser.parse_args()

    benchmarks = None
    with open(args.input, 'r') if args.input is not None else sys.stdin as json_file:
        benchmarks = json.load(json_file)['benchmarks']

    if not benchmarks:
        print("Problem loading benchmark data, invalid JSON?")
        sys.exit(1)

    with open(args.output, 'wb') if args.output is not None else sys.stdout as png_file:
        if args.type == 'parse_cpu_time':
            render_parse_cpu_time_graph(benchmarks, png_file)
        elif args.type == 'write_cpu_time':
            render_write_cpu_time_graph(benchmarks, png_file)
        elif args.type == 'parse_transfer_speed':
            render_parse_transfer_speed_graph(benchmarks, png_file)
        elif args.type == 'write_transfer_speed':
            render_write_transfer_speed_graph(benchmarks, png_file)

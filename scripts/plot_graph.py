#!/usr/bin/env python

import argparse
import json
import matplotlib.pyplot
import numpy
import subprocess
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

def render_graph(benchmarks, output_png_file, benchmark_metric, benchmark_parser_names, title, ylabel):
    # Mapping from a parser name and model name to the time required to parse that
    # model by that parser.
    cpu_times_by_parser_and_model = {}

    # Create a mapping, from PLY file to parser benchmark results.
    for benchmark in benchmarks:
        # Extract the model name, and the parser name.
        benchmark_name, _, model_name = benchmark['name'].partition('/')

        # Strip off quotes from the model name, and split put the model format type
        # on a separate line.
        model_name = '\n('.join(model_name.strip('"').split(' ('))

        if benchmark_name in benchmark_parser_names:
            cpu_times_by_parser_and_model[(benchmark_name, model_name)] = benchmark[benchmark_metric]

    # List of 3D model names, sorted on the format type first, model name second.
    model_names = sorted(list(set([model_name for _, model_name in cpu_times_by_parser_and_model.keys()])), key=lambda n: tuple(reversed(n.split('\n'))))

    fig, ax = matplotlib.pyplot.subplots(figsize=(width / dpi, height / dpi))
    x = numpy.arange(len(model_names))

    bar_width = bars_width / len(benchmark_parser_names)
    offset = -0.5 * bars_width

    for benchmark_name, parser_name in benchmark_parser_names.items():
        cpu_times = [cpu_times_by_parser_and_model.get((benchmark_name, model_name), float("NaN")) for model_name in model_names]
        rect = ax.bar(x + offset, cpu_times, bar_width, label=parser_name)
        offset += bar_width
        ax.bar_label(rect, fmt='%.2f', padding=3, fontsize=bar_label_fontsize, rotation=70)

    ax.set_title(title, fontsize=title_fontsize)
    ax.set_xticks(x, model_names)
    ax.set_ylabel(ylabel, fontsize=ylabel_fontsize)
    ax.tick_params(axis='both', labelsize=ticks_fontsize)
    ax.legend(fontsize=legend_fontsize)

    fig.tight_layout()

    matplotlib.pyplot.savefig(output_png_file, format='png', dpi=dpi)

def render_parse_cpu_time_graph(benchmarks, output_png_file):
    time_unit = None if not benchmarks else benchmarks[0]['time_unit']
    render_graph(benchmarks,
                 output_png_file,
                 'cpu_time',
                 parse_benchmark_parser_names,
                 'Average CPU time [%s] spent parsing various models by different PLY parser libraries' % time_unit,
                 'CPU time [%s]' % time_unit
    )

def render_write_cpu_time_graph(benchmarks, output_png_file):
    time_unit = None if not benchmarks else benchmarks[0]['time_unit']
    render_graph(benchmarks,
                 output_png_file,
                 'cpu_time',
                 write_benchmark_parser_names,
                 'Average CPU time [%s] spent writing random mesh data per output PLY format type for different PLY parser libraries' % time_unit,
                 'CPU time [%s]' % time_unit
    )

def render_parse_bytes_per_second_graph(benchmarks, output_png_file):
    for benchmark in benchmarks:
        benchmark['mib_per_second'] = benchmark['bytes_per_second'] / (1024 * 1024)

    render_graph(benchmarks,
                 output_png_file,
                 'mib_per_second',
                 parse_benchmark_parser_names,
                 'Read performance [MiB/s] for different models and PLY parser libraries',
                 'Read performance [MiB/s]'
    )

def render_write_bytes_per_second_graph(benchmarks, output_png_file):
    for benchmark in benchmarks:
        benchmark['mib_per_second'] = benchmark['bytes_per_second'] / (1024 * 1024)

    render_graph(benchmarks,
                 output_png_file,
                 'mib_per_second',
                 write_benchmark_parser_names,
                 'Write performance [MiB/s] per output PLY format type for different PLY parser libraries',
                 'Write performance [MiB/s]'
    )

if __name__ == '__main__':
    parser = argparse.ArgumentParser(
            prog='plot_graph.py',
            description='Plots various graphs given the JSON output generated by PLYbench.')

    graph_type_choices = ['parse_cpu_time', 'write_cpu_time', 'parse_bytes_per_second', 'write_bytes_per_second']

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
        elif args.type == 'parse_bytes_per_second':
            render_parse_bytes_per_second_graph(benchmarks, png_file)
        elif args.type == 'write_bytes_per_second':
            render_write_bytes_per_second_graph(benchmarks, png_file)

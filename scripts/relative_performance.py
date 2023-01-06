#!/usr/bin/env python

import argparse
import json
import sys

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

def print_relative_performance(benchmarks, benchmark_parser_names, format_type_fn):
    # Mapping from a parser name and model name to the time required to parse that
    # model by that parser.
    cpu_time_by_parser = {}
    cpu_time_by_model_and_parser = {}
    cpu_time_by_format_type_and_parser = {}

    for benchmark in benchmarks:
        benchmark_name, _, model_name = benchmark['name'].partition('/')

        if benchmark_name in benchmark_parser_names:
            format_type = format_type_fn(model_name)

            if format_type not in cpu_time_by_format_type_and_parser:
                cpu_time_by_format_type_and_parser[format_type] = dict()

            cpu_time = benchmark['cpu_time']
            parser_name = benchmark_parser_names[benchmark_name]
            cpu_time_by_format_type_and_parser[format_type][parser_name] = cpu_time_by_parser.get(parser_name, 0) + cpu_time

    relative_performance_by_format_type_and_parser = {}
    for format_type, cpu_time_by_parser in cpu_time_by_format_type_and_parser.items():
        relative_performance_by_format_type_and_parser[format_type] = dict()
        min_cpu_time = min(cpu_time_by_parser.values())
        for parser_name, cpu_time in cpu_time_by_parser.items():
            relative_performance_by_format_type_and_parser[format_type][parser_name] = cpu_time / min_cpu_time

    for format_type, relative_performance_by_parser in relative_performance_by_format_type_and_parser.items():
        print('%s' % format_type)
        print('-' * len(format_type) + '\n')
        for parser_name, relative_performance in sorted(relative_performance_by_parser.items(), key=lambda item: item[1]):
            print('%s: %.2f' % (parser_name, relative_performance))
        print()

if __name__ == '__main__':
    parser = argparse.ArgumentParser(
            prog='plot_graph.py',
            description='Plots various graphs given the JSON output generated by PLYbench.')

    benchmark_choices = ['parse', 'write']

    parser.add_argument('-i', '--input',
                        help='input JSON file generated by PLYbench, in case this is not specified, stdin is used instead')
    parser.add_argument('-t', '--type',
                        help='benchmark type, may be any one of [%s], default: %s' % (','.join(benchmark_choices), benchmark_choices[0]),
                        default=benchmark_choices[0],
                        choices=benchmark_choices)

    args = parser.parse_args()

    benchmarks = None
    with open(args.input, 'r') if args.input is not None else sys.stdin as json_file:
        benchmarks = json.load(json_file)['benchmarks']

    if not benchmarks:
        print("Problem loading benchmark data, invalid JSON?")
        sys.exit(1)

    if args.type == 'parse':
        print_relative_performance(benchmarks, parse_benchmark_parser_names, lambda model_name : model_name.strip('"').split(' (')[1][:-1])
    elif args.type == 'write':
        print_relative_performance(benchmarks, write_benchmark_parser_names, lambda model_name : model_name.strip('"'))

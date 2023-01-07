#!/usr/bin/env python

import argparse
import json
import string
import sys

library_url = {
    'hapPLY': 'https://github.com/nmwsharp/happly',
    'miniply': 'https://github.com/vilya/miniply',
    'msh_ply': 'https://github.com/mhalber/msh',
    'nanoply': 'https://github.com/cnr-isti-vclab/vcglib/tree/main/wrap/nanoply',
    'plylib': 'https://github.com/cnr-isti-vclab/vcglib/tree/main/wrap/ply',
    'PLYwoot': 'https://github.com/ton/plywoot',
    'RPly': 'https://w3.impa.br/~diego/software/rply',
    'tinyply 2.3': 'https://github.com/ddiakopoulos/tinyply',
}

# Mapping from a parse benchmark name to the associated human readable library name.
parse_benchmark_library_names = { \
    'BM_ParseHapply' : 'hapPLY', \
    'BM_ParseMiniply' : 'miniply', \
    'BM_ParseMshPly' : 'msh_ply', \
    'BM_ParseNanoPly' : 'nanoply', \
    'BM_ParsePlywoot' : 'PLYwoot', \
    'BM_ParsePlyLib' : 'plylib', \
    'BM_ParseRPly' : 'RPly', \
    'BM_ParseTinyply' : 'tinyply 2.3'
}

# Mapping from a write benchmark name to the associated human readable library name.
write_benchmark_library_names = { \
    'BM_WriteHapply' : 'hapPLY', \
    'BM_WriteMshPly' : 'msh_ply', \
    'BM_WriteNanoPly' : 'nanoply', \
    'BM_WritePlywoot' : 'PLYwoot', \
    'BM_WriteRPly' : 'RPly', \
    'BM_WriteTinyply' : 'tinyply 2.3' \
}

def make_results_table(benchmarks, benchmark_library_names, format_type_fn):
    # Mapping from a library name and model name to the time required to parse
    # that model by that library.
    cpu_time_by_library = {}
    cpu_time_by_model_and_library = {}
    cpu_time_by_format_type_and_library = {}

    format_types = set()
    models_per_format_type = dict()

    for benchmark in benchmarks:
        benchmark_name, _, model_name = benchmark['name'].partition('/')

        if benchmark_name in benchmark_library_names:
            format_type = format_type_fn(model_name)

            if format_type not in format_types:
                format_types.add(format_type)
                models_per_format_type[format_type] = 1
                cpu_time_by_format_type_and_library[format_type] = dict()
            else:
                models_per_format_type[format_type] += 1

            cpu_time = benchmark['cpu_time']
            library = benchmark_library_names[benchmark_name]
            cpu_time_by_format_type_and_library[format_type][library] = cpu_time_by_library.get(library, 0) + cpu_time

    relative_performance_by_format_type_and_library = {}
    for format_type, cpu_time_by_library in cpu_time_by_format_type_and_library.items():
        relative_performance_by_format_type_and_library[format_type] = dict()
        min_cpu_time = min(cpu_time_by_library.values())
        for library, cpu_time in cpu_time_by_library.items():
            relative_performance_by_format_type_and_library[format_type][library] = cpu_time / min_cpu_time

    # Calculate an overall relative performance, by weighting the performance
    # per format type according to the number of models for that format type.
    overall_relative_performance_by_library = dict()
    for format_type, relative_performance_by_library in relative_performance_by_format_type_and_library.items():
        for library, relative_performance in relative_performance_by_library.items():
            overall_relative_performance_by_library[library] = overall_relative_performance_by_library.get(library, []) + models_per_format_type[format_type] * [relative_performance]

    for library, relative_performance in overall_relative_performance_by_library.items():
        overall_relative_performance_by_library[library] = sum(relative_performance) / len(relative_performance)

    # Normalize...?
    fastest_overall = min(overall_relative_performance_by_library.values())
    for library, relative_performance in overall_relative_performance_by_library.items():
        overall_relative_performance_by_library[library] /= fastest_overall

    # Calculate the amount of characters required for the library name column. A
    # library name is printed as a link: [name](url).
    library_name_columns = 0
    for name, url in library_url.items():
        library_name_columns = max(library_name_columns, len(name) + len(url) + len(' []() '))

    # The text displayed per column is of the form 'Nx slower', where N is a
    # fractional value, assuming it fits in the format xxx.xx.
    max_columns = len(' 100.00x slower ')

    format_types = list(sorted(format_types))
    format_type_columns = []
    for format_type in format_types:
        format_type_columns.append(max(max_columns, len(format_type) + 2))

    ranking_columns = 3
    overall_columns = max_columns
    columns = [ranking_columns, library_name_columns, overall_columns] + format_type_columns

    # Print the table heading. The first column is used to display a ranking.
    format_types_table_header = ''
    for format_type, width in zip(format_types, format_type_columns):
        format_types_table_header += '{:{width}}|'.format(' %s' % (format_type[0].upper() + format_type[1:]), width=width)

    result = '|{}|{:{x}}|{:{y}}|{}\n'.format(' # ', ' Library name ', ' Overall ', format_types_table_header, x=library_name_columns, y=overall_columns)
    result += '|:' + ':|:'.join(['{:{width}}'.format('-' * (column - 2), width=column-2) for column in columns]) + ':|\n';

    rank = 1
    for library, relative_performance in sorted(overall_relative_performance_by_library.items(), key=lambda item : item[1]):
        library_text = f'[{library}]({library_url[library]})'
        performance_text = '*1.00*' if relative_performance == 1.0 else f'{relative_performance:.2f}x slower'
        result += f"| {rank} | {library_text:{library_name_columns - 2}} | {performance_text:{overall_columns - 2}} "

        for format_type, width in zip(format_types, format_type_columns):
            if library not in relative_performance_by_format_type_and_library[format_type]:
                result += f'| {"N/A":{width - 2}} '
            else:
                relative_performance = relative_performance_by_format_type_and_library[format_type][library]
                performance_text = '*1.00*' if relative_performance == 1.0 else f'{relative_performance:.2f}x slower'
                result += f'| {performance_text:{width - 2}} '

        result += '|\n'
        rank += 1

    return result

if __name__ == '__main__':
    parser = argparse.ArgumentParser(
            prog='generate_readme.py',
            description='Generates README.md for the PLYbench repository, given Google Benchmark results in JSON format.')

    parser.add_argument('-i', '--input',
                        help='input JSON file generated by PLYbench, in case this is not specified, stdin is used instead')

    args = parser.parse_args()

    benchmarks = None
    with open(args.input, 'r') if args.input is not None else sys.stdin as json_file:
        benchmarks = json.load(json_file)['benchmarks']

    if not benchmarks:
        print('Problem loading benchmark data, invalid JSON?')
        sys.exit(1)

    template = None
    with open('README.md.in', 'r') as template_file:
        template = string.Template(template_file.read())

    if not template:
        print('Problem opening template file "README.md.in", this script needs to be executed from the PLYbench root directory.')

    with open('README.md', 'w+') as readme_file:
        readme_file.write(template.substitute({
            'parse_results_table' : make_results_table(benchmarks, parse_benchmark_library_names, lambda model_name : model_name.strip('"').split(' (')[1][:-1]),
            'write_results_table' : make_results_table(benchmarks, write_benchmark_library_names, lambda model_name : model_name.strip('"')),
        }))

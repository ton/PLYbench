#!/usr/bin/env python

import json
import matplotlib.pyplot
import numpy
import subprocess
import sys

dpi = 163
width = 3840
height = 2160

bar_label_fontsize = 12
legend_fontsize = 20
ticks_fontsize = 20
title_fontsize = 28
xlabel_fontsize = 20
ylabel_fontsize = 28

bars_width = 0.85

benchmark_data = json.load(sys.stdin)
benchmarks = benchmark_data['benchmarks']
if not benchmarks:
    sys.exit(1)

# Mapping from a parser name and model name to the time required to parse that
# model by that parser.
cpu_times_by_parser_and_model = {}
time_unit = None

# Create a mapping, from PLY file to parser benchmark results.
for benchmark in benchmarks:
    time_unit = benchmark['time_unit']

    # Extract the model name, and the parser name.
    benchmark_name, _, model_name = benchmark['name'].partition('/')

    # Strip off the (" and ") pre- and postfix from the model name.
    model_name = '\n('.join(model_name[2:-2].split(' ('))

    cpu_times_by_parser_and_model[(benchmark_name, model_name)] = benchmark['cpu_time']

# Mapping from a raw benchmark name to a human readable parser name.
benchmark_parser_names = {'BM_ParseHapply' : 'hapPLY', 'BM_ParseMiniply' : 'miniply', 'BM_ParseMshPly' : 'msh_ply', 'BM_ParseNanoPly' : 'nanoply', 'BM_ParsePlywoot' : 'PLYwoot'}

# List of 3D model names, sorted on the format type first, model name second.
model_names = sorted(list(set([model_name for _, model_name in cpu_times_by_parser_and_model.keys()])), key=lambda n: tuple(reversed(n.split('\n'))))

fig, ax = matplotlib.pyplot.subplots(figsize=(width / dpi, height / dpi))
x = numpy.arange(len(model_names))

bar_width = bars_width / len(benchmark_parser_names)
offset = -0.5 * bars_width

for benchmark_name, parser_name in benchmark_parser_names.items():
    cpu_times = [cpu_times_by_parser_and_model[(benchmark_name, model_name)] for model_name in model_names]
    rect = ax.bar(x + offset, cpu_times, bar_width, label=parser_name)
    offset += bar_width
    ax.bar_label(rect, fmt='%.2f', padding=3, fontsize=bar_label_fontsize)

ax.set_title('CPU time [%s] by model and parser' % time_unit, fontsize=title_fontsize)
ax.set_xticks(x, model_names)
ax.set_ylabel('CPU time [%s]' % time_unit, fontsize=ylabel_fontsize)
ax.tick_params(axis='both', labelsize=ticks_fontsize)
ax.legend(fontsize=legend_fontsize)

fig.tight_layout()

matplotlib.pyplot.savefig(sys.stdout, format='png', dpi=dpi, transparent=True)

#!/bin/sh

# This script will:
#
#   1. run the benchmarks and store the results in a temporary JSON file
#   2. process the JSON file to regenerate the graphs in assets/
#   3. process the JSON file to update README.md

die() { echo "$*" 1>&2; exit 1; }

command -v build/plybench > /dev/null || die "Please first build PLYbench using './configure && ninja -C build'."

json_file=$(mktemp)

build/plybench --benchmark_out="$json_file" --benchmark_out_format=json --benchmark_color=true

# Generating graphs.
scripts/plot_graph.py -i "$json_file" -o assets/parse_cpu_time.png -t parse_cpu_time
scripts/plot_graph.py -i "$json_file" -o assets/write_cpu_time.png -t write_cpu_time
scripts/plot_graph.py -i "$json_file" -o assets/parse_transfer_speed.png -t parse_transfer_speed
scripts/plot_graph.py -i "$json_file" -o assets/write_transfer_speed.png -t write_transfer_speed

# Update README.md.
scripts/generate_readme.py -i "$json_file"

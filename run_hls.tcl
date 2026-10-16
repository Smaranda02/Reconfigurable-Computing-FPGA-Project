# SPDX-License-Identifier: MIT
# Copyright (c) 2026 ARH Laboratory
# Author: Alexandru Ulmamei <alexandru.ulmamei@upb.ro>

set project_name "merge_sort_naive"
set solution_name "sol1"
set part_name "xc7z020clg400-1"
set clock_period_ns 10

set stage "all"
foreach arg $argv {
  if { $arg eq "csim" || $arg eq "csynth" || $arg eq "export" || $arg eq "all" } {
    set stage $arg
    break
  }
}

open_project -reset "${project_name}"
set_top merge_sort_naive

add_files src/merge_sort_naive.cpp
add_files src/merge_sort_naive.hpp
add_files -tb tb/tb_merge_sort_naive.cpp

open_solution -reset "${solution_name}"
set_part ${part_name}
create_clock -period ${clock_period_ns} -name default

if { $stage eq "csim" } {
  csim_design
} elseif { $stage eq "csynth" } {
  csynth_design
} elseif { $stage eq "export" } {
  csynth_design
  export_design -format ip_catalog -output build/export
} elseif { $stage eq "all" } {
  csim_design
  csynth_design
  export_design -format ip_catalog -output build/export
} else {
  puts "ERROR: Unknown stage '${stage}'. Use one of: csim, csynth, export, all"
  puts "INFO: argv was: $argv"
  exit 1
}

exit

block_size := 64
L1_size := 1024
L1_assoc := 2
L2_size := 65536
L2_assoc := 8
files := `find ./memory_trace_files -mindepth 1 | sort`
num := 1

compile: cache_simulate.hpp cache_simulate.cpp
	g++ cache_simulate.cpp cache_simulate.hpp -o cache_simulate

run: cache_simulate.hpp cache_simulate.cpp
	g++ cache_simulate.cpp cache_simulate.hpp -o cache_simulate
	./cache_simulate $(block_size) $(L1_size) $(L1_assoc) $(L2_size) $(L2_assoc) $(files)

execute: cache_simulate.hpp cache_simulate.cpp
	g++ cache_simulate.cpp cache_simulate.hpp -o cache_simulate
	./cache_simulate $(block_size) $(L1_size) $(L1_assoc) $(L2_size) $(L2_assoc) ./memory_trace_files/trace$(num).txt
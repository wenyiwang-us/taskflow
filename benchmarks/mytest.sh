./benchmarks.py -m tf omp tbb \
                   -b graph_traversal wavefront \
                   -t 1 4 8 16 \
                   -r 10 \
                   -p true \
                   -o result.png

#!/bin/bash
mkdir results
./driver -trace bzip2.log_l1misstrace -parts 2  >> results/bzip2 &
./driver -trace gcc.log_l1misstrace -parts 2 >> results/gcc &
./driver -trace gromacs.log_l1misstrace -parts 1 >> results/gromacs &
./driver -trace h264ref.log_l1misstrace -parts 1 >> results/h264ref &
./driver -trace hmmer.log_l1misstrace -parts 1 >> results/hmmer &
./driver -trace sphinx3.log_l1misstrace -parts 2 >> results/sphinx3 &


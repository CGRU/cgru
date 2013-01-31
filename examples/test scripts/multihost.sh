#!/bin/bash

#./job.sh --mhmin 3 --mhmax 5 --mhwaitmax 5
./job.sh --mhmin 3 --mhmax 5 --mhwaitmax 5 --mhwaitsrv 1 --mhsame 0 --mhservice 'sleep 10'


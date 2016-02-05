#!/bin/bash

./job.sh --pkp 0 --mhmin 3 --mhmax 5 --mhwaitmax 5 --mhignorelost 1 "$@"

#./job.sh --pkp 0 --mhmin 3 --mhmax 5 --mhwaitmax 5 --mhwaitsrv 1 --mhsame 0 --mhignorelost 1 --mhservice 'sleep 10000' "$@"


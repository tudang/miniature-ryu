#!/bin/bash

ncol=$(awk '{print NF; exit}' $1)
for i in `seq 1 $ncol`; do
    eval "awk '{print \$$i}' $1 | ./validate"
done

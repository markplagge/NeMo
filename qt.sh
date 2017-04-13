#!/bin/bash

while read line; do
    ((histogram[${#line}]++))
done < demo.csv 

echo "Length Occurrence"
for length in "${!histogram[@]}"; do
    printf "%-6s %s\n" "${length}" "${histogram[$length]}"
done

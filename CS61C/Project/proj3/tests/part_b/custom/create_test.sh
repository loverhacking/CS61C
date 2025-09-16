#!/bin/bash

# iterate all .s file in inputs directory
for file in inputs/*.s; do
    # check file exist
    if [ -f "$file" ]; then
        # run following command to create test
        python3 create-test.py "$file"
    fi
done
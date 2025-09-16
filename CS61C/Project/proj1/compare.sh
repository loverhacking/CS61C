#!/bin/bash


for i in {10000..10100}; do
    student_file="./studentOutputs/GliderGuns/GliderGuns_0x1808_${i}.ppm"
    test_file="./testOutputs/GliderGuns/GliderGuns_0x1808_${i}.ppm"

    if [ ! -f "$student_file" ]; then
        echo "error:student_file does not exist - $student_file"
        exit 1
    fi

    if [ ! -f "$test_file" ]; then
        echo "error:test_file does not exist - $test_file"
        exit 1
    fi

    if cmp -s "$student_file" "$test_file"; then
        echo "file ${i}.ppm is identical"
    else
        echo "notice difference: file ${i}.ppm is different"
        exit 1
    fi
done

echo "All files (10000-10100) have been compared and the contents are completely consistent"
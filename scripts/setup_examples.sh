#!/bin/bash
# One-time script: adds idf_component.yml to all examples and removes EXTRA_COMPONENT_DIRS
# Run from repo root: bash scripts/setup_examples.sh

set -e

EXAMPLES_DIR="components/inkplate/examples"
COMPONENT_YML_CONTENT='dependencies:
  solderedelectronics/inkplate:
    version: ">=1.0.0"
  idf:
    version: ">=6.0.0"
'

# Find all project-level CMakeLists.txt (exclude main/ subdirs)
find "$EXAMPLES_DIR" -name "CMakeLists.txt" -not -path "*/main/*" | while read -r cmake_file; do
    example_dir=$(dirname "$cmake_file")

    # Add idf_component.yml if not already present
    yml_file="$example_dir/idf_component.yml"
    if [ ! -f "$yml_file" ]; then
        echo "$COMPONENT_YML_CONTENT" > "$yml_file"
        echo "Created: $yml_file"
    fi

    # Remove EXTRA_COMPONENT_DIRS line from CMakeLists.txt
    if grep -q "EXTRA_COMPONENT_DIRS" "$cmake_file"; then
        sed -i.bak '/EXTRA_COMPONENT_DIRS/d' "$cmake_file"
        rm -f "${cmake_file}.bak"
        echo "Fixed:   $cmake_file"
    fi
done

echo ""
echo "Done. Verify with:"
echo "  find $EXAMPLES_DIR -name 'idf_component.yml' | wc -l"

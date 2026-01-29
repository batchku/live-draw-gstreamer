#!/bin/bash
# Universal PlantUML script with automatic download/caching and format support
# Usage: ./plantuml.sh [OPTIONS] input.puml [output-file]
#
# Options:
#   -f, --format FORMAT    Output format: svg (default) or png
#   -h, --help            Show this help message
#
# Examples:
#   ./plantuml.sh diagram.puml                    # Generates diagram.svg
#   ./plantuml.sh -f png diagram.puml             # Generates diagram.png
#   ./plantuml.sh diagram.puml output.svg         # Generates output.svg
#   ./plantuml.sh -f png diagram.puml out.png     # Generates out.png

set -e

# Configuration
PLANTUML_VERSION="1.2024.7"
PLANTUML_JAR="plantuml-${PLANTUML_VERSION}.jar"
CACHE_DIR="/tmp/plantuml-cache-$$"
PLANTUML_URL="https://github.com/plantuml/plantuml/releases/download/v${PLANTUML_VERSION}/plantuml-${PLANTUML_VERSION}.jar"

# Default format
FORMAT="svg"

# Parse command line options
show_help() {
    head -n 12 "$0" | tail -n 11
    exit 0
}

while [[ $# -gt 0 ]]; do
    case $1 in
        -f|--format)
            FORMAT="$2"
            if [[ "$FORMAT" != "svg" && "$FORMAT" != "png" ]]; then
                echo "Error: Format must be 'svg' or 'png'" >&2
                exit 1
            fi
            shift 2
            ;;
        -h|--help)
            show_help
            ;;
        -*)
            echo "Error: Unknown option: $1" >&2
            echo "Use -h or --help for usage information" >&2
            exit 1
            ;;
        *)
            break
            ;;
    esac
done

# Check for input file
if [ $# -eq 0 ]; then
    echo "Error: No input file specified" >&2
    echo "Use -h or --help for usage information" >&2
    exit 1
fi

INPUT_FILE="$1"
OUTPUT_FILE="${2:-${INPUT_FILE%.puml}.${FORMAT}}"

# Validate input file exists
if [ ! -f "$INPUT_FILE" ]; then
    echo "Error: Input file '$INPUT_FILE' not found" >&2
    exit 1
fi

# Check if Java is installed
if ! command -v java &> /dev/null; then
    echo "Error: Java is not installed" >&2
    echo "Install with: sudo apt-get install default-jre" >&2
    exit 1
fi

# Create cache directory if it doesn't exist
mkdir -p "$CACHE_DIR"

# Download PlantUML if not cached
PLANTUML_PATH="$CACHE_DIR/$PLANTUML_JAR"
if [ ! -f "$PLANTUML_PATH" ]; then
    echo "Downloading PlantUML ${PLANTUML_VERSION}..." >&2

    # Check if wget or curl is available
    if command -v wget &> /dev/null; then
        wget -q --show-progress -O "$PLANTUML_PATH" "$PLANTUML_URL"
    elif command -v curl &> /dev/null; then
        curl -L -o "$PLANTUML_PATH" "$PLANTUML_URL"
    else
        echo "Error: Neither wget nor curl is available" >&2
        echo "Install with: sudo apt-get install wget" >&2
        exit 1
    fi

    if [ $? -eq 0 ]; then
        echo "PlantUML downloaded successfully to $PLANTUML_PATH" >&2
    else
        echo "Error: Failed to download PlantUML" >&2
        rm -f "$PLANTUML_PATH"
        exit 1
    fi
else
    echo "Using cached PlantUML at $PLANTUML_PATH" >&2
fi

# Determine output format flag
case "$FORMAT" in
    svg)
        FORMAT_FLAG="-tsvg"
        ;;
    png)
        FORMAT_FLAG="-tpng"
        ;;
esac

# Generate diagram
echo "Converting $INPUT_FILE to $OUTPUT_FILE..." >&2

# Get absolute paths
INPUT_ABS=$(realpath "$INPUT_FILE")
OUTPUT_DIR=$(dirname "$(realpath "$OUTPUT_FILE")")
OUTPUT_NAME=$(basename "$OUTPUT_FILE")
INPUT_NAME=$(basename "$INPUT_FILE")

# Generate in temp directory then move to final location
TEMP_OUTPUT="${INPUT_NAME%.puml}.${FORMAT}"

java -jar "$PLANTUML_PATH" $FORMAT_FLAG "$INPUT_ABS" -o "$OUTPUT_DIR"

# Rename if output filename was specified differently
if [ "$TEMP_OUTPUT" != "$OUTPUT_NAME" ]; then
    mv "$OUTPUT_DIR/$TEMP_OUTPUT" "$OUTPUT_DIR/$OUTPUT_NAME"
fi

if [ -f "$OUTPUT_FILE" ]; then
    echo "Successfully created: $OUTPUT_FILE" >&2
    exit 0
else
    echo "Error: Failed to create output file" >&2
    exit 1
fi

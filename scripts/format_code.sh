#!/bin/bash
#
# Code formatting script for Video Looper
#
# Uses clang-format to enforce consistent code style throughout the project.
# Automatically formats all C source files in src/ and test/ directories.
#
# Usage: ./scripts/format_code.sh [options]
# Options:
#   --check          Check formatting without modifying files
#   --inplace        Format files in place (default)
#   --dry-run        Show what would be formatted without modifying
#

set -e

# Script directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_ROOT="$( dirname "$SCRIPT_DIR" )"

# Default options
CHECK_ONLY=false
DRY_RUN=false

# Parse command-line arguments
while [[ $# -gt 0 ]]; do
  case $1 in
    --check)
      CHECK_ONLY=true
      shift
      ;;
    --dry-run)
      DRY_RUN=true
      shift
      ;;
    --inplace)
      # Default behavior
      shift
      ;;
    *)
      echo "Unknown option: $1"
      exit 1
      ;;
  esac
done

# Print header
echo "=========================================="
echo "Video Looper - Code Formatting"
echo "=========================================="
echo "Check Only: $CHECK_ONLY"
echo "Dry Run: $DRY_RUN"
echo ""

# Check if clang-format is available
if ! command -v clang-format &> /dev/null; then
  echo "Error: clang-format not found"
  echo "Please install: brew install clang-format"
  exit 1
fi

# Collect all C source files
echo "Scanning for C source files..."
SOURCE_FILES=()
while IFS= read -r -d '' file; do
  SOURCE_FILES+=("$file")
done < <(find "$PROJECT_ROOT/src" "$PROJECT_ROOT/test" -type f -name "*.c" -o -name "*.h" -print0)

if [ ${#SOURCE_FILES[@]} -eq 0 ]; then
  echo "No source files found"
  exit 0
fi

echo "Found ${#SOURCE_FILES[@]} source files to format"
echo ""

# Format each file
FORMATTED_COUNT=0
NEEDS_FORMAT_COUNT=0

for file in "${SOURCE_FILES[@]}"; do
  if [ "$DRY_RUN" = true ]; then
    # Check if file needs formatting without modifying it
    if ! clang-format --output-replacements-xml "$file" | grep -q "</replacement>"; then
      echo "  OK: $file"
    else
      echo "  NEEDS FORMAT: $file"
      ((NEEDS_FORMAT_COUNT++))
    fi
  elif [ "$CHECK_ONLY" = true ]; then
    # Check format without modifying
    if ! clang-format -output-replacements-xml "$file" | grep -q "</replacement>"; then
      echo "  OK: $file"
    else
      echo "  NOT FORMATTED: $file"
      ((NEEDS_FORMAT_COUNT++))
    fi
  else
    # Format in place
    echo "Formatting: $file"
    clang-format -i "$file"
    ((FORMATTED_COUNT++))
  fi
done

echo ""
echo "=========================================="
if [ "$DRY_RUN" = true ]; then
  echo "Dry run: Found $NEEDS_FORMAT_COUNT files needing format"
  if [ "$NEEDS_FORMAT_COUNT" -gt 0 ]; then
    echo "Run './scripts/format_code.sh' to format"
  fi
elif [ "$CHECK_ONLY" = true ]; then
  echo "Check result: $NEEDS_FORMAT_COUNT files not formatted"
  if [ "$NEEDS_FORMAT_COUNT" -gt 0 ]; then
    echo "Run './scripts/format_code.sh' to fix"
  fi
else
  echo "Formatted $FORMATTED_COUNT files"
  echo "Code formatting complete!"
fi
echo "=========================================="

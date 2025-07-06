#!/bin/bash

# ---
# compile.sh: A script to compile C++ source files with specific framework, debug options, and sanitizers.
#
# This script automates the compilation process by adjusting g++ flags and output
# filenames based on user-provided arguments.
#
# Usage: ./compile.sh {xtf|tf} {debug|prof|original} {none|asan|tsan|ubsan} <source_file_1> [source_file_2 ...]
#
# Arguments:
#   $1: Framework type.
#       - 'xtf':      Adds the -DTF_USE_XQUEUE=1 compile-time definition.
#       - 'tf':       Uses standard compilation.
#
#   $2: Build type.
#       - 'debug':    Compiles with debug symbols and no optimization (-g -O0).
#       - 'prof':     Compiles with debug symbols and moderate optimization (-g -O2).
#       - 'original': Compiles with full optimization (-O3).
#
#   $3: Sanitizer type.
#       - 'none':     No sanitizer enabled.
#       - 'asan':     Address Sanitizer.
#       - 'tsan':     Thread Sanitizer.
#       - 'ubsan':    Undefined Behavior Sanitizer.
#
#   $4 and onwards: One or more C++ source files to compile.
#
# Output:
#   An executable file named using the first source file's name, followed by
#   suffixes for the framework, build type, and sanitizer.
#   Example: for './compile.sh xtf debug asan main.cpp', the output is 'main_xtf_debug_asan'.
# ---

# 1. Validate that the correct number of arguments has been provided.
if [ "$#" -lt 4 ]; then
    echo "Error: Missing arguments."
    echo "Usage: $0 {xtf|tf} {debug|prof|original} {none|asan|tsan|ubsan} <source_file...>"
    echo "Example: $0 xtf debug asan main.cpp utils.cpp"
    exit 1
fi

# 2. Assign arguments to variables for clarity.
FRAMEWORK="$1"
BUILD_TYPE="$2"
SANITIZER="$3"
shift 3 # The remaining arguments are the source files.
SOURCE_FILES=("$@")

# 3. Initialize variables for compiler flags and output suffixes.
CXX="g++"
# Start with some sensible default flags.
CXXFLAGS="-std=c++17 -Wall -I../taskflow"
FRAMEWORK_SUFFIX=""
BUILD_SUFFIX=""
SANITIZER_SUFFIX=""

# 4. Set compiler flags and suffix based on the chosen FRAMEWORK.
case "$FRAMEWORK" in
    xtf)
        CXXFLAGS+=" -DTF_USE_XQUEUE=1"
        FRAMEWORK_SUFFIX="_xtf"
        ;;
    tf)
        # No extra flag is needed for the 'tf' framework.
        FRAMEWORK_SUFFIX="_tf"
        ;;
    *)
        echo "Error: Invalid framework type '$FRAMEWORK'. Choose 'xtf' or 'tf'."
        exit 1
        ;;
esac

# 5. Set compiler flags and suffix based on the chosen BUILD_TYPE.
case "$BUILD_TYPE" in
    debug)
        CXXFLAGS+=" -g -O0"
        BUILD_SUFFIX="_debug"
        ;;
    prof)
        CXXFLAGS+=" -g -O2"
        BUILD_SUFFIX="_prof"
        ;;
    original)
        CXXFLAGS+=" -O3"
        BUILD_SUFFIX="_original"
        ;;
    *)
        echo "Error: Invalid build type '$BUILD_TYPE'. Choose 'debug', 'prof', or 'original'."
        exit 1
        ;;
esac

# 6. Set compiler flags and suffix based on the chosen SANITIZER.
case "$SANITIZER" in
    none)
        # No sanitizer flags needed
        ;;
    asan)
        CXXFLAGS+=" -fsanitize=address"
        SANITIZER_SUFFIX="_asan"
        ;;
    tsan)
        CXXFLAGS+=" -fsanitize=thread"
        SANITIZER_SUFFIX="_tsan"
        ;;
    ubsan)
        CXXFLAGS+=" -fsanitize=undefined"
        SANITIZER_SUFFIX="_ubsan"
        ;;
    *)
        echo "Error: Invalid sanitizer type '$SANITIZER'. Choose 'none', 'asan', 'tsan', or 'ubsan'."
        exit 1
        ;;
esac

# 7. Determine the final output filename.
# The name is based on the first source file provided.
FIRST_SOURCE="${SOURCE_FILES[0]}"
# Get the filename without the directory path or extension.
BASENAME=$(basename "$FIRST_SOURCE")
BASENAME_NO_EXT="${BASENAME%.*}"
OUTPUT_FILE="${BASENAME_NO_EXT}${FRAMEWORK_SUFFIX}${BUILD_SUFFIX}${SANITIZER_SUFFIX}"

# 8. Construct and execute the final compilation command.
# Using an array for the command is safer as it correctly handles spaces in paths.
COMPILE_COMMAND=("$CXX" $CXXFLAGS -o "$OUTPUT_FILE" "${SOURCE_FILES[@]}")

# Display a summary of the compilation job.
echo "------------------------------------"
echo "Starting Compilation Job"
echo "------------------------------------"
echo "Framework:   $FRAMEWORK"
echo "Build Type:  $BUILD_TYPE"
echo "Sanitizer:   $SANITIZER"
echo "Sources:     ${SOURCE_FILES[*]}"
echo "Output Name: $OUTPUT_FILE"
echo "g++ Flags:   $CXXFLAGS"
echo "------------------------------------"
echo "Executing command..."
echo "\$ ${COMPILE_COMMAND[@]}"
echo "------------------------------------"

# Execute the command.
"${COMPILE_COMMAND[@]}"

# 9. Check the exit code of the compiler and report the result.
if [ $? -eq 0 ]; then
    echo "✅ Compilation successful!"
    echo "   Executable created at: ./$OUTPUT_FILE"
    exit 0
else
    echo "❌ Compilation failed."
    exit 1
fi

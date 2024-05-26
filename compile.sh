#!/bin/sh

set -e # Exit immediately if a command exits with a non-zero status

# ANSI color codes
RED="\033[0;31m"
GREEN="\033[0;32m"
YELLOW="\033[0;33m"
NC="\033[0m" # No Color

# Function to clean the build directory
clean_build() {
    echo "${YELLOW}> Cleaning build directory ...${NC}"
    rm -rf build
    mkdir -p build
}

# Function to preconfigure the build directory
preconfigure_build() {
    echo "${YELLOW}> Preconfiguring build ...${NC}"
    mkdir -p build
    cp -r assets build/

    # only copy index.html if the target is web
    if [ "$TARGET" = "web" ]; then
        cp index.html build/
    fi
}

# Function to build for web
build_web() {
    echo "${YELLOW}> Building for web ...${NC}"
    if emcc -O2 main.cpp -o build/index.js -s USE_SDL=2 -s NO_EXIT_RUNTIME=0 --preload-file assets; then
        echo "${GREEN}> Web build successful. Files found at build/.${NC}"
        if [ "$EXECUTE" = true ]; then
            echo "${GREEN}> Starting http server...${NC}"
            python3 -m http.server -d build
        fi
    else
        echo "${RED}> Web build failure. Server will not start.${NC}"
        exit 1
    fi
}

# Function to build for native
build_native() {
    echo "${YELLOW}> Building for native ...${NC}"
    if g++ main.cpp -o build/main.elf -lSDL2; then
        echo "${GREEN}> Native build successful. Executible found at build/main.elf${NC}"
        if [ "$EXECUTE" = true ]; then
            echo "${GREEN}> Running the executible...${NC}"
            ./build/main.elf
        fi
    else
        echo "${RED}> Native build failure.${NC}"
        exit 1
    fi
}

# Main script logic
main() {
    # default behaviors
    TARGET="web"
    CLEAN=false
    EXECUTE=false

    # Parse commandline arguments
    for arg in "$@"; do
        case $arg in
            web)
                TARGET="web"
                ;;
            native)
                TARGET="native"
                ;;
            clean)
                CLEAN=true
                ;;
            execute)
                EXECUTE=true
                ;;
            *)
                echo -e "${RED}> Invalid argument: $arg. Use 'web' (default), 'native', or 'clean'.${NC}"
                exit 1
                ;;
        esac
    done

    # handle clean
    if [ "$CLEAN" = true ]; then
        clean_build
    fi

    # preconfigure build
    preconfigure_build

    if [ "$TARGET" = "web" ]; then
        build_web
    elif [ "$TARGET" = "native" ]; then
        build_native
    else
        echo -e "${RED}> Invalid target: $TARGET. Use 'web' (default) or 'native'.${NC}"
        exit 1
    fi
}

# Run the main function with all the script arguments
main "$@"

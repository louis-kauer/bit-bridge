# Makefile for bit-bridge C++ project
# Uses CMake as the underlying build system

# Project settings
PROJECT_NAME := bit_bridge
BUILD_DIR_DEBUG := cmake-build-debug
BUILD_DIR_RELEASE := cmake-build-release
BUILD_DIR_RELWITHDEBINFO := cmake-build-relwithdebinfo
BUILD_DIR_MINSIZEREL := cmake-build-minsizerel
BUILD_DIR_ASAN := cmake-build-asan

# CMake settings
CMAKE ?= cmake
CMAKE_GENERATOR ?= "Unix Makefiles"
JOBS ?= $(shell sysctl -n hw.ncpu 2>/dev/null || nproc 2>/dev/null || echo 4)

# Compiler settings (can be overridden)
CXX ?=
CC ?=

# Setting SHELL to bash allows bash commands to be executed by recipes.
# Options are set to exit when a recipe line exits non-zero or a piped command fails.
SHELL = /usr/bin/env bash -o pipefail
.SHELLFLAGS = -ec

.PHONY: all
all: build

##@ General

# The help target prints out all targets with their descriptions organized
# beneath their categories. The categories are represented by '##@' and the
# target descriptions by '##'. The awk command is responsible for reading the
# entire set of makefiles included in this invocation, looking for lines of the
# file as xyz: ## something, and then pretty-format the target and help. Then,
# if there's a line with ##@ something, that gets pretty-printed as a category.
# More info on the usage of ANSI control characters for terminal formatting:
# https://en.wikipedia.org/wiki/ANSI_escape_code#SGR_parameters
# More info on the awk command:
# http://linuxcommand.org/lc3_adv_awk.php

.PHONY: help
help: ## Display this help.
	@awk 'BEGIN {FS = ":.*##"; printf "\nUsage:\n  make \033[36m<target>\033[0m\n"} /^[a-zA-Z_0-9-]+:.*?##/ { printf "  \033[36m%-20s\033[0m %s\n", $$1, $$2 } /^##@/ { printf "\n\033[1m%s\033[0m\n", substr($$0, 5) } ' $(MAKEFILE_LIST)

##@ Build

.PHONY: build
build: configure-debug ## Build debug configuration (default).
	$(CMAKE) --build $(BUILD_DIR_DEBUG) -j$(JOBS)

.PHONY: debug
debug: configure-debug ## Build debug configuration.
	$(CMAKE) --build $(BUILD_DIR_DEBUG) -j$(JOBS)

.PHONY: release
release: configure-release ## Build release configuration.
	$(CMAKE) --build $(BUILD_DIR_RELEASE) -j$(JOBS)

.PHONY: relwithdebinfo
relwithdebinfo: configure-relwithdebinfo ## Build release with debug info.
	$(CMAKE) --build $(BUILD_DIR_RELWITHDEBINFO) -j$(JOBS)

.PHONY: minsizerel
minsizerel: configure-minsizerel ## Build minimum size release.
	$(CMAKE) --build $(BUILD_DIR_MINSIZEREL) -j$(JOBS)

.PHONY: build-all
build-all: debug release relwithdebinfo minsizerel ## Build all configurations.

.PHONY: debug-verbose
debug-verbose: configure-debug ## Build debug with verbose output.
	$(CMAKE) --build $(BUILD_DIR_DEBUG) -j$(JOBS) --verbose

.PHONY: release-verbose
release-verbose: configure-release ## Build release with verbose output.
	$(CMAKE) --build $(BUILD_DIR_RELEASE) -j$(JOBS) --verbose

##@ Configure

.PHONY: configure-debug
configure-debug: ## Configure debug build.
	@mkdir -p $(BUILD_DIR_DEBUG)
	$(CMAKE) -S . -B $(BUILD_DIR_DEBUG) -G $(CMAKE_GENERATOR) \
		-DCMAKE_BUILD_TYPE=Debug \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
		$(if $(CXX),-DCMAKE_CXX_COMPILER=$(CXX)) \
		$(if $(CC),-DCMAKE_C_COMPILER=$(CC))

.PHONY: configure-release
configure-release: ## Configure release build.
	@mkdir -p $(BUILD_DIR_RELEASE)
	$(CMAKE) -S . -B $(BUILD_DIR_RELEASE) -G $(CMAKE_GENERATOR) \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
		$(if $(CXX),-DCMAKE_CXX_COMPILER=$(CXX)) \
		$(if $(CC),-DCMAKE_C_COMPILER=$(CC))

.PHONY: configure-relwithdebinfo
configure-relwithdebinfo: ## Configure release with debug info build.
	@mkdir -p $(BUILD_DIR_RELWITHDEBINFO)
	$(CMAKE) -S . -B $(BUILD_DIR_RELWITHDEBINFO) -G $(CMAKE_GENERATOR) \
		-DCMAKE_BUILD_TYPE=RelWithDebInfo \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
		$(if $(CXX),-DCMAKE_CXX_COMPILER=$(CXX)) \
		$(if $(CC),-DCMAKE_C_COMPILER=$(CC))

.PHONY: configure-minsizerel
configure-minsizerel: ## Configure minimum size release build.
	@mkdir -p $(BUILD_DIR_MINSIZEREL)
	$(CMAKE) -S . -B $(BUILD_DIR_MINSIZEREL) -G $(CMAKE_GENERATOR) \
		-DCMAKE_BUILD_TYPE=MinSizeRel \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
		$(if $(CXX),-DCMAKE_CXX_COMPILER=$(CXX)) \
		$(if $(CC),-DCMAKE_C_COMPILER=$(CC))

.PHONY: reconfigure
reconfigure: clean-cache configure-debug ## Force reconfigure debug build.

.PHONY: reconfigure-release
reconfigure-release: clean-cache-release configure-release ## Force reconfigure release build.

##@ Clean

.PHONY: clean
clean: ## Clean debug build.
	@if [ -d $(BUILD_DIR_DEBUG) ]; then $(CMAKE) --build $(BUILD_DIR_DEBUG) --target clean 2>/dev/null || rm -rf $(BUILD_DIR_DEBUG); fi

.PHONY: clean-release
clean-release: ## Clean release build.
	@if [ -d $(BUILD_DIR_RELEASE) ]; then $(CMAKE) --build $(BUILD_DIR_RELEASE) --target clean 2>/dev/null || rm -rf $(BUILD_DIR_RELEASE); fi

.PHONY: clean-all
clean-all: ## Clean all build directories.
	rm -rf $(BUILD_DIR_DEBUG) $(BUILD_DIR_RELEASE) $(BUILD_DIR_RELWITHDEBINFO) $(BUILD_DIR_MINSIZEREL)

.PHONY: clean-cache
clean-cache: ## Clean CMake cache (debug).
	rm -f $(BUILD_DIR_DEBUG)/CMakeCache.txt
	rm -rf $(BUILD_DIR_DEBUG)/CMakeFiles

.PHONY: clean-cache-release
clean-cache-release: ## Clean CMake cache (release).
	rm -f $(BUILD_DIR_RELEASE)/CMakeCache.txt
	rm -rf $(BUILD_DIR_RELEASE)/CMakeFiles

.PHONY: distclean
distclean: ## Remove all build directories and generated files.
	rm -rf $(BUILD_DIR_DEBUG) $(BUILD_DIR_RELEASE) $(BUILD_DIR_RELWITHDEBINFO) $(BUILD_DIR_MINSIZEREL)
	rm -f compile_commands.json

##@ Rebuild

.PHONY: rebuild
rebuild: clean debug ## Clean and rebuild debug.

.PHONY: rebuild-release
rebuild-release: clean-release release ## Clean and rebuild release.

.PHONY: rebuild-all
rebuild-all: clean-all build-all ## Clean and rebuild all configurations.

##@ Run

.PHONY: run
run: ## Run debug binary.
	./$(BUILD_DIR_DEBUG)/$(PROJECT_NAME)

.PHONY: run-release
run-release: ## Run release binary.
	./$(BUILD_DIR_RELEASE)/$(PROJECT_NAME)

.PHONY: run-args
run-args: ## Run debug binary with ARGS (e.g., make run-args ARGS="arg1 arg2").
	./$(BUILD_DIR_DEBUG)/$(PROJECT_NAME) $(ARGS)

##@ Analysis

.PHONY: compile-commands
compile-commands: configure-debug ## Generate compile_commands.json for IDE/clangd.
	ln -sf $(BUILD_DIR_DEBUG)/compile_commands.json compile_commands.json

.PHONY: lint
lint: compile-commands ## Run clang-tidy static analysis.
	@if command -v clang-tidy >/dev/null 2>&1; then \
		find src -name '*.cpp' | xargs clang-tidy -p $(BUILD_DIR_DEBUG); \
	else \
		echo "clang-tidy not found. Install with: brew install llvm"; \
	fi

.PHONY: format
format: ## Format source files with clang-format.
	@if command -v clang-format >/dev/null 2>&1; then \
		find . -name "*.cpp" -o -name "*.hpp" -o -name "*.h" | xargs clang-format -i; \
	else \
		echo "clang-format not found. Install with: brew install clang-format"; \
	fi

.PHONY: format-check
format-check: ## Check formatting without modifying files.
	@if command -v clang-format >/dev/null 2>&1; then \
		find . -name "*.cpp" -o -name "*.hpp" -o -name "*.h" | xargs clang-format --dry-run -Werror; \
	else \
		echo "clang-format not found. Install with: brew install clang-format"; \
	fi

##@ Test

.PHONY: test
test: debug ## Run tests with CTest.
	@if [ -f $(BUILD_DIR_DEBUG)/CTestTestfile.cmake ]; then \
		cd $(BUILD_DIR_DEBUG) && ctest --output-on-failure; \
	else \
		echo "No tests configured. Add tests to CMakeLists.txt to enable testing."; \
	fi

.PHONY: test-verbose
test-verbose: debug ## Run tests with verbose output.
	@if [ -f $(BUILD_DIR_DEBUG)/CTestTestfile.cmake ]; then \
		cd $(BUILD_DIR_DEBUG) && ctest -V; \
	else \
		echo "No tests configured. Add tests to CMakeLists.txt to enable testing."; \
	fi

.PHONY: test-asan
test-asan: ## Run tests with AddressSanitizer enabled.
	@$(CMAKE) -S . -B $(BUILD_DIR_ASAN) -G $(CMAKE_GENERATOR) \
		-DCMAKE_BUILD_TYPE=Debug \
		-DENABLE_ASAN=ON \
		$(if $(CXX),-DCMAKE_CXX_COMPILER=$(CXX),) \
		$(if $(CC),-DCMAKE_C_COMPILER=$(CC),)
	@$(CMAKE) --build $(BUILD_DIR_ASAN) --target bit_bridge_tests -j$(JOBS)
	@$(BUILD_DIR_ASAN)/bit_bridge_tests

.PHONY: run-asan
run-asan: ## Run the app with AddressSanitizer (quit the app to see the report).
	@$(CMAKE) -S . -B $(BUILD_DIR_ASAN) -G $(CMAKE_GENERATOR) \
		-DCMAKE_BUILD_TYPE=Debug \
		-DENABLE_ASAN=ON \
		$(if $(CXX),-DCMAKE_CXX_COMPILER=$(CXX),) \
		$(if $(CC),-DCMAKE_C_COMPILER=$(CC),)
	@$(CMAKE) --build $(BUILD_DIR_ASAN) --target bit_bridge -j$(JOBS)
	@$(BUILD_DIR_ASAN)/bit_bridge

##@ Setup

.PHONY: install-hooks
install-hooks: ## Install git hooks from scripts/ directory.
	@cp scripts/pre-push .git/hooks/pre-push
	@chmod +x .git/hooks/pre-push
	@echo "Git hooks installed."

##@ Info

.PHONY: info
info: ## Show build information.
	@echo "Project:         $(PROJECT_NAME)"
	@echo "Debug Dir:       $(BUILD_DIR_DEBUG)"
	@echo "Release Dir:     $(BUILD_DIR_RELEASE)"
	@echo "Parallel Jobs:   $(JOBS)"
	@echo "CMake:           $(CMAKE)"
	@echo "Generator:       $(CMAKE_GENERATOR)"
	@echo ""
	@if [ -f $(BUILD_DIR_DEBUG)/$(PROJECT_NAME) ]; then \
		echo "Debug binary:    EXISTS"; \
	else \
		echo "Debug binary:    NOT BUILT"; \
	fi
	@if [ -f $(BUILD_DIR_RELEASE)/$(PROJECT_NAME) ]; then \
		echo "Release binary:  EXISTS"; \
	else \
		echo "Release binary:  NOT BUILT"; \
	fi
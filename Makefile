# Project-specific settings
EMP_DIR := third-party/Empirical/include
SGP_DIR := third-party/signalgp-lite/include

#####################################################
# ---- Set which executable to compile ----
# Directed evolution model setup:
PROJECT ?= directed-digital-evolution
MAIN_CPP ?= source/native.cpp
THREADING ?= -DDIRDEVO_THREADING -pthread
# GP setup:
# PROJECT ?= avidagp-ec
# MAIN_CPP ?= source/native-ec.cpp
# THREADING ?= -DDIRDEVO_SINGLE_THREAD
#######################################################

# Flags to use regardless of compiler
CFLAGS_all := $(THREADING) -Wall -Wno-unused-function -std=c++17 -I$(EMP_DIR)/ -I$(SGP_DIR)/ -Iinclude/ -Ithird-party/
# -DDIRDEVO_THREADING -pthread
# Native compiler information
CXX ?= g++
CFLAGS_nat := -O3 -DNDEBUG -msse4.2 $(CFLAGS_all)
CFLAGS_nat_debug := -g $(CFLAGS_all)

# Emscripten compiler information
CXX_web := emcc
OFLAGS_web_all := -s "EXTRA_EXPORTED_RUNTIME_METHODS=['ccall', 'cwrap']" -s TOTAL_MEMORY=67108864 --js-library $(EMP_DIR)/emp/web/library_emp.js -s EXPORTED_FUNCTIONS="['_main', '_empCppCallback', '_empDoCppCallback']" -s DISABLE_EXCEPTION_CATCHING=1 -s NO_EXIT_RUNTIME=1 #--embed-file configs
OFLAGS_web := -Oz -DNDEBUG
OFLAGS_web_debug := -g4 -Oz -Wno-dollar-in-identifier-extension

CFLAGS_web := $(CFLAGS_all) $(OFLAGS_web) $(OFLAGS_web_all)
CFLAGS_web_debug := $(CFLAGS_all) $(OFLAGS_web_debug) $(OFLAGS_web_all)

default: $(PROJECT)
native: $(PROJECT)
# web: $(PROJECT).js
all: $(PROJECT) #$(PROJECT).js

debug:	CFLAGS_nat := $(CFLAGS_nat_debug)
debug:	$(PROJECT)

# debug-web:	CFLAGS_web := $(CFLAGS_web_debug)
# debug-web:	$(PROJECT).js

# web-debug:	debug-web

# see https://stackoverflow.com/a/57760267 RE: -lstdc++fs
$(PROJECT):	${MAIN_CPP} include/
	$(CXX) $(CFLAGS_nat) ${MAIN_CPP} -o $(PROJECT) -lstdc++fs
# @echo To build the web version use: make web

serve:
	python3 -m http.server

clean:
	rm -f $(PROJECT) rm debug_file web/$(PROJECT).js web/*.js.map web/*.js.map *~ source/*.o web/*.wasm web/*.wast

tests:
	cd tests && make
	cd tests && make opt
	cd tests && make fulldebug

coverage:
	cd tests && make coverage

install-dependencies:
	git submodule update --init --recursive && cd third-party && bash ./install_emsdk.sh && bash ./install_force_cover.sh

.PHONY: tests clean test serve debug native web tests install-test-dependencies documentation-coverage documentation-coverage-badge.json version-badge.json doto-badge.json

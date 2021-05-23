# Copyright 2021 Gauthier Voron
#
# This file is part of Metasys.
#
# Metasys is free software: you can redistribute it and/or modify it under the
# terms of the GNU General Public License as published by the Free Software
# Foundation, either version 3 of the License, or (at your option) any later
# version.
#
# Metasys is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
# A PARTICULAR PURPOSE. See the GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along with
# Metasys. If not, see <https://www.gnu.org/licenses/>.
#


DEP := .depends.d/
OBJ := obj/
LIB := lib/
BIN := bin/


include Function.mk
include Command.mk


config-commands := clean .depends.mk $(DEP)%

ifeq ($(filter $(config-commands), $(MAKECMDGOALS)),)
  # There is no goal or there are only build commands
  mode := build
else
  # There is at least one config command
  ifneq ($(filter-out $(config-commands), $(MAKECMDGOALS)),)
    # There is at least one build command
    mode := mixed
  else
    # There is no build command
    mode := config
  endif
endif


ifeq ($(mode),mixed)

  %:
	$(call cmd-make, $@)

  .NOTPARALLEL:

else


modules  := fs io net sched sys

sources  := $(foreach module, $(modules), $(wildcard $(strip $(module))/*.cxx))
objects  := $(patsubst %.cxx, $(OBJ)%.o, $(sources))

utest-sources := $(foreach m, $(modules), \
                   $(wildcard test/unit/$(strip $(m))/*.cxx))
utest-objects := $(patsubst %.cxx, $(OBJ)%.o, $(utest-sources))

atest-sources := $(foreach m, $(modules), \
                   $(wildcard test/asm/$(strip $(m))/*.cxx))
atest-objects := $(patsubst %.cxx, $(BIN)%, $(atest-sources))


-include .config/Makefile


all: $(LIB)libmetasys.a

install: $(LIB)libmetasys.a
	$(call cmd-install, /usr/lib)
	$(call cmd-install, /usr/lib/libmetasys.a, $<)
	$(call cmd-install, /usr/include)
	$(call cmd-install, /usr/include/metasys, include/metasys/)

.PHONY: all install


test:
	$(call cmd-make, unit-test)
	$(call cmd-make, asm-test)

unit-test: $(BIN)utest
	$(call cmd-run, ./$<)

asm-test: $(atest-objects)
	$(call cmd-run, ./tools/asmcmp, --score=0 $^)
	$(call cmd-run, ./tools/asmcmp-overhead, $(atest-sources))

.PHONY: test unit-test asm-test


$(call REQUIRE-DIR, $(BIN)utest $(BIN)atest)

$(BIN)utest: $(utest-objects) $(LIB)libmetasys.a
	$(call cmd-ldcxx, $@, $(filter %.o, $^), \
          pthread metasys gtest gtest_main, $(LIB))


$(call REQUIRE-DEP, $(atest-sources), $(DEP)%.d)
$(call REQUIRE-DIR, $(atest-objects))

$(BIN)%: %.cxx $(LIB)libmetasys.a
	$(call cmd-aldcxx, $@, $<, pthread metasys, $(LIB), include/ tools/)


$(call REQUIRE-DIR, $(LIB)libmetasys.a)

$(LIB)libmetasys.a: $(objects)
	$(call cmd-ar, $@, $^)


$(call REQUIRE-DEP, $(sources) $(utest-sources), $(DEP)%.d)
$(call REQUIRE-DIR, $(objects) $(utest-objects))

$(OBJ)%.o: %.cxx
	$(call cmd-ccxx, $@, $<, include/)


ifeq ($(mode),build)
  -include .depends.mk
endif

.depends.mk:
	$(call cmd-cat, $@, $^)

$(DEP)%.cxx.d:
	$(call cmd-depcxx, $@, $(patsubst %.cxx, $(OBJ)%.o, $<), $<, include/)

$(DEP)test/asm/%.cxx.d:
	$(call cmd-depcxx, $@, $(patsubst %.cxx, $(BIN)%, $<), $<, \
               include/ tools/)


clean:
	$(call cmd-clean, .depends.mk $(DEP) $(OBJ) $(LIB) $(BIN))

.PHONY: clean


endif

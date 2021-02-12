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


config-commands := tidy clean .depends.mk $(DEP)%

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


modules  := net sched sys

sources  := $(foreach module, $(modules), $(wildcard $(strip $(module))/*.cxx))
objects  := $(patsubst %.cxx, $(OBJ)%.o, $(sources))


-include .config/Config.mk


default: check

all: $(BIN)test

install: $(LIB)libmetasys.a
	$(call cmd-install, /usr/lib)
	$(call cmd-install, /usr/lib/libmetasys.a, $<)
	$(call cmd-install, /usr/include)
	$(call cmd-install, /usr/include/metasys, include/metasys/)

.PHONY: default all


check: $(BIN)test
	./$<

.PHONY: default all


$(call REQUIRE-DIR, $(BIN)test)

$(BIN)test: $(OBJ)main.o $(LIB)libmetasys.a
	$(call cmd-ldxx, $@, $<, pthread metasys, $(LIB))


$(call REQUIRE-DIR, $(LIB)libmetasys.a)

$(LIB)libmetasys.a: $(objects)
	$(call cmd-ar, $@, $^)


$(call REQUIRE-DEP, main.cxx $(sources), $(DEP)%.d)
$(call REQUIRE-DIR, $(OBJ)main.o $(objects))

$(OBJ)%.o: %.cxx
	$(call cmd-ccxx, $@, $<, include/)


ifeq ($(mode),build)
  -include .depends.mk
endif

.depends.mk:
	$(call cmd-cat, $@, $^)

$(DEP)%.cxx.d:
	$(call cmd-depcxx, $@, $(patsubst %.cxx, $(OBJ)%.o, $<), $<, include/)


clean:
	$(call cmd-clean, .depends.mk $(DEP) $(OBJ) $(LIB) $(BIN))

.PHONY: clean


endif

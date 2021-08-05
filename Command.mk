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


CXXFLAGS  ?= -Wall -Wextra -O3 -std=c++20 -g -flto
LDXXFLAGS ?= -flto
ACXXFLAGS ?= -Wall -Wextra -O3 -std=c++20 -g -DNDEBUG


define cmd-aldcxx
  $(call cmd-print,  LDCXX   $(strip $(1)))
  $(Q)g++ $(ACXXFLAGS) -o $(1) $(2) $(addprefix -I, $(5)) \
    $(addprefix -L, $(4)) $(addprefix -l, $(3))
endef

define cmd-ar
  $(call cmd-print,  AR      $(strip $(1)))
  $(Q)ar -crs $(1) $(2)
endef

define cmd-cat
  $(call cmd-print,  CAT     $(strip $(1)))
  $(Q)cat $(2) > $(1)
endef

define cmd-ccxx
  $(call cmd-print,  CCXX    $(strip $(1)))
  $(Q)g++ $(CXXFLAGS) -c $(2) -o $(1) $(addprefix -I, $(3))
endef

define cmd-clean
  $(call cmd-print,  CLEAN)
  $(Q)rm -rf $(1) || true 2> '/dev/null'
endef

define cmd-depcxx
  $(call cmd-info,  DEPCXX  $(strip $(1)))
  $(Q)g++ $(CXXFLAGS) -MM $(3) -o $(1) -MT $(2) $(addprefix -I, $(4))
endef

define cmd-install
  $(call cmd-print,  INSTALL $(strip $(call NOSLASH, $(PREFIX)))$(strip $(1)))
  $(if $(2), \
   $(if $(filter %/, $(2)), \
    $(Q)cp -RT $(2) $(strip $(call NOSLASH, $(PREFIX)))$(strip $(1)), \
    $(Q)install -m644 $(2) $(strip $(call NOSLASH, $(PREFIX)))$(strip $(1))), \
   $(Q)install -d $(strip $(call NOSLASH, $(PREFIX)))$(strip $(1)))
endef

define cmd-ldcxx
  $(call cmd-print,  LDCXX   $(strip $(1)))
  $(Q)g++ $(LDXXFLAGS) -o $(1) $(2) $(addprefix -L, $(4)) $(addprefix -l, $(3))
endef

define cmd-make
  $(call cmd-info,  MAKE    $(strip $(1)))
  $(Q)+$(MAKE) --no-print-directory $(1)
endef

define cmd-mkdir
  $(call cmd-info,  MKDIR   $(strip $(1)))
  $(Q)mkdir $(1)
endef

define cmd-run
  $(call cmd-print,  RUN     $(strip $(patsubst ./%,%,$(1))))
  $(Q)$(1) $(2)
endef

#
# Copyright (C) 2020 Linux Studio Plugins Project <https://lsp-plug.in/>
#           (C) 2020 Vladimir Sadovnikov <sadko4u@gmail.com>
#
# This file is part of lsp-r3d-wgl-lib
#
# lsp-r3d-wgl-lib is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# any later version.
#
# lsp-r3d-wgl-lib is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with lsp-r3d-wgl-lib.  If not, see <https://www.gnu.org/licenses/>.
#

#------------------------------------------------------------------------------
# List of common dependencies
DEPENDENCIES = \
  LSP_COMMON_LIB \
  LSP_R3D_IFACE \
  LSP_R3D_BASE_LIB

TEST_DEPENDENCIES = \
  LSP_TEST_FW

#------------------------------------------------------------------------------
# Linux dependencies
LINUX_DEPENDENCIES =

LINUX_TEST_DEPENDENCIES =

ifeq ($(PLATFORM),Linux)
  DEPENDENCIES             += $(LINUX_DEPENDENCIES)
  TEST_DEPENDENCIES        += $(LINUX_TEST_DEPENDENCIES)
endif

#------------------------------------------------------------------------------
# BSD dependencies
BSD_DEPENDENCIES =

BSD_TEST_DEPENDENCIES =

ifeq ($(PLATFORM),BSD)
  DEPENDENCIES             += $(BSD_DEPENDENCIES)
  TEST_DEPENDENCIES        += $(BSD_TEST_DEPENDENCIES)
endif

#------------------------------------------------------------------------------
# Windows dependencies
WINDOWS_DEPENDENCIES = \
  LIBGDI32 \
  LIBOPENGL32

WINDOWS_TEST_DEPENDENCIES = \
  LIBSHLWAPI

ifeq ($(PLATFORM),Windows)
  DEPENDENCIES             += $(WINDOWS_DEPENDENCIES)
  TEST_DEPENDENCIES        += $(WINDOWS_TEST_DEPENDENCIES)
endif

#------------------------------------------------------------------------------
# Overall system dependencies
ALL_DEPENDENCIES = \
  $(DEPENDENCIES) \
  $(LINUX_DEPENDENCIES) \
  $(BSD_DEPENDENCIES) \
  $(WINDOWS_DEPENDENCIES) \
  $(TEST_DEPENDENCIES) \
  $(LINUX_TEST_DEPENDENCIES) \
  $(BSD_TEST_DEPENDENCIES) \
  $(WINDOWS_TEST_DEPENDENCIES)
  
#-----------------------------*-cmake-*----------------------------------------#
# file   device/test/CMakeLists.txt
# author Gabriel Rockefeller
# date   2011 June 13
# brief  Instructions for building device/test Makefile.
# note   � Copyright 2011 Los Alamos National Security, All rights reserved.
#------------------------------------------------------------------------------#
# $Id$

# If this is an x86 build, build the unit tests.  If this is a ppe
# build, build the ppe helper binaries.  The ppe binaries must be
# installed (i.e., available in PPE_BINDIR) for the x86 tests to pass.
#
# Another possible approach might have been to build this package only
# during x86 builds and then somehow just build the ppe helper
# binaries using the ppe compiler (which must be available, since this
# is a het-only package).  In that scenario, the ppe binaries wouldn't
# need to be installed; they would just be built in the test
# directory, next to the unit tests that use them.  CMake really
# prefers the "one build tree-one toolchain" model, though, and it's
# probably better not to fight the system.

if( NOT "${CMAKE_CXX_COMPILER}" MATCHES "[sp]pu-g[+][+]" )  # if x86 build

# ---------------------------------------------------------------------------- #
# Source files
# ---------------------------------------------------------------------------- #

   set( test_sources
      # dacs_noop_ppe.cc
      # dacs_wait_for_cmd_ppe.cc
      tstDACS_Device.cc
      tstDACS_Device_Interface.cc
      tstDACS_Device_Process.cc
      tstDACS_External_Process.cc
      )

# ---------------------------------------------------------------------------- #
# Directories to search for include directives
# ---------------------------------------------------------------------------- #

   include_directories( 
      ${PROJECT_SOURCE_DIR}        # headers for tests
      ${PROJECT_SOURCE_DIR}/..     # headers for package
      ${PROJECT_BINARY_DIR}/..     # config.h
      )

# ---------------------------------------------------------------------------- #
# Build Unit tests
# ---------------------------------------------------------------------------- #

   set( test_deps
      Lib_c4
      Lib_dsxx
      ${MPI_LIBRARIES}
      )

   # Add tests
   add_parallel_tests(
      SOURCES  "${test_sources}"
      PE_LIST  "1;2;4"
      DEPS     "${test_deps}"
      MPIFLAGS "-q"
      RESOURCE_LOCK "singleton"
      )

else() # if ppe build

# ---------------------------------------------------------------------------- #
# Directories to search for include directives
# ---------------------------------------------------------------------------- #

   include_directories(
      ${PROJECT_SOURCE_DIR}        # headers for tests
      ${PROJECT_SOURCE_DIR}/..     # headers for package
      ${PROJECT_BINARY_DIR}/..     # config.h
      ${draco_src_dir_SOURCE_DIR}  # ds++ header files
      ${dsxx_BINARY_DIR}           # ds++/config.h
      )

# ---------------------------------------------------------------------------- #
# Build helper binaries
# ---------------------------------------------------------------------------- #

   add_executable( Exe_dacs_noop_ppe dacs_noop_ppe.cc )
   set_target_properties( Exe_dacs_noop_ppe
      PROPERTIES
      OUTPUT_NAME dacs_noop_ppe_exe
      LINK_FLAGS  "-Wl,-m,elf64ppc"
      )
   target_link_libraries( Exe_dacs_noop_ppe
      Lib_dsxx
      /usr/lib64/libdacs_hybrid.so
      )
   install( TARGETS Exe_dacs_noop_ppe DESTINATION bin )

   add_executable( Exe_dacs_wait_for_cmd_ppe dacs_wait_for_cmd_ppe.cc )
   set_target_properties( Exe_dacs_wait_for_cmd_ppe
      PROPERTIES
      OUTPUT_NAME dacs_wait_for_cmd_ppe_exe
      LINK_FLAGS  "-Wl,-m,elf64ppc"
      )
   target_link_libraries( Exe_dacs_wait_for_cmd_ppe
      Lib_dsxx
      /usr/lib64/libdacs_hybrid.so
      )
   install( TARGETS Exe_dacs_wait_for_cmd_ppe DESTINATION bin )

endif()

# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.25

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/gong/projects/kvstore/client/src

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/gong/projects/kvstore/client/src/build

# Include any dependencies generated for this target.
include CMakeFiles/kvstore-cli.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/kvstore-cli.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/kvstore-cli.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/kvstore-cli.dir/flags.make

CMakeFiles/kvstore-cli.dir/kvstore-cli.cc.o: CMakeFiles/kvstore-cli.dir/flags.make
CMakeFiles/kvstore-cli.dir/kvstore-cli.cc.o: /home/gong/projects/kvstore/client/src/kvstore-cli.cc
CMakeFiles/kvstore-cli.dir/kvstore-cli.cc.o: CMakeFiles/kvstore-cli.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/gong/projects/kvstore/client/src/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/kvstore-cli.dir/kvstore-cli.cc.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/kvstore-cli.dir/kvstore-cli.cc.o -MF CMakeFiles/kvstore-cli.dir/kvstore-cli.cc.o.d -o CMakeFiles/kvstore-cli.dir/kvstore-cli.cc.o -c /home/gong/projects/kvstore/client/src/kvstore-cli.cc

CMakeFiles/kvstore-cli.dir/kvstore-cli.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/kvstore-cli.dir/kvstore-cli.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/gong/projects/kvstore/client/src/kvstore-cli.cc > CMakeFiles/kvstore-cli.dir/kvstore-cli.cc.i

CMakeFiles/kvstore-cli.dir/kvstore-cli.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/kvstore-cli.dir/kvstore-cli.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/gong/projects/kvstore/client/src/kvstore-cli.cc -o CMakeFiles/kvstore-cli.dir/kvstore-cli.cc.s

CMakeFiles/kvstore-cli.dir/config.cc.o: CMakeFiles/kvstore-cli.dir/flags.make
CMakeFiles/kvstore-cli.dir/config.cc.o: /home/gong/projects/kvstore/client/src/config.cc
CMakeFiles/kvstore-cli.dir/config.cc.o: CMakeFiles/kvstore-cli.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/gong/projects/kvstore/client/src/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/kvstore-cli.dir/config.cc.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/kvstore-cli.dir/config.cc.o -MF CMakeFiles/kvstore-cli.dir/config.cc.o.d -o CMakeFiles/kvstore-cli.dir/config.cc.o -c /home/gong/projects/kvstore/client/src/config.cc

CMakeFiles/kvstore-cli.dir/config.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/kvstore-cli.dir/config.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/gong/projects/kvstore/client/src/config.cc > CMakeFiles/kvstore-cli.dir/config.cc.i

CMakeFiles/kvstore-cli.dir/config.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/kvstore-cli.dir/config.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/gong/projects/kvstore/client/src/config.cc -o CMakeFiles/kvstore-cli.dir/config.cc.s

# Object files for target kvstore-cli
kvstore__cli_OBJECTS = \
"CMakeFiles/kvstore-cli.dir/kvstore-cli.cc.o" \
"CMakeFiles/kvstore-cli.dir/config.cc.o"

# External object files for target kvstore-cli
kvstore__cli_EXTERNAL_OBJECTS =

kvstore-cli: CMakeFiles/kvstore-cli.dir/kvstore-cli.cc.o
kvstore-cli: CMakeFiles/kvstore-cli.dir/config.cc.o
kvstore-cli: CMakeFiles/kvstore-cli.dir/build.make
kvstore-cli: CMakeFiles/kvstore-cli.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/gong/projects/kvstore/client/src/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX executable kvstore-cli"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/kvstore-cli.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/kvstore-cli.dir/build: kvstore-cli
.PHONY : CMakeFiles/kvstore-cli.dir/build

CMakeFiles/kvstore-cli.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/kvstore-cli.dir/cmake_clean.cmake
.PHONY : CMakeFiles/kvstore-cli.dir/clean

CMakeFiles/kvstore-cli.dir/depend:
	cd /home/gong/projects/kvstore/client/src/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/gong/projects/kvstore/client/src /home/gong/projects/kvstore/client/src /home/gong/projects/kvstore/client/src/build /home/gong/projects/kvstore/client/src/build /home/gong/projects/kvstore/client/src/build/CMakeFiles/kvstore-cli.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/kvstore-cli.dir/depend

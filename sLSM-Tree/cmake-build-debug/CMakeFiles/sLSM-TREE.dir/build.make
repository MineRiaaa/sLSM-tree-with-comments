# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
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
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/ria/lsmtree/sLSM-Tree-master/sLSM-Tree

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/ria/lsmtree/sLSM-Tree-master/sLSM-Tree/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/sLSM-TREE.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/sLSM-TREE.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/sLSM-TREE.dir/flags.make

CMakeFiles/sLSM-TREE.dir/main.cpp.o: CMakeFiles/sLSM-TREE.dir/flags.make
CMakeFiles/sLSM-TREE.dir/main.cpp.o: ../main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ria/lsmtree/sLSM-Tree-master/sLSM-Tree/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/sLSM-TREE.dir/main.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/sLSM-TREE.dir/main.cpp.o -c /home/ria/lsmtree/sLSM-Tree-master/sLSM-Tree/main.cpp

CMakeFiles/sLSM-TREE.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/sLSM-TREE.dir/main.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ria/lsmtree/sLSM-Tree-master/sLSM-Tree/main.cpp > CMakeFiles/sLSM-TREE.dir/main.cpp.i

CMakeFiles/sLSM-TREE.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/sLSM-TREE.dir/main.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ria/lsmtree/sLSM-Tree-master/sLSM-Tree/main.cpp -o CMakeFiles/sLSM-TREE.dir/main.cpp.s

CMakeFiles/sLSM-TREE.dir/MurmurHash.cpp.o: CMakeFiles/sLSM-TREE.dir/flags.make
CMakeFiles/sLSM-TREE.dir/MurmurHash.cpp.o: ../MurmurHash.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ria/lsmtree/sLSM-Tree-master/sLSM-Tree/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/sLSM-TREE.dir/MurmurHash.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/sLSM-TREE.dir/MurmurHash.cpp.o -c /home/ria/lsmtree/sLSM-Tree-master/sLSM-Tree/MurmurHash.cpp

CMakeFiles/sLSM-TREE.dir/MurmurHash.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/sLSM-TREE.dir/MurmurHash.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ria/lsmtree/sLSM-Tree-master/sLSM-Tree/MurmurHash.cpp > CMakeFiles/sLSM-TREE.dir/MurmurHash.cpp.i

CMakeFiles/sLSM-TREE.dir/MurmurHash.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/sLSM-TREE.dir/MurmurHash.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ria/lsmtree/sLSM-Tree-master/sLSM-Tree/MurmurHash.cpp -o CMakeFiles/sLSM-TREE.dir/MurmurHash.cpp.s

# Object files for target sLSM-TREE
sLSM__TREE_OBJECTS = \
"CMakeFiles/sLSM-TREE.dir/main.cpp.o" \
"CMakeFiles/sLSM-TREE.dir/MurmurHash.cpp.o"

# External object files for target sLSM-TREE
sLSM__TREE_EXTERNAL_OBJECTS =

sLSM-TREE: CMakeFiles/sLSM-TREE.dir/main.cpp.o
sLSM-TREE: CMakeFiles/sLSM-TREE.dir/MurmurHash.cpp.o
sLSM-TREE: CMakeFiles/sLSM-TREE.dir/build.make
sLSM-TREE: CMakeFiles/sLSM-TREE.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/ria/lsmtree/sLSM-Tree-master/sLSM-Tree/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX executable sLSM-TREE"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/sLSM-TREE.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/sLSM-TREE.dir/build: sLSM-TREE

.PHONY : CMakeFiles/sLSM-TREE.dir/build

CMakeFiles/sLSM-TREE.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/sLSM-TREE.dir/cmake_clean.cmake
.PHONY : CMakeFiles/sLSM-TREE.dir/clean

CMakeFiles/sLSM-TREE.dir/depend:
	cd /home/ria/lsmtree/sLSM-Tree-master/sLSM-Tree/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/ria/lsmtree/sLSM-Tree-master/sLSM-Tree /home/ria/lsmtree/sLSM-Tree-master/sLSM-Tree /home/ria/lsmtree/sLSM-Tree-master/sLSM-Tree/cmake-build-debug /home/ria/lsmtree/sLSM-Tree-master/sLSM-Tree/cmake-build-debug /home/ria/lsmtree/sLSM-Tree-master/sLSM-Tree/cmake-build-debug/CMakeFiles/sLSM-TREE.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/sLSM-TREE.dir/depend


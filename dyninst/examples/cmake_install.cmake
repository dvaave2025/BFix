# Install script for directory: /home/examples

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/home/examples/CFGraph/cmake_install.cmake")
  include("/home/examples/codeCoverage/cmake_install.cmake")
  include("/home/examples/dataflowAPI/cmake_install.cmake")
  include("/home/examples/disassemble/cmake_install.cmake")
  include("/home/examples/DynC/cmake_install.cmake")
  include("/home/examples/instrumentAFunction/cmake_install.cmake")
  include("/home/examples/instrumentMemoryAccess/cmake_install.cmake")
  include("/home/examples/insertSnippet/cmake_install.cmake")
  include("/home/examples/instructionAPI/cmake_install.cmake")
  include("/home/examples/interceptOutput/cmake_install.cmake")
  include("/home/examples/maxMallocSize/cmake_install.cmake")
  include("/home/examples/memoryAccessCounter/cmake_install.cmake")
  include("/home/examples/parseAPI/cmake_install.cmake")
  include("/home/examples/proccontrol/cmake_install.cmake")
  include("/home/examples/readGlobalVariables/cmake_install.cmake")
  include("/home/examples/stackwalker/cmake_install.cmake")
  include("/home/examples/symbolicEvalInstructions/cmake_install.cmake")
  include("/home/examples/tracetool/cmake_install.cmake")
  include("/home/examples/unstrip/cmake_install.cmake")
  include("/home/examples/unusedArgs/cmake_install.cmake")

endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "/home/examples/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")

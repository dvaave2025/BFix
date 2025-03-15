# Install script for directory: /home/wen/bin-mleak-fix/dyninst/newexams/examples

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
  include("/home/wen/bin-mleak-fix/dyninst/newexams/examples/CFGraph/cmake_install.cmake")
  include("/home/wen/bin-mleak-fix/dyninst/newexams/examples/codeCoverage/cmake_install.cmake")
  include("/home/wen/bin-mleak-fix/dyninst/newexams/examples/dataflowAPI/cmake_install.cmake")
  include("/home/wen/bin-mleak-fix/dyninst/newexams/examples/disassemble/cmake_install.cmake")
  include("/home/wen/bin-mleak-fix/dyninst/newexams/examples/DynC/cmake_install.cmake")
  include("/home/wen/bin-mleak-fix/dyninst/newexams/examples/dyninstAPI/cmake_install.cmake")
  include("/home/wen/bin-mleak-fix/dyninst/newexams/examples/instrumentAFunction/cmake_install.cmake")
  include("/home/wen/bin-mleak-fix/dyninst/newexams/examples/instrumentMemoryAccess/cmake_install.cmake")
  include("/home/wen/bin-mleak-fix/dyninst/newexams/examples/insertSnippet/cmake_install.cmake")
  include("/home/wen/bin-mleak-fix/dyninst/newexams/examples/instructionAPI/cmake_install.cmake")
  include("/home/wen/bin-mleak-fix/dyninst/newexams/examples/interceptOutput/cmake_install.cmake")
  include("/home/wen/bin-mleak-fix/dyninst/newexams/examples/maxMallocSize/cmake_install.cmake")
  include("/home/wen/bin-mleak-fix/dyninst/newexams/examples/memoryAccessCounter/cmake_install.cmake")
  include("/home/wen/bin-mleak-fix/dyninst/newexams/examples/parseAPI/cmake_install.cmake")
  include("/home/wen/bin-mleak-fix/dyninst/newexams/examples/patchAPI/cmake_install.cmake")
  include("/home/wen/bin-mleak-fix/dyninst/newexams/examples/proccontrol/cmake_install.cmake")
  include("/home/wen/bin-mleak-fix/dyninst/newexams/examples/readGlobalVariables/cmake_install.cmake")
  include("/home/wen/bin-mleak-fix/dyninst/newexams/examples/stackwalker/cmake_install.cmake")
  include("/home/wen/bin-mleak-fix/dyninst/newexams/examples/symbolicEvalInstructions/cmake_install.cmake")
  include("/home/wen/bin-mleak-fix/dyninst/newexams/examples/symtabAPI/cmake_install.cmake")
  include("/home/wen/bin-mleak-fix/dyninst/newexams/examples/tracetool/cmake_install.cmake")
  include("/home/wen/bin-mleak-fix/dyninst/newexams/examples/unstrip/cmake_install.cmake")
  include("/home/wen/bin-mleak-fix/dyninst/newexams/examples/unusedArgs/cmake_install.cmake")

endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "/home/wen/bin-mleak-fix/dyninst/newexams/examples/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")

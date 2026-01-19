BFix is an auto binary-level memory leak fixing tool.

The static analysis is based on Dyninst, a binary analysis tool. BFix uses Dyninst to analyze the binary to get the needed information, such as instruction operands, opcode, allocation and deallocation, etc. Based on the data, BFix can identify the leak objects and the leak object paths, and find the appropriate point or edge to insert the patch code to fix the memory leak.

Environment Requirements

Only supports x86-64 architecture. If you are using a Mac or other device with Arm/Aarch64 or other architectures, you cannot run this Docker.
Ubuntu 20.04 is recommended


Background:

	Build dyninst:
	Configure Dyninst with CMake
	cmake /path/to/dyninst/source -DCMAKE_INSTALL_PREFIX=/path/to/installation
	make install -jN
	Build dyninst examples:
	cmake . -DDyninst_DIR=path/to/Dyninst/install/lib/cmake/Dyninst

Download:

	bfix.tar: https://zenodo.org/records/18264808


Install:
	
	Install Docker on your server. 
	Download bfix.tar.
	Use the command to import the image: docker import bfix.tar ubuntu:2
	Use the command to run the container: docker run -it ubuntu:2 /bin/bash


Usage:

    fixing analysis: /Path/to/Dyninst/examples/target leak-binary target-func

Testing:

	To use this artifact, please run the script “run.sh” under the path “/home”.

	You may find the paper under the path “/home”.
	More details could be found in README.txt under the path "/".



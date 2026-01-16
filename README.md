BFix is an auto binary-level memory leak fixing tool.

The static analysis is based on Dyninst, a binary analysis tool. BFix uses Dyninst to analyze the binary to get the needed information, such as instruction operands, opcode, allocation and deallocation, etc. Based on the data, BFix can identify the leak objects and the leak object paths, and find the appropriate point or edge to insert the patch code to fix the memory leak.


Install:
	Install Docker on your server. 
	Download bfix.tar.part_*
	Use the command: cat bfix.tar.part_* > bfix.tar
	Use the command to import the image: docker import bfix.tar ubuntu:2
	Use the command to run the container: docker run -it ubuntu:2 /bin/bash


Usage:

    fixing analysis: /Path/to/Dyninst/examples/target leak-binary


# BFix

BFix is an auto binary-level memory leak fixing tool.

The static analysis is based on Dyninst, a binary analysis tool. BFix uses Dyninst to analyze the binary to get the needed information, such as instruction operands, opcode, allocation and deallocation, etc. Based on the data, BFix can identify the leak objects and the leak object paths, and find the appropriate point or edge to insert the patch code to fix the memory leak.


Install:

    compile Dyninst 12.3.0 from the source code.
    compile Dyninst examples repo based on build dyninst.

Usage:

    fixing analysis: /Path/to/Dyninst/examples/target leak-binary


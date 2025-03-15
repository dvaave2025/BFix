#include <iostream>
#include <vector>

int main() {
    // Assembly code
    const char* assemblyCode =
        "push %rdi;"
        "mov -0x18(%rbp), %rax;"
        "mov %rax, %rdi;"
        "callq 0x2090;"
        "pop %rdi;";

    // Vector to store machine code
    std::vector<unsigned char> machineCode;

    // Parse assembly code and convert to machine code
    const char* ptr = assemblyCode;
    unsigned char currentByte = 0;
    int byteCount = 0;

    while (*ptr != '\0') {
        char hexDigit = *ptr;

        if ((hexDigit >= '0' && hexDigit <= '9') || (hexDigit >= 'a' && hexDigit <= 'f') || (hexDigit >= 'A' && hexDigit <= 'F')) {
            // Convert hex digit to integer
            int value = 0;
            if (hexDigit >= '0' && hexDigit <= '9') {
                value = hexDigit - '0';
            } else if (hexDigit >= 'a' && hexDigit <= 'f') {
                value = hexDigit - 'a' + 10;
            } else if (hexDigit >= 'A' && hexDigit <= 'F') {
                value = hexDigit - 'A' + 10;
            }

            // Update the current byte with the hex digit
            if (byteCount % 2 == 0) {
                currentByte = static_cast<unsigned char>(value << 4);
            } else {
                currentByte |= static_cast<unsigned char>(value);
                machineCode.push_back(currentByte);
            }

            ++byteCount;
        }

        ++ptr;
    }

    // Print the machine code (for verification)
    std::cout << "Machine code:" << std::endl;
    for (const auto& byte : machineCode) {
        std::cout << "0x" << std::hex << static_cast<int>(byte) << ", ";
    }
    std::cout << std::endl;

    // Your code to use the machine code vector as needed...

    return 0;
}


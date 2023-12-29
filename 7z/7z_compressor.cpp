#include "include/include/bit7z/bitfilecompressor.hpp"
#include <vector>
#include <string>
#include <iostream>

int main() {
    try {
        using namespace bit7z;

        Bit7zLibrary lib{ "7z.dll" };
        BitFileCompressor compressor{ lib, BitFormat::Zip };

        // 要压缩的字符串
        std::string inputString = "This is a sample string to be compressed.";

        // 将字符串写入文件
        std::ofstream inputFile("input.txt");
        inputFile << inputString;
        inputFile.close();

        // 准备用于保存压缩后数据的缓冲区
        std::vector<byte_t> compressedBuffer;

        // 压缩文件到缓冲区
        compressor.compressFile("input.txt", compressedBuffer);

        // 将压缩后的数据写入文件
        std::ofstream outputFile("output.zip", std::ios::binary);
        outputFile.write(reinterpret_cast<char*>(compressedBuffer.data()), compressedBuffer.size());
        outputFile.close();

        std::cout << "String compressed and saved to output.zip." << std::endl;

    } catch (const bit7z::BitException& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}

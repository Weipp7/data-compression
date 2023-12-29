#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <zlib.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

#define CHUNK_SIZE 16384

// 函数原型
int compressFile(const char *inputFileName, const char *outputFileName);
int uncompressFile(const char *inputFileName, const char *outputFileName);
void processDirectory(const char *dirPath);
void processFile(const char *filePath,const char *fileName);

double getCurrentTime();

int main() {
    char dirPath[256];

    // 获取用户输入的文件夹路径
    printf("Enter the directory path: ");
    scanf("%s", dirPath);

    // 记录开始时间
    double startTime = getCurrentTime();

    // 处理文件夹下的所有文件
    processDirectory(dirPath);

    // 记录结束时间
    double endTime = getCurrentTime();
    double totalTime = endTime - startTime;

    printf("Total compression time: %.2f seconds\n", totalTime);

    return 0;
}

void processDirectory(const char *dirPath) {
    DIR *dir;
    struct dirent *ent;

    if ((dir = opendir(dirPath)) != NULL) {
        // 遍历文件夹中的文件
        while ((ent = readdir(dir)) != NULL) {
            if (ent->d_type == DT_REG) { // 仅处理普通文件
                char filePath[512];
                snprintf(filePath, sizeof(filePath), "%s/%s", dirPath, ent->d_name);
                processFile(filePath,ent->d_name);
            }
        }
        closedir(dir);
    } else {
        perror("Error opening directory");
    }
}

void processFile(const char *filePath,const char *fileName) {
    const char *outputFile;
    char result[256+5];  // 这里假设文件名不会超过 256 个字符
    sprintf(result,"%s.zlib",fileName);
    outputFile = result;

    // 压缩文件
    if (compressFile(filePath, outputFile)) {
        printf("Compression for %s successful.\n", filePath);

        // 获取原文件大小
        struct stat inputStat;
        stat(filePath, &inputStat);
        long long originalSize = inputStat.st_size;

        // 获取压缩后文件大小
        struct stat outputStat;
        stat(outputFile, &outputStat);
        long long compressedSize = outputStat.st_size;

        // 计算压缩比率
        double compressionRatio = (double)originalSize / (double)compressedSize;

        // 输出结果
        printf("Original Size: %lld bytes\n", originalSize);
        printf("Compressed Size: %lld bytes\n", compressedSize);
        printf("Compression Ratio: %.2f\n", compressionRatio);

        // 解压缩文件
        // char decompressedFile[256+4+5];
        // sprintf(decompressedFile,"dec_%s",fileName);
        // uncompressFile(outputFile, decompressedFile);

        // 删除压缩文件
        remove(outputFile);
    } else {
        fprintf(stderr, "Compression for %s failed.\n", filePath);
    }
}

int compressFile(const char *inputFileName, const char *outputFileName) {
    FILE *inputFile = fopen(inputFileName, "rb");
    FILE *outputFile = fopen(outputFileName, "wb");

    if (!inputFile || !outputFile) {
        fprintf(stderr, "Error opening files.\n");
        return 0; // 失败
    }

    z_stream stream;
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;
    if (deflateInit(&stream, Z_DEFAULT_COMPRESSION) != Z_OK) {
        fclose(inputFile);
        fclose(outputFile);
        fprintf(stderr, "deflateInit failed.\n");
        return 0; // 失败
    }

    // 设置输入输出缓冲区
    unsigned char in[CHUNK_SIZE];
    unsigned char out[CHUNK_SIZE];

    // 读取文件并压缩
    while (!feof(inputFile)) {
        stream.avail_in = fread(in, 1, CHUNK_SIZE, inputFile);
        if (ferror(inputFile)) {
            fclose(inputFile);
            fclose(outputFile);
            deflateEnd(&stream);
            fprintf(stderr, "Error reading from file.\n");
            return 0; // 失败
        }

        if (stream.avail_in == 0)
            break;

        stream.next_in = in;

        do {
            stream.avail_out = CHUNK_SIZE;
            stream.next_out = out;
            if (deflate(&stream, Z_FINISH) == Z_STREAM_ERROR) {
                fclose(inputFile);
                fclose(outputFile);
                deflateEnd(&stream);
                fprintf(stderr, "deflate failed.\n");
                return 0; // 失败
            }
            fwrite(out, 1, CHUNK_SIZE - stream.avail_out, outputFile);
        } while (stream.avail_out == 0);
    }

    // 结束压缩
    do {
        stream.avail_out = CHUNK_SIZE;
        stream.next_out = out;
        if (deflate(&stream, Z_FINISH) == Z_STREAM_ERROR) {
            fclose(inputFile);
            fclose(outputFile);
            deflateEnd(&stream);
            fprintf(stderr, "deflate failed.\n");
            return 0; // 失败
        }
        fwrite(out, 1, CHUNK_SIZE - stream.avail_out, outputFile);
    } while (stream.avail_out == 0);

    // 结束压缩流
    if (deflateEnd(&stream) != Z_OK) {
        fclose(inputFile);
        fclose(outputFile);
        fprintf(stderr, "deflateEnd failed.\n");
        return 0; // 失败
    }

    fclose(inputFile);
    fclose(outputFile);

    return 1; // 成功
}

int uncompressFile(const char *inputFileName, const char *outputFileName) {
    FILE *inputFile = fopen(inputFileName, "rb");
    FILE *outputFile = fopen(outputFileName, "wb");
    // printf("in:%s out:%s\n",inputFileName,outputFileName);
    // sleep(2);
    if (!inputFile || !outputFile) {
        fprintf(stderr, "Error opening files.\n");
        return 0; // 失败
    }
    
    z_stream stream;
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;
    if (inflateInit(&stream) != Z_OK) {
        fclose(inputFile);
        fclose(outputFile);
        fprintf(stderr, "inflateInit failed.\n");
        return 0; // 失败
    }

    // 设置输入输出缓冲区
    unsigned char in[CHUNK_SIZE];
    unsigned char out[CHUNK_SIZE];

    // 读取文件并解压缩
    while (!feof(inputFile)) {
        stream.avail_in = fread(in, 1, CHUNK_SIZE, inputFile);
        if (ferror(inputFile)) {
            fclose(inputFile);
            fclose(outputFile);
            inflateEnd(&stream);
            fprintf(stderr, "Error reading from file.\n");
            return 0; // 失败
        }

        if (stream.avail_in == 0)
            break;

        stream.next_in = in;

        do {
            stream.avail_out = CHUNK_SIZE;
            stream.next_out = out;
            if (inflate(&stream, Z_NO_FLUSH) == Z_STREAM_ERROR) {
                fclose(inputFile);
                fclose(outputFile);
                inflateEnd(&stream);
                fprintf(stderr, "inflate failed.\n");
                return 0; // 失败
            }
            fwrite(out, 1, CHUNK_SIZE - stream.avail_out, outputFile);
        } while (stream.avail_out == 0);
    }

    // 结束解压缩流
    if (inflateEnd(&stream) != Z_OK) {
        fclose(inputFile);
        fclose(outputFile);
        fprintf(stderr, "inflateEnd failed.\n");
        return 0; // 失败
    }

    fclose(inputFile);
    fclose(outputFile);

    return 1; // 成功
}

double getCurrentTime() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)tv.tv_sec + (double)tv.tv_usec / 1000000.0;
}

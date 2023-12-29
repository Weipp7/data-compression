#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <bzlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <time.h>


#define BUFFER_SIZE 4096

void compressFile(const char *inputPath, const char *outputPath) {
    FILE *inputFile = fopen(inputPath, "rb");
    FILE *outputFile = fopen(outputPath, "wb");
    if (inputFile == NULL || outputFile == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    BZFILE *bzFile = BZ2_bzWriteOpen(NULL, outputFile, 9, 0, 30);
    if (bzFile == NULL) {
        perror("Error opening BZ2 file");
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFER_SIZE];
    int bytesRead;

    while ((bytesRead = fread(buffer, 1, BUFFER_SIZE, inputFile)) > 0) {
        BZ2_bzWrite(NULL, bzFile, buffer, bytesRead);
    }

    BZ2_bzWriteClose(NULL, bzFile, 0, NULL, NULL);
    fclose(inputFile);
    fclose(outputFile);
}

void decompressFile(const char *inputPath, const char *outputPath) {
    FILE *inputFile = fopen(inputPath, "rb");
    FILE *outputFile = fopen(outputPath, "wb");
    if (inputFile == NULL || outputFile == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    BZFILE *bzFile = BZ2_bzReadOpen(NULL, inputFile, 0, 0, NULL, 0);
    if (bzFile == NULL) {
        perror("Error opening BZ2 file");
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFER_SIZE];
    int bytesRead;

    while ((bytesRead = BZ2_bzRead(NULL, bzFile, buffer, BUFFER_SIZE)) > 0) {
        fwrite(buffer, 1, bytesRead, outputFile);
    }

    BZ2_bzReadClose(NULL, bzFile);
    fclose(inputFile);
    fclose(outputFile);
}

void processDirectory(const char *path) {
    DIR *dir;
    struct dirent *ent;

    if ((dir = opendir(path)) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            // printf("file:%s\n",ent->d_name);
            if (ent->d_type == DT_REG)  { // Regular file
                printf("file:%s\n",ent->d_name);
                char filePath[1024];
                sprintf(filePath, "%s/%s", path, ent->d_name);

                struct stat fileStat;
                if (stat(filePath, &fileStat) == 0) {
                    printf("File: %s\n", filePath);
                    printf("Original Size: %lld bytes\n", (long long)fileStat.st_size);

                     // Compress the file
                    char compressedPath[1024+4];
                    snprintf(compressedPath, sizeof(compressedPath), "%s.bz2", filePath);
                    clock_t start = clock();
                    compressFile(filePath, compressedPath);
                    clock_t end = clock();
                    // 获取压缩后文件大小
                    struct stat outputStat;
                    stat(compressedPath, &outputStat);
                    long long compressedSize = outputStat.st_size;

                    printf("Compressed Size: %lld bytes\n", compressedSize);
                    printf("Compression Ratio: %.2f\n", (double)fileStat.st_size / compressedSize);
                    printf("Compression Time: %.2f seconds\n\n", (double)(end - start) / CLOCKS_PER_SEC);

                    // Decompress the file for verification
                    // char decompressedPath[1024+4];
                    // snprintf(decompressedPath, sizeof(decompressedPath), "%s.dec", filePath);
                    // decompressFile(compressedPath, decompressedPath);

                    // 删除压缩文件
                    remove(compressedPath);
                } else {
                    perror("Error getting file stat");
                }
            }
        }
        closedir(dir);
    } else {
        perror("Error opening directory");
        exit(EXIT_FAILURE);
    }
}

int main() {
    char path[1024];
    printf("Enter the directory path: ");
    scanf("%s", path);

    processDirectory(path);

    return 0;
}

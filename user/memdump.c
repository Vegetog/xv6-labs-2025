#include "kernel/fcntl.h"
#include "kernel/types.h"
#include "user/user.h"

void memdump(char* fmt, char* data);

int main(int argc, char* argv[]) {
    if (argc == 1) {
        printf("Example 1:\n");
        int a[2] = {61810, 2025};
        memdump("ii", (char*)a);

        printf("Example 2:\n");
        memdump("S", "a string");

        printf("Example 3:\n");
        char* s = "another";
        memdump("s", (char*)&s);

        struct sss {
            char* ptr;
            int num1;
            short num2;
            char byte;
            char bytes[8];
        } example;

        example.ptr = "hello";
        example.num1 = 1819438967;
        example.num2 = 100;
        example.byte = 'z';
        strcpy(example.bytes, "xyzzy");

        printf("Example 4:\n");
        memdump("pihcS", (char*)&example);

        printf("Example 5:\n");
        memdump("sccccc", (char*)&example);
    } else if (argc == 2) {
        // format in argv[1], up to 512 bytes of data from standard input.
        char data[512];
        int n = 0;
        memset(data, '\0', sizeof(data));
        while (n < sizeof(data)) {
            int nn = read(0, data + n, sizeof(data) - n);
            if (nn <= 0) break;
            n += nn;
        }
        memdump(argv[1], data);
    } else {
        printf("Usage: memdump [format]\n");
        exit(1);
    }
    exit(0);
}

void memdump(char* fmt, char* data) {
    // Your code here.
    char* p = fmt;
    char* curr = data;  // 使用一个临时指针来遍历数据

    while (*p) {
        if (*p == 'i') {
            // 打印 4 字节整数
            printf("%d\n", *(int*)curr);
            curr += 4;
        } else if (*p == 'p') {
            // 打印 8 字节十六进制整数
            printf("%lx\n", *(uint64*)curr);
            curr += 8;
        } else if (*p == 'h') {
            // 打印 2 字节短整数
            // 注意：xv6 的 printf 可能需要 %d 来打印 short
            printf("%d\n", *(short*)curr);
            curr += 2;
        } else if (*p == 'c') {
            // 打印 1 字节字符
            printf("%c\n", *curr);
            curr += 1;
        } else if (*p == 's') {
            // 数据里存的是一个指向字符串的 8 字节指针
            char* str_ptr = *(char**)curr;
            printf("%s\n", str_ptr);
            curr += 8;
        } else if (*p == 'S') {
            // 数据里直接就是字符串的内容
            printf("%s\n", curr);
            // 'S' 标志通常意味着处理剩余的所有数据，或者直到遇到 \0
            // 根据题目描述 "the rest of the data"，打印后可以结束
            return;
        }
        p++;
    }
}

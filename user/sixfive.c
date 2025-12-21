#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"

// 定义状态
#define ST_READY   0  // 准备好接收新数字
#define ST_DIGITS  1  // 正在读数字
#define ST_INVALID 2  // 非法字符状态

void sixfive(int fd) {
    char c;
    int state = ST_READY;
    long long current_val = 0;
    char *seps = " -\r\t\n./,";

    // 逐字符读取，这是题目给出的 Hint
    while (read(fd, &c, 1) > 0) {
        if (strchr(seps, c)) {
            // 遇到分隔符，如果之前正在读有效数字，则判断并打印
            if (state == ST_DIGITS) {
                if (current_val % 5 == 0 || current_val % 6 == 0) {
                    printf("%lld\n", current_val);
                }
            }
            // 遇到分隔符后，重置状态为 READY
            state = ST_READY;
            current_val = 0;
        } else if (c >= '0' && c <= '9') {
            // 遇到数字
            if (state == ST_READY || state == ST_DIGITS) {
                state = ST_DIGITS;
                current_val = current_val * 10 + (c - '0');
            }
            // 如果本来就是 INVALID 状态，则保持 INVALID
        } else {
            // 遇到既非分隔符也非数字的字符（如 'v'）
            state = ST_INVALID;
            current_val = 0;
        }
    }

    // 隐式分隔符：处理文件末尾的情况
    if (state == ST_DIGITS) {
        if (current_val % 5 == 0 || current_val % 6 == 0) {
            printf("%lld\n", current_val);
        }
    }
}

int main(int argc, char *argv[]) {
    int fd, i;

    if (argc < 2) {
        fprintf(2, "usage: sixfive files...\n");
        exit(1);
    }

    for (i = 1; i < argc; i++) {
        if ((fd = open(argv[i], O_RDONLY)) < 0) {
            fprintf(2, "sixfive: cannot open %s\n", argv[i]);
            exit(1);
        }
        sixfive(fd);
        close(fd);
    }
    exit(0);
}
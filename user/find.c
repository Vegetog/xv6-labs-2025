#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "kernel/fs.h"
#include "kernel/stat.h"
#include "user/user.h"

// 获取路径中的文件名部分（指向 path 字符串中最后一个 '/' 之后的字符）
// 例如: "a/b/c" -> "c", "file" -> "file"
char* fmtname(char* path) {
    char* p;
    // 从字符串末尾开始向前查找第一个 '/'
    for (p = path + strlen(path); p >= path && *p != '/'; p--);
    p++;  // 移动到 '/' 之后的字符，或者如果没找到 '/' 则指向 path 开头
    return p;
}

void find(char* path, char* target) {
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;

    // 打开路径
    if ((fd = open(path, O_RDONLY)) < 0) {
        fprintf(2, "find: cannot open %s\n", path);
        return;
    }

    // 获取文件状态
    if (fstat(fd, &st) < 0) {
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
        return;
    }

    switch (st.type) {
        case T_FILE:
            // 如果是文件，检查文件名是否匹配
            // fmtname(path) 获取当前路径的文件名部分
            if (strcmp(fmtname(path), target) == 0) {
                printf("%s\n", path);
            }
            break;

        case T_DIR:
            // 如果是目录，检查路径长度是否溢出缓冲区
            if (strlen(path) + 1 + DIRSIZ + 1 > sizeof buf) {
                printf("find: path too long\n");
                break;
            }
            // 构造当前目录的路径前缀，例如 "a/"
            strcpy(buf, path);
            p = buf + strlen(buf);
            *p++ = '/';

            // 循环读取目录项
            while (read(fd, &de, sizeof(de)) == sizeof(de)) {
                if (de.inum == 0) continue;

                // 【关键】必须跳过 "." 和 ".."，否则会无限递归
                if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
                    continue;

                // 将目录项名称拼接到路径后，例如 "a/b"
                memmove(p, de.name, DIRSIZ);
                p[DIRSIZ] = 0;  // 确保字符串以 null 结尾

                // 递归调用 find
                find(buf, target);
            }
            break;
    }
    close(fd);
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(2, "Usage: find <path> <name>\n");
        exit(1);
    }
    // 调用 find 函数，传入路径和目标文件名
    find(argv[1], argv[2]);
    exit(0);
}
#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "kernel/riscv.h"

int
main(int argc, char *argv[])
{
  // 1. 申请足够的堆空间，尽可能覆盖 secret 释放的物理页
  // secret 分配了 32KB 的数据区，我们申请更多一点，比如 1MB (256个页)
  int total_size = 4096 * 256;
  char *buf = sbrk(total_size);
  
  if(buf == (char*)-1) exit(1);

  // 2. 在申请到的内存中搜索特征字符串 "This may help."
  for (char *p = buf; p < buf + total_size - 32; p++) {
    // 检查是否匹配特征字符串
    if (p[0] == 'T' && p[1] == 'h' && p[2] == 'i' && p[3] == 's' &&
        p[4] == ' ' && p[5] == 'm' && p[6] == 'a' && p[7] == 'y') {
      
      // 确认完整匹配 "This may help."
      if (strcmp(p, "This may help.") == 0) {
        // 3. 按照 secret.c 的逻辑，秘密在偏移 16 字节处
        char *secret = p + 16;
        if (*secret != '\0') {
          printf("%s\n", secret);
          exit(0);
        }
      }
    }
  }

  exit(1);
}

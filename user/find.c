#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
#include "kernel/param.h"  // 提供 MAXARG 定义

// 获取路径中的文件名部分
char*
fmtname(char *path)
{
  char *p;
  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;
  return p;
}

// 运行 exec 命令
// cmd_argv: 用户在命令行提供的命令参数 (例如 {"echo", "hi"})
// cmd_argc: 命令参数的个数
// file_path: 当前找到的文件路径 (例如 "./a/b")
void
run_exec(char **cmd_argv, int cmd_argc, char *file_path)
{
  char *argv[MAXARG];
  int i;

  // 1. 检查参数数量是否溢出
  // 需要 cmd_argc 个原参数 + 1 个文件路径 + 1 个 NULL 结束符
  if(cmd_argc + 2 > MAXARG){
    fprintf(2, "find: too many arguments for exec\n");
    return;
  }

  // 2. 构造新的 argv 数组
  // 复制原本的命令参数
  for(i = 0; i < cmd_argc; i++){
    argv[i] = cmd_argv[i];
  }
  // 追加当前文件路径
  argv[i] = file_path;
  // 追加 NULL 结束符
  argv[i+1] = 0;

  // 3. Fork 和 Exec
  int pid = fork();
  if(pid == 0){
    // 子进程
    exec(argv[0], argv);
    // 如果 exec 返回，说明出错了
    fprintf(2, "find: exec %s failed\n", argv[0]);
    exit(1);
  } else if (pid > 0){
    // 父进程等待子进程结束
    wait(0);
  } else {
    fprintf(2, "find: fork failed\n");
  }
}

// 查找函数
// path: 当前搜索路径
// target: 目标文件名
// exec_argv: 如果非空，则指向 -exec 后的命令参数数组
// exec_argc: 命令参数个数
void
find(char *path, char *target, char **exec_argv, int exec_argc)
{
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;

  if((fd = open(path, O_RDONLY)) < 0){
    fprintf(2, "find: cannot open %s\n", path);
    return;
  }

  if(fstat(fd, &st) < 0){
    fprintf(2, "find: cannot stat %s\n", path);
    close(fd);
    return;
  }

  // 检查是否匹配
  // 逻辑：如果是文件，且名字匹配 target，则处理
  // 注意：标准 find 会在目录匹配时也执行，但根据题目输出示例，
  // 主要是针对找到的 target 执行。这里我们保持文件名的匹配逻辑。
  if(st.type == T_FILE && strcmp(fmtname(path), target) == 0){
    if(exec_argv != 0){
      // 如果有 -exec 选项，执行命令
      run_exec(exec_argv, exec_argc, path);
    } else {
      // 否则，打印路径
      printf("%s\n", path);
    }
  }

  // 如果是目录，则递归
  if(st.type == T_DIR){
    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
      printf("find: path too long\n");
    } else {
      strcpy(buf, path);
      p = buf+strlen(buf);
      *p++ = '/';
      
      while(read(fd, &de, sizeof(de)) == sizeof(de)){
        if(de.inum == 0)
          continue;
        if(strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
          continue;
        
        memmove(p, de.name, DIRSIZ);
        p[DIRSIZ] = 0;
        
        // 递归调用，传递 exec 参数
        find(buf, target, exec_argv, exec_argc);
      }
    }
  }
  close(fd);
}

int
main(int argc, char *argv[])
{
  if(argc < 3){
    fprintf(2, "Usage: find <path> <name> [-exec <cmd>...]\n");
    exit(1);
  }

  char *path = argv[1];
  char *target = argv[2];
  char **exec_argv = 0;
  int exec_argc = 0;

  // 检查是否有 -exec 参数
  if(argc > 3){
    if(strcmp(argv[3], "-exec") == 0){
      // argv[4] 开始是命令，例如 {"echo", "hi"}
      exec_argv = &argv[4];
      exec_argc = argc - 4; // 计算命令参数个数
    } else {
      fprintf(2, "find: syntax error, expected -exec\n");
      exit(1);
    }
  }

  find(path, target, exec_argv, exec_argc);
  exit(0);
}
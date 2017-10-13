#ifdef _WIN32
#include <direct.h>
#include <io.h>
#define ACCESS _access
#define MKDIR(a) _mkdir((a))
#define STRDUP(a) _strdup((a))
#else
#include <stdarg.h>
#include <unistd.h>
#include <sys/stat.h>
#define ACCESS access
#define MKDIR(a) mkdir((a),0755)
#define STRDUP(a) strdup((a))
#endif

struct Dir { //Notice：直接在头文件写全局函数的实现，被多个cpp包含后，链接报错，c的编译原理啊~，包一层类声明就只编译一次了(貌似类是先扫描再编译的)

static void CreatDir(char* szDir /* "..\\log\\test\\" */)
{
    int kLen = (int)strlen(szDir);
    for (int i = 0; i < kLen; ++i) {
        if (szDir[i] == '\\' || szDir[i] == '/') {
            szDir[i] = '\0';

            //如果不存在,创建
            if (ACCESS(szDir, 0) != 0){
                if (MKDIR(szDir) != 0){
                    return;
                }
            }
            szDir[i] = '/'; //支持linux,将所有\\换成/
        }
    }
}
static void CreatDir(const char* dir)
{
    char* szDir = STRDUP(dir);
    CreatDir(szDir);
    free(szDir);
}

static const char* FindName(const char* file) {
    const int len = (int)strlen(file);
    for (int i = len - 1; i >= 0; --i)
    {
        if (file[i] == '\\' || file[i] == '/')
        {
            return &file[i + 1];
        }
    }
    return file;
}

};

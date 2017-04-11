#ifdef _WIN32
#include <direct.h>
#include <io.h>
#define ACCESS _access
#define MKDIR(a) _mkdir((a))
#elif _LINUX
#include <stdarg.h>
#include <unistd.h>
#include <sys/stat.h>
#define ACCESS access
#define MKDIR(a) mkdir((a),0755)
#endif

struct Dir { //Notice��ֱ����ͷ�ļ�дȫ�ֺ�����ʵ�֣������cpp���������ӱ���c�ı���ԭ��~����һ����������ֻ����һ����(ò��������ɨ���ٱ����)

static void CreatDir(char* szDir /* "..\\log\\test\\" */)
{
    int kLen = strlen(szDir);
    for (int i = 0; i < kLen; ++i) {
        if (szDir[i] == '\\' || szDir[i] == '/') {
            szDir[i] = '\0';

            //���������,����
            if (ACCESS(szDir, 0) != 0){
                if (MKDIR(szDir) != 0){
                    return;
                }
            }
            szDir[i] = '/'; //֧��linux,������\\����/
        }
    }
}
static void CreatDir(const char* dir)
{
    char* szDir = _strdup(dir);
    CreatDir(szDir);
    free(szDir);
}

static const char* FindName(const char* file) {
    const int len = strlen(file);
    for (int i = len - 1; i >= 0; --i)
    {
        if (file[i] == '\\')
        {
            return &file[i + 1];
        }
    }
    return file;
}

};
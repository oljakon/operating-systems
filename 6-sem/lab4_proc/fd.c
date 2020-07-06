#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>

#define BUF_SIZE 0x1000

int main()
{
    struct dirent *dirp;
    DIR *dp;
    
    char str[BUF_SIZE];
    char path[BUF_SIZE];
    
    dp = opendir("/proc/self/fd/");
 
    while ((dirp = readdir(dp)) != NULL)
    {
        if ((strcmp(dirp->d_name, ".") != 0) && (strcmp(dirp->d_name, "..") != 0))
        {
            sprintf(path, "%s%s", "/proc/self/fd/", dirp->d_name);
            readlink(path, str, BUF_SIZE);
            printf("%s -> %s\n", dirp->d_name, str);
        }
    }

    closedir(dp);
    return 0;
}

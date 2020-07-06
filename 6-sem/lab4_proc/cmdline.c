#include <stdio.h>
#include <unistd.h>

#define BUF_SIZE 0x1000

int main()
{
    char buffer[BUF_SIZE];
    FILE *f;
    int len;
    
    f = fopen("/proc/self/cmdline", "r");

    len = fread(buffer, 1, BUF_SIZE, f);
    buffer[--len] = 0;

    printf("cmdline for %d \nprocess = %s\n", getpid(), buffer);
    
    fclose(f);
    return 0;
}

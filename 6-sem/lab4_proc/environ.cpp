#include <stdio.h>

#define BUF_SIZE 0x1000

int main()
{
    char buffer[BUF_SIZE];
    int len, i;
    FILE *f;
    
    f = fopen("/proc/self/environ", "r");

    while ((len = fread(buffer, 1, BUF_SIZE, f)) > 0)
    {
        for (i = 0; i < len; i++)
            if (buffer[i] == 0)
                buffer[i] = 10;
        buffer[len] = 0;
        printf("%s", buffer);
    }
    
    fclose(f);
    return 0;
}

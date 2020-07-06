#include <fcntl.h>

int main()
{
    char c;
    int fd1 = open("alphabet.txt",O_RDONLY);
    int fd2 = open("alphabet.txt",O_RDONLY);
  
    char c1, c2;
    while ((read(fd1, &c1, 1)) && (read(fd2, &c2, 1)))
    {
        write(1, &c1, 1);
        write(1, &c2, 1);
    }

    close(fd1);
    close(fd2);

    return 0;
}

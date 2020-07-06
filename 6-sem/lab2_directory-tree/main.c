#include <stdio.h>
#include <dirent.h>    //opendir()/readdir()/closedir()
#include <sys/stat.h>
#include <string.h>
#include <error.h>
#include <unistd.h>
#include <errno.h>
            
static int dopath(const char *filename, int depth) {
    struct stat statbuf;
    struct dirent *dirp;
    DIR *dp;
    
    
    if (strcmp(filename, ".") == 0 || strcmp(filename, "..") == 0)
        return 0;

    //Вызов lstat() идентичен stat(), но в случае, если filename является символьной ссылкой, то
    //возвращается информация о самой ссылке, а не о файле, на который она указывает.
    if (lstat(filename, &statbuf) < 0) {
        switch(errno)
        {
            case EBADF:
                printf("Неверный файловый описатель filedes.");
                break;
            case ENOENT:
                printf("Компонент полного имени файла file_name не существует или полное имя является пустой строкой.");
                break;
            case ENOTDIR:
                printf("Компонент пути не является каталогом. ");
                break;
            case ELOOP:
                printf("При поиске файла встретилось слишком много символьных ссылок.");
                break;
            case EFAULT:
                printf("Некорректный адрес. ");
                break;
            case EACCES:
                printf("Запрещен доступ. ");
                break;
            case ENOMEM:
                printf("Недостаточно памяти в системе.");
                break;
            case ENAMETOOLONG:
                printf("Слишком длинное название файла. ");
                break;
        }
        printf("Ошибка вызова функции stat");
        return -1;

    }
    
    
    for (int i = 0; i < depth; ++i)
        printf("       |");
    

    // не каталог просто печатаем имя файла
    if (S_ISDIR(statbuf.st_mode) == 0) {
        printf("%s\n", filename);
        return 0;
    }

    // каталог
    printf("%s\n", filename);
    if ((dp = opendir(filename)) == NULL) {
        printf("couldn't open directory '%s'\n", filename);
        return 1;
    }
    
    chdir(filename);
    
    // для каждого элемента каталога
    while ((dirp = readdir(dp)) != NULL)
        dopath(dirp -> d_name, depth+1); //рекурсия
    chdir("..");

    //закрывает поток каталога и освобождает ресурсы, выделенные ему.
    //возвращает 0 в случае успеха и -1 при наличии ошибки.
    closedir(dp);
}

int main(int argc, char *argv[]) {
    int ret; ///код возврата

    if (argc != 2) // нет параметра
        ret = dopath("./",0); // начиная с текущей папки
    else
        ret = dopath(argv[1],0);    // начиная с папки в параметре

    return ret;
}

#include <syslog.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#define LOCKFILE "var/run/daemon.pid"
#define LOCKMODE (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)

int lockfile(int fd)
{
    struct flock fl;
    fl.l_type = F_WRLCK;
    fl.l_start = 0;
    fl.l_whence = SEEK_SET;
    fl.l_len = 0;
    fl.l_pid = getpid();

    return fcntl(fd, F_SETLK, &fl);
}

int already_running()
{
    int fd;
    char buf[16];

    fd = open(LOCKFILE, O_RDWR|O_CREAT, LOCKMODE);
    if (fd < 0)
    {
        syslog(LOG_ERR, "невозможно открыть %s: %s", LOCKFILE, strerror(errno));
        exit(1);
    }
    if (lockfile(fd) < 0)
    {
        if (errno == EACCES || errno == EAGAIN)  // EACCES - запрет доступа, EAGAIN - ресурс недоступен
        {
            close(fd);
            return 1;
        }
        syslog(LOG_ERR, "невозможно установить блокировку на %s: %s", LOCKFILE, strerror(errno));
        exit(1);
    }

    ftruncate(fd, 0);
    sprintf(buf, "%ld", (long)getpid());
    write(fd, buf, strlen(buf) + 1);
    return 0;
}

void daemonize(const char *cmd)
{
    int fd0, fd1, fd2;
    pid_t pid;
    struct rlimit rl; // описывает ограничения, установленные на ресурс
    struct sigaction sa;

    
    // 1 правило. Сбрасывание маски режима создания файла
    // umask можно вызвать в процессе предке, потому что потомок наследует маску режима создания
    // файлов, а сам предок вскоре завершается
    umask(0);

    // Получение максимального возможного номера дискриптора
    if (getrlimit(RLIMIT_NOFILE, &rl) < 0)
        perror("невозможно получить максимально возможный номер дескриптора ");
    
    // 2 правило. Стать лидером новой сессии, чтобы утратить управляющий терминал
    if ((pid = fork()) < 0)
        perror("ошибка fork ");
    else if (pid)
        exit(0);
    
    // 3 правило. Создать новую сессию
    setsid();

    // Игнорируем сигнал SIGHUP о потере терминала, который приведет к завершению процесса
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGHUP, &sa, NULL) < 0)
        perror("невозможно игнорировать сигнал SIGHUP ");

    // 4 правило. Назначить корневой каталог текущим рабочим каталогом,
    // чтобы впоследствии можно было отмонтировать файловую систему
    if (chdir("/") < 0)
        perror("невозможно стать текущим рабочим каталогом ");

    // 5 правило. Закрыть все файловые дескрипторы
    if (rl.rlim_max == RLIM_INFINITY)
        rl.rlim_max = 1024;
    for (int i = 0; i < rl.rlim_max; i++)
        close(i);

    // 6 правило. Присоеденить файловые дескрипторы 0, 1, 2 к /dev/null, чтобы можно было
    // использовать функции стандартных библиотек ввода-вывода и они не выдавали ошибки
    fd0 = open("/dev/null", O_RDWR);
    fd1 = dup(0);
    fd2 = dup(0);

    // Инициализировать файл журнала
    openlog(cmd, LOG_CONS, LOG_DAEMON);
    if (fd0 != 0 || fd1 != 1 || fd2 != 2)
    {
        syslog(LOG_ERR, "ошибочные файловые дескрипторы %d, %d, %d", fd0, fd1, fd2);
        exit(1);
    }
}

int main()
{
    daemonize("daemon");
    
    // Блокировка файла для одной существующей копии демона
    if (already_running())
    {
        syslog(LOG_ERR, "демон уже запущен");
        exit(1);
    }

    while (1)
    {
        syslog(LOG_INFO, "демон работает");

        sleep(5);
    }

    exit(0);
}

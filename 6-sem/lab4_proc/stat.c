#include <stdio.h>
#include <string.h>

#define BUF_SIZE 0x1000

static char* stat_parameters[] = {"pid", "comm", "state", "ppid", "pgrp", "session",
"tty_nr", "tpgid", "flags", "minflt", "cminflt", "majflt", "cmajflt", "utime",
"stime", "cutime", "cstime", "priority", "nice", "num_threads", "itrealvalue",
"starttime", "vsize", "rss", "rsslim", "startcode", "endcode", "startstack",
"kstkesp", "kstkeip", "signal", "blocked", "sigignore","sigcatch", "wchan",
"nswap", "сnswap", "exit_signal", "processor", "rt_priority", "policy",
"delayacct_blkio_ticks","guest_time", "cguest_time", "start_data", "end_data",
"start_brk", "arg_start", "arg_end", "env_start", "env_end", "exit_code"};

int main()
{
    char buffer[BUF_SIZE];
    FILE *f;
    
    f = fopen("/proc/self/stat", "r");

    fread(buffer, 1, BUF_SIZE, f);
    char *pch = strtok(buffer, " ");

    int i = 0;
    while (pch != NULL)
    {
        printf("%s = ", stat_parameters[i]);
        i++;
        printf("%s\n", pch);
        pch = strtok(NULL, " ");
    }

    fclose(f);
}

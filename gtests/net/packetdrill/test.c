#include <stdio.h>
#include <time.h>
#include <stdarg.h>

#include "test.h"
#include "types.h"
#include "run.h"

extern void LogTest(const char*s, ...){
    time_t now = time(NULL);
    FILE *log = fopen(LOGPATH, "a+");
    struct tm *t = localtime(&now);
    fprintf(log,"\n\n\nThis scrip %s run at the time: %04d/%02d/%02d %02d:%02d:%02d\n",s,
           t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
           t->tm_hour, t->tm_min, t->tm_sec);
    fclose(log);
}


extern void Loginfo(const char *s, ...){
    va_list args;
    va_start(args,s);
    char* arg1 = va_arg(args, char*);
   // char* arg3 = va_arg(args, char*);
    FILE *log = fopen(LOGPATH, "a+");
    if(arg1!=NULL)
        fprintf(log,"%s ", arg1);
    fprintf(log,"%s : %9.6f\n",
            s,usecs_to_secs(now_usecs(NULL)));
    fclose(log);
    va_end(args);
}
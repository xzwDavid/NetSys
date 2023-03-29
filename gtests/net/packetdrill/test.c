#include <stdio.h>
#include <time.h>
#include <stdarg.h>

#include "test.h"
#include "types.h"
#include "run.h"



struct event_log{
    double begin,end;
    char name[20];
    const char* event_type;
    int packet_type;//out:1 in:0
};

struct syscall_log{
    double picked,invoked,received,returned;
};

struct packet_log{
    double picked,dev_net,received,send;
    int type;//out:1 //in:0
    int index;
};

struct syscall_log syscallLog[10];
struct packet_log packetLog[10];
struct event_log eventLog[10];
int index1 = 0,index2 = 0,index3 = 0;
double time_begin,time_end;
double time_picked,time_first,time_done;

int Verbose=0;

extern void setVerbose(int verbose){
    Verbose = verbose;
}


extern void LogTest(const char*s, ...){
    if(Verbose){
        time_t now = time(NULL);
        FILE *log = fopen(TIMESTAMPPATH, "a+");
        struct tm *t = localtime(&now);
        fprintf(log,"\n\n\nThis scrip %s run at the time: %04d/%02d/%02d %02d:%02d:%02d\n",s,
                t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
                t->tm_hour, t->tm_min, t->tm_sec);
        fclose(log);
    }
}


extern void Loginfo(const char *s, ...){
    if(Verbose){
        va_list args;
        va_start(args,s);
        char* arg1 = va_arg(args, char*);
        FILE *log = fopen(TIMESTAMPPATH, "a+");
        if(arg1!=NULL)
            fprintf(log,"%s ", arg1);
        double num = usecs_to_secs(now_usecs(NULL));
        int integer = (int)num;
        int deci = (int)(1000000*(num-integer));
        fprintf(log,"%s : %d.%d\n",
                s,integer,deci);
        fclose(log);
        va_end(args);
    }
}

extern void writeLog(){
//    for(int i=0;i<=index2;i++){
//        printf("type:%f, name:%f \n",packetLog[i].dev_net,packetLog[i].end);
//    }
    FILE *fp = fopen(LOGPATH,"a+");
    fprintf(fp,"1.Total time to run one PD script:%9.6f\n\n",time_end-time_begin);
    fprintf(fp,"2.Initialization stage: %9.6f\n",time_first-time_begin);
    fprintf(fp,"\n3.Time to run each command in a script:\n");
    for(int i=1;i<=index3;i++){
        fprintf(fp,"the event name:%s,type:%s,time_len:%9.6f\n",
                eventLog[i].name,eventLog[i].event_type,eventLog[i].end-eventLog[i].begin);
    }
    fprintf(fp,"\n4.Time to run a system call command:\n");
    int tmp=0;
    for(int i=1;i<=index3;i++){
        if(strcmp(eventLog[i].event_type,"packet") != 0) {
            //printf("the name:%s\n",eventLog[i].event_type);
            fprintf(fp,"Function %s :\n",eventLog[i].name);
            fprintf(fp, "a.Pre-processing stage:%9.6f\n", syscallLog[tmp].invoked - syscallLog[tmp].picked);
            fprintf(fp, "b.Transmission stage:%9.6f\n", syscallLog[tmp].received - syscallLog[tmp].invoked);
            fprintf(fp, "c.Execution stage:%9.6f\n", syscallLog[tmp].returned - syscallLog[tmp].received);
            fprintf(fp, "d.Result processing stage:%9.6f\n",eventLog[i].end-syscallLog[tmp++].returned);
        }
    }
    fprintf(fp,"\n5.Time to run an inbound packet command\n");
    tmp = 0;
    for(int i=1;i<=index3;i++){
        if(strcmp(eventLog[i].event_type,"packet") != 0)
            continue;
        if(eventLog[i].packet_type == 0){
            while(packetLog[tmp].type !=0){tmp++;}
            fprintf(fp,"a.Pre-processing stage:%9.6f\n",packetLog[tmp].send-packetLog[tmp].picked);
            // printf("the end event:%f,  %f\n",packetLog[tmp].send,packetLog[tmp].picked);
            fprintf(fp,"b.Packet waiting stage:%9.6f\n",packetLog[tmp].received-packetLog[tmp].send);
            fprintf(fp,"c.Packet processing stage:%9.6f\n",eventLog[packetLog[tmp].index].end-packetLog[tmp].received);
        }
        tmp++;

    }
    fprintf(fp,"\n6.Time to run an outbound packet command\n");
    tmp = 0;
    for(int i=1;i<=index3;i++) {
        if(strcmp(eventLog[i].event_type,"packet") != 0)
            continue;
        // printf("%d,%s\n",eventLog[i].packet_type,eventLog[i].event_type);
        if (eventLog[i].packet_type == 1) {
            while(packetLog[tmp].type !=1){tmp++;}
            fprintf(fp,"a.Pre-processing stage:%9.6f\n",packetLog[tmp].dev_net-packetLog[tmp].picked);
            fprintf(fp,"b.Packet waiting stage:%9.6f\n",packetLog[tmp].received-packetLog[tmp].dev_net);
            // printf("the end event:%f,  %f\n",eventLog[i].end,packetLog[tmp].received);
            fprintf(fp,"c.Packet processing stage;%9.6f\n",eventLog[packetLog[tmp].index].end-packetLog[tmp].received);
//            printf("%f,%f\n",packetLog[tmp].dev_net,packetLog[tmp].picked);
//            printf("%f,%f\n",packetLog[tmp].received,packetLog[tmp].dev_net);
//            printf("%f,%f\n",eventLog[packetLog[tmp].index].end,packetLog[tmp].received);
        }
        tmp++;

    }
    fprintf(fp,"\n7.Finalization Stage:%9.6f\n\n",time_end-time_done);

    fclose(fp);
    return;
}

extern void process(){
    int First = 1;
    char line[200];
    FILE *tm = fopen(TIMESTAMPPATH, "r");
    int packet_filter = 0;
    while (fgets(line, 200, tm)) {
        if (sscanf(line, "Begin : %lf", &time_begin) == 1) {
            //printf("Begin:%f",time_begin);
            continue;
        }else if(strstr(line,"next event syscall")){
            sscanf(line, "next event syscall : %lf\n", &time_picked);
            if(First){First=0;time_first = time_picked;}
            eventLog[index3].end = time_picked;
            eventLog[++index3].begin = time_picked;
            eventLog[index3].event_type = "syscall";
        }else if(strstr(line,"next event packet")){
            sscanf(line, "next event packet : %lf\n", &time_picked);
            eventLog[index3].end = time_picked;
            eventLog[++index3].begin = time_picked;
            eventLog[index3].event_type = "packet";
            packet_filter = 1;
            packetLog[index2].index = index3;
        }else if (strstr(line, "syscall picked")) {
            sscanf(line, "%s syscall picked : %lf\n",eventLog[index3].name, &time_picked);
            syscallLog[index1].picked = time_picked;
        }else if (strstr(line, "syscall invoked")) {
            sscanf(line, "%*s syscall invoked : %lf\n", &time_picked);
            syscallLog[index1].invoked = time_picked;
        }else if (strstr(line, "syscall received")) {
            sscanf(line, "%*s %*s received : %lf\n", &time_picked);
            syscallLog[index1].received = time_picked;
        }else if (strstr(line, "syscall returned")) {
            sscanf(line, "%*s syscall returned : %lf\n", &time_picked);
            syscallLog[index1++].returned = time_picked;
        }else if (strstr(line, "packet picked")) {
            sscanf(line, "%*s picked : %lf\n", &time_picked);
            packetLog[index2].picked = time_picked;
        }else if (strstr(line, "net_dev")) {
            sscanf(line, "net_dev %*s %*s : %lf\n", &time_picked);
            //printf("dev:%f\n",time_picked);
            //printf("the netdev:%f\n",time_picked);
            packetLog[index2].dev_net = time_picked;
        }else if (strstr(line, "received Packet")) {
            // printf("line%s",line);
            sscanf(line, "%s %*s : %lf\n", eventLog[index3].name,&time_picked);
            if(packet_filter){
                packet_filter = 0;
                eventLog[index3].packet_type = 1;
                packetLog[index2].type = 1;
            }
            //  printf("received:%f\n",time_picked);
            packetLog[index2++].received = time_picked;
        }else if(strstr(line,"send packet")){
            sscanf(line, "%*s packet : %lf\n", &time_picked);
            if(packet_filter){
                packet_filter = 0;
                eventLog[index3].packet_type = 0;
                packetLog[index2].type = 0;
            }
            packetLog[index2].send = time_picked;
        }else if(strstr(line,"script end")){
            sscanf(line,"script end : %lf",&time_done);
            eventLog[index3].end = time_done;
        }else if(strstr(line,"done Packet send")){
            sscanf(line, "done Packet %s : %lf\n",eventLog[index3].name, &time_picked);
            packetLog[index2++].received = time_picked;
        }else if (sscanf(line, "End : %lf", &time_end) == 1) {
            break;
        }
    }
    // printf("the value of index1 is %d,index2 is %d,index3 is %d\n",index1,index2,index3);
    writeLog();
    fclose(tm);
//        printf("\n\n");
//        for(int i=0;i<index2;i++){
//            printf("%d\n",packetLog[i].type);
//        }
    return;
}


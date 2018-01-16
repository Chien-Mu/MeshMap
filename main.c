/*
 * -host 06:00:00:00:00:00 -ab "MAC: 06:01:22:00:00:01, RSSI: -38/-42" -ac "MAC: 06:01:22:00:00:D3, RSSI: -43/-37" -bc "MAC: 06:01:22:00:00:D3, RSSI: -51/-54"
 * -in 1.txt
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "meshpos.h"
#define ERR_EXIT(m) \
    do { \
        perror(m); \
        exit(EXIT_FAILURE); \
    } while(0)

char *cgi = "-host 06:00:00:00:00:00"
            " -ab \"MAC: 06:01:22:00:00:01, RSSI: -38/-42\""
            " -ac \"MAC: 06:01:22:00:00:D3, RSSI: -43/-37\""
            " -bc \"MAC: 06:01:22:00:00:D3, RSSI: -51/-54\"";

void handler(int, siginfo_t *, void *);

int main(int argc, char *argv[])
{
    int _argc=0;
    unsigned int isfile=0,check=0,i=0;
    char **_argv = NULL;
    struct wap_t waps[3];
    waps[0] = init_wap();
    waps[1] = init_wap();
    waps[2] = init_wap();

    //register siganl
    struct sigaction act;
    act.sa_sigaction = handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO;

    if (sigaction(2, &act, NULL) < 0) //use SIGRTMIN signal
        ERR_EXIT("sigaction error");

    while(1){
        //wait signal
        //pause();

        //find parameter
        for(i=0;i<(unsigned int)argc;i++)
            if(indexOf(argv[i],IN) != -1 && i+1 < (unsigned int)argc){
                _argv = (char**)malloc(((PARAMETER_COUNT*2) + 1)  * sizeof(char*));
                _argv[0] = (char*)malloc(strlen(argv[i+1]) + 1);
                strcpy(_argv[0], argv[i+1]); //copy filename string
                isfile = 1;
                break;
            }

        //input waps array
        if(isfile == 1){
            check = readfile(_argv[0], &_argc, _argv); //read file

            if(check == 1){
                release(&_argc, _argv);
                print_file_error(_argv[0]);
                return 0;
            }else if(check == 2){
                release(&_argc, _argv);
                return 0;
            }

            if(argv_to_struct(_argc, _argv, waps)){
                release(&_argc, _argv);
                print_file_error(_argv[0]);
                return 0;
            }
        }else{
            printf("parameter error.\n./MeshMap -in filename\n");
            return 0;
        }

        //calc
        rssi2dist(waps);
        dist2coordinate(waps);

        //output
        printf("5G:\n");
        for(i=0;i<3;i++)
            printf("coor=%s=(%.2f,%.2f);\n", waps[i].bssid, waps[i].X,waps[i].Y);
        for(i=0;i<3;i++)
            printf("dist=%s-%s=%.1fm=%.f;\n", waps[i].bssid, waps[i].neighbor.link->bssid,
                   waps[i].neighbor.distance, waps[i].neighbor.rssi_merge);

        //release
        if(isfile == 1)
            release(&_argc, _argv);
    }

    return 0;
}

void handler(int sig, siginfo_t *info, void *ctx)
{
    printf("recv a sig=%d data=%d\n", sig, info->si_value.sival_int);
}

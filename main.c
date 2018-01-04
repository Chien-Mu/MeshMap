/*
 * -host 06:00:00:00:00:00 -ab "MAC: 06:01:22:00:00:01, RSSI: -38/-42" -ac "MAC: 06:01:22:00:00:D3, RSSI: -43/-37" -bc "MAC: 06:01:22:00:00:D3, RSSI: -51/-54"
 * -in 1.txt
 */

#include <stdio.h>
#include "meshpos.h"

char *cgi = "-host 06:00:00:00:00:00"
            " -ab \"MAC: 06:01:22:00:00:01, RSSI: -38/-42\""
            " -ac \"MAC: 06:01:22:00:00:D3, RSSI: -43/-37\""
            " -bc \"MAC: 06:01:22:00:00:D3, RSSI: -51/-54\"";

int main(int argc, char *argv[])
{
    int _argc=0;
    unsigned int isfile=0,i=0;
    char **_argv = NULL;
    struct wap_t waps[3];
    waps[0] = init_wap();
    waps[1] = init_wap();
    waps[2] = init_wap();

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
        readfile(_argv[0], &_argc, _argv);
        if(argv_to_struct(_argc, _argv, waps)){
            release(&_argc, _argv);
            return 0;
        }
    }else
        if(argv_to_struct(argc, argv, waps))
            return 0;

    //calc
    rssi2dist(waps);
    dist2coordinate(waps);


    for(i=0;i<3;i++)
        printf("(%.2f,%.2f)\n", waps[i].X,waps[i].Y);

    //release
    if(isfile == 1)
        release(&_argc, _argv);

    return 0;
}

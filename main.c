#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include "meshpos.h"

void daemon_init();
unsigned isExecute();   //check time
time_t default_time;
time_t execute_time_previous;
struct regular {
    unsigned range;
    unsigned cycle;
};

const unsigned regular_size = 3;
const struct regular regular_table[] = {
    /* range 需案大小排列 */
    {10, 1},
    {20, 3},
    {30, 5}
};


int main(int argc, char *argv[])
{    
    //cycle
    default_time = time(NULL);
    execute_time_previous = default_time;

    //calc
    node_t *node = 0;
    line_t *line = 0;    
    const unsigned int size = 3;
    data_t data[3];      

    daemon_init();

    while(1){
        sleep(1); //cycle delay
        if(!isExecute())
            continue;

        //test data
        strcpy(data[0].bssid1,"aaa");
        strcpy(data[0].bssid2,"bbb");
        data[0].rssi1 = -38;
        data[0].rssi2 = -37;
        strcpy(data[1].bssid1,"aaa");
        strcpy(data[1].bssid2,"ccc");
        data[1].rssi1 = -36;
        data[1].rssi2 = -35;
        strcpy(data[2].bssid1,"ccc");
        strcpy(data[2].bssid2,"bbb");
        data[2].rssi1 = -34;
        data[2].rssi2 = -33;

        //position calc
        triangle_calc(data, size, &node, &line);

        //resul
        for(unsigned i=0; i<size; i++)
            printf("%s = (%f,%f)\n", node[i].bssid, node[i].point.X, node[i].point.Y);
        for(unsigned i=0; i<size; i++)
            printf("%s:%s, distance = %f, RSSI = %f\n", line[i].node1->bssid, line[i].node2->bssid, line[i].distance, line[i].rssi_merge);

        //release
        if(node)
            free(node);
        if(line)
            free(line);

        printf("cycle time:%ld, total time:%ld\n", time(NULL) - execute_time_previous, time(NULL) - default_time);
        execute_time_previous = time(NULL); //default
    }

    return 0;
}

void daemon_init(){
    pid_t pid;

    if((pid = fork()) != 0)
        exit(0);
}

unsigned isExecute(){
    unsigned int i;
    time_t current_time = time(NULL);
    time_t execute_time = current_time - default_time;

    //最長 range 時間判斷
    if(execute_time >= regular_table[regular_size - 1].range){
        if(labs(current_time - execute_time_previous) >= regular_table[regular_size - 1].cycle) //與上一次經過了多久
            return 1;
        else
            return 0;
    }

    //若沒超過最長 range 時間
    for(i=0; i<regular_size; i++)
        if(execute_time <= regular_table[i].range){ //在哪個範圍裡
            if(labs(current_time - execute_time_previous) >= regular_table[i].cycle) //與上一次經過了多久
                return 1;
            else
                return 0;
        }

    return 0;
}


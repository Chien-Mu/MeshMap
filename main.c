#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include "meshpos.h"

enum Frequency {
    /* 越上面，優先權越高 */
    Fast,
    Normal,
    Stable
};
struct rule_t {
    enum Frequency freq;
    unsigned int interval;
};

void daemon_init();
unsigned int get_interval(enum Frequency freq); //取得 Frequency 定義的間格時間
enum Frequency get_high_priority_frequency(enum Frequency *freqs, unsigned int size);   //取得參數中最高的優先權
enum Frequency check_mesh_state();
enum Frequency check_rssi_stability();
void update_frequency();    //執行可能改變 frequency 的條件
unsigned int isExecute();   //check time

/* rule table, unit: sec */
const unsigned int rule_size = 3;
const struct rule_t rule_table[] = {
    {Fast, 1},
    {Normal, 2},
    {Stable, 5}
};

/* global variable */
enum Frequency current_frequency = Fast;
time_t previous_execute_time;
time_t start_time; //test

int main(int argc, char *argv[])
{    
    //default
    start_time = time(NULL);
    previous_execute_time = time(NULL);

    //calc
    node_t *node = 0;
    line_t *line = 0;    
    const unsigned int data_size = 3;
    data_t data[data_size];

    //daemon_init();

    while(1){
        sleep(1); //cycle delay

        update_frequency();

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
        triangle_calc(data, data_size, &node, &line);

        //resul
        for(unsigned i=0; i<data_size; i++)
            printf("%s = (%f,%f)\n", node[i].bssid, node[i].point.X, node[i].point.Y);
        for(unsigned i=0; i<data_size; i++)
            printf("%s:%s, distance = %f, RSSI = %f\n", line[i].node1->bssid, line[i].node2->bssid, line[i].distance, line[i].rssi_merge);

        //release
        if(node)
            free(node);
        if(line)
            free(line);

        printf("total time:%ld, frequency=%d\n", time(NULL) - start_time, current_frequency);
    }

    return 0;
}

void daemon_init(){
    pid_t pid;

    if((pid = fork()) != 0)
        exit(0);
}

unsigned int get_interval(enum Frequency freq){
    unsigned int i;
    for(i=0; i<rule_size; i++)
        if(freq == rule_table[i].freq)
            return rule_table[i].interval;

    return rule_table[0].interval; //防呆
}

enum Frequency get_high_priority_frequency(enum Frequency *freqs, unsigned int size){
    unsigned int i;
    enum Frequency min = 1000;
    for(i=0; i<size ;i++)
        if(freqs[i] < min)
            min = freqs[i];

    return min;
}

enum Frequency check_mesh_state(){
    time_t total_time = time(NULL) - start_time;
    if(total_time > 50)
        return Fast;
    return Stable;
}

enum Frequency check_rssi_stability(){
    time_t total_time = time(NULL) - start_time;
    if(total_time > 10 && total_time < 30)
        return Normal;
    else if(total_time > 30)
        return Stable;
    else
        return Fast;
}

void update_frequency(){
    enum Frequency freqs[2];
    freqs[0] = check_mesh_state();
    freqs[1] = check_rssi_stability();

    current_frequency = get_high_priority_frequency(freqs, 2);
}

unsigned int isExecute(){
    unsigned int i;
    unsigned int isExe = 0;
    time_t interval_time = time(NULL) - previous_execute_time;

    for(i=0; i<rule_size; i++)
        if(interval_time >= get_interval(current_frequency)){
            isExe = 1;
            break;
        }

    if(isExe){
        previous_execute_time = time(NULL); //default
        return 1; //execute
    }else{
        return 0;
    }
}


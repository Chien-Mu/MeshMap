#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "libcmm.h"

/* function */
void daemon_init();
void input_test_cm_info_data(cm_info_t *ref_cm);
cm_info_t init_cm_into(){
    cm_info_t d;
    memset(&d, 0, sizeof(cm_info_t));
    return d;
}

/* global variable */
time_t DURATION_TIME;

int main()
{
    daemon_init();

    cm_info_t mesh_info;
    srand((unsigned)time(NULL));
    printf("Mesh Map Test!\n");

    while(1){
        mesh_info = init_cm_into();

        get_mesh_info(&mesh_info);
        input_test_cm_info_data(&mesh_info);
        set_mesh_position(&mesh_info);
	
	sleep(10);
    }
    return 0;
}

void daemon_init(){
    pid_t pid;

    if((pid = fork()) != 0)
        exit(0);
}

void input_test_cm_info_data(cm_info_t *ref_cm){
    ref_cm->node_cnt = 3;

    strcpy(ref_cm->node[0].id, "aaa");
    strcpy(ref_cm->node[0].bssid_5g, "aaa_5g");
    ref_cm->node[0].master = 1;
    ref_cm->node[0].online = 1;
    ref_cm->node[0].vap5g_cnt = 0;
    ref_cm->node[0].rssi = 0 - (rand()%21) + 30;
    ref_cm->node[0].dist = rand()%20;
    ref_cm->node[0].x = 0;
    ref_cm->node[0].y = 0;

    strcpy(ref_cm->node[1].id, "bbb");
    strcpy(ref_cm->node[1].bssid_5g, "bbb_5g");
    ref_cm->node[1].online = 1;
    strcpy(ref_cm->node[1].uplink, "aaa");
    ref_cm->node[1].rssi = 0 - (rand()%21) + 30;
    ref_cm->node[1].dist = rand()%20;
    ref_cm->node[1].x = rand()%20;
    ref_cm->node[1].y = rand()%20;

    ref_cm->node[1].vap5g_cnt = 5;
    strcpy(ref_cm->node[1].vap5g[0].bssid, "123");
    strcpy(ref_cm->node[1].vap5g[1].bssid, "1234");
    strcpy(ref_cm->node[1].vap5g[2].bssid, "aaa_5g");
    strcpy(ref_cm->node[1].vap5g[3].bssid, "ccc_5g");
    strcpy(ref_cm->node[1].vap5g[4].bssid, "bbb_5g");
    ref_cm->node[1].vap5g[3].signal = 0 - (rand()%21) + 30;

    strcpy(ref_cm->node[2].id, "ccc");
    strcpy(ref_cm->node[2].bssid_5g, "ccc_5g");
    ref_cm->node[2].online = 1;
    strcpy(ref_cm->node[2].uplink, "aaa");
    ref_cm->node[2].vap5g_cnt = 0;
    ref_cm->node[2].rssi = 0 - (rand()%21) + 30;
    ref_cm->node[2].dist = rand()%20;
    ref_cm->node[2].x = rand()%20;
    ref_cm->node[2].y = rand()%20;
}

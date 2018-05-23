#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include "meshpos.h"
#include "libcmm/libcmm.h"

#define RSSI_BOX_COLLECT_SIZE   6   //搜集平均長度
#define RSSI_BOX_MAX            32  //最多搜集幾台 AP
#define RULE_SIZE               3   //RULE_TABLE size

enum Frequency {
    /* 越上面，優先權越高 */
    Fast,
    Normal,
    Stable
};
struct rule_t {
    enum Frequency freq;                //define frequency
    unsigned int interval;              //unit: sec
    unsigned int rssi_stability_min;    //dbm average value
    unsigned int rssi_stability_max;    //dbm average value
};
struct rssi_box_t {
    char node1_id[18];
    char node2_id[18];
    int rssi_collect[RSSI_BOX_COLLECT_SIZE];    //搜集 rssi 做 delta 差距判斷用
    unsigned int next_collect_pos;              //指向下一個儲存 collect index
};

/* rule table */
const struct rule_t RULE_TABLE[] = {
    {Fast, 1, 5, 1000},
    {Normal, 2, 3, 4},
    {Stable, 5, 0, 2}
};

/* Sharing function */
unsigned int get_interval(enum Frequency freq); //取得 Frequency 定義的間格時間
enum Frequency get_high_priority_frequency(enum Frequency *freqs, unsigned int size);   //取得參數中最高的優先權

/* main function */
void daemon_init();
unsigned int is_execute();  //注意：執行後，若回傳 true，則 PREVIOUS_EXECUTE_TIME 會被覆蓋(等於會重新計算時間)

/* cm_info to pos_data function */
int get_cm_info_index(char *bssid_5g, cm_info *ref_cm); //檢查 bssid_5g 是否為 ap node 設備，並回傳 cm_info index，若非 ap node 則 return -1
unsigned int is_pos_data_link_exist(char *id1, char *id2, pos_data_t *data, unsigned int data_size);    //檢查 pos_data 裡面是否有存在此兩 id 的 link 關係
unsigned int insert_pos_data(char *id1, char *id2, int rssi1, int rssi2, pos_data_t *data, unsigned int data_size); //若 id1 or id2 有一項為空值(0),或是 pos_data 陣列內資料已滿，則不會存入 pos_data，且 return 0.
void cm_info_to_pos_data(pos_data_t *data, cm_info *ref_cm);
void test_data_to_pos_data(pos_data_t *data);
unsigned int get_pos_data_link_data_size(pos_data_t *data, unsigned int data_size); //取得 pos_data link 資料的數量(not pos_data array length)

/* change frequency */
enum Frequency check_mesh_state();
enum Frequency check_rssi_stability();
void update_frequency();    //執行可能改變 frequency 的條件

/* check rssi stability */
int get_rssibox_rssi_index(char *node1_id, char *node2_id); //找出 rssi_box 同一個連線, -1 為找不到
void insert_rssibox(pos_data_t *data, unsigned int data_size);  //如果 rssi_box 沒有相對應得 pos_data 資料會自動新增 rssi_box data，若 rssi_box 有相對應的 pos_data 則 rssi 資料會處於 Queue(先進先出) 覆蓋邏輯中.
unsigned int is_rssibox_collect_full(); //檢查 rssi_boxs 是否每組 collect 都以裝滿
unsigned int get_rssibox_max_delta();   //輸出 rssi_boxs 間距差最大的值
//要增加一個 check func，如果 collect 一直填不滿的情況，要將這組 rssi_box 刪除

/* global variable */
enum Frequency CURRENT_FREQUENCY;
unsigned int RSSI_BOX_SIZE;
struct rssi_box_t RSSI_BOXS[RSSI_BOX_MAX];
time_t PREVIOUS_EXECUTE_TIME;
time_t START_TIME; //test

int main(int argc, char *argv[])
{    
    //daemon_init();

    //default
    CURRENT_FREQUENCY = Fast;
    START_TIME = time(NULL);
    PREVIOUS_EXECUTE_TIME = time(NULL);
    RSSI_BOX_SIZE = 0;
    for(unsigned int i = 0; i<RSSI_BOX_MAX; i++){
        RSSI_BOXS[i].node1_id = 0;
        RSSI_BOXS[i].node2_id = 0;
        RSSI_BOXS[i].next_collect_pos = 0;
        for(unsigned int j = 0; j<RSSI_BOX_COLLECT_SIZE; j++)
            RSSI_BOXS[i].rssi_collect[j] = 0;
    }

    //calc
    unsigned int data_size = 0;
    pos_data_t *data = 0;
    pos_node_t *node = 0;
    pos_line_t *line = 0;

    while(1){
        sleep(1); //cycle delay        

        if(!is_execute())
            continue;

        //get rssi info
        cm_info_t mesh_info;
        get_sitesurvey(&mesh_info);
        get_mesh_info(&mesh_info);

        if(mesh_info.node_cnt != 3) //是否就直接不執行了？
            continue;

        //alloc memory for pos_data
        data_size = mesh_info.node_cnt;
        data = (pos_data_t*)malloc(data_size * sizeof(pos_data_t));
        for(unsigned int i=0; i<data_size; i++)
            data[i] = init_pos_data();

        //assign to pos_data
        cm_info_to_pos_data(data, &mesh_info);
        if(get_pos_data_link_data_size(data, data_size) != 3){  //如果 cm_info 取出的連線資料未達 3 組
            if(data)
                free(data);
            continue;
        }

        //check frequency
        insert_rssibox(data, data_size);
        update_frequency();

        //position calc        
        triangle_calc(data, data_size, &node, &line);

        //resuls
        for(unsigned int i=0; i<data_size; i++)
            printf("%s = (%f,%f)\n", node[i].bssid, node[i].point.X, node[i].point.Y);
        for(unsigned int i=0; i<data_size; i++)
            printf("%s:%s, distance = %f, RSSI = %f\n", line[i].node1->bssid, line[i].node2->bssid, line[i].distance, line[i].rssi_merge);

        //release
        if(node)
            free(node);
        if(line)
            free(line);
        if(data)
            free(data);

        printf("total time:%ld, frequency=%d\n", time(NULL) - START_TIME, CURRENT_FREQUENCY);
    }

    return 0;
}

unsigned int get_interval(enum Frequency freq){
    unsigned int i;
    for(i=0; i<RULE_SIZE; i++)
        if(freq == RULE_TABLE[i].freq)
            return RULE_TABLE[i].interval;

    return RULE_TABLE[0].interval; //防呆
}

enum Frequency get_high_priority_frequency(enum Frequency *freqs, unsigned int size){
    unsigned int i;
    enum Frequency min = 1000;
    for(i=0; i<size ;i++)
        if(freqs[i] < min)
            min = freqs[i];

    return min;
}

void daemon_init(){
    pid_t pid;

    if((pid = fork()) != 0)
        exit(0);
}

unsigned int is_execute(){
    unsigned int i;
    unsigned int isExe = 0;
    time_t interval_time = time(NULL) - PREVIOUS_EXECUTE_TIME;

    for(i=0; i<RULE_SIZE; i++)
        if(interval_time >= get_interval(CURRENT_FREQUENCY)){
            isExe = 1;
            break;
        }

    if(isExe){
        PREVIOUS_EXECUTE_TIME = time(NULL); //default
        return 1; //execute
    }else{
        return 0;
    }
}

int get_cm_info_index(char *bssid_5g, cm_info *ref_cm){
    unsigned int i;
    unsigned int index = -1;
    for(i = 0; i<ref_cm->node_cnt; i++)
        if(!strcmp(ref_cm->node[i].bssid_5g, bssid_5g)){ //survey 是使用 5g
            index = i;
            break;
        }

    return index;
}

unsigned int is_pos_data_link_exist(char *id1, char *id2, pos_data_t *data, unsigned int data_size){
    unsigned int i;
    unsigned int tag = 0;
    unsigned int isExist = 0;

    for(i = 0; i<data_size; i++){
        tag = 0;

        if(!strcmp(data[i].node1_id, id1))
            tag = 1;
        else if(!strcmp(data[i].node1_id, id2))
            tag = 2;

        if(tag != 0){
            if(tag == 1 && !strcmp(data[i].node2_id, id2))
                isExist = 1;
            else if(tag == 2 && !strcmp(data[i].node2_id, id1))
                isExist = 1;
        }

        if(isExist != 0)
            break;
    }

    return isExist;
}

unsigned int insert_pos_data(char *id1, char *id2, int rssi1, int rssi2, pos_data_t *data, unsigned int data_size){
    unsigned int i;
    unsigned int is_success = 0;

    //如果沒有 uplink 資料則不存
    if(id1 == 0 || id2 == 0)
        return is_success;

    for(i = 0; i<data_size; i++)
        if(data[i].has_data == 0){
            strcpy(data[i].node1_id, id1);
            strcpy(data[i].node2_id, id2);
            data[i].rssi1 = rssi1;
            data[i].rssi2 = rssi2;
            data[i].has_data = 1;

            is_success = 1;
            break;
        }

    return is_success;
}

void cm_info_to_pos_data(pos_data_t *data, cm_info *ref_cm){
    unsigned int i,j;
    unsigned int index;

    //insert cm_info data
    for(i = 0; i<ref_cm->node_cnt; i++)
        insert_pos_data(ref_cm->node[i].id,
                        ref_cm->node[i].uplink,
                        ref_cm->node[i].rssi,
                        ref_cm->node[i].rssi,
                        data,
                        ref_cm->node_cnt);

    //insert survey data
    for(i = 0; i<ref_cm->node_cnt; i++)
        for(j = 0; j<ref_cm->node[i].vap5g_cnt ; j++){  //search survey data
            index = get_cm_info_index(ref_cm->node[i].vap5g[j].bssid, ref_cm);  //找出 ap node
            if(index != -1)
                if(!is_pos_data_link_exist(ref_cm->node[i].id, ref_cm->node[index].id, data, ref_cm->node_cnt)) //找出的 ap node 與發射 survey 的 node 是否已經有互相link (uplink).
                    insert_pos_data(ref_cm->node[i].id,
                                    ref_cm->node[index].id,
                                    ref_cm->node[i].vap5g[j].signal,
                                    ref_cm->node[i].vap5g[j].signal,
                                    data,
                                    ref_cm->node_cnt);
        }    
}

void test_data_to_pos_data(pos_data_t *data){
    strcpy(data[0].node1_id,"aaa");
    strcpy(data[0].node2_id,"bbb");
    data[0].rssi1 = -38;
    data[0].rssi2 = -37;
    strcpy(data[1].node1_id,"aaa");
    strcpy(data[1].node2_id,"ccc");
    data[1].rssi1 = -36;
    data[1].rssi2 = -35;
    strcpy(data[2].node1_id,"ccc");
    strcpy(data[2].node2_id,"bbb");
    data[2].rssi1 = -34;
    data[2].rssi2 = -33;
}

unsigned int get_pos_data_link_data_size(pos_data_t *data, unsigned int data_size){
    unsigned int i;
    unsigned int count = 0;

    for(i = 0; i<data_size ;i++)
        if(data[i].has_data == 1)
            count++;

    return count;
}

enum Frequency check_mesh_state(){    
    return Stable;
}

enum Frequency check_rssi_stability(){

}

void update_frequency(){
    enum Frequency freqs[2];
    freqs[0] = check_mesh_state();
    freqs[1] = check_rssi_stability();

    CURRENT_FREQUENCY = get_high_priority_frequency(freqs, 2);
}

int get_rssibox_rssi_index(char *node1_id, char *node2_id){
    unsigned int i,tag;
    unsigned int index = -1;    //-1 == null

    for(i=0; i<RSSI_BOX_SIZE; i++){
        tag = 0;

        if(!strcmp(RSSI_BOXS[i].node1_id, node1_id))
            tag = 1;
        else if(!strcmp(RSSI_BOXS[i].node2_id, node1_id))
            tag = 2;

        if(tag != 0){
            if(tag == 1 && !strcmp(RSSI_BOXS[i].node2_id, node2_id))
                index = i;
            else if(tag == 2 && !strcmp(RSSI_BOXS[i].node1_id, node2_id))
                index = i;
        }

        if(index != 0)
            break;
    }

    return index;
}

void insert_rssibox(pos_data_t *data, unsigned int data_size){
    unsigned int i;
    int index;

    for(i=0; i<data_size; i++){
        index = get_rssibox_rssi_index(data[i].node1_id, data[i].node2_id);
        if(index == -1){
            //new
            RSSI_BOXS[RSSI_BOX_SIZE].node1_id = data[i].node1_id;
            RSSI_BOXS[RSSI_BOX_SIZE].node2_id = data[i].node2_id;
            RSSI_BOXS[RSSI_BOX_SIZE].rssi_collect[0] = (data[i].rssi1 + data[i].rssi2) / 2;
            RSSI_BOXS[RSSI_BOX_SIZE].next_collect_pos = 1;
            RSSI_BOX_SIZE++;
        }else{
            //update collect rssi
            RSSI_BOXS[index].rssi_collect[ RSSI_BOXS[index].next_collect_pos ] = (data[i].rssi1 + data[i].rssi2) / 2;
            if(RSSI_BOXS[index].next_collect_pos >= RSSI_BOX_COLLECT_SIZE - 1)
                RSSI_BOXS[index].next_collect_pos = 0;
            else
                RSSI_BOXS[index].next_collect_pos++;
        }
    }
}

unsigned int is_rssibox_collect_full(){
    unsigned int i,j;

    for(i = 0; i<RSSI_BOX_SIZE; i++)
        for(j = 0; j<RSSI_BOX_SIZE; j++)
            if(RSSI_BOXS[i].rssi_collect[j] == 0)
                return 0;

    return 1;
}

unsigned int get_rssibox_max_delta(){
    unsigned int i,j;
    unsigned int delta = 0,delta_max = 0,max,min;

    for(i=0; i<RSSI_BOX_SIZE; i++){
        max = abs(RSSI_BOXS[i].rssi_collect[0]);
        min = max;
        for(j=0; j<RSSI_BOX_COLLECT_SIZE; j++){
            if(abs(RSSI_BOXS[i].rssi_collect[j]) > max)
                max = abs(RSSI_BOXS[i].rssi_collect[j]);
            if(abs(RSSI_BOXS[i].rssi_collect[j]) < min)
                min = abs(RSSI_BOXS[i].rssi_collect[j]);
        }

        delta = max - min;
        if(delta > delta_max)
            delta_max = delta;
    }

    return delta_max;
}









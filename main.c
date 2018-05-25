#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include "meshpos.h"
#include "libcmm/libcmm.h"

#define EXECUTE_NODE_MIN        3   //node 數量要大於等於此值才會執行 survey、定位動作
#define HIGH_FREQUENCY          0   //最高優先權，目前是 Fast:0
#define LOW_FREQUENCY           2   //最低優先權，目前是 Stable:2
#define RSSI_BOX_COLLECT_SIZE   6   //搜集平均長度
#define RSSI_BOX_MAX            32  //最多可搜集幾台 AP node
#define RULE_SIZE               3   //RULE_TABLE array size
#define CHECK_STATE_SEC         10  //is_check_state() 執行頻率，單位:sec

enum Frequency {
    /* 定義順序為越上面，優先權越高 */
    Fast,
    Normal,
    Stable
};
typedef struct rule_t_ {
    enum Frequency freq;                //define frequency
    unsigned int interval;              //unit: sec
    unsigned int rssi_stability_min;    //dbm average value
    unsigned int rssi_stability_max;    //dbm average value
}rule_t;
typedef struct rssi_box_t_ {
    char node1_id[18];
    char node2_id[18];
    int rssi_collect[RSSI_BOX_COLLECT_SIZE];    //搜集 rssi 做 delta 差距判斷用
    unsigned int next_collect_pos;              //指向下一個儲存 collect index
}rssi_box_t;

/* rule table */
const rule_t RULE_TABLE[] = {
    {   Fast,   5,  5,  1000    },
    {   Normal, 8,  3,  4       },
    {   Stable, 20, 0,  2       }
};

/* Sharing function */
unsigned int get_interval(enum Frequency freq); //取得 Frequency 定義的間格時間
enum Frequency get_high_priority_frequency(enum Frequency *freqs, unsigned int size);   //取得參數中最高的優先權
unsigned int get_cm_info_online_total(cm_info_t *ref_cm);
cm_info_t init_cm_into(){
    cm_info_t d;
    memset(&d, 0, sizeof(cm_info_t));
    return d;
}

/* main function */
void daemon_init();
uint8_t is_execute();  //注意：執行後，若回傳 true，則 PREVIOUS_EXECUTE_TIME 會被覆蓋(等於會重新計算時間)
uint8_t is_check_state();  //執行比較不耗 mesh 設備效能的檢查，因此執行頻率與 is_execute() 不同。注意：執行後，若回傳 true，則 PREVIOUS_STATE_TIME 會被覆蓋(等於會重新計算時間)

/* cm_info to pos_data function */
int get_cm_info_index_for_bssid_5g(char *bssid_5g, cm_info_t *ref_cm); //回傳 cm_info index，若參數 bssid_5g 非 ap node 設備則 return -1
int get_cm_info_index_for_link_id(char *id1, char *id2, cm_info_t *ref_cm); //回傳 cm_info index，若 link id 沒有在 cm_info 找出對應關係則 return -1
int get_cm_info_index_for_nodeid(char *node_id, cm_info_t *ref_cm); //回傳 cm_info index，若找不到對應的 node_id  則 return -1
uint8_t is_pos_data_link_exist(char *id1, char *id2, pos_data_t *data, unsigned int data_size);    //檢查 pos_data 裡面是否有存在此兩 id 的 link 關係
uint8_t insert_pos_data(char *id1, char *id2, int rssi1, int rssi2, enum pos_link_type_e type, pos_data_t *data, unsigned int data_size);   //若 id1 or id2 有一項為空值(0),或是 pos_data 陣列內資料已滿，則不會存入 pos_data，且 return 0.
void cm_info_to_pos_data(pos_data_t *data, cm_info_t *ref_cm);
void test_data_to_pos_data(pos_data_t *data);
void input_test_cm_info_data(cm_info_t *ref_cm);
unsigned int get_pos_data_link_data_size(pos_data_t *data, unsigned int data_size); //取得 pos_data link 資料的數量(not pos_data array length)

/* change frequency */
enum Frequency check_mesh_state();
enum Frequency check_rssi_stability();  //檢查 rssi 的穩定度。注意：若有任一組 rssi_collect[] rssi data 未塞滿至 RSSI_BOX_COLLECT_SIZE 量，則會直接 true HIGH_FREQUENCY，不會去判斷 rssi 穩定度
void update_frequency();    //執行可能改變 frequency 的條件

/* check rssi stability */
int get_rssibox_rssi_index(char *node1_id, char *node2_id); //找出 rssi_box 同一個連線, -1 為找不到
void insert_rssibox(pos_data_t *data, unsigned int data_size);  //如果 rssi_box 沒有相對應得 pos_data 資料會自動新增 rssi_box data，若 rssi_box 有相對應的 pos_data 則 rssi 資料會處於 Queue(先進先出) 覆蓋邏輯中.
uint8_t is_rssibox_collect_full(); //檢查 rssi_boxs 是否每組 collect 都以裝滿
unsigned int get_rssibox_max_delta();   //輸出 rssi_boxs 間距差最大的值
//要增加一個 check func，如果 collect 一直填不滿的情況，要將這組 rssi_box 刪除

/* global variable */
enum Frequency CURRENT_FREQUENCY;
unsigned int CURRENT_ONLINE_TOTAL;
time_t DURATION_TIME;
time_t PREVIOUS_EXECUTE_TIME;
time_t PREVIOUS_STATE_TIME;
unsigned int RSSI_BOX_SIZE;
rssi_box_t RSSI_BOXS[RSSI_BOX_MAX];

int main(void)
{
    //daemon_init();

    //default
    unsigned int i = 0;
    CURRENT_FREQUENCY = HIGH_FREQUENCY;
    CURRENT_ONLINE_TOTAL = 0;
    DURATION_TIME = time(NULL);
    PREVIOUS_EXECUTE_TIME = time(NULL);
    PREVIOUS_STATE_TIME = time(NULL);
    RSSI_BOX_SIZE = 0;
    for(i = 0; i<RSSI_BOX_MAX; i++)
        memset(&RSSI_BOXS[i], 0, sizeof(rssi_box_t));

    //calc    
    unsigned int data_size = 0;
    pos_data_t *data = 0;
    pos_node_t *node = 0;
    pos_line_t *line = 0;
    cm_info_t mesh_info;
    int index;

    while(1){
        sleep(1); //cycle delay
        mesh_info = init_cm_into(); //default

        //check mesh state
        if(is_check_state()){
            //get_mesh_info(&mesh_info);
            input_test_cm_info_data(&mesh_info);
            CURRENT_ONLINE_TOTAL = get_cm_info_online_total(&mesh_info);
            update_frequency();
            printf("is_check_state():\tCURRENT_ONLINE_TOTAL=%d, duration time=%ld\n", CURRENT_ONLINE_TOTAL, time(NULL) - DURATION_TIME);
        }

        //如果有 offline 那就不用 update 定位位置了
        if(CURRENT_ONLINE_TOTAL < EXECUTE_NODE_MIN)
            continue;

        //check position calc execute
        if(!is_execute())
            continue;

        //get all rssi info
        mesh_info = init_cm_into(); //execute 與 state get_mesh_info() 的值不能混合
        //get_mesh_info(&mesh_info);
        input_test_cm_info_data(&mesh_info);
        //get_sitesurvey(&mesh_info);        

        //data size 沒有 三台就不用定位了        
        if(mesh_info.node_cnt != 3){
            printf("Excepion:\tis_execute == true, mesh_info.node_cnt != 3 (%d), duration time=%ld\n", mesh_info.node_cnt, time(NULL) - DURATION_TIME);
            continue;
        }

        //alloc memory for pos_data        
        data_size = mesh_info.node_cnt;
        data = (pos_data_t*)malloc(data_size * sizeof(pos_data_t));
        for(i=0; i<data_size; i++)
            data[i] = init_pos_data();

        //assign to pos_data
        cm_info_to_pos_data(data, &mesh_info);
        if(get_pos_data_link_data_size(data, data_size) != 3){  //如果 cm_info 取出的連線資料未達 3 組
            if(data)
                free(data);
            printf("Exception:\tcm_info 取出的連線資料未達 3 組, duration time=%ld\n", time(NULL) - DURATION_TIME);
            continue;
        }

        //check frequency
        insert_rssibox(data, data_size);
        update_frequency();

        //position calc        
        triangle_calc(data, data_size, &node, &line);

        //resuls        
        //node
        for(i=0; i<data_size; i++){
            index = get_cm_info_index_for_nodeid(node[i].node_id, &mesh_info);
            if(index != -1){
                mesh_info.node[index].x = node[i].point.X;
                mesh_info.node[index].y = node[i].point.Y;
            }
        }
        //line
        for(i=0; i<data_size; i++){
            index = get_cm_info_index_for_link_id(line[i].node1->node_id, line[i].node2->node_id, &mesh_info);
            if(index != -1)
                mesh_info.node[index].dist = line[i].distance;
        }
        //set_mesh_position(&mesh_info);

        printf("update position:\n");
        for(i=0; i<data_size; i++)
            printf("\t%s = (%f,%f)\n", node[i].node_id, node[i].point.X, node[i].point.Y);
        for(i=0; i<data_size; i++)
            printf("\t%s:%s, distance = %f, RSSI = %f, link type = %d\n", line[i].node1->node_id, line[i].node2->node_id, line[i].distance, line[i].rssi_merge, line[i].type);
        printf("\tduration time=%ld\n", time(NULL) - DURATION_TIME);

        //release
        if(node)
            free(node);
        if(line)
            free(line);
        if(data)
            free(data);
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
    enum Frequency min = LOW_FREQUENCY;
    for(i=0; i<size ;i++)
        if(freqs[i] < min)
            min = freqs[i];

    return min;
}

unsigned int get_cm_info_online_total(cm_info_t *ref_cm){
    unsigned int i;
    unsigned int count = 0;
    for(i=0; i<(unsigned)ref_cm->node_cnt; i++)
        if(ref_cm->node[i].online == 1)
            count++;

    return count;
}

void daemon_init(){
    pid_t pid;

    if((pid = fork()) != 0)
        exit(0);
}

uint8_t is_execute(){
    uint8_t isExe = 0;
    time_t interval_time = time(NULL) - PREVIOUS_EXECUTE_TIME;

    if(interval_time >= get_interval(CURRENT_FREQUENCY))
        isExe = 1;
    printf("is_execute():\tisExe=%d, interval_time=%ld, get_interval=%d, duration time=%ld\n", isExe, interval_time, get_interval(CURRENT_FREQUENCY), time(NULL) - DURATION_TIME);
    if(isExe)
        PREVIOUS_EXECUTE_TIME = time(NULL); //default

    return isExe;
}

uint8_t is_check_state(){
    uint8_t isExe = 0;
    time_t interval_time = time(NULL) - PREVIOUS_STATE_TIME;

    if(interval_time >= CHECK_STATE_SEC)
        isExe = 1;

    if(isExe)
        PREVIOUS_STATE_TIME = time(NULL); //default

    return isExe;
}

int get_cm_info_index_for_bssid_5g(char *bssid_5g, cm_info_t *ref_cm){
    unsigned int i;
    int index = -1;
    for(i = 0; i<(unsigned)ref_cm->node_cnt; i++)
        if(!strcmp(ref_cm->node[i].bssid_5g, bssid_5g)){ //survey 是使用 5g
            index = i;
            break;
        }

    return index;
}

int get_cm_info_index_for_link_id(char *id1, char *id2, cm_info_t *ref_cm){
    unsigned int i;
    unsigned int tag = 0;
    int index = -1;
    for(i = 0; i<(unsigned)ref_cm->node_cnt; i++){
        tag = 0;

        if(!strcmp(ref_cm->node[i].id, id1))
            tag = 1;
        else if(!strcmp(ref_cm->node[i].id, id2))
            tag = 2;

        if(tag != 0){
            if(tag == 1 && !strcmp(ref_cm->node[i].uplink, id2))
                index = i;
            else if(tag == 2 && !strcmp(ref_cm->node[i].uplink, id1))
                index = i;
        }

        if(index != -1)
            break;
    }

    return index;
}

int get_cm_info_index_for_nodeid(char *node_id, cm_info_t *ref_cm){
    unsigned int i;
    int index = -1;
    for(i = 0; i<(unsigned)ref_cm->node_cnt; i++)
        if(!strcmp(ref_cm->node[i].id, node_id)){
            index = i;
            break;
        }

    return index;
}

uint8_t is_pos_data_link_exist(char *id1, char *id2, pos_data_t *data, unsigned int data_size){
    unsigned int i;
    uint8_t tag = 0;
    uint8_t isExist = 0;

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

        if(isExist)
            break;
    }

    return isExist;
}

uint8_t insert_pos_data(char *id1, char *id2, int rssi1, int rssi2, enum pos_link_type_e type, pos_data_t *data, unsigned int data_size){
    unsigned int i;
    uint8_t is_success = 0;

    //如果沒有 uplink 資料則不存
    if(!strcmp(id1, "") || !strcmp(id2, ""))
        return is_success;

    for(i = 0; i<data_size; i++)
        if(data[i].has_data == 0){
            strcpy(data[i].node1_id, id1);
            strcpy(data[i].node2_id, id2);
            data[i].rssi1 = rssi1;
            data[i].rssi2 = rssi2;
            data[i].type = type;
            data[i].has_data = 1;

            is_success = 1;
            break;
        }

    return is_success;
}

void cm_info_to_pos_data(pos_data_t *data, cm_info_t *ref_cm){
    unsigned int i,j;
    int index;

    //insert cm_info data
    for(i = 0; i<(unsigned)ref_cm->node_cnt; i++)
        insert_pos_data(ref_cm->node[i].id,
                        ref_cm->node[i].uplink,
                        ref_cm->node[i].rssi,
                        ref_cm->node[i].rssi,
                        Bridge,
                        data,
                        ref_cm->node_cnt);

    //insert survey data
    for(i = 0; i<(unsigned)ref_cm->node_cnt; i++)
        for(j = 0; j<(unsigned)ref_cm->node[i].vap5g_cnt ; j++){  //search survey data
            index = get_cm_info_index_for_bssid_5g(ref_cm->node[i].vap5g[j].bssid, ref_cm);  //找出 ap node
            if(index != -1)
                if(!is_pos_data_link_exist(ref_cm->node[i].id, ref_cm->node[index].id, data, ref_cm->node_cnt)) //找出的 ap node 與發射 survey 的 node 是否已經有互相link (uplink).
                    insert_pos_data(ref_cm->node[i].id,
                                    ref_cm->node[index].id,
                                    ref_cm->node[i].vap5g[j].signal,
                                    ref_cm->node[i].vap5g[j].signal,
                                    SiteSurvey,
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

void input_test_cm_info_data(cm_info_t *ref_cm){
    ref_cm->node_cnt = 3;

    strcpy(ref_cm->node[0].id, "aaa");
    strcpy(ref_cm->node[0].bssid_5g, "aaa_5g");
    ref_cm->node[0].online = 1;
    ref_cm->node[0].rssi = -38;
    ref_cm->node[0].vap5g_cnt = 0;

    strcpy(ref_cm->node[1].id, "bbb");
    strcpy(ref_cm->node[1].bssid_5g, "bbb_5g");
    ref_cm->node[1].online = 1;
    ref_cm->node[1].rssi = -37;
    strcpy(ref_cm->node[1].uplink, "aaa");

    ref_cm->node[1].vap5g_cnt = 5;
    strcpy(ref_cm->node[1].vap5g[0].bssid, "123");
    strcpy(ref_cm->node[1].vap5g[1].bssid, "1234");
    strcpy(ref_cm->node[1].vap5g[2].bssid, "aaa_5g");
    strcpy(ref_cm->node[1].vap5g[3].bssid, "ccc_5g");
    strcpy(ref_cm->node[1].vap5g[4].bssid, "bbb_5g");
    ref_cm->node[1].vap5g[3].signal = -35;

    strcpy(ref_cm->node[2].id, "ccc");
    strcpy(ref_cm->node[2].bssid_5g, "ccc_5g");
    ref_cm->node[2].online = 1;
    ref_cm->node[2].rssi = -36;
    strcpy(ref_cm->node[2].uplink, "aaa");
    ref_cm->node[2].vap5g_cnt = 0;
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
    enum Frequency reFreq = HIGH_FREQUENCY;
    if(CURRENT_ONLINE_TOTAL < EXECUTE_NODE_MIN)
        reFreq = HIGH_FREQUENCY;
    else
        reFreq = LOW_FREQUENCY;

    return reFreq;
}

enum Frequency check_rssi_stability(){
    enum Frequency reFreq = HIGH_FREQUENCY;

    //判斷是否有塞滿
    if(!is_rssibox_collect_full())
        return reFreq;

    //判斷穩定度
    unsigned int i;
    unsigned int delta = get_rssibox_max_delta();

    for(i=0; i<RULE_SIZE; i++)
        if(delta >= RULE_TABLE[i].rssi_stability_min && delta <= RULE_TABLE[i].rssi_stability_max){
            reFreq = RULE_TABLE[i].freq;
            break;
        }

    return reFreq;
}

void update_frequency(){
    enum Frequency freqs[2];
    freqs[0] = check_mesh_state();
    freqs[1] = check_rssi_stability();

    CURRENT_FREQUENCY = get_high_priority_frequency(freqs, 2);
    printf("update_frequency():\tCURRENT_FREQUENCY=%d "
           "(check_mesh_state()=%d, check_rssi_stability=%d), "
           "duration time=%ld\n", CURRENT_FREQUENCY, freqs[0], freqs[1], time(NULL) - DURATION_TIME);
}

int get_rssibox_rssi_index(char *node1_id, char *node2_id){
    unsigned int i,tag;
    int index = -1;    //-1 == null

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

        if(index != -1)
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
            strcpy(RSSI_BOXS[RSSI_BOX_SIZE].node1_id, data[i].node1_id);
            strcpy(RSSI_BOXS[RSSI_BOX_SIZE].node2_id, data[i].node2_id);
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

uint8_t is_rssibox_collect_full(){
    unsigned int i,j;

    //如果 rssi_box 小於最低可計算數量 一樣判斷未滿
    if(RSSI_BOX_SIZE < EXECUTE_NODE_MIN)
        return 0;

    for(i = 0; i<RSSI_BOX_SIZE; i++)
        for(j = 0; j<RSSI_BOX_SIZE; j++)
            if(RSSI_BOXS[i].rssi_collect[j] == 0)
                return 0;

    return 1;
}

unsigned int get_rssibox_max_delta(){
    unsigned int i,j;
    int delta = 0,delta_max = 0,max,min;

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









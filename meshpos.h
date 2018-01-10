#ifndef MESHPOS_H
#define MESHPOS_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define PARAMETER_LEN 6         //paramter char length
#define PARAMETER_COUNT 4       //paramter total
#define PARAMETER_SIZE 128      //parameter content length
#define FLAG_MAC "MAC:"
#define FLAG_RSSI "RSSI:"
#define IN "-in"
#define HOST "-host"
#define AB "-ab"
#define AC "-ac"
#define BC "-bc"

struct wap_t{
    char bssid[18];
    unsigned int bssid_tag;
    unsigned int isHost;
    float X;                    //計算結果位置
    float Y;
    struct neighbor_t{        
        struct wap_t *link;
        int rssi_1;
        int rssi_2;
        float rssi_merge;
        float distance;         //計算結果距離
    }neighbor;
};

struct wap_t init_wap(void);
void print_file_error(const char *filename);
void print_para_error();
int indexOf(char *src,const char *flag);
int indexOfLast(char *src,const char *flag);
void release(int *argc, char **argv);
unsigned int readfile(char *filename, int *argc, char **argv);
unsigned int argv_to_struct(int argc, char **argv,struct wap_t *waps);
void rssi2dist(struct wap_t *waps);
void dist2coordinate(struct wap_t *waps);


#endif // MESHPOS_H

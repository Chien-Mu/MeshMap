#ifndef MESHPOS_H
#define MESHPOS_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

struct wap_t{
    char bssid[18];
    unsigned int bssid_tag;
    unsigned int isHost;
    float X;                        //計算結果位置
    float Y;
    struct neighbor_t{        
        struct wap_t *link;
        int rssi_5g;
        int rssi_2g;
        float distance;             //計算結果距離
    }neighbor;
};

struct wap_t init_wap(void);
unsigned int to_struct(char *str, struct wap_t *waps);
void rssi2dist(struct wap_t *waps);
void dist2coordinate(struct wap_t *waps);


#endif // MESHPOS_H

#ifndef MESHPOS_H
#define MESHPOS_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef struct node_t {
    char bssid[18];
    unsigned int bssid_tag;
    float X;                    //計算結果位置
    float Y; 
}node_t;

typedef struct line_t {
    unsigned int flag;
    node_t *node1;
    node_t *node2;
    int rssi_1;
    int rssi_2;
    float rssi_merge;
    float distance;              //計算結果距離
    struct neighbor_t {
        struct line_t *link;
        unsigned int size;
    }neighbor;
}line_t;

typedef struct triangle_t {    
    line_t *ab; //ab (x axis)
    line_t *ac; //ac (top-left)
    line_t *bc; //bc (top-right)
}triangle_t;

node_t init_node(void);
line_t init_line(void);
triangle_t init_triangle(void);
void testRead(node_t **node, line_t **line);
void rssi2dist(line_t *lines, unsigned int size);
triangle_t dist2coor(triangle_t triangle);


#endif // MESHPOS_H

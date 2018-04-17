#ifndef MESHPOS_H
#define MESHPOS_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef struct data_t {
    char bssid1[18];
    char bssid2[18];
    int rssi1;
    int rssi2;
}data_t;

typedef struct point_t {
    float X;
    float Y;
}point_t;

typedef struct node_t {
    char bssid[18];
    unsigned int index;
    unsigned int flag;
    unsigned int tag;
    point_t point;
}node_t;

typedef struct line_t {
    unsigned int index;
    unsigned int flag;
    node_t *node1;
    node_t *node2;
    int rssi_1;
    int rssi_2;
    float rssi_merge;
    float distance;              //計算結果距離 
}line_t;

typedef struct triangle_t {
    line_t *ab; //ab (x axis)
    line_t *ac; //ac (top-left)
    line_t *bc; //bc (top-right)
}triangle_t;

node_t init_node(void);
line_t init_line(void);
triangle_t init_triangle(void);
unsigned int assign_data(data_t *data, unsigned int size, node_t **node, line_t **line);
void rssi2dist(line_t *lines, unsigned int size);
void dist2coor(triangle_t triangle);
void triangle_calc(data_t *data, unsigned int size, node_t **node, line_t **line);

#endif // MESHPOS_H

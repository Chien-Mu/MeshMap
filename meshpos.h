#ifndef MESHPOS_H
#define MESHPOS_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef struct pos_data_t {
    char node1_id[18];
    char node2_id[18];
    int rssi1;
    int rssi2;
    unsigned int has_data;
}pos_data_t;

typedef struct point_t {
    float X;
    float Y;
}point_t;

typedef struct pos_node_t {
    char node_id[18];
    unsigned int index;
    unsigned int flag;
    unsigned int tag;
    point_t point;
}pos_node_t;

typedef struct pos_line_t {
    unsigned int index;
    unsigned int flag;
    pos_node_t *node1;
    pos_node_t *node2;
    int rssi_1;
    int rssi_2;
    float rssi_merge;
    float distance;              //計算結果距離 
}pos_line_t;

typedef struct triangle_t {
    pos_line_t *ab; //ab (x axis)
    pos_line_t *ac; //ac (top-left)
    pos_line_t *bc; //bc (top-right)
}triangle_t;

pos_data_t init_pos_data(void);
pos_node_t init_node(void);
pos_line_t init_line(void);
triangle_t init_triangle(void);
void triangle_calc(pos_data_t *data, unsigned int size, pos_node_t **node, pos_line_t **line);  //Triangular Positioning Calculation,parameter data: rssi data source,parameter node and line is the return result.

#endif // MESHPOS_H

#ifndef MESHPOS_H
#define MESHPOS_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef struct point_t
{
    float X;
    float Y;
}point_t;

typedef struct node_t {
    char bssid[18];
    unsigned int bssid_tag;
    unsigned int flag;
    point_t point;
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

typedef struct line_value_t {
    unsigned int flag;
    node_t node1;
    node_t node2;
    int rssi_1;
    int rssi_2;
    float rssi_merge;
    float distance;              //計算結果距離
}line_value_t;

typedef struct triangle_value_t {
    line_value_t ab;
    line_value_t ac;
    line_value_t bc;
}triangle_value_t;

node_t init_node(void);
line_t init_line(void);
triangle_t init_triangle(void);
void testRead(node_t **node, line_t **line);
void marker(triangle_t triangle);
void rssi2dist(line_t *lines, unsigned int size);
void dist2coor(triangle_t triangle);
float dir_angle (point_t a,point_t b);
point_t rotate_coor(point_t p, float angle, point_t center);
float corner_angle(point_t A, point_t B, point_t C);

#endif // MESHPOS_H

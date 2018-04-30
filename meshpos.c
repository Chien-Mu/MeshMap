#include "meshpos.h"

/* ---------5G--------- */
const float P0 = -33.0;
const float Attenuation = 1.90;
/* -------------------- */

node_t init_node(void){
    node_t d;
    memset(&d, 0, sizeof(node_t));
    return d;
}

line_t init_line(void){
    line_t d;
    memset(&d, 0, sizeof(line_t));
    return d;
}

triangle_t init_triangle(void){
    triangle_t d;
    memset(&d, 0, sizeof(triangle_t));
    return d;
}

unsigned int node_contain(node_t *node, unsigned int size, char *s){
    unsigned int i;
    for(i=0; i<size; i++)
        if(!strcmp(s, node[i].bssid))
            return 1;
    return 0; //沒找到
}

int node_findIndex(node_t *node, unsigned int size, char *s){
    /* 以字串(bssid)找出 node */
    unsigned int i;
    for(i=0; i<size; i++)
        if(!strcmp(s, node[i].bssid))
            return i;
    return -1; //沒找到
}

unsigned int assign_data(data_t *data, unsigned int size, node_t **node, line_t **line){
    unsigned int i;
    unsigned int node_c = 0;

    //check
    if(size < 3)
        return 0; //fail

    //alloc memory
    *node = (node_t*)malloc(size*sizeof(node_t));
    *line = (line_t*)malloc(size*sizeof(line_t));
    for(i=0; i<size; i++){
        (*node)[i] = init_node();
        (*line)[i] = init_line();
    }

    //insert
    for(i=0; i<size; i++){
        //node
        if(!node_contain(*node, size, data[i].bssid1)){
            strcpy((*node)[node_c].bssid, data[i].bssid1);
            (*node)[node_c].index = node_c;
            node_c++;
        }
        if(!node_contain(*node, size, data[i].bssid2)){
            strcpy((*node)[node_c].bssid, data[i].bssid2);
            (*node)[node_c].index = node_c;
            node_c++;
        }

        //line
        (*line)[i].index = i;
        (*line)[i].node1 = &(*node)[node_findIndex(*node, size, data[i].bssid1)];
        (*line)[i].node2 = &(*node)[node_findIndex(*node, size, data[i].bssid2)];
        (*line)[i].rssi_1 = data[i].rssi1;
        (*line)[i].rssi_2 = data[i].rssi2;
    }

    return 1; //success
}

void rssi2dist(line_t *lines, unsigned int size){
    /* 5G 與 2.4G 方程式不同 */
    const float n = Attenuation;
    float rssi;
    unsigned int i;

    for(i=0;i<size;i++){
        rssi = (float)(lines[i].rssi_1 + lines[i].rssi_2) / 2.0;
        lines[i].distance = pow(10.0, (P0-rssi)/(10.0*n));
        lines[i].rssi_merge = rssi;
    }
}

void dist2coor(triangle_t triangle){
    float d01=0.0, d02=0.0, d12=0.0;
    float alpha=0.0, cosine=0.0;
    node_t *a,*b,*c;

    //a coor (0,0)
    a = triangle.ab->node1;
    a->point.X = 0;
    a->point.Y = 0;

    //b coor (ab,0)
    b = triangle.ab->node2;
    b->point.X =  triangle.ab->distance;
    b->point.Y = 0;

    //c coor calc
    /*
    //define
    d01 = 4.3; //ab (x axis)
    d02 = 3.23; //ac (top-left)
    d12 = 4; //bc (top-right)
    */
    d01 = triangle.ab->distance; //ab
    d02 = triangle.ac->distance; //ac
    d12 = triangle.bc->distance; //bc
    cosine = (pow(d01,2) + pow(d02,2) - pow(d12,2)) / (2*d01*d02);

    //長度失焦(兩邊之和 < 第三邊)
    if(cosine > 1)
        cosine = 1;
    else if(cosine < -1)
        cosine = -1;
    alpha = acos(cosine);

    //警告
    //if(cosine == 1 || cosine == -1)
    //printf("ERROR:\n5G: Triangle side out of focus!\n");

    //找出c點(與a 不一樣的就是c點)
    if(triangle.ac->node1 == a)
        c = triangle.ac->node2;
    else
        c = triangle.ac->node1;

    //set
    c->point.X = (float)(d02*cos(alpha));
    c->point.Y = (float)(d02*sin(alpha));

    return;

    printf("x=%f\n",(float)(d02*cos(alpha)));
    printf("y=%f\n",(float)(d02*sin(alpha)));
    //----------------------------------
    //利用畢氏定理去補，或是直接 y 給 0.5 之類的小值
    //printf("(%f ,%f)\n", waps[2].X,waps[2].Y);
}

float dist(point_t p1, point_t p2){
    return sqrt(pow(p2.X - p1.X,2) + pow(p2.Y - p1.Y,2));
}

void triangle_calc(data_t *data, unsigned int size, node_t **node, line_t **line){
    triangle_t triangle;

    //allocate
    if(!assign_data(data, size, &(*node), &(*line)))
        return;

    //按順序定義 abc
    triangle = init_triangle();
    triangle.ab = &(*line)[0];
    triangle.ac = &(*line)[1];
    triangle.bc = &(*line)[2];

    //calc
    rssi2dist(*line, size);
    dist2coor(triangle);
}
















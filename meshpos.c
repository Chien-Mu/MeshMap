#include "meshpos.h"

/* ---------5G--------- */
const float P0 = -33.0;
const float Attenuation = 1.90;
/* -------------------- */

pos_data_t init_pos_data(void){
    pos_data_t d;
    memset(&d, 0, sizeof(pos_data_t));
    return d;
}

pos_node_t init_node(void){
    pos_node_t d;
    memset(&d, 0, sizeof(pos_node_t));
    return d;
}

pos_line_t init_line(void){
    pos_line_t d;
    memset(&d, 0, sizeof(pos_line_t));
    return d;
}

triangle_t init_triangle(void){
    triangle_t d;
    memset(&d, 0, sizeof(triangle_t));
    return d;
}

///判斷字串(node_id) 是否存在 node array
unsigned int is_nodes_contain(pos_node_t *nodes, unsigned int size, char *node_id){
    unsigned int i;
    for(i=0; i<size; i++)
        if(!strcmp(node_id, nodes[i].node_id))
            return 1;
    return 0; //沒找到
}

///以字串(node_id)找出 node array index
int get_node_index(pos_node_t *nodes, unsigned int size, char *node_id){
    unsigned int i;
    for(i=0; i<size; i++)
        if(!strcmp(node_id, nodes[i].node_id))
            return i;
    return -1; //沒找到
}

///將 pos_data 資料轉成 nodes and lines
unsigned int assign_data(pos_data_t *data, unsigned int size, pos_node_t **nodes, pos_line_t **lines){
    unsigned int i;
    unsigned int node_count = 0;

    //check
    if(size < 3)
        return 0; //fail

    //alloc memory
    *nodes = (pos_node_t*)malloc(size*sizeof(pos_node_t));
    *lines = (pos_line_t*)malloc(size*sizeof(pos_line_t));
    for(i=0; i<size; i++){
        (*nodes)[i] = init_node();
        (*lines)[i] = init_line();
    }

    //insert
    for(i=0; i<size; i++){
        //node
        if(!is_nodes_contain(*nodes, size, data[i].node1_id)){
            strcpy((*nodes)[node_count].node_id, data[i].node1_id);
            (*nodes)[node_count].index = node_count;
            node_count++;
        }
        if(!is_nodes_contain(*nodes, size, data[i].node2_id)){
            strcpy((*nodes)[node_count].node_id, data[i].node2_id);
            (*nodes)[node_count].index = node_count;
            node_count++;
        }

        //line
        (*lines)[i].index = i;
        (*lines)[i].node1 = &(*nodes)[get_node_index(*nodes, size, data[i].node1_id)];
        (*lines)[i].node2 = &(*nodes)[get_node_index(*nodes, size, data[i].node2_id)];
        (*lines)[i].rssi_1 = data[i].rssi1;
        (*lines)[i].rssi_2 = data[i].rssi2;
    }

    return 1; //success
}

///座標距離計算
float dist(point_t p1, point_t p2){
    return sqrt(pow(p2.X - p1.X,2) + pow(p2.Y - p1.Y,2));
}

///rssi to distance
void rssi2dist(pos_line_t *lines, unsigned int size){
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

///distance to coordinate
void dist2coor(triangle_t triangle){
    float d01=0.0, d02=0.0, d12=0.0;
    float alpha=0.0, cosine=0.0;
    pos_node_t *a,*b,*c;

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

void triangle_calc(pos_data_t *data, unsigned int size, pos_node_t **node, pos_line_t **line){
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
















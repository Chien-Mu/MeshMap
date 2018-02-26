#include "meshpos.h"

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

void testRead(node_t **node, line_t **line){
    /* "a-b -38/-42;"
       "a-c -51/-54;"
       "b-c -43/-37;"
    */
    unsigned int i;
    unsigned int lineSize = 3, nodeSize = 3;
    *node = (node_t *)malloc(nodeSize * sizeof(node_t));
    *line = (line_t *)malloc(lineSize * sizeof(line_t));
    for(i=0;i<nodeSize;i++)
        (*node)[i] = init_node();
    for(i=0;i<nodeSize;i++)
        (*line)[i] = init_line();

    strcpy((*node)[0].bssid,"a");
    strcpy((*node)[1].bssid,"b");
    strcpy((*node)[2].bssid,"c"); //(*node)[2] == node[0][2]

    (*line)[0].node1 = &(*node)[0]; //&(*node)[0] == &(*node)[0][0]
    (*line)[0].node2 = &(*node)[1];
    (*line)[0].rssi_1 = -38;
    (*line)[0].rssi_2 = -42;
    (*line)[0].distance = 5;

    (*line)[1].node1 = &(*node)[0];
    (*line)[1].node2 = &(*node)[2];
    (*line)[1].rssi_1 = -51;
    (*line)[1].rssi_2 = -54;
    (*line)[1].distance = 4.53;

    (*line)[2].node1 = &(*node)[1];
    (*line)[2].node2 = &(*node)[2];
    (*line)[2].rssi_1 = -43;
    (*line)[2].rssi_2 = -37;
    (*line)[2].distance = 2.3;
}


void rssi2dist(line_t *lines, unsigned int size){
    /* 5G 與 2.4G 方程式不同 */
    float rssi;
    unsigned int i;

    //----------5G-----------
    const float P0 = -33.0;
    const float n = 1.90;

    for(i=0;i<size;i++){
        rssi = (float)(lines[i].rssi_1 + lines[i].rssi_2) / 2.0;
        lines[i].distance = pow(10.0, (P0-rssi)/(10.0*n));
        lines[i].rssi_merge = rssi;
    }
}

triangle_t dist2coor(triangle_t triangle){
    float d01=0.0, d02=0.0, d12=0.0;
    float alpha=0.0, cosine=0.0;
    node_t *a,*b,*c;

    //a coor (0,0)   
    a = triangle.ab->node1;
    a->X = 100;
    a->Y = 0;

    //b coor (ab,0)
    b = triangle.ab->node2;
    b->X = triangle.ab->distance;
    b->Y = 0;

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
    c->X = (float)(d02*cos(alpha));
    c->Y = (float)(d02*sin(alpha));

    printf("x=%f\n",(float)(d02*cos(alpha)));
    printf("y=%f\n",(float)(d02*sin(alpha)));
    return triangle;



    //----------------------------------
    //利用畢氏定理去補，或是直接 y 給 0.5 之類的小值
    //printf("(%f ,%f)\n", waps[2].X,waps[2].Y);
}


























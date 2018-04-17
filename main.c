#include <stdio.h>
#include "meshpos.h"

int main(int argc, char *argv[])
{
    node_t *node = 0;
    line_t *line = 0;
    triangle_t triangle;
    const unsigned int size = 3;

    //test data
    data_t data[3];
    strcpy(data[0].bssid1,"aaa");
    strcpy(data[0].bssid2,"bbb");
    data[0].rssi1 = -38;
    data[0].rssi2 = -37;
    strcpy(data[1].bssid1,"aaa");
    strcpy(data[1].bssid2,"ccc");
    data[1].rssi1 = -36;
    data[1].rssi2 = -35;
    strcpy(data[2].bssid1,"ccc");
    strcpy(data[2].bssid2,"bbb");
    data[2].rssi1 = -34;
    data[2].rssi2 = -33;
    assign_data(data, size, &node, &line);

    //按順序定義 abc
    triangle = init_triangle();
    triangle.ab = &line[0];
    triangle.ac = &line[1];
    triangle.bc = &line[2];

    //calc
    rssi2dist(line, size);
    dist2coor(triangle);

    //result
    for(unsigned i=0; i<size; i++)
        printf("%s=(%f,%f)\n", node[i].bssid, node[i].point.X, node[i].point.Y);

    //release
    if(node)
        free(node);
    if(line)
        free(line);

    return 0;
}

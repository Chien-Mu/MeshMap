#include <stdio.h>
#include "meshpos.h"

char *cgi = "a-b -38/-42;"
            "a-c -51/-54;"
            "b-c -43/-37;";

int main(int argc, char *argv[])
{
    node_t *node = 0;
    line_t *line = 0;
    testRead(&node,&line);    

    triangle_t triangle;
    triangle.ab = &line[0];
    triangle.ac = &line[1];
    triangle.bc = &line[2];
    dist2coor(triangle);



    /*
    //output
    printf("5G:\n");
    for(i=0;i<3;i++)
        printf("coor=%s=(%.2f,%.2f);\n", waps[i].bssid, waps[i].X,waps[i].Y);
    for(i=0;i<3;i++)
        printf("dist=%s-%s=%.1fm=%.f;\n", waps[i].bssid, waps[i].neighbor.link->bssid,
               waps[i].neighbor.distance, waps[i].neighbor.rssi_merge);
    */
    free(line);
    free(node);
    return 0;
}

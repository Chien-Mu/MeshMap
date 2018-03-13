#include <stdio.h>
#include "meshpos.h"

char *cgi = "a-b -38/-42;"
            "a-c -51/-54;"
            "b-c -43/-37;";

int main(int argc, char *argv[])
{
    unsigned int i,m;
    unsigned int isAllMarker;
    node_t *node = NULL;
    line_t *line = NULL;
    unsigned int lineSize = 5;
    unsigned int associateSize = 0;
    line_t **associate_line = NULL;
    associate_line = (line_t **)malloc(lineSize * sizeof(line_t *)); //儲存 *line_t 的陣列
    node_t **associate_node = NULL;
    associate_node = (node_t **)malloc(lineSize * sizeof(node_t *)); //儲存 *node_t 的陣列
    triangle_t triangle;

    //alloc
    testRead(&node,&line);

    //一開始
    triangle = init_triangle();
    triangle.ab = &line[0];
    triangle.ac = &line[1];
    triangle.bc = &line[2];
    dist2coor(triangle);
    marker(triangle);

    unsigned int corrIndex1 = 0, corrIndex2 = 0, D_index = 0;
    line_t *corrLine1,*corrLine2, *linkSideLine;
    point_t D, mirror_D;

    i=3;
    while(1){
        if(!line[i].flag) //找尋沒 marker 的 line
            if(line[i].node1->flag ^ line[i].node2->flag){  //在沒 marker 的 line 中，找尋至少有其中一個 node 有 marker 的 line
                if(getAssLine(line, lineSize, i, associate_line, associate_node, &associateSize)){ //找尋與 node1,node2 有關聯的 lines,nodes
                    if(getCorrLine(associate_node, associateSize, &corrIndex1, &corrIndex2)){ //從關聯 node 中，再找出相對應的兩個 line
                        //printf("%f - %f\n", associate_line[corrIndex1]->distance , associate_line[corrIndex2]->distance);
                        corrLine1 = associate_line[corrIndex1];
                        corrLine2 = associate_line[corrIndex2];

                        //如果有連接邊(link side)
                        if(getLinkSide_D(&line[i], corrLine1, corrLine2, &D, &mirror_D, &D_index)){
                            //find "link side(B-C)" calc mirror coor
                            if(corrLine1->flag)
                                linkSideLine = corrLine1;
                            else
                                linkSideLine = corrLine2;

                            //find "A"
                            getAssLine(line, lineSize, linkSideLine->index, associate_line, associate_node ,&associateSize);
                            for(m=0;m<associateSize;m++)
                                associate_node[m]->tag = 0;
                            for(m=0;m<associateSize;m++)
                                if(associate_node[m]->index != D_index)
                                    associate_node[m]->tag++;

                            //find max distance
                            for(m=0;m<associateSize;m++){
                                if(associate_node[m]->tag >= 2){ //tag >= 2 就是 A
                                    node_t *A = associate_node[m];
                                    if(dist(D , A->point) > dist(mirror_D, A->point))
                                        node[D_index].point = D;
                                    else
                                        node[D_index].point = mirror_D;
                                    break;
                                }
                                //printf("%s %d\n", associate_node[m]->bssid, associate_node[m]->tag);
                            }
                        }


                        //marker two side line
                        corrLine1->flag = 1;
                        corrLine1->node1->flag = 1;
                        corrLine1->node2->flag = 1;
                        corrLine2->flag = 1;
                        corrLine2->node1->flag = 1;
                        corrLine2->node2->flag = 1;
                    }
                }
                //marker work line
                line[i].flag = 1;
                line[i].node1->flag = 1;
                line[i].node2->flag = 1;
            }


        //incrment or default
        i++;
        if(i >= lineSize)
            i=3;

        //check marker
        isAllMarker = 1;
        for(m=0;m<lineSize;m++)
            if(!line[m].flag){
                isAllMarker = 0;
                break;
            }
        if(isAllMarker)
            break;
    }

    //output
    for(i=0;i<3;i++)
        printf("%s(%.2f,%.2f)\n", node[i].bssid, node[i].point.X, node[i].point.Y);
    free(associate_line);
    free(associate_node);
    free(line);
    free(node);
    return 0;
}

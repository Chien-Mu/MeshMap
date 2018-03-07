#include <stdio.h>
#include "meshpos.h"

char *cgi = "a-b -38/-42;"
            "a-c -51/-54;"
            "b-c -43/-37;";

int main(int argc, char *argv[])
{
    unsigned int i,ii,j,m;
    unsigned int isAllMarker,isMarker;
    node_t *node = NULL;
    line_t *line = NULL;
    unsigned int lineSize = 5;
    unsigned int associateSize = 0;
    unsigned int *associate_line_index = (unsigned int *)malloc(lineSize * sizeof(unsigned int));
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

    associate_node[0] = line[0].node1;
    associate_node[1] = line[0].node2;
    associate_node[2] = line[1].node1;
    associate_node[3] = line[1].node2;
    associate_node[4] = line[2].node1;

    i=3;
    while(1){       
        if(!line[i].flag) //找尋沒 marker 的 line
            if(line[i].node1->flag ^ line[i].node2->flag){  //在沒 marker 的 line 中，找尋至少有其中一個 node 有 marker 的 line
                //找尋與 node1,node2 有關聯的 lines,nodes
                m = 0;
                for(j=0;j<lineSize;j++){

                    //pass 比較方
                    if(j == i)
                        continue;

                    if(!strcmp(line[j].node1->bssid,line[i].node1->bssid)){
                        associate_line_index[m] = j;
                        associate_node[m++] = line[j].node2;
                    }else if(!strcmp(line[j].node2->bssid,line[i].node1->bssid)){
                        associate_line_index[m] = j;
                        associate_node[m++] = line[j].node1;
                    }

                    if(!strcmp(line[j].node1->bssid,line[i].node2->bssid)){
                        associate_line_index[m] = j;
                        associate_node[m++] = line[j].node2;
                    }else if(!strcmp(line[j].node2->bssid,line[i].node2->bssid)){
                        associate_line_index[m] = j;
                        associate_node[m++] = line[j].node1;
                    }
                }
                associateSize = m;

                //從關聯 node 中，再找出相對應的兩個 line
                for(ii=0;ii<associateSize;ii++){
                    for(j=ii+1;j<associateSize;j++){
                        printf("%d-%d ",ii,j);
                        printf("%s-%s\n",associate_node[ii], associate_node[j]->bssid);
                        if(associate_node[ii]->bssid_tag == associate_node[j]->bssid_tag){
                            line_t *line1,*line2;
                            line1 = &line[associate_line_index[ii]];
                            line2 = &line[associate_line_index[j]];
                            linkSide(&line[i], line1, line2);
                            //檢查找到的兩個line是否有已勾
//                            if(t.ac->flag && t.bc->flag){
//                                t.ab->flag = 1;  //如果都有勾則直接將 ab 打勾即可，因為 b、c都已經有座標
//                                break;
//                            }
                        }
                    }
                }
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
    free(associate_line_index);
    free(associate_node);
    free(line);
    free(node);
    return 0;
}

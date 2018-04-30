#include <stdio.h>
#include <unistd.h>
#include "meshpos.h"

void readin(FILE *in, char **result);

int main(int argc, char *argv[])
{
    FILE *file;
    char *config = "";
    pid_t pid;
    node_t *node = 0;
    line_t *line = 0;
    const unsigned int size = 3;
    data_t data[3];    

    //read file
    if( (file = fopen("config", "r")) == NULL ){
        printf("error: cat't open config file\n");
        return 0;
    }else{
        readin(file, &config);
        printf("%s\n", config);
        return 0;
    }

    //daemon
    pid = fork();
    switch(pid){
    case -1:
        printf("error: pid = -1\n");
        return 0;
    case 0:
        //child
        break;
    default:
        //parent
        return 0;
    }

    //test data
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

    //position calc
    triangle_calc(data, size, &node, &line);

    //resul
    for(unsigned i=0; i<size; i++)
        printf("%s = (%f,%f)\n", node[i].bssid, node[i].point.X, node[i].point.Y);
    for(unsigned i=0; i<size; i++)
        printf("%s:%s, distance = %f, RSSI = %f\n", line[i].node1->bssid, line[i].node2->bssid, line[i].distance, line[i].rssi_merge);

    //release
    if(node)
        free(node);
    if(line)
        free(line);
    fclose(file);
    free(config);
    return 0;
}

void readin(FILE *in, char **result) {
    char tmp[80];
    while( fgets( tmp, 80, in)!=NULL )
        asprintf(&(*result), "%s%s", *result, tmp);
}

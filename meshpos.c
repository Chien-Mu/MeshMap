#include "meshpos.h"

typedef enum {
    stop_e,
    start_e,
    end_e
}FLAG;

struct wap_t init_wap(void){
    struct wap_t d;
    memset(&d, 0, sizeof(struct wap_t));
    return d;
}

void print_file_error(const char *filename){
    printf("ERROR;\n");
    printf("%s:Invalid argument format.\n", filename);
    printf("File content example:\n");
    printf("-host 06:00:00:00:00:00\n"
           "-ab MAC: 06:01:22:00:00:01, RSSI: -38/-42\n"
           "-ac MAC: 06:01:22:00:00:D3, RSSI: -43/-37\n"
           "-bc MAC: 06:01:22:00:00:D3, RSSI: -51/-54\n");
}

void print_para_error(){
    printf("ERROR;\n");
    printf("Invalid argument format.\n");
    printf("Argument example:\n");
    printf("./MeshMap -host 06:00:00:00:00:00"
           " -ab \"MAC: 06:01:22:00:00:01, RSSI: -38/-42\""
           " -ac \"MAC: 06:01:22:00:00:D3, RSSI: -43/-37\""
           " -bc \"MAC: 06:01:22:00:00:D3, RSSI: -51/-54\"\n");
}

int indexOf(char *src,const char *flag){
    int index = -1;
    char *found = strstr( src, flag );
    if (found != NULL)
       index = found - src;

    return index;
}

int indexOfLast(char *src,const char *flag){
    int index = -1;
    char *found = strstr( src, flag );
    if (found != NULL)
       index = found - src;

    return index + strlen(flag) - 1;
}

void release(int *argc, char **argv){
    unsigned int i=0;

    for(i=0;i < (unsigned int)*argc;i++)
        if(argv[i])
            free(argv[i]);
    if(argv)
        free(argv);
}

unsigned int readfile(char *filename,int *argc, char **argv){
    FILE *pf;
    char buffer[PARAMETER_SIZE] = {'\0'};
    const char *flags[4] = { HOST,AB,AC,BC };
    unsigned int flags_count=0;
    unsigned int i=1; //argv[0] save filename string
    unsigned int j=0,c=0,len=0,index=0,indexlast=0;


    //default
    *argc=0;

    //read
    pf=fopen(filename,"r");
    if(pf == NULL){
        printf("ERROR;\n%s:File does not exist\n", filename);
        return 1;
    }

    while(fgets(buffer,PARAMETER_SIZE - 1,pf) != NULL){
        if(flags_count >= PARAMETER_COUNT)
            return 1;

        //parameter
        index = indexOf(buffer,flags[flags_count]);
        indexlast = indexOfLast(buffer,flags[flags_count]) + 1;
        len = indexlast - index;
        argv[i] = (char*)malloc(len + 1);
        for(j=index;j<indexlast;j++)
            argv[i][c++] = buffer[j];
        argv[i][c] = '\0';
        i++;
        c=0;

        //parameter content
        for(j=indexlast; buffer[j] != '\0';j++)
            if(buffer[j] == '\r' || buffer[j] == '\n' || buffer[j] == '\0')
                break;
        len = j - indexlast - 1;

        argv[i] = (char*)malloc(len + 1);
        for(j=indexlast+1; j<indexlast+len+1 ;j++)
            argv[i][c++] = buffer[j];
        argv[i][c] = '\0';
        i++;
        c=0;

        flags_count++;
    }
    *argc = i;
    fclose(pf);

    return 0;
}

unsigned int argv_to_struct(int argc, char **argv, struct wap_t *waps){
    int i=0,j=0,c=0;
    char data[120] = {'\0'};
    char rssi_5g[5] = {'\0'}, rssi_2g[5] = {'\0'};
    char host[18] = {'\0'};
    char ab[40] = {'\0'}, ac[40] = {'\0'}, bc[40] = {'\0'};

    //check
    if(argc > (PARAMETER_COUNT*2) + 1)
        return 1;

    for(i=1;i<argc;i++){
        if(!strcmp(argv[i],HOST) && i+1 < argc){
            for(j=0;argv[i+1][j] != '\0';j++)
                host[c++] = argv[i+1][j];
            host[c] = '\0';
            i++;
        }else if(!strcmp(argv[i],AB) && i+1 < argc){
            for(j=0;argv[i+1][j] != '\0';j++)
                ab[c++] = argv[i+1][j];
            ab[c] = ';';
            ab[c+1] = '\0';
            i++;
        }else if(!strcmp(argv[i],AC) && i+1 < argc){
            for(j=0;argv[i+1][j] != '\0';j++)
                ac[c++] = argv[i+1][j];
            ac[c] = ';';
            ac[c+1] = '\0';
            i++;
        }else if(!strcmp(argv[i],BC) && i+1 < argc){
            for(j=0;argv[i+1][j] != '\0';j++)
                bc[c++] = argv[i+1][j];
            bc[c] = ';';
            bc[c+1] = '\0';
            i++;
        }else{
            return 1;
        }
        c=0;
    }

    //-ac 與 host MAC 交換
    i = indexOfLast(ac,FLAG_MAC) + 1;
    while(ac[i] == ' ')
        i++;
    for(;ac[i] != ',';i++)
        ac[i] = host[c++];
    i=0;
    c=0;

    //strcat(依照 ab bc ca 順序 加入 data)
    strcat(data, ab);
    strcat(data, bc);
    strcat(data, ac);

    //linked list
    waps[0].isHost = 1;
    waps[0].bssid_tag = 0;
    waps[1].bssid_tag = 1;
    waps[2].bssid_tag = 2;
    waps[0].neighbor.link = &waps[1];
    waps[1].neighbor.link = &waps[2];
    waps[2].neighbor.link = &waps[0];

    //assign
    for(j=0;j<3;j++){
        c=0;
        for(i+=5;data[i] != ',';i++)
            waps[j].neighbor.link->bssid[c++] = data[i];
        waps[j].neighbor.link->bssid[c] = '\0';

        c=0;
        for(i+=8;data[i] != '/';i++)
            rssi_5g[c++] = data[i];
        rssi_5g[c] = '\0';
        waps[j].neighbor.rssi_5g = atoi(rssi_5g);

        c=0;
        for(i+=1 ;data[i] != ';';i++)
            rssi_2g[c++] = data[i];
        rssi_2g[c] = '\0';
        waps[j].neighbor.rssi_2g = atoi(rssi_2g);
        i++;
    }

    return 0;
}

void rssi2dist(struct wap_t *waps){
    float rssi;

    /* 5G 與 2.4G 方程式不同 */
    rssi = waps[0].neighbor.rssi_5g;
    waps[0].neighbor.distance_5g = exp((rssi+32.851)/(-8.782));

    rssi = waps[1].neighbor.rssi_5g;
    waps[1].neighbor.distance_5g = exp((rssi+32.851)/(-8.782));

    rssi = waps[2].neighbor.rssi_5g;
    waps[2].neighbor.distance_5g = exp((rssi+32.851)/(-8.782));

    //-------------------------
    rssi = waps[0].neighbor.rssi_2g;
    waps[0].neighbor.distance_2g = exp((rssi+32.851)/(-8.782));

    rssi = waps[1].neighbor.rssi_2g;
    waps[1].neighbor.distance_2g = exp((rssi+32.851)/(-8.782));

    rssi = waps[2].neighbor.rssi_2g;
    waps[2].neighbor.distance_2g = exp((rssi+32.851)/(-8.782));
}

void dist2coordinate(struct wap_t *waps){
    float d01=0.0, d02=0.0, d12=0.0;
    float alpha=0.0, cosine=0.0;

    //waps[0]
    waps[0].X_5g = 0;
    waps[0].Y_5g = 0;
    waps[0].X_2g = 0;
    waps[0].Y_2g = 0;

    //waps[1]
    waps[1].X_5g = waps[0].neighbor.distance_5g;
    //waps[1].X_5g = 2;
    waps[1].Y_5g = 0;
    waps[1].X_2g = waps[0].neighbor.distance_2g;
    waps[1].Y_2g = 0;

    //waps[2]
//    d01 = 2.0; //ab
//    d02 = 5.0; //ac
//    d12 = 3.5; //bc
    d01 = waps[0].neighbor.distance_5g; //ab
    d02 = waps[2].neighbor.distance_5g; //ac
    d12 = waps[1].neighbor.distance_5g; //bc
    cosine = (pow(d01,2) + pow(d02,2) - pow(d12,2)) / (2*d01*d02);

    //長度失焦(兩邊之和 < 第三邊)
    if(cosine > 1)
        cosine = 1;
    else if(cosine < -1)
        cosine = -1;
    if(cosine == 1 || cosine == -1)
        printf("ERROR;\n5G: Triangle side out of focus!\n");

    alpha = acos(cosine);
    waps[2].X_5g = (float)(d02*cos(alpha)); //得C座標
    waps[2].Y_5g = (float)(d02*sin(alpha));

    //----------------------------------
    d01 = waps[0].neighbor.distance_2g; //ab
    d02 = waps[2].neighbor.distance_2g; //ac
    d12 = waps[1].neighbor.distance_2g; //bc
    cosine = (pow(d01,2) + pow(d02,2) - pow(d12,2)) / (2*d01*d02);

    //長度失焦(兩邊之和 < 第三邊)
    if(cosine > 1)
        cosine = 1;
    else if(cosine < -1)
        cosine = -1;
    if(cosine == 1 || cosine == -1)
        printf("ERROR;\n2.4G: Triangle side out of focus!\n");

    alpha = acos(cosine);
    waps[2].X_2g = (float)(d02*cos(alpha)); //得C座標
    waps[2].Y_2g = (float)(d02*sin(alpha));

    //----------------------------------
    //利用畢氏定理去補，或是直接 y 給 0.5 之類的小值
    //printf("(%f ,%f)\n", waps[2].X,waps[2].Y);
}
























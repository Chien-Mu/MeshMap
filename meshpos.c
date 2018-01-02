#include "meshpos.h"
#define PARAMTER_LAN 6      //paramter char langth
#define PARAMTER_COUNT 4    //paramter total

const char *HOST = "-host";
const char *AB = "-ab";
const char *AC = "-ac";
const char *BC = "-bc";

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

unsigned int to_struct(char *str ,struct wap_t *waps){
    FLAG flag = stop_e;
    int i = 0, j = 0, k = 0, c = 0;
    unsigned int parameter_count = 0;
    char parameter[PARAMTER_LAN] = {'\0'};
    char data[120] = {'\0'};
    char host[18] = {'\0'};
    char ab[40] = {'\0'}, ac[40] = {'\0'}, bc[40] = {'\0'};
    char rssi_5g[5] = {'\0'}, rssi_2g[5] = {'\0'};

    //順序分類
    for(i=0;str[i] != '\0';i++){
        if(str[i] == '-' && flag == stop_e)
            flag = start_e;
        if(str[i] == ' ' && flag == start_e){
            flag = end_e;
            parameter_count++;
        }
        if(flag == start_e){
            parameter[k++] = str[i];

            //check
            if(k > PARAMTER_LAN)
                return 0;
        }

        if(flag != end_e)
            continue;

        if(!strcmp(parameter, HOST))
            for(j=i+1;str[j] != ';';j++)
                host[c++] = str[j];
        else if(!strcmp(parameter, AB)){
            for(j=i+1;str[j] != ';';j++)
                ab[c++] = str[j];
            ab[c] = ';';
        }else if(!strcmp(parameter, AC)){
            for(j=i+1;str[j] != ';';j++)
                ac[c++] = str[j];
            ac[c] = ';';
        }else if(!strcmp(parameter, BC)){
            for(j=i+1;str[j] != ';';j++)
                bc[c++] = str[j];
            bc[c] = ';';
        }else
            return 0;   //invalid parameter

        //default
        i = j;
        k=0;
        c=0;
        memset(parameter, 0, sizeof(parameter));
        flag = stop_e;
    }
    k=0;
    c=0;

    //check
    if(parameter_count > PARAMTER_COUNT)
        return 0;

    //-ac 與 host MAC 交換
    for(i=5;i<22;i++)
        ac[i] = host[k++];
    i=0;

    //strcat(依照 ab bc ca 順序)
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
        k=0;
        for(i+=5;data[i] != ',';i++)
            waps[j].neighbor.link->bssid[k++] = data[i];

        k=0;
        for(i+=8;data[i] != '/';i++)
            rssi_5g[k++] = data[i];
        waps[j].neighbor.rssi_5g = atoi(rssi_5g);

        k=0;
        for(i+=1 ;data[i] != ';';i++)
            rssi_2g[k++] = data[i];
        waps[j].neighbor.rssi_2g = atoi(rssi_2g);
        i++;
    }

    return 1;
}

void rssi2dist(struct wap_t *waps){
    float rssi;
    /* 5G 與 2.4G 方程式不同 */
    rssi = waps[0].neighbor.rssi_5g;
    waps[0].neighbor.distance = exp((rssi+32.851)/(-8.782));

    rssi = waps[1].neighbor.rssi_5g;
    waps[1].neighbor.distance = exp((rssi+32.851)/(-8.782));

    rssi = waps[2].neighbor.rssi_5g;
    waps[2].neighbor.distance = exp((rssi+32.851)/(-8.782));
}

void dist2coordinate(struct wap_t *waps){
    float d01=0.0, d02=0.0, d12=0.0;
    float alpha=0.0, cosine=0.0;

    //waps[0]
    waps[0].X = 0;
    waps[0].Y = 0;

    //waps[1]
    waps[1].X = waps[0].neighbor.distance;
    waps[1].Y = 0;

    //waps[2]
    d01 = waps[0].neighbor.distance; //ab
    d02 = waps[2].neighbor.distance; //ac
    d12 = waps[1].neighbor.distance; //bc
//    d01 = 2.0; //ab
//    d02 = 5.0; //ac
//    d12 = 3.5; //bc

    cosine = (pow(d01,2) + pow(d02,2) - pow(d12,2)) / (2*d01*d02);

    //長度失焦
    if(cosine > 1)
        cosine = 1;
    else if(cosine < -1)
        cosine = -1;

    alpha = acos(cosine);
    waps[2].X = (float)(d02*cos(alpha)); //得C座標
    waps[2].Y = (float)(d02*sin(alpha));

    if(waps[2].Y == 0.0)
        printf("Triangle side out of focus!\n"); //三角形邊長失焦

    //利用畢氏定理去補，或是直接 y 給 0.5 之類的小值
    //printf("(%f ,%f)\n", waps[2].X,waps[2].Y);
}
























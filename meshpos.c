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

unsigned int to_struct(char *str ,struct wap_t *wap){
    FLAG flag = stop_e;
    int i = 0, j = 0, k = 0, c = 0;
    unsigned int parameter_count = 0;
    char parameter[PARAMTER_LAN] = {'\0'};
    char data[160] = {'\0'};
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
    wap[0].isHost = 1;
    wap[0].bssid_tag = 0;
    wap[1].bssid_tag = 1;
    wap[2].bssid_tag = 2;
    wap[0].neighbor.link = &wap[1];
    wap[1].neighbor.link = &wap[2];
    wap[2].neighbor.link = &wap[0];

    //assign
    for(j=0;j<3;j++){
        k=0;
        for(i+=5;data[i] != ',';i++)
            wap[j].neighbor.link->bssid[k++] = data[i];

        k=0;
        for(i+=8;data[i] != '/';i++)
            rssi_5g[k++] = data[i];
        wap[j].neighbor.rssi_5g = atoi(rssi_5g);

        k=0;
        for(i+=1 ;data[i] != ';';i++)
            rssi_2g[k++] = data[i];
        wap[j].neighbor.rssi_2g = atoi(rssi_2g);
        i++;
    }

    return 1;
}

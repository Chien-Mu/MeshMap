#include <stdio.h>
#include "meshpos.h"

char *cgi = "-host 06:00:00:00:00:00;"
            " -ab MAC: 06:01:22:00:00:01, RSSI: -38/-42;"
            " -ac MAC: 06:01:22:00:00:D3, RSSI: -43/-37;"
            " -bc MAC: 06:01:22:00:00:D3, RSSI: -51/-54;";

int main(int argc, char *argv[])
{
    struct wap_t wap[3];
    wap[0] = init_wap();
    wap[1] = init_wap();
    wap[2] = init_wap();        

    to_struct(cgi, wap);
    printf("%s",cgi);
    return 0;
}

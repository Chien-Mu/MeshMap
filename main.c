#include <stdio.h>
#include "meshpos.h"

char *cgi = "-host 06:00:00:00:00:00;"
            " -ab MAC: 06:01:22:00:00:01, RSSI: -38/-42;"
            " -ac MAC: 06:01:22:00:00:D3, RSSI: -43/-37;"
            " -bc MAC: 06:01:22:00:00:D3, RSSI: -51/-54;";

int main(int argc, char *argv[])
{
    struct wap_t waps[3];
    waps[0] = init_wap();
    waps[1] = init_wap();
    waps[2] = init_wap();

    if(to_struct(cgi, waps)){
        rssi2dist(waps);
        dist2coordinate(waps);
    }
    return 0;
}

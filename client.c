#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>

#include "libcmm.h"



void dump_sitevurvey(cm_info_t *d)
{

	printf("node_cnt=%d\n", d->node_cnt);
	printf("id=%s\n", d->node[0].id);
	printf("ap_cnt=%d\n", d->node[0].vap5g_cnt);
	printf("bssid=%s\n", d->node[0].vap5g[0].bssid);
	printf("signal=%d\n",d->node[0].vap5g[0].signal);
	
}


void position_setting(cm_info_t *d)
{

	d->node_cnt = 3;

	d->node[0].dist = 0;
	d->node[0].x    = 0;
	d->node[0].y    = 0;

	d->node[1].dist = 10.5;
	d->node[1].x    = 9.3;
	d->node[1].y    = 14.5;

	d->node[2].dist = 14.5;
	d->node[2].x    = 11.3;
	d->node[2].y    = 17.5;

}

void main(void)
{

	//node_survey_t ns;
	cm_info_t info;
	cm_info_t info2;

	get_sitesurvey(&info);
	dump_sitevurvey(&info);

	get_mesh_info(&info2);
	position_setting(&info2);
	set_mesh_position(&info2);

	return ;
}


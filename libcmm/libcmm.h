#ifndef __LIBCMM_H__
#define __LIBCMM_H__

#include <sys/types.h>
#include <stdint.h>


#define ADDRESS     "/tmp/.cmsocket"  /* addr to connect */

/* String length */
#define BSSID_LEN		(sizeof("xx:xx:xx:xx:xx:xx"))	/* WiFi MAC */
#define ESSID_LEN		32				/* 802.11 */	
#define GROUP_ID_LEN		16				/* CyberTAN Mesh Group ID */
#define NODE_NAME_LEN		16				/* Mesh node alias name, for refence only */
#define NODE_ID_LEN		sizeof("xx:xx:xx:xx:xx:xx")	/* Grouping interface (Ethernent) MAC */
#define ADDR_IPV4_LEN		sizeof("255.255.255.255")
#define MACADDR_LEN		(sizeof("xx:xx:xx:xx:xx:xx"))	/* WiFi MAC */

/* Mash node */
#define NODE_CNT_MAX            8
#define STA_CNT_MAX		32

       

/* 
typedef struct linked_sta {
        char mac[BSSID_LEN];
        char rssi;
} linked_sta_t; 


struct survey {
	char	id[BSSID_LEN];
	int	ap_cnt;
	ap_survey_t	ap[NODE_CNT_MAX];
} ;

typedef struct node_survey {
	int	node_cnt;
	struct	survey node[NODE_CNT_MAX];
} node_survey_t;
*/


#define CMM_GET_MESHINFO	1
#define CMM_GET_SITESURVEY	2

#define CMM_SET_MESHINFO	11
#define CMM_SET_POSITION	12	

#undef DEBUG 

#ifdef DEBUG
	#define CMMDBG(str, args...) printf(str "\n", ## args)
#else
	#define CMMDBG(str, args...) do {;} while(0)
#endif

typedef struct request{
	int action;
} request_t;

typedef struct response{
	int action;
	int retcode;
} response_t;

typedef struct sta_list {
	char		mac[MACADDR_LEN];
	int 		rssi;
	uint32_t	dist;
} sta_link_t;

typedef struct ap_visible {
        char bssid[BSSID_LEN];
        int signal;
} ap_visible_t;
 

typedef struct node_info {
	uint32_t	update_cnt;		/* increase one for each update */
	char		id[NODE_ID_LEN];
	char		name[NODE_NAME_LEN];	/* alias name */
	char		bssid_2g[NODE_ID_LEN];
	char		bssid_5g[NODE_ID_LEN];
	uint8_t		online;			/* 0: offline, 1: online */
	uint8_t		master;			/* 0: slave, 1: master */
	char		uplink[NODE_ID_LEN];	/* uplink id */
	int 		rssi;			/* with uplink */
	float		dist;			/* with uplink */
	float		x;			/* position - x axis */
	float		y;			/* position - y axis */
        int		sta2g_cnt;		/* connected 5GHz mobile stations */
	sta_link_t 	sta2g[STA_CNT_MAX];	
        int		sta5g_cnt;		/* connected 2.4GHz mobile stations */
	sta_link_t 	sta5g[STA_CNT_MAX];	

	/* used by position daemon below */
	int		vap5g_cnt; 		/* visible 5G nodes count */
	ap_visible_t	vap5g[NODE_CNT_MAX];
} node_info_t;

typedef struct cm_info {
	char		essid[ESSID_LEN];	/* mesh group SSID */
	char		passwd[64];		/* WiFi passphrase */
        int		node_cnt;		/* mesh group AP nodes */
	node_info_t	node[NODE_CNT_MAX];
} cm_info_t;


int ipc_connect(void);

int get_mesh_info(cm_info_t *info);
int get_sitesurvey(cm_info_t *info);

int set_mesh_info(cm_info_t *info);
int set_mesh_position(cm_info_t *info);

void dump_data(char *buf, unsigned len);

#endif

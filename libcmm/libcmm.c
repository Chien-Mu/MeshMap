#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>

#include "libcmm.h"


int
ipc_connect(void)
{
    int s, len;
    struct sockaddr_un saun;

    if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        perror("client: socket");
	return -1;
    }

    saun.sun_family = AF_UNIX;
    strcpy(saun.sun_path, ADDRESS);

    len = sizeof(saun.sun_family) + strlen(saun.sun_path);

    if (connect(s, (struct sockaddr*) &saun, len) < 0) {
        perror("client: connect");
	close(s);
	return -1;
    }

    return s;
};


void
dump_data(char *buf, unsigned len)
{
#ifdef DEBUG
	int i;

	printf("Data[%d] = ", len);

	for (i= 0; i< len; i++) {
		if (i%16 == 0) printf("\n");
		printf("%02x ", *(buf+i));
	}

	printf("\n");
#endif
}

static int
__cm_common_set (int act, void *data, int dlen)
{
	int s;
	request_t req;
	response_t res;
	unsigned int len, blen;
	int i;
	uint32_t slen, rlen;


	if (act == CMM_SET_MESHINFO) 
		CMMDBG("\nCMM_SET_MESHINFO\n");
	else if (act == CMM_SET_POSITION) 
		CMMDBG("\nCMM_SET_POSITION\n");
	else
		return 0;

	if ((s = ipc_connect()) < 0)
		return 0;

	/* Request */
	req.action = act;

	if ((slen = send(s, &req, sizeof(req), 0)) != sizeof(req)) {
		perror("client: send");
		close(s);
		return 0;
	}
	CMMDBG("slen=%u, sizeof(req)=%lu\n", slen, sizeof(req));

	if ((slen = send(s, data, dlen, 0)) != dlen) {
		perror("client: send");
		close(s);
		return 0;
	}
	CMMDBG("slen=%u, sizeof(info)=%d\n", slen, dlen);

	/* Response */
	if ((rlen = read(s, &res, sizeof(res))) < 0) {
		perror("client: read");
		close(s);
		return 0;
	}
	CMMDBG("rlen=%u, sizeof(res)=%lu\n", rlen, sizeof(res));

	dump_data((char *) &res, sizeof(res));

	if (res.action != act || res.retcode != 1) {
		CMMDBG("cmmcli: wrong action or return code\n");
		close(s);
		return 0;
	}

	close(s);
	return 1;
}

static int
__cm_common_get (int act, void *data, int dlen)
{
	int s;
	request_t req;
	response_t res;
	unsigned int len, blen;
	int i;
	uint32_t slen, rlen;


	if (act == CMM_GET_MESHINFO) 
		CMMDBG("\nCMM_GET_MESHINFO\n");
	else if (act == CMM_GET_SITESURVEY)
		CMMDBG("\nCMM_GET_SITESURVEY\n");
	else
		return 0;

	if ((s = ipc_connect()) < 0)
		return 0;

	req.action = act;

	if ((slen = send(s, &req, sizeof(req), 0)) != sizeof(req)) {
		perror("client: send");
		close(s);
		return 0;
	}
	CMMDBG("slen=%u, sizeof(req)=%lu\n", slen, sizeof(req));

	if ((rlen = read(s, &res, sizeof(res))) < 0) {
		perror("client: read");
		close(s);
		return 0;
	}
	CMMDBG("rlen=%u, sizeof(res)=%lu\n", rlen, sizeof(res));

	dump_data((char *) &res, sizeof(res));

	if (res.action != act || res.retcode != 1) {
		CMMDBG("cmmcli: wrong action or return code\n");
		close(s);
		return 0;
	}

	if ((rlen = read(s, data, dlen)) < 0) {
		perror("client: read");
		close(s);
		return 0;
	}
	CMMDBG("rlen=%u, sizeof(cm_info_t)=%lu\n", rlen, sizeof(cm_info_t));

/*
	if (act == GET_MESHINFO) {
		CMMDBG("GET_MESHINFO\n");

		if ((rlen = read(s, data, dlen)) < 0) {
			close(s);
			return 0;
		}
		CMMDBG("rlen=%u, sizeof(res)=%lu\n", rlen, sizeof(cm_info_t));
	}
	else if (act == GET_SITESURVEY) {
		CMMDBG("GET_SITESURVEY\n");

		if ((rlen = read(s, data, dlen)) < 0) {
			close(s);
			return 0;
		}
		CMMDBG("rlen=%u, sizeof(res)=%lu\n", rlen, sizeof(cm_info_t));
	}

*/
	close(s);
	return 1;
}


int get_mesh_info(cm_info_t *info)
{
	return __cm_common_get (CMM_GET_MESHINFO, (void *)info, sizeof(cm_info_t));
}

int get_sitesurvey(cm_info_t *info)
{
	return __cm_common_get (CMM_GET_SITESURVEY, (void *)info, sizeof(cm_info_t));
}

int set_mesh_position(cm_info_t *info)
{
	return __cm_common_set (CMM_SET_POSITION, (void *)info, sizeof(cm_info_t));
}

int set_mesh_info(cm_info_t *info)
{
	return __cm_common_set (CMM_SET_MESHINFO, (void *)info, sizeof(cm_info_t));
}

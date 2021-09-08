
#include "../../core/sr_module.h"
#include "../../core/str.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

MODULE_VERSION

static int udp_send(struct sip_msg* msg, char* txt);
static int mod_init(void);
char *udp_srv_str;

int udpsend_socket;
struct sockaddr_in udpsend_addr;
 
static cmd_export_t cmds[]={
	{"udp_send", (cmd_function)udp_send, 1, 0, 0, REQUEST_ROUTE | FAILURE_ROUTE | ONREPLY_ROUTE | BRANCH_ROUTE },
	{0, 0, 0, 0, 0, 0}
};

static param_export_t params[]={
	{"destination", PARAM_STR, &udp_srv_str },
	{0,0,0}
};

struct module_exports exports = {
	"udp_send",
	DEFAULT_DLFLAGS,
	cmds,
	params,
	0,        /* RPC methods */
	0,        /* response function*/
	0,        /* destroy function */

	mod_init, /* module initialization function */
	0,        /* oncancel function */
	0         /* per-child init function */
};


static int mod_init(void)
{
	int ret;
	char* destination;
	char* port;
	unsigned int len = 0;
	
	if(udp_srv_str == NULL){
		WARN("Invalid destination");
		return -1;
	}

	if ((port = strchr(udp_srv_str, ':')) != NULL) {
		port = port + 1;
		len = strlen(udp_srv_str) - strlen(port) - 1;
	} else {
		WARN("Invalid destination");
		return -1;
	}
	
	destination = pkg_malloc(len);
	if (destination == NULL) {
		PKG_MEM_ERROR;
		return -1;
	}

	strncpy(destination, udp_srv_str, len);
	destination[len] = '\0';
	
  udpsend_addr.sin_family = AF_INET;
	ret = inet_aton(destination, &udpsend_addr.sin_addr);
	if (ret == 0) { 
		WARN("Invalid destination");
		return -1;
	}
  udpsend_addr.sin_port = htons(atoi(port));
 
	udpsend_socket = socket(PF_INET, SOCK_DGRAM, 0);
  if (udpsend_socket == -1) { 
		WARN("Could not create socket");
		return -1;
	}
	
	return 0;
}


static int udp_send(struct sip_msg* msg, char* txt)
{
	int ret;
 
	ret = sendto(udpsend_socket, txt, strlen(txt), 0, (struct sockaddr *)&udpsend_addr, sizeof(udpsend_addr));
 	if (ret == -1) {
		WARN("UDP send failed");
	}
	return 1;
}


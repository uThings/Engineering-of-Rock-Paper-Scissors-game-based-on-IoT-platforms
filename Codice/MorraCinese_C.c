#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "gpio.h"

#define BUFLEN 1024
#define NPACK 10
#define PORT 9000
#define SRV_IP "192.168.100.100"

void diep(char *s)
{
	perror(s);
	exit(1);
}

int main(void)
{
	struct sockaddr_in si_other;
	int s, n, slen = sizeof(si_other);
	char buf[BUFLEN];
	
	if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
		diep("socket");
	
	memset((char *) &si_other, 0, sizeof(si_other));
	
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(PORT);
	
	if (inet_aton(SRV_IP, &si_other.sin_addr)==0)
	{
		fprintf(stderr, "inet_aton() failed\n");
		exit(1);
	}
	

	gpio_open(4); //Forbici
	gpio_open(5); //Carta
	gpio_open(19); //Sasso
	gpio_open(18);
	gpio_set_direction(4, "in");
	gpio_set_direction(5, "in");
	gpio_set_direction(19, "in");
	gpio_set_direction(18, "out");
	
	while (1)
	{
		gpio_set(18, 1);
		
		if ((gpio_get(4) == 0) && (gpio_get(5) == 1) && (gpio_get(19) == 1))
			n = 8;
		
		if ((gpio_get(4) == 1) && (gpio_get(5) == 0) && (gpio_get(19) == 1))
			n = 1;
		
		if ((gpio_get(4) == 1) && (gpio_get(5) == 1) && (gpio_get(19) == 0))
			n = 19;
		
		if (((gpio_get(19) == 1) && (gpio_get(5) == 1) && (gpio_get(4) == 1)) || ((gpio_get(19) == 0) &&
		(gpio_get(4) == 0)) || ((gpio_get(19) == 0) && (gpio_get(5) == 0)) || ((gpio_get(5) == 0) &&
		(gpio_get(4) == 0)) || ((gpio_get(19) == 0) && (gpio_get(4) == 0) && (gpio_get(5) == 0)))
			n = 0;
			
		sprintf(buf, "%d", n);
		
		if (sendto(s, buf, BUFLEN, 0, &si_other, slen) == -1)
			diep("sendto()");
		
		if (n != 0)
		{
			printf("Sending %d\n\n", atoi(buf));
			gpio_set(18, 0);
			n = 0;
			delay(3000);
		}
	}
	
	close(s);
	gpio_close_all();
	
	return 0;
	
}

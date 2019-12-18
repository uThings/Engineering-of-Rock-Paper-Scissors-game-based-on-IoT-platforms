#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>

#include "gpio.h"
#include "oled-exp.h"

#define BUFLEN 1024
#define NPACK 10
#define PORT 9000

#define CL1 "192.168.100.101"
#define CL2 "192.168.100.102"

void diep(char *s)
{
	perror(s);
	exit(1);
}

int main(void)
{

	struct sockaddr_in si_me, si_other;
	int s;
	int slen = sizeof(si_other);
	char buf[1024];
	int dim;
	int choise_101, choise_102;
	int best_of_3 = 0, best_of_5 = 0, best_of_10 = 0;
	int cl_101 = 0, cl_102 = 0;
	int flag1 = 0, flag2 = 0;
	int res_button_1, res_button_2, res_button_3;
	int tot_A = 0;
	int tot_B = 0;
	int match_tipe; //al meglio di 3, al meglio di 5, al meglio di 10
	
	char a, b;

	int j = 0;


	oledDriverInit();

	gpio_open(19); //tasto di reset della partita
	gpio_open(18); //led di stato

	gpio_set_direction(19, "in");
	gpio_set_direction(18, "out");

/*********************************************************************************************************
				CONNESSIONE AI CLIENT
*********************************************************************************************************/


	if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
		diep("socket");

	memset((char *) &si_me, 0, sizeof(si_me));

	si_me.sin_family = AF_INET;
	si_me.sin_port = htons(PORT);
	si_me.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(s, &si_me, sizeof(si_me)) == -1)
		diep("bind");


/*********************************************************************************************************
				GESTIONE DELLA PARTITA
*********************************************************************************************************/


	while (1) //Ciclo while generale (fa andare avanti il gioco all'infinito)
	{


/*********************************************************************************************************
				FASE DI SCELTA DEL TIPO DI PARTITA
*********************************************************************************************************/
	
		oledClear();

		oledWrite("Scegliere partita:");
		oledSetCursor(2,0);	
		oledWrite("Meglio di 3 --> 1");
		oledSetCursor(4,0);
		oledWrite("Meglio di 5 --> 2");
		oledSetCursor(6,0);
		oledWrite("Meglio di 10 --> 3");

		delay(1000);
		gpio_set(18, 1);

		match_tipe = 0;

		choise_101 = 0;
		choise_102 = 0;


		while (match_tipe == 0) //Richiede all'utente di scegliere il tipo di partita
								//Entrambi i client possono effettuare la scelta
		{
			dim = recvfrom(s, buf, BUFLEN, 0, &si_other, &slen);
   			
   			if (dim > 0)
   			{
				if ((strcmp(CL1, inet_ntoa(si_other.sin_addr))) == 0)
					choise_101 = atoi(buf);

				if ((strcmp(CL2, inet_ntoa(si_other.sin_addr))) == 0)
					choise_102 = atoi(buf);	

				if ((choise_101 == 19) || (choise_102 == 19))
					match_tipe = 3;

				if ((choise_101 == 1) || (choise_102 == 1))
					match_tipe = 5;

				if ((choise_101 == 8) || (choise_102 == 8))
					match_tipe = 10;
			}
		}

	
		oledClear();


		if (match_tipe == 3)
		{
			oledWrite("Partita al meglio di 3");
			delay(3000);
			oledClear();
		}
		if (match_tipe == 5)
		{
			oledWrite("Partita al meglio di 5");
			delay(3000);
			oledClear();
		}
		if (match_tipe == 10)
		{
			oledWrite("Partita al meglio di 10");
			delay(3000);
			oledClear();
		}


/*********************************************************************************************************
				CONTROLLO DELLA PARTITA
				19: sasso
				8: forbici
				1: carta
*********************************************************************************************************/
	

		while ((tot_A < match_tipe) && (tot_B < match_tipe) && (gpio_get(19) == 1)) //Ciclo while di durata pari a quella scelta dagli utenti. Se si schiaccia il tasto
																					//di reset vengono stampati i risultati temporanei e la partita ricomincia 
		{

			if (recvfrom(s, buf, BUFLEN, 0, &si_other, &slen) == -1)
				diep("recvfrom()");

			if ((strcmp(CL1, inet_ntoa(si_other.sin_addr))) == 0 && flag1 == 0)
				cl_101 = atoi(buf);

			if ((strcmp(CL2, inet_ntoa(si_other.sin_addr))) == 0 && flag2 == 0)
				cl_102 = atoi(buf);	


			if ((cl_101 == 19 || cl_101 == 1 || cl_101 == 8) && (cl_102 == 19 || cl_102 == 1 || cl_102 == 8))
			{
				if (cl_101 == 19 && cl_102 == 19)
				{
					oledWrite("A: Sasso");
					oledSetCursor(2,0);
					oledWrite("B: Sasso");
					oledSetCursor(4,0);
					oledWrite("RISULTATO: Parita");
					delay(3000);
					oledClear();
				}
			
				if (cl_101 == 1 && cl_102 == 1)
				{
					oledWrite("A: Carta");
					oledSetCursor(2,0);
					oledWrite("B: Carta");
					oledSetCursor(4,0);
					oledWrite("RISULTATO: Parita");
					delay(3000);
					oledClear();
				}
			
				if (cl_101 == 8 && cl_102 == 8)
				{
					oledWrite("A: Forbici");
					oledSetCursor(2,0);
					oledWrite("B: Forbici");
					oledSetCursor(4,0);
					oledWrite("RISULTATO: Parita");
					delay(3000);
					oledClear();
				}
			
				if (cl_101 == 19 && cl_102 == 1)
				{
					oledWrite("A: Sasso");
					oledSetCursor(2,0);
					oledWrite("B: Carta");
					oledSetCursor(4,0);
					oledWrite("RISULTATO: Vince B");
					tot_B++;
					delay(3000);
					oledClear();
				}
			
				if (cl_101 == 1 && cl_102 == 19)
				{
					oledWrite("A: Carta");
					oledSetCursor(2,0);
					oledWrite("B: Sasso");
					oledSetCursor(4,0);
					oledWrite("RISULTATO: Vince A");
					tot_A++;
					delay(3000);
					oledClear();
				}
			
				if (cl_101 == 19 && cl_102 == 8)
				{
					oledWrite("A: Sasso");
					oledSetCursor(2,0);
					oledWrite("B: Forbici");
					oledSetCursor(4,0);
					oledWrite("RISULTATO: Vince A");
					tot_A++;
					delay(3000);
					oledClear();
				}
			
				if (cl_101 == 8 && cl_102 == 19)
				{
					oledWrite("A: Forbici");
					oledSetCursor(2,0);
					oledWrite("B: Sasso");
					oledSetCursor(4,0);
					oledWrite("RISULTATO: Vince B ");
					tot_B++;
					delay(3000);
					oledClear();
				}
			
				if (cl_101 == 8 && cl_102 == 1)
				{	
					oledWrite("A: Forbici");
					oledSetCursor(2,0);
					oledWrite("B: Carta");
					oledSetCursor(4,0);
					oledWrite("RISULTATO: Vince A ");
					tot_A++;
					delay(3000);
					oledClear();
				}
			
				if (cl_101 == 1 && cl_102 == 8)
				{
					oledWrite("A: Carta");
					oledSetCursor(2,0);
					oledWrite("B: Forbici");
					oledSetCursor(4,0);
					oledWrite("RISULTATO: Vince B");
					tot_B++;
					delay(3000);
					oledClear();
				}

				flag1 = 0;
				flag2 = 0;

				cl_101 = 0;
				cl_102 = 0;


			} else if ((cl_101 == 19 || cl_101 == 1 || cl_101 == 8) && cl_102 == 0)
				flag1 = 1;
			else if (cl_101 == 0 && (cl_102 == 19 || cl_102 == 1 || cl_102 == 8))
				flag2=1;

		}


/*********************************************************************************************************
				STAMPA DEL RISULTATO
*********************************************************************************************************/


		a = tot_A +'0';
		b = tot_B + '0';
	
		oledWrite("FINALE A:");
		oledSetCursor(2,0);	
		oledWriteChar(a);
		oledSetCursor(4,0);
		oledWrite("FINALE B:");
		oledSetCursor(6,0);
		oledWriteChar(b);	
		delay(3000);

		tot_A = 0;
		tot_B = 0;

		gpio_set(18, 0);

	}

	close(s);
	gpio_close_all();
	
	return EXIT_SUCCESS;

}

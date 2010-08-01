#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/parport.h>
#include <linux/ppdev.h>
#include <math.h>
#include <SDL.h>

int fd_pp;

void send_parport(unsigned char data)
{
	ioctl(fd_pp, PPWDATA, & data);
}

void send_control(unsigned char data)
{
	ioctl(fd_pp, PPWCONTROL, & data);
}

int init_parport()
{
	int i;

	fd_pp = open("/dev/parport0", O_WRONLY);
	if (fd_pp < 0)
	{
		perror("open lp");
		return 1;
	}

	send_parport(0);
	send_control(1<<1);

	ioctl(fd_pp, PPCLAIM);
	i = PARPORT_MODE_COMPAT;
	ioctl(fd_pp, PPSETMODE, & i);
	i = IEEE1284_MODE_COMPAT;
	ioctl(fd_pp, PPNEGOT, & i);
	return 0;
}




int avance(long moteur1, long moteur2, long moteur3, long moteur4, int mode)
{
	static long etat1 = 0l;
	static long etat2 = 0l;
	static long etat3 = 0l;
	static long etat4 = 0l;

	static unsigned char dmasque = 0;
	int retour = 0;
	unsigned char dmasque2 = 0;
	static unsigned char cmasque = (1<<1);
	unsigned char cmasque2 = 0;

	if (mode == 0)
	{
		if (moteur1 != etat1)
		{
			dmasque2 |= (1<<0);
			retour = 1;
		}

		if (moteur2 != etat2)
		{
			dmasque2 |= (1<<2);
			retour = 1;
		}

		if (moteur3 != etat3)
		{
			dmasque2 |= (1<<4);
			retour = 1;
		}

		if (moteur4 != etat4)
		{
			cmasque2 |= (1<<3);
			retour = 1;
		}



		if (moteur1 < etat1) 
		{
			dmasque |= (1<<1);
			etat1--;
		}

		if (moteur2 < etat2)
		{
			dmasque |= (1<<3);
			etat2--;
		}

		if (moteur3 < etat3)
		{
			dmasque |= (1<<5);
			etat3--;
		}

		if (moteur4 > etat4) //inversé
		{
			cmasque |= (1<<2);
			etat4++;
		}



		if (moteur1 > etat1)
		{
			dmasque &= ~(1<<1);
			etat1++;
		}

		if (moteur2 > etat2)
		{
			dmasque &= ~(1<<3);
			etat2++;
		}

		if (moteur3 > etat3)
		{
			dmasque &= ~(1<<5);
			etat3++;
		}

		if (moteur4 < etat4) //inversé
		{
			cmasque &= ~(1<<2);
			etat4--;
		}



		send_parport(dmasque);
		send_control(cmasque);
		usleep(500);

		send_parport(dmasque|dmasque2);

		send_control(cmasque|cmasque2);
		usleep(500);

	/*	send_parport(0);
		usleep(1000);*/

	//	printf("%X %X %ld %ld\n", (unsigned int) 0, (unsigned int) 0|dmasque2, moteur1, etat1);
	}
	else if (mode == 1)
	{
		etat1 += moteur1;
		etat2 += moteur2;
		etat3 += moteur3;
		etat4 += moteur4;
	}
	return retour;

}

void impulsion48()
{
	int i;
	for (i = 0; i < 48*8; i++)
	{
		unsigned char data;
		unsigned char control;
		control = (1<<1) /*| (1<<3)*/;
		data = (1<<0) | (0<<1);
		send_parport(data);
		send_control(control);
		usleep (1000);
		data = (0<<1);
		control = (1<<1);
		send_parport(data);
		send_control(control);
		usleep (1000);
	}
}	

void sinusoide(float ampli1, float periode1, float dephasage1, float offset1, float ampli2, float periode2, float dephasage2, float offset2, float dephasage, int mode)
{
	static int i = 0;
	if (mode == 0)
	{
		avance(ampli1*sin((float) i / periode1 * 2 * M_PI) + offset1,
		       ampli2*sin((float) i / periode2 * 2 * M_PI + dephasage * M_PI) + offset2,
		       ampli2*sin((float) i / periode2 * 2 * M_PI + M_PI * dephasage2 + dephasage * M_PI) + offset2,
		       ampli1*sin((float) i / periode1 * 2 * M_PI + M_PI * dephasage1) + offset1,
		       0);
		i++;
	}
	else
	{
		i = 0;
	}
}



void rembobine()
{
		avance(0,0,0,0,0);
}

void depart(float ampli, float periode, float dephasage)
{
	int i;
	for (i = 0; i < 1000; i++)
	{
		avance(0, ampli*sin((float) 0 / periode * 2 * M_PI),ampli*sin((float) 0 / periode * 2 * M_PI + M_PI * dephasage), 0,0);
	}
}

int close_parport()
{
	unsigned char data;

	send_parport(0);
	send_control(0);

	ioctl(fd_pp, PPRELEASE);

	return close(fd_pp);
}


int main()
{
	init_parport();
	ouvre();
	    SDL_Surface *ecran = NULL;
	    SDL_Event event;
	    int continuer = 1;
	    int tempsPrecedent = 0, tempsActuel = 0;
	    int increment0 = 0;
	    int increment1 = 0;
	    int increment2 = 0;
	    int increment3 = 0;
	    int dmx = 1;
	    int dmx_objectif = 0;
	    int i_dmx = 0;
	    int mode = 0;

	    int compteur = 0;

	    SDL_Init(SDL_INIT_VIDEO);

	    ecran = SDL_SetVideoMode(640, 480, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);


	
	    while (continuer)
	    {
		compteur++;
		while (SDL_PollEvent(&event))
		{
			switch(event.type)
			{
			    case SDL_QUIT:
			        continuer = 0;
				dmx = 0;
				ecrit(2,0);
			        break;

			    case SDL_KEYDOWN:
				switch(event.key.keysym.sym)
				{
				    case SDLK_KP1:
					increment0 = (event.key.keysym.mod & KMOD_RCTRL)?-1:1;
				        break;

				    case SDLK_KP2:
					increment1 = (event.key.keysym.mod & KMOD_RCTRL)?-1:1;
				        break;

				    case SDLK_KP3:
					increment2 = (event.key.keysym.mod & KMOD_RCTRL)?-1:1;
				        break;

				    case SDLK_KP4:
					increment3 = (event.key.keysym.mod & KMOD_RCTRL)?-1:1;
				        break;

				    case SDLK_F1:
					dmx_objectif = 0;
					mode = 0;
				        break;

				    case SDLK_F2:
					dmx_objectif = 64;
					mode = 1;
				        break;

				    case SDLK_F3:
					dmx_objectif = 64;
					mode = 2;
				        break;

    				    case SDLK_F4:
					dmx_objectif = 64;
					mode = 3;
				        break;


    				    case SDLK_F5:
					dmx_objectif = 128;
					mode = 4;
				        break;

    				    case SDLK_F6:
					dmx_objectif = 255;
					mode = 5;
				        break;

    				    case SDLK_F7:
					dmx_objectif = 255;
					mode = 6;
				        break;

    				    case SDLK_F8:
					dmx_objectif = 0;
					mode = 7;
				        break;


				    default:
					break;

				}


				break;

			    case SDL_KEYUP:
				switch(event.key.keysym.sym)
				{
				    case SDLK_KP1:
					increment0 = 0;
				        break;

				    case SDLK_KP2:
					increment1 = 0;
				        break;

				    case SDLK_KP3:
					increment2 = 0;
				        break;

				    case SDLK_KP4:
					increment3 = 0;
				        break;
				    default:
					break;

				}
				break;

			}
		}

		switch(mode)
		{
			case 0:
				avance(0,0,0,0,0);
				sinusoide(300.f, 1000.f, .75f, 0.f, 300.f, 1000.f, .75f, 0.f, 0.f, 1);
				avance(increment0,increment1,increment2,increment3, 1);
				break;


			case 1:
				sinusoide(0.f, 2000.f, .75f, 1000.f, //1 un au dessus autre
					  0.f, 2000.f, .75f, 0.f,
					  1.f, 0);
				break;

			case 2: 
				sinusoide(0.f, 2000.f, 1.f, 1000.f, //1 simple
					  50.f, 2000.f, 1.f, 0.f,
					  1.f, 0);
				break;

			case 3:
				sinusoide(50.f, 2000.f, 1.f, 1000.f, //1 double fort
					  50.f, 2000.f, 1.f,  0.f,
					  1.f, 0);
				break;



			case 4: 
				sinusoide(100.f, 2000.f, .75f, -1000.f, //2 double
					  100.f, 2000.f, .75f, -1000.f,
					  0.3f, 0);
				break;

			case 5:
				sinusoide(300.f, 1500.f, .75f, -3000.f, //2 double fort
					  300.f, 1500.f, .75f, -3000.f,
					  0.1f, 0);
				break;

			case 6:
				sinusoide(0.f, 2000.f, .75f, -8000.f, //2 un au dessus autre
					  0.f, 2000.f, .75f, -8000.f,
					  1.f, 0);
				break;

			case 7:
				sinusoide(0.f, 2000.f, .75f, 0.f, //2 un au dessus autre
					  0.f, 2000.f, .75f, 0.f,
					  1.f, 0);
				break;

		}

		if (i_dmx < 1)
		{
			i_dmx++;
		}
		else
		{
			printf ("%d %d\n", dmx, dmx_objectif);
			i_dmx = 0;
			if (dmx_objectif != dmx)
			{
				if (dmx_objectif < dmx)
				{
					dmx--;
				}
				else
				{
					dmx++;
				}
				char string[200];
				ecrit(dmx,2);
			}

		}
				
	    }

	    SDL_Quit();
	while (avance(0,0,0,0,0));

	ferme();
	close_parport();
	return 0;
}

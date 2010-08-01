#include <unistd.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


int serie;

void ouvre()
{
	serie = open("/dev/ttyS3", O_RDWR | O_NOCTTY | O_NDELAY);
	if(serie<0)
	{
		perror("serial port open");
		exit(-1);
	}
	else
	{
                 fcntl(serie,F_SETFL,0);
        }

	struct termios options;
	tcgetattr(serie, &options);
	cfsetospeed(&options, B38400);
	options.c_cflag |= (CLOCAL | CREAD);
	options.c_cflag &= ~PARENB;
	options.c_cflag &= ~CSTOPB;
	options.c_cflag &= ~CSIZE;
	options.c_cflag |= CS8;

	tcsetattr(serie, TCSANOW, &options);


}

void ferme()
{
	close(serie);
}

void ecrit(unsigned char data, int canal)
{
	unsigned char donnee[3] = {(1<<7)|(canal&0x7F), ((canal & 0x180)>>2) | (data&0x0F), (data>>4)};
	if (write(serie, donnee, 3) < 0){perror("write:");}
	tcdrain(serie);
}


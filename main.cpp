#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>

#define BAUDRATE B115200
//#define MODEMDEVICE "/dev/tty.ArctisNova7"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1
//ls /dev/tty.*
//ttyama0 is gpio tx and rx
volatile int STOP = FALSE;

extern "C" void signal_handler_IO(int status);

int wait_flag = TRUE;

int main(int argc, char* argv[])
{

    if(argc == 1)
    {
        printf("No Device Listed!\n");
        exit(-1);
    }
    
    int fd,c, res;
    struct termios oldtio,newtio;
    struct sigaction saio;           /* definition of signal action */
    char buf[255];
    /* open the device to be non-blocking (read will return immediatly) */

    printf("Attempting to read Device @ %s\n", argv[0]);
    fd = open(argv[1], O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd == -1) 
    {
        perror(argv[1]);
        exit(-1);
    }

    /* install the signal handler before making the device asynchronous */
    saio.sa_handler = signal_handler_IO;
    sigemptyset(&saio.sa_mask);  
    saio.sa_flags = 0;
    sigaction(SIGIO, &saio, NULL);
    
    fcntl(fd, F_SETOWN, getpid()); /* allow the process to receive SIGIO */

    fcntl(fd, F_SETFL, FASYNC); // make async
    // only O_APPEND and O_NONBLOCK, will work with F_SETFL
    tcgetattr(fd,&oldtio); /* save current port settings for easy revert*/
   
    newtio.c_cflag = BAUDRATE | CRTSCTS | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR | ICRNL;
    newtio.c_oflag = 0;
    newtio.c_lflag = ICANON; //waits till nl or del. is found, then processes whole line, zero this
    newtio.c_cc[VMIN]=1;
    newtio.c_cc[VTIME]=0;
    tcflush(fd, TCIFLUSH);
    tcsetattr(fd,TCSANOW,&newtio);

    /* loop while waiting for input. normally we would do something
        useful here */ 
    while (STOP==FALSE) {
    printf(".\n");
    usleep(100000);
        /* after receiving SIGIO, wait_flag = FALSE, input is available
            and can be read */
        if (wait_flag==FALSE) { 
            res = read(fd,buf,255);
            buf[res]=0;
            printf(":%s:%d\n", buf, res);
            if (res==1) STOP=TRUE; /* stop loop if only a CR was input */
            wait_flag = TRUE;      /* wait for new input */
        }
    }
            /* restore old port settings */
    tcsetattr(fd,TCSANOW,&oldtio);
    return 0;
}

// signals version of an ISR, very nice!
void signal_handler_IO(int status)
{
    printf("Received SIGIO Signal. \n");
    wait_flag = FALSE;
}
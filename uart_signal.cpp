#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include "uart_signal.hpp"
#include <errno.h>
#include <string>


UART_touch::UART_touch(int argc, char* argv[])
{
    printf("Attempting to read Device @ %s\n", argv[0]);

    fd = open(argv[1], O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd == -1) 
    {
        perror(argv[1]);
        exit(-1);
    }
    // use sig action if shit gets hairy
    if(signal(SIGIO, UART_touch::signal_handler) == SIG_ERR)
    {
        throw SignalException(std::string("!! Error In Signal Setup !!"));
    }

    fcntl(fd, F_SETOWN, getpid()); 
    // only O_APPEND and O_NONBLOCK, will work with F_SETFL
    fcntl(fd, F_SETFL, FASYNC); // make async
    //save current port settings for easy revert
    tcgetattr(fd,&oldtio); 
    newtio.c_cflag = BAUDRATE | CRTSCTS | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR | ICRNL;
    newtio.c_oflag = 0;
    newtio.c_lflag = ICANON; 
    newtio.c_cc[VMIN] = 1;
    newtio.c_cc[VTIME] = 0;
    tcflush(fd, TCIFLUSH);
    tcsetattr(fd,TCSANOW,&newtio);
}

UART_touch::~UART_touch()
{
    // Revert old settings
    tcsetattr(fd,TCSANOW,&oldtio);
}    

void UART_touch::signal_handler(int status)
{
    printf("Received SIGIO Signal. \n");
    wait_flag = true; //false
}



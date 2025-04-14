#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
//https://stackoverflow.com/questions/343219/is-it-possible-to-use-signal-inside-a-c-class
#define BAUDRATE B115200
//#define MODEMDEVICE "/dev/tty.ArctisNova7"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

extern "C" void signal_handler_IO(int status);


class uart {       // The class
    public:             // Access specifier
      volatile sig_atomic_t wait_flag = 1;
 
    volatile int STOP = FALSE;

    int wait_flag = TRUE;

    uart(){
        /* install the signal handler before making the device asynchronous */
        
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
    }
    ~uart(){
        //restore port settings
        tcsetattr(fd,TCSANOW,&oldtio);
    }
    void Execute()
    {   
        

        printf("Attempting to read Device @ %s\n", argv[0]);
        fd = open(argv[1], O_RDWR | O_NOCTTY | O_NONBLOCK);
        if (fd == -1) 
        {
            perror(argv[1]);
            exit(-1);
        }

        

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
        //restore port settings
        //tcsetattr(fd,TCSANOW,&oldtio);
        return;
    }
    void signal_handler_IO(int status)
    {
        printf("Received SIGIO Signal. \n");
        wait_flag = FALSE;
    }
    private:
        int fd,c, res;
        struct termios oldtio,newtio;
        struct sigaction saio{};
        saio.sa_handler = signal_handler_IO; 
        char buf[255];
        /* open the device to be non-blocking (read will return immediatly) */
};


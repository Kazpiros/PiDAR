#ifndef __UARTSIGNAL_H__
#define __UARTSIGNAL_H__
#include <stdexcept>
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

using std::runtime_error;

class SignalException : public runtime_error
{
    public:
        SignalException(const std::string& _message)
            : std::runtime_error(_message){}
};


class UART_touch
{
    public:
        static bool wait_flag;
        UART_touch(int argc, char* argv[]);
        ~UART_touch();
        static void signal_handler(int status);
    private:
        int fd,c, res;
        struct termios oldtio,newtio;
        char buf[255];
        
};
#endif
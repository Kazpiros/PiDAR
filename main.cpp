#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include "uart_signal.hpp"

int main(int argc, char* argv[])
{
    if(argc == 1)
    {
        printf("No Device Listed!\n");
        exit(-1);
    }

    try
    {
        UART_touch touch(argc, argv);
        while(touch.wait_flag == true);
    }
    
    catch (SignalException& e){
        std::cout << "exception of type: " << e.what() << std::endl;
    }
    return 0;
}
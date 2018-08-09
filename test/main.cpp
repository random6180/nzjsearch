#include "../include/tcpserver.h"
#include <iostream>
using namespace std;

int main(int argc, char *argv[]) {
    cout << "start: " << endl; 
    Tcpserver server(12345, 2, 50);
    server.Serve();
    return 0;
}

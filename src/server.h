#ifndef SERVER_H
#define SERVER_H

#include <Arduino.h>

class Server {
public:
    Server();
    void begin();
    void handleClient();
private:
    // Private members
};

#endif

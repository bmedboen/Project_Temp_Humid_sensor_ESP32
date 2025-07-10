// web_server.h

#pragma once

#include <WebServer.h>

void initWebServerAP();
void setupWebServerRoutes();
void startWebServer();
void handleWebServerClients();
bool isWebServerActive();
void resetWebServerActivityTimer();
bool isWebServerTimeoutReached();

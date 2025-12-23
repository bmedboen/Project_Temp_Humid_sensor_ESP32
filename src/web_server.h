// web_server.h

#pragma once

#include <WebServer.h>

bool activateWebServer();
void handleWebServerClients();
bool isWebServerActive();
bool stopWebServerIfIdle();

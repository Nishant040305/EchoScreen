#include<constants.h>
#include<SocketIOclient.h>

void sendSocketEvent(String eventName, String data);
void sendSocketEventWithJson(String eventName, JsonObject data);
#include "task_update.h"

// TODO - all update message fields should have static global variables here


static void sendUpdate(void) {

    // format and send udpate message using global variables from this file
    // TODO
}

void vUpdate_MainTask(void *pvParams) {

    while(1) {
        // repeat this loop as often as possible
        // TODO - establish values for all field of update message

        // TODO - calculate demod for all channels
        
        sendUpdate();
    }
}
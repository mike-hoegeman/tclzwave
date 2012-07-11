/*
 *
 */
#ifndef _OZW_H
#define _OZW_H

#include "tcl.h"
#include <unistd.h>
#include <pthread.h>
#include "Options.h"
#include "Manager.h"
#include "Driver.h"
#include "Node.h"
#include "Group.h"
#include "Notification.h"
#include "Log.h"


typedef struct OzwManagerClientData {
    int notificationSendSocket;
    int notificationSendPort;
    Tcl_Channel notificationSendChannel;
    /**/
    Tcl_DString notificationRecvChanName;
    Tcl_DString notificationRecvCommand;
} OzwManagerClientData;

extern void Ozw_Watcher ( 
    OpenZWave::Notification const* _notification, 
    void* _context
);

/* end include guard */
#endif 

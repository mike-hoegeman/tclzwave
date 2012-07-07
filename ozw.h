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

typedef struct Ozw_WatcherContext {
    int initialized;
    Tcl_DString command; 
    Tcl_Interp *interp;
} Ozw_WatcherContext;

typedef struct Ozw_NotificationClientData {
    Ozw_WatcherContext *watcherContextPtr;
    const OpenZWave::Notification *notificationPtr;
} Ozw_NotificationClientData;


extern void Ozw_Watcher ( 
    OpenZWave::Notification const* _notification, void* _context
);
extern void Ozw_WatcherContextInit(
    Ozw_WatcherContext *ctxt, 
    Tcl_Interp *interp,
    const char *command
);
extern void Ozw_WatcherContextUnInit(Ozw_WatcherContext *ctxt);
extern void Ozw_WatcherContextReset(Ozw_WatcherContext *ctxt);
extern int Ozw_CreateNotificationInst(Ozw_NotificationClientData *ncd);
extern pthread_mutex_t Ozw_MainMutex;

#endif

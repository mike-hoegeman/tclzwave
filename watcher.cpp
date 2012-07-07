/*
 *
 */
#include "ozw.h"

void Ozw_WatcherContextInit(
    Ozw_WatcherContext *ctxt, 
    Tcl_Interp *interp,
    const char *command
) {
    Tcl_DStringInit(&(ctxt->command));
    Tcl_DStringAppend(&(ctxt->command), command , -1);
    ctxt->initialized = 1;
    ctxt->interp = interp;
};
void Ozw_WatcherContextUnInit(Ozw_WatcherContext *ctxt) {
    Tcl_DStringFree(&(ctxt->command));
    ctxt->initialized = 0;
    ctxt->interp = NULL;
};
/* callback employed by ozw::manager addwatcher -command {xxx} */
/* which then in turn runs the tcl command {xxx} */
void Ozw_Watcher (
    OpenZWave::Notification const* _notification, 
    void* _context
) {
    Ozw_WatcherContext *ctxt = (Ozw_WatcherContext *)_context;
    Ozw_NotificationClientData *ncd =
     (Ozw_NotificationClientData *) ckalloc(sizeof(Ozw_NotificationClientData));
    if (ncd == NULL) {
        fprintf(stderr, "could not make notification client data block");
        /* ??? */
    }
    ncd->watcherContextPtr = ctxt;
    ncd->notificationPtr = _notification;

    pthread_mutex_lock( &Ozw_MainMutex );
    {
        Ozw_CreateNotificationInst(ncd); /* frees ncd evetually */
    }
    pthread_mutex_unlock( &Ozw_MainMutex );
    return;
}

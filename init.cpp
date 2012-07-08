/*
 *
 */
#include "tcl.h"
#include <unistd.h>
#include <pthread.h>
#include <string.h>

#if TARGET_API_MAC_CARBON
#   include <Tcl/tcl.h>
#else
#   include "tcl.h"
#endif

static int OzwVersionObjCmd(
    ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]
);

pthread_mutex_t Ozw_MainMutex;

static int Ozw_Exit = 0;
static int OzwExitObjCmd(
    ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]
) {
    Ozw_Exit = 1;
    Tcl_SetObjResult(interp, Tcl_NewIntObj(0));
    return TCL_OK;
}
static void Ozw_TclMainLoop() {
    while (!Ozw_Exit) {
        pthread_mutex_lock( &Ozw_MainMutex );
        Tcl_DoOneEvent(0);
        pthread_mutex_unlock( &Ozw_MainMutex );
    }
}
static void OzwVersionDeleteProc(ClientData clientData) {
    // convenient place to put the mutex cleanup;
    pthread_mutex_t *mainMutexPtr = (pthread_mutex_t *)clientData;
    pthread_mutex_destroy(mainMutexPtr);
    memset(mainMutexPtr, sizeof(pthread_mutex_t), '\0');
}

DLLEXPORT int Ozw_Init(Tcl_Interp *interp) {
    if (Tcl_InitStubs(interp, "8.1", 0) == NULL) {
        return TCL_ERROR;
    }

    /* 
     * set up a lock so that the openwave thread that handles notifications
     * can share use of the tcl interpreter w/ main thread in an orderly manner
     #
     * we (should?) have the notification callback at the c level
     * just place the notification tcl commands into the "after idle" queue
     * and then return. then at the tcl level the commands are getting executed
     * in the main thread. much lest contention possibilities that way.
     */
    {
        pthread_mutexattr_t mutexattr;
        pthread_mutexattr_init ( &mutexattr );
        pthread_mutexattr_settype( &mutexattr, PTHREAD_MUTEX_RECURSIVE );
        pthread_mutex_init(&Ozw_MainMutex, &mutexattr );
        pthread_mutexattr_destroy( &mutexattr );
    }
    Tcl_CreateObjCommand(interp, 
        "::ozw::exit", OzwExitObjCmd, (ClientData) NULL, NULL);
    Tcl_CreateObjCommand(interp, 
        "::ozw::version", OzwVersionObjCmd, 
        (ClientData) &Ozw_MainMutex, OzwVersionDeleteProc);
    Tcl_SetMainLoop(Ozw_TclMainLoop);

    extern int Ozw_NotificationInitArrays(Tcl_Interp *interp);
    if (Ozw_NotificationInitArrays(interp) != TCL_OK) { return TCL_ERROR; }
    extern int OzwOptions_Init(Tcl_Interp *interp);
    if (OzwOptions_Init(interp) != TCL_OK) { return TCL_ERROR; }
    extern int OzwManager_Init(Tcl_Interp *interp);
    if (OzwManager_Init(interp) != TCL_OK) { return TCL_ERROR; }
    extern int OzwLog_Init(Tcl_Interp *interp);
    if (OzwLog_Init(interp) != TCL_OK) { return TCL_ERROR; }

    return Tcl_PkgProvide(interp, "ozw", "0.1");
}

DLLEXPORT int Ozw_SafeInit(Tcl_Interp *interp) {
    return Ozw_Init(interp);
}


static int OzwVersionObjCmd(
    ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]
) {
    Tcl_SetObjResult(interp, Tcl_NewStringObj("0.1", -1));
    return TCL_OK;
}

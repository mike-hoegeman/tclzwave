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
static int Ozw_RunEventsUnLocked = 0;
static int OzwExitObjCmd(
    ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]
) {
    Ozw_Exit = 1;
    Tcl_SetObjResult(interp, Tcl_NewIntObj(0));
    return TCL_OK;
}
#if 0
static void Ozw_TclMainLoop() {
    while (!Ozw_Exit) {
        if (Ozw_RunEventsUnLocked) {
            Tcl_DoOneEvent(0);
        } else {
            pthread_mutex_lock( &Ozw_MainMutex );
            Tcl_DoOneEvent(0);
            pthread_mutex_unlock( &Ozw_MainMutex );
        }
    }
}
#endif

static int OzwUnlockObjCmd(
    ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]
) {
    if (objc != 2) {
        Tcl_WrongNumArgs(interp, 1, objv, "command");
        return TCL_ERROR;
    }
    const char *command = Tcl_GetString(objv[1]);
        if (command == NULL) {
        Tcl_SetObjResult(interp, 
            Tcl_NewStringObj("error getting command", -1));
        return TCL_ERROR;
    }
    Ozw_RunEventsUnLocked = 1;
    const int eval_result = Tcl_EvalEx(interp, command, -1, 0);
    Ozw_RunEventsUnLocked = 0;
    return eval_result;
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
    Tcl_CreateObjCommand(interp, 
        "::ozw::unlockevents", OzwUnlockObjCmd, 
        (ClientData) &Ozw_MainMutex, NULL);
#if  0
    Tcl_SetMainLoop(Ozw_TclMainLoop);
#endif

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


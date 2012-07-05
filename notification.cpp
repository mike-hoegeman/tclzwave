/*
 *
 */
#include "ozw.h"
#include <string>
using namespace std;

static long Ozw_NotificationInstNo = 0;

static void Ozw_NotificationInstDelProc(
    ClientData clientData
) {
    ckfree(clientData);
    return; 
}

static int Ozw_NotificationInstCmd(
    ClientData clientData, 
    Tcl_Interp *interp, 
    int objc, 
    Tcl_Obj *CONST objv[]
) {
    Tcl_SetObjResult(interp, Tcl_NewStringObj("foo", -1));
    return TCL_OK;
}

int Ozw_CreateNotificationInst(
    Ozw_NotificationClientData *ncd
) {
    char command_name[200]; 
    sprintf(command_name, "::ozw::inst::notification%lx", 
        Ozw_NotificationInstNo++); 
    Tcl_CreateObjCommand(
        ncd->watcherContextPtr->interp, 
        command_name, 
        Ozw_NotificationInstCmd,
        (ClientData)ncd, 
        Ozw_NotificationInstDelProc
    );
    Tcl_SetObjResult(
        ncd->watcherContextPtr->interp, 
        Tcl_NewStringObj(command_name, -1));
    return TCL_OK;
}

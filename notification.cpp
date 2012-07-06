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
    char inst_name[200]; 
    int eval_result = TCL_ERROR;
    sprintf(inst_name, "::ozw::inst::notification%lx", 
        Ozw_NotificationInstNo++); 
    Tcl_Command command_token = Tcl_CreateObjCommand(
        ncd->watcherContextPtr->interp, 
        inst_name, 
        Ozw_NotificationInstCmd,
        (ClientData)ncd, 
        Ozw_NotificationInstDelProc
    );
    {
        Tcl_DString ds;
        Tcl_DStringInit(&ds);
        Tcl_DStringAppend(
            &ds, Tcl_DStringValue(&(ncd->watcherContextPtr->command)), -1
        );
        Tcl_DStringAppendElement(&ds, inst_name);
        eval_result = Tcl_EvalEx(
            ncd->watcherContextPtr->interp, 
            Tcl_DStringValue(&ds), -1, 
            TCL_EVAL_GLOBAL
        );
        Tcl_DStringGetResult(ncd->watcherContextPtr->interp, &ds);
        if (eval_result != TCL_OK) {
            fprintf(stderr, 
                "Error running notification callback: %s\n----\n", 
                Tcl_DStringValue(&ds));
        }
        if (Tcl_DeleteCommandFromToken(
            ncd->watcherContextPtr->interp,
            command_token) != TCL_OK) {
            fprintf(stderr, "error deleting notification instance"); 
        }
        Tcl_DStringFree(&ds);
    }

    return TCL_OK;
}

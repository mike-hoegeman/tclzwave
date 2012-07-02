/*
 *
 */
#include "tcl.h"

#if TARGET_API_MAC_CARBON
#   include <Tcl/tcl.h>
#else
#   include "tcl.h"
#endif

static int OzwOptionsObjCmd_SerialNo = 0;

static int OzwOptionsInstObjCmd(
    ClientData clientData, 
    Tcl_Interp *interp, 
    int objc, 
    Tcl_Obj *CONST objv[]
) {
    Tcl_SetObjResult(interp, Tcl_NewStringObj("foo", -1));
    return TCL_OK;
}

static int OzwOptionsObjCmd(
    ClientData clientData, 
    Tcl_Interp *interp, 
    int objc, 
    Tcl_Obj *CONST objv[]
) {
    char command_name[200]; 
    sprintf(command_name, "::ozw::cmd::options%x", OzwOptionsObjCmd_SerialNo++); 
    Tcl_CreateObjCommand(
        interp, command_name, OzwOptionsInstObjCmd,
        (ClientData)NULL, NULL
    );
    Tcl_SetObjResult(interp, Tcl_NewStringObj(command_name, -1));
    return TCL_OK;
}

DLLEXPORT int OzwOptions_Init(Tcl_Interp *interp) {
    if (Tcl_InitStubs(interp, "8.1", 0) == NULL) {
        return TCL_ERROR;
    }
    Tcl_CreateObjCommand(interp, 
        "::ozw::options", 
        OzwOptionsObjCmd, 
        (ClientData) NULL, 
        NULL);
}

DLLEXPORT int OzwOptions_SafeInit(Tcl_Interp *interp) {
    return OzwOptions_Init(interp);
}

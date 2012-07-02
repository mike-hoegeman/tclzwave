/*
 *
 */
#include "tcl.h"

#if TARGET_API_MAC_CARBON
#   include <Tcl/tcl.h>
#else
#   include "tcl.h"
#endif

static int OzwObjCmd(
    ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]
);

DLLEXPORT int Ozw_Init(Tcl_Interp *interp) {
    if (Tcl_InitStubs(interp, "8.1", 0) == NULL) {
        return TCL_ERROR;
    }
    Tcl_CreateObjCommand(interp, "minimal", OzwObjCmd, (ClientData) NULL, NULL);
    return Tcl_PkgProvide(interp, "minimal", "0.1");
}

DLLEXPORT int Ozw_SafeInit(Tcl_Interp *interp) {
    return Ozw_Init(interp);
}

static int OzwObjCmd(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]) {
    Tcl_SetObjResult(interp, Tcl_NewStringObj("minimal seems ok", -1));
    return TCL_OK;
}

/*
 *
 */
#include "tcl.h"

#if TARGET_API_MAC_CARBON
#   include <Tcl/tcl.h>
#else
#   include "tcl.h"
#endif

static int OzwVersionObjCmd(
    ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]
);

DLLEXPORT int Ozw_Init(Tcl_Interp *interp) {
    if (Tcl_InitStubs(interp, "8.1", 0) == NULL) {
        return TCL_ERROR;
    }
    Tcl_CreateObjCommand(interp, 
        "::ozw::version", OzwVersionObjCmd, (ClientData) NULL, NULL
    );

    extern int OzwOptions_Init(Tcl_Interp *interp);
    if (OzwOptions_Init(interp) != TCL_OK) {
        return TCL_ERROR;
    }
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

/*
 *
 */
#include "ozw.h"

#include <string>
using namespace std;

static int OzwManagerObjCmd(
    ClientData clientData, 
    Tcl_Interp *interp, 
    int objc, 
    Tcl_Obj *CONST objv[]
) {
    const char *subcommand = NULL;
    if (objc < 2) {
        Tcl_WrongNumArgs(interp, 1, objv, "create|destroy ?arg ...?");
        return TCL_ERROR;
    }
    subcommand = Tcl_GetString(objv[1]);
    if (subcommand == NULL) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj("error getting subcommand", -1));
        return TCL_ERROR;
    }

    if (!strcmp(subcommand, "create")) {
        if (objc != 2) {
            Tcl_WrongNumArgs(interp, 1, objv, "create");
            return TCL_ERROR;
        }

        if (OpenZWave::Options::Get() == NULL) { 
            Tcl_AppendResult(interp, "options element does not exist. it is required to create a manager", NULL);
            return TCL_ERROR;
        }

        if (OpenZWave::Options::Get()->AreLocked() == false) {
            Tcl_AppendResult(interp, "options element is not locked. it needs to be locked to create a manager", NULL);
            return TCL_ERROR;
        }

        if (OpenZWave::Manager::Get() != NULL) {
            Tcl_AppendResult(interp, "manager already exists", NULL);
            return TCL_ERROR;
        }
        if (OpenZWave::Manager::Create() == NULL) {
            Tcl_AppendResult(interp, "error creating manager", NULL);
            return TCL_ERROR;
        }
        return TCL_OK;

    } else if (!strcmp(subcommand, "destroy")) {
        if (OpenZWave::Manager::Get() == NULL) {
            Tcl_AppendResult(interp, "manager does not exist", NULL);
            return TCL_ERROR;
        }
        OpenZWave::Manager::Destroy();
        if (OpenZWave::Manager::Get() != NULL) {
            Tcl_AppendResult(interp, "error destroying manager", NULL);
            return TCL_ERROR;
        }
        return TCL_OK;

    } else {
        Tcl_AppendResult(interp, 
            "illegal manager subcommand \"", subcommand, "\"", NULL);
        return TCL_ERROR;
    }

    return TCL_OK;
}

int OzwManager_Init(Tcl_Interp *interp) {
    Tcl_CreateObjCommand(interp, 
        "::ozw::manager", OzwManagerObjCmd, (ClientData) NULL, NULL);
    return TCL_OK;
}
int OzwManager_SafeInit(Tcl_Interp *interp) { return OzwManager_Init(interp); }

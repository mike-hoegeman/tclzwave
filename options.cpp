/*
 *
 */
#include "ozw.h"

#include <string>
using namespace std;

#if 0
static int OzwOptionsInstObjCmd(
    ClientData clientData, 
    Tcl_Interp *interp, 
    int objc, 
    Tcl_Obj *CONST objv[]
) {
    Tcl_SetObjResult(interp, Tcl_NewStringObj("foo", -1));
    return TCL_OK;
}
#endif

static int created = 0;

static int OzwOptionsObjCmd(
    ClientData clientData, 
    Tcl_Interp *interp, 
    int objc, 
    Tcl_Obj *CONST objv[]
) {
    const char *subcommand = NULL;
#if 0
    char command_name[200]; 
    sprintf(command_name, "::ozw::inst::options%x", OzwOptionsObjCmd_SerialNo++); 
#endif

    if (objc < 2) {
        Tcl_WrongNumArgs(interp, 1, objv, "create|configure|lock ?arg ...?");
        return TCL_ERROR;
    }

    subcommand = Tcl_GetString(objv[1]);
    if (subcommand == NULL) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj("error getting subcommand", -1));
        return TCL_ERROR;
    }

    const char *configurationpath = "./";
    const char *userpath = "";
    const char *commandline = "";

    if (!strcmp(subcommand, "create")) {

        int idx = -1;


        if (created != 0) {
            Tcl_AppendResult(interp, "Options element already exists", NULL);
            return TCL_ERROR;
        } 

        for (idx = 2; idx < objc; ) {
            const char *opt = Tcl_GetString(objv[idx++]);
            std::string val;
            const char *v = NULL;
            if (idx < objc) {
                v = Tcl_GetString(objv[idx++]);
            } else {
                Tcl_AppendResult(interp, 
                    "missing value for option \"", opt, "\"", NULL);
                return TCL_ERROR;
            }
            if (opt == NULL || v == NULL) {
                Tcl_SetObjResult(interp, 
                    Tcl_NewStringObj("error getting create option value pair", -1));
                return TCL_ERROR;
            }

            /* 
             * process option value pair
             */
            if (!strcmp(opt, "-configurationpath")) {
                configurationpath = v; 
            }
            else if (!strcmp(opt, "-userpath")) {
                userpath = v;
            }
            else if (!strcmp(opt, "-commandline")) {
                commandline = v;
            }
            else {
                Tcl_AppendResult(interp, 
                    "illegal options create option \"" , opt, "\"", NULL);
                return TCL_ERROR;
            }
            created = 1;
        }

        OpenZWave::Options::Create( "../../../../config/", "", "" );


        Tcl_AppendResult(interp, "ok", NULL);
        return TCL_OK;

    } else {
        Tcl_AppendResult(interp, "illegal options subcommand \"", subcommand, "\"", NULL);
        return TCL_ERROR;
    }

#if 0
    Tcl_CreateObjCommand(
        interp, command_name, OzwOptionsInstObjCmd,
        (ClientData)NULL, NULL
    );
    Tcl_SetObjResult(interp, Tcl_NewStringObj(command_name, -1));
#endif
    return TCL_OK;

}

int OzwOptions_Init(Tcl_Interp *interp) {
    Tcl_CreateObjCommand(interp, 
        "::ozw::options", 
        OzwOptionsObjCmd, 
        (ClientData) NULL, 
        NULL);
    return TCL_OK;
}

int OzwOptions_SafeInit(Tcl_Interp *interp) {
    return OzwOptions_Init(interp);
}

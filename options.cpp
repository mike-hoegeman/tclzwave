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

static OpenZWave::Options *Ozw_OptionsSingleton = NULL;

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
        Tcl_WrongNumArgs(interp, 1, objv, "create|configure|lock|destroy ?arg ...?");
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

        if (Ozw_OptionsSingleton != NULL) {
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
        }

        Ozw_OptionsSingleton = OpenZWave::Options::Create(
            configurationpath, userpath, commandline
        );
        if (Ozw_OptionsSingleton == NULL) {
            Tcl_AppendResult(interp, "error creating Options object", NULL);
        }
        return TCL_OK;

    } else if (!strcmp(subcommand, "arelocked")) {

        if (objc != 2) {
            Tcl_WrongNumArgs(interp, 1, objv, "arelocked");
            return TCL_ERROR;
        }
        if (OpenZWave::Options::Get() == NULL) {
            Tcl_AppendResult(interp, 
                "Cannot find options element", NULL);
            return TCL_ERROR;
        }
        const char *result = OpenZWave::Options::Get()->AreLocked() ? "1" : "0";
        Tcl_SetResult(interp, (char *)result, NULL);
        return TCL_OK;

    } else if (!strcmp(subcommand, "lock")) {
        if (objc != 2) {
            Tcl_WrongNumArgs(interp, 1, objv, "lock");
            return TCL_ERROR;
        }
        if (OpenZWave::Options::Get() == NULL) {
            Tcl_AppendResult(interp, 
                "Cannot find options element", NULL);
            return TCL_ERROR;
        }
        bool ok = OpenZWave::Options::Get()->Lock();
        if (!ok) {
            Tcl_AppendResult(interp, 
                "Cannot lock options element", NULL);
            return TCL_ERROR;
        }
        return TCL_OK;

    } else if (!strcmp(subcommand, "destroy")) {
        if (objc != 2) {
            Tcl_WrongNumArgs(interp, 1, objv, "destroy");
            return TCL_ERROR;
        }
        /* shouldn't be done until associated manager Object is destroyed  */
        /* need a check in maybe to make sure manager is gone */
        if (OpenZWave::Options::Destroy() == true) {
            Ozw_OptionsSingleton = NULL;
            return TCL_OK;
        } else {
            /* not entirely obvious what to do if this happens */
            Ozw_OptionsSingleton = NULL;
            Tcl_AppendResult(interp, "error destroying Options object", NULL);
            return TCL_ERROR;
        }

    } else if (!strncmp(subcommand, "addoption", 9)) {

        int idx = 2;
        const char *nm = Tcl_GetString(objv[idx]);

        OpenZWave::Options *o = OpenZWave::Options::Get();
        if (o == NULL) {
            Tcl_AppendResult(interp, 
                "Cannot find options element", NULL);
            return TCL_ERROR;
        }

        if (o->AreLocked()) {
            Tcl_AppendResult(interp, "Cannot add to locked options", NULL);
            return TCL_ERROR;
        }

        bool add_res = false;
        int tcl_res = TCL_ERROR;

        /* */
        if (!strcmp(subcommand, "addoptionbool")) {
            if (objc != 4) {
                Tcl_WrongNumArgs(interp, 2, objv, "name defaultvalue");
                return TCL_ERROR;
            }
            int tcl_bool = -1;
            tcl_res = Tcl_GetBooleanFromObj(interp, objv[idx+1], &tcl_bool);
            if (tcl_res != TCL_OK) {
                return TCL_ERROR;
            }
            add_res = o->AddOptionBool(nm, tcl_bool != 0 ? true : false);
            if (add_res == false) {
                const char *def = Tcl_GetString(objv[idx+1]);
                Tcl_AppendResult(interp, 
                    "cannot add \"", nm, "\" (", def!=NULL ? def : "???", ")");
                return TCL_ERROR;
            }
            return TCL_OK;

        } else if (!strcmp(subcommand, "addoptionint")) {
            if (objc != 4) {
                Tcl_WrongNumArgs(interp, 2, objv, "name defaultvalue");
                return TCL_ERROR;
            }
            int int_arg = -1;
            tcl_res = Tcl_GetIntFromObj(interp, objv[idx+1], &int_arg);
            if (tcl_res != TCL_OK) {
                return TCL_ERROR;
            }
            add_res = o->AddOptionInt(nm, int_arg);
            if (add_res == false) {
                const char *def = Tcl_GetString(objv[idx+1]);
                Tcl_AppendResult(interp, 
                    "cannot add \"", nm, "\" (", def!=NULL ? def : "???", ")");
                return TCL_ERROR;
            }
            return TCL_OK;

        } else if (!strcmp(subcommand, "addoptionstring")) {
            if (objc != 5) {
                Tcl_WrongNumArgs(
                    interp, 2, objv, "name defaultvalue appendmode");
                return TCL_ERROR;
            }
            int appendmode = -1;
            tcl_res = Tcl_GetBooleanFromObj(interp, objv[4], &appendmode);
            if (tcl_res != TCL_OK) {
                Tcl_AppendResult(interp, " (for appendmode parameter)", NULL);
                return TCL_ERROR;
            }
            const char *val = Tcl_GetString(objv[3]);
            add_res = o->AddOptionString(
                nm, val, appendmode != 0 ? true : false);
            if (add_res == false) {
                const char *def = Tcl_GetString(objv[idx+1]);
                Tcl_AppendResult(interp, 
                    "cannot add \"", nm, "\" (", def!=NULL ? def : "???", ")");
                return TCL_ERROR;
            }
            return TCL_OK;

        } else {

            Tcl_ResetResult(interp);
            Tcl_AppendResult(interp, 
                "illegal options (add) subcommand \"", subcommand, "\"", NULL);
            return TCL_ERROR;
        }


    } else if (!strcmp(subcommand, "getoptiontype")) {
        if (objc != 3) {
            Tcl_WrongNumArgs(interp, 2, objv, "name");
            return TCL_ERROR;
        }
        const char *nm = Tcl_GetString(objv[2]);
        OpenZWave::Options *o = OpenZWave::Options::Get();
        if (o == NULL) {
            Tcl_AppendResult(interp, 
                "Cannot find options element", NULL); return TCL_ERROR;
        }
        OpenZWave::Options::OptionType ot = o->GetOptionType(nm);
        const char *otype_tcl = "";
        if (ot == OpenZWave::Options::OptionType_Bool) {
            otype_tcl = "bool";
        } else if (ot == OpenZWave::Options::OptionType_Int) {
            otype_tcl = "int";
        } else if (ot == OpenZWave::Options::OptionType_String) {
            otype_tcl = "string";
        } else if (ot == OpenZWave::Options::OptionType_Invalid) {
            Tcl_AppendResult(interp, 
                "Cannot find option \"", nm, "\"", NULL);
            return TCL_ERROR;
        } else {
            Tcl_AppendResult(interp, 
                "Uknown type for option \"", nm, "\"", NULL);
            return TCL_ERROR;
        }
        Tcl_AppendResult(interp, otype_tcl, NULL);
        return TCL_OK;

    } else if (!strcmp(subcommand, "getoptionasbool")) {
        if (objc != 3) {
            Tcl_WrongNumArgs(interp, 2, objv, "name"); return TCL_ERROR;
        }
        OpenZWave::Options *o = OpenZWave::Options::Get();
        if (o == NULL) {
            Tcl_AppendResult(interp, 
                "Cannot find options element", NULL); return TCL_ERROR;
        }
        const char *nm = Tcl_GetString(objv[2]); bool val;
        if (o->GetOptionAsBool(nm, &val) == false) {
            Tcl_AppendResult(interp, 
                "Cannot get bool option \"", nm, "\"", NULL);
            return TCL_ERROR;
        }
        if (o->GetOptionAsBool(nm, &val) == false) {
            Tcl_AppendResult(interp, 
                "Cannot get int option \"", nm, "\"", NULL);
            return TCL_ERROR;
        }
        Tcl_SetObjResult(interp, Tcl_NewIntObj(val == true ? 1 : 0));
        return TCL_OK;

    } else if (!strcmp(subcommand, "getoptionasint")) {
        if (objc != 3) {
            Tcl_WrongNumArgs(interp, 2, objv, "name"); return TCL_ERROR;
        }
        const char *nm = Tcl_GetString(objv[2]); int val = -1;
        OpenZWave::Options *o = OpenZWave::Options::Get();
        if (o == NULL) {
            Tcl_AppendResult(interp, 
                "Cannot find options element", NULL); return TCL_ERROR;
        }
        if (o->GetOptionAsInt(nm, &val) == false) {
            Tcl_AppendResult(interp, 
                "Cannot get int option \"", nm, "\"", NULL);
            return TCL_ERROR;
        }
        Tcl_SetObjResult(interp, Tcl_NewIntObj(val));
        return TCL_OK;

    } else if (!strcmp(subcommand, "getoptionasstring")) {
        if (objc != 3) {
            Tcl_WrongNumArgs(interp, 2, objv, "name"); return TCL_ERROR;
        }
        const char *nm = Tcl_GetString(objv[2]); string val = "";
        OpenZWave::Options *o = OpenZWave::Options::Get();
        if (o == NULL) {
            Tcl_AppendResult(interp, 
                "Cannot find options element", NULL); return TCL_ERROR;
        }
        if (o->GetOptionAsString(nm, &val) == false) {
            Tcl_AppendResult(interp, 
                "Cannot get string option \"", nm, "\"", NULL);
            return TCL_ERROR;
        }
        Tcl_SetObjResult(interp, Tcl_NewStringObj(val.c_str(), -1));
        return TCL_OK;

    } else {
        Tcl_AppendResult(interp, 
            "illegal options subcommand \"", subcommand, "\"", NULL);
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

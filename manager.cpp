/*
 *
 */
#include "ozw.h"

#include <string>
using namespace std;

static struct Ozw_WatcherContext _WatcherContext;

static int OzwManagerObjCmd(
    ClientData clientData, 
    Tcl_Interp *interp, 
    int objc, 
    Tcl_Obj *CONST objv[]
) {
    const char *subcommand = NULL;
    if (objc < 2) {
        Tcl_WrongNumArgs(interp, 1, objv, 
            "create|destroy|addwatcher ?arg ...?");
        return TCL_ERROR;
    }
    subcommand = Tcl_GetString(objv[1]);
    if (subcommand == NULL) {
        Tcl_SetObjResult(interp, 
            Tcl_NewStringObj("error getting subcommand", -1));
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
        if (objc != 2) {
            Tcl_WrongNumArgs(interp, 1, objv, "destroy");
            return TCL_ERROR;
        }
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

    } else if (!strcmp(subcommand, "addwatcher")) {
        if (_WatcherContext.initialized != 0) {
                Tcl_AppendResult(interp, 
                "there is already a watcher installed", NULL);
                return TCL_ERROR;
        }
        /* collect options */
        const char *command = NULL;
        for (int x = 2; x < objc;) {
            const char *opt = Tcl_GetString(objv[x++]);
            const char *val = NULL;
            if (x < objc) {
                val = Tcl_GetString(objv[x++]);
            } else {
                Tcl_AppendResult(interp, 
                    "missing value for option \"", opt, "\"", NULL);
                return TCL_ERROR;
            }
            if (opt == NULL || val == NULL) {
                Tcl_SetObjResult(interp, 
                 Tcl_NewStringObj("error getting addwatcher option value pair", 
                 -1));
                return TCL_ERROR;
            }
            if (!strcmp(opt, "-command")) {
                command = val;
            } else {
                Tcl_AppendResult(interp, 
                "illegal option \"", opt, "\"", NULL);
                return TCL_ERROR;
            }
        }
        /* process options */
        if (command == NULL) {
            Tcl_AppendResult(interp, 
                "missing -command option. -command {...} is a required option pair", NULL);
            return TCL_ERROR;
        }

        OpenZWave::Manager *m = OpenZWave::Manager::Get();
        if (m == NULL) {
            Tcl_AppendResult(interp, "Cannot find manager element", NULL);
            return TCL_ERROR;
        }
        Ozw_WatcherContextInit(&_WatcherContext, interp, command);
        m->AddWatcher(Ozw_Watcher, (void *)&_WatcherContext);

        return TCL_OK;

    } else if (!strcmp(subcommand, "removewatcher")) {
        OpenZWave::Manager *m = OpenZWave::Manager::Get();
        if (m == NULL) {
            Tcl_AppendResult(interp, "Cannot find manager element", NULL);
            return TCL_ERROR;
        }
        if (_WatcherContext.initialized == 0) {
            Tcl_AppendResult(interp, "no watcher currently installed", NULL);
            return TCL_ERROR;
        }
        Ozw_WatcherContextUnInit(&_WatcherContext);
        m->RemoveWatcher(Ozw_Watcher, (void *)&_WatcherContext);
        return TCL_OK;

    } else if (!strcmp(subcommand, "adddriver") || 
               !strcmp(subcommand, "removedriver")) {
        OpenZWave::Manager *m = OpenZWave::Manager::Get();
        if (m == NULL) {
            Tcl_AppendResult(interp, "Cannot find manager element", NULL);
            return TCL_ERROR;
        }
        const char *path = NULL;
        if (objc != 3) {
            Tcl_WrongNumArgs(interp, 1, objv, "adddriver /port/path|usb");
            return TCL_ERROR;
        }
        path = Tcl_GetString(objv[2]);
        if (path == NULL) {
            Tcl_SetObjResult(interp, 
                Tcl_NewStringObj("error getting path", -1));
        }
        bool result = false;
        if (!strcmp(subcommand, "adddriver")) {
            if (!strcmp(path, "usb")) {
                result = m->AddDriver(
                 "HID Controller", OpenZWave::Driver::ControllerInterface_Hid 
                );
            } else {
                result = m->AddDriver(path);
            }
        } else /* removedriver */ {
            if (!strcmp(path, "usb")) {
                result = m->RemoveDriver("HID Controller");
            } else {
                result = m->RemoveDriver(path);
            }
        }
        if (result == false) {
            Tcl_AppendResult(interp, 
                subcommand, "failed for \"", path, "\"", NULL);
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

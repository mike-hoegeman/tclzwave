/*
 *
 */
#include "ozw.h"
#include <string>
#include <sys/socket.h>
using namespace std;

static OzwManagerClientData The_OzwManagerClientData;

static int OzwManagerMakeNotificationChannel(
    Tcl_Interp *interp, OzwManagerClientData *mgrDataPtr
);

static int OzwManagerObjCmd(
    ClientData clientData, 
    Tcl_Interp *interp, 
    int objc, 
    Tcl_Obj *CONST objv[]
) {
    OzwManagerClientData *mgrDataPtr = (OzwManagerClientData *)clientData;

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
        if (Tcl_DStringLength
            (&The_OzwManagerClientData.notificationRecvCommand) != 0) {
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
        if (OzwManagerMakeNotificationChannel(interp, mgrDataPtr)!=TCL_OK) {
            Tcl_AppendResult(interp, 
                " ( while trying to make tcl notification channel )", 
                NULL);
            return TCL_ERROR;
        }
        m->AddWatcher(Ozw_Watcher, (void *)&The_OzwManagerClientData);
        return TCL_OK;

    } else if (!strcmp(subcommand, "removewatcher")) {
        OpenZWave::Manager *m = OpenZWave::Manager::Get();
        if (m == NULL) {
            Tcl_AppendResult(interp, "Cannot find manager element", NULL);
            return TCL_ERROR;
        }
        if (Tcl_DStringLength
            (&The_OzwManagerClientData.notificationRecvCommand) == 0) {
            Tcl_AppendResult(interp, "no watcher currently installed", NULL);
            return TCL_ERROR;
        }
        m->RemoveWatcher(Ozw_Watcher, (void *)&The_OzwManagerClientData);

        /* un-init The_OzwManagerClientData */
        {
            Tcl_DStringSetLength(
                &(The_OzwManagerClientData.notificationRecvCommand), 0);
            Tcl_DStringSetLength(
                &(The_OzwManagerClientData.notificationRecvChanName), 0);
            Tcl_Close((Tcl_Interp *)NULL, 
                The_OzwManagerClientData.notificationSendChannel);
            /*close(The_OzwManagerClientData.notificationSendSocket); */
            The_OzwManagerClientData.notificationSendSocket = -1;
        }

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
            Tcl_WrongNumArgs(interp, 2, objv, "/port/path|usb");
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

    } else if (!strcmp(subcommand, "writeconfig")) {
        OpenZWave::Manager *m = OpenZWave::Manager::Get();
        if (m == NULL) {
            Tcl_AppendResult(interp, "Cannot find manager element", NULL);
            return TCL_ERROR;
        }
        if (objc != 3) {
            Tcl_WrongNumArgs(interp, 2, objv, "homeid");
            return TCL_ERROR;
        }
        int homeid = 0; 
        int tcl_res = Tcl_GetIntFromObj(interp, objv[2], &homeid);
        if (tcl_res != TCL_OK) {
            Tcl_AppendResult(interp, " (for homeid parameter)", NULL);
            return TCL_ERROR;
        }
        m->WriteConfig(homeid);
        return TCL_OK;

    } else {
        Tcl_AppendResult(interp, 
            "illegal manager subcommand \"", subcommand, "\"", NULL);
        return TCL_ERROR;
    }

    return TCL_OK;
}

static int OzwManagerMakeNotificationChannel(
    Tcl_Interp *interp, 
    OzwManagerClientData *mgrDataPtr
) {
#define RETURN(x) {Tcl_DStringFree(&ds); return(x);}

#if 0
    /* 49152-65535 is the RFC sanctioned ephemeral port range 
     * we start 100 from the bottom just to avoid clashes 
     * with bottom feeders 
     */
    unsigned int start_e 49152+100;
    unsigned int end_e 65535;
    /*
     */
 #endif
    int start_e = 0;/* tells tcl have to have the os assign the port */
    int end_e = 0;
    int e;

    int eval_result = TCL_ERROR;
    Tcl_DString ds;
    Tcl_DStringInit(&ds);
    for (e = start_e; e <= end_e; e++) {
        char estr[200];
        snprintf(estr, 200, "%d", e);
        Tcl_DStringSetLength(&ds, 0);

        Tcl_DStringAppend(&ds, 
        "proc ::ozw::notificationserveraccept {sock addr port} { \
         fconfigure $sock -buffering line; \
         fileevent $sock readable [list ozw::nreader $sock]; \
         puts stderr ZZZZZZZZZZZZZZZZZZZZZZZZZZ; \
        }; ",
        -1);
        Tcl_DStringAppend(&ds, 
        "socket -server ::ozw::notificationserveraccept -myaddr 127.0.0.1 0", -1);
        Tcl_DStringAppend(&ds, estr, -1);
        eval_result = Tcl_EvalEx(interp, Tcl_DStringValue(&ds), -1, TCL_EVAL_GLOBAL|TCL_EVAL_DIRECT);
        Tcl_DStringGetResult(interp, &ds);
        if (eval_result == TCL_OK) {
            OpenZWave::Log::Write(OpenZWave::LogLevel_Info, "notification server '%s' completed", Tcl_DStringValue(&ds));

            Tcl_DStringSetLength(&(mgrDataPtr->notificationRecvChanName), 0);
            Tcl_DStringAppend(&(mgrDataPtr->notificationRecvChanName), Tcl_DStringValue(&ds), -1);
            Tcl_DStringSetLength(&ds, 0);
            Tcl_DStringAppend(&ds, "chan configure ", -1);
            Tcl_DStringAppend(&ds, Tcl_DStringValue( &(mgrDataPtr->notificationRecvChanName)), -1);
            Tcl_DStringAppend(&ds,  " -sockname ", -1);
            eval_result = Tcl_EvalEx(interp, Tcl_DStringValue(&ds), -1, TCL_EVAL_GLOBAL|TCL_EVAL_DIRECT);
            Tcl_DStringGetResult(interp, &ds);
            if (eval_result == TCL_ERROR) {
                return eval_result;
            }

            {
                int port_number;
                int split_c;
                int status;
                const char ** split_v;
                status = Tcl_SplitList(
                    interp, Tcl_DStringValue(&ds),
                    &split_c, &split_v
                );
                if (status != TCL_OK || split_c < 3) {
                    Tcl_DStringFree(&ds);
                    RETURN(TCL_ERROR);
                }
                port_number = atoi(split_v[2]);
                Tcl_Free((char *)split_v);
                if (port_number < 0) {
                    Tcl_AppendResult(interp, "bad port number in ", Tcl_DStringValue(&ds), NULL);
                    Tcl_DStringFree(&ds);
                    RETURN(TCL_ERROR);
                }
                mgrDataPtr->notificationSendPort = port_number;
            }
            break;

        } else {
            OpenZWave::Log::Write(OpenZWave::LogLevel_Info, 
             "notification server '%s' unsuccessful", Tcl_DStringValue(&ds));
        }
    }

    RETURN(eval_result);
#undef RETURN
}

int OzwManager_Init(Tcl_Interp *interp) {
    
    /* init The_OzwManagerClientData */
    {
        The_OzwManagerClientData.notificationSendSocket = -1;
        The_OzwManagerClientData.notificationSendPort = -1;
        Tcl_DStringInit(&(The_OzwManagerClientData.notificationRecvChanName));
        Tcl_DStringInit(&(The_OzwManagerClientData.notificationRecvCommand));
        The_OzwManagerClientData.notificationSendSocket = -1;
    }

    Tcl_CreateObjCommand(interp, 
        "::ozw::manager", OzwManagerObjCmd, 
        (ClientData) &The_OzwManagerClientData, NULL);
    return TCL_OK;
}
int OzwManager_SafeInit(Tcl_Interp *interp) { return OzwManager_Init(interp); }

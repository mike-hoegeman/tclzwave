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
    Ozw_NotificationClientData *ncd =
        (Ozw_NotificationClientData *) clientData;
    if (objc < 2) {
        Tcl_WrongNumArgs(interp, 1, objv, "configure|cget ?arg ...?");
        return TCL_ERROR;
    }

    const char *subcommand = Tcl_GetString(objv[1]);
    if (subcommand == NULL) {
        Tcl_SetObjResult(interp, 
            Tcl_NewStringObj("error getting subcommand", -1));
        return TCL_ERROR;
    }

    if (!strcmp(subcommand, "cget")) {
        const char *opt = Tcl_GetString(objv[2]);
        if (objc != 3) {
            Tcl_WrongNumArgs(interp, 2, objv, "-option");
            return TCL_ERROR;
        }
        if (!strcmp(opt, "-typecode")) {
            Tcl_SetObjResult(interp, 
                Tcl_NewIntObj((int)ncd->notificationPtr->GetType()));
            return TCL_OK;

        } else if (!strcmp(opt, "-typestring")) {
            char cbuf[100];
            int code = (int)ncd->notificationPtr->GetType();
            snprintf(cbuf, 100, "%d", code);
            const char *s = Tcl_GetVar2(interp, 
                "::ozw::notification::TypeStrings", cbuf, TCL_GLOBAL_ONLY);
            if (s == NULL) {
                return TCL_ERROR;
            }
            Tcl_SetObjResult(interp, Tcl_NewStringObj(s, -1));
            return TCL_OK;

        } else {
            Tcl_AppendResult(interp, 
                "unrecognized option \"", opt, "\"", NULL);
                return TCL_ERROR;
        }

    } else {
        Tcl_AppendResult(interp, 
            "unrecognized subcommand \"", subcommand, "\"", NULL);
        return TCL_ERROR;
    }

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

using namespace OpenZWave;

/* stilted, but i don't want to get into using maps or boost, etc.. */
/* just going to make a tcl Array with the mappings */
typedef struct ntpair { int t; const char *s; } ntpair;
static ntpair ntpairs[] = {
  { (int) Notification::Type_ValueAdded , "Type_ValueAdded" }, 
  { (int) Notification::Type_ValueRemoved , "Type_ValueRemoved" },
  { (int) Notification::Type_ValueChanged , "Type_ValueChanged" },
  { (int) Notification::Type_ValueRefreshed , "Type_ValueRefreshed" },
  { (int) Notification::Type_Group , "Type_Group" },
  { (int) Notification::Type_NodeNew , "Type_NodeNew" },
  { (int) Notification::Type_NodeAdded , "Type_NodeAdded" },
  { (int) Notification::Type_NodeRemoved , "Type_NodeRemoved" },
  { (int) Notification::Type_NodeProtocolInfo , "Type_NodeProtocolInfo" },
  { (int) Notification::Type_NodeNaming , "Type_NodeNaming" },
  { (int) Notification::Type_NodeEvent , "Type_NodeEvent" },
  { (int) Notification::Type_PollingDisabled , "Type_PollingDisabled" },
  { (int) Notification::Type_PollingEnabled , "Type_PollingEnabled" },
  { (int) Notification::Type_CreateButton , "Type_CreateButton" },
  { (int) Notification::Type_DeleteButton , "Type_DeleteButton" },
  { (int) Notification::Type_ButtonOn , "Type_ButtonOn" },
  { (int) Notification::Type_ButtonOff , "Type_ButtonOff" },
  { (int) Notification::Type_DriverReady , "Type_DriverReady" },
  { (int) Notification::Type_DriverFailed , "Type_DriverFailed" },
  { (int) Notification::Type_DriverReset , "Type_DriverReset" },
  { (int) Notification::Type_MsgComplete , "Type_MsgComplete" },
  { (int) Notification::Type_EssentialNodeQueriesComplete , "Type_EssentialNodeQueriesComplete" },
  { (int) Notification::Type_NodeQueriesComplete , "Type_NodeQueriesComplete" },
  { (int) Notification::Type_AwakeNodesQueried , "Type_AwakeNodesQueried" },
  { (int) Notification::Type_AllNodesQueried , "Type_AllNodesQueried" },
  { (int) Notification::Type_Error , "Type_Error" },
  { -1, NULL},
};

int
Ozw_NotificationInitArrays(Tcl_Interp *interp)
{
 int res;
 char tbuf[100]; 
 const char *result = NULL;
 res = Tcl_EvalEx(interp, 
     "namespace eval ::ozw {}; namespace eval ::ozw::notification {};", -1,
     TCL_EVAL_GLOBAL
 );
 if (res != TCL_OK) { return res; }
 for (int x = 0; (ntpairs[x]).s != NULL; x++) {
  snprintf(tbuf, 100, "%d", ntpairs[x].t);
  result = Tcl_SetVar2(interp, 
     "::ozw::notification::TypeStrings", tbuf, ntpairs[x].s, TCL_GLOBAL_ONLY); 
  if (result == NULL) { return TCL_ERROR; }
  result = Tcl_SetVar2(interp, 
      "::ozw::notification::TypeCodes", ntpairs[x].s, tbuf, TCL_GLOBAL_ONLY); 
  if (result == NULL) { return TCL_ERROR; }
 }
 return TCL_OK;
}

/*
 *
 */
#include "ozw.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include "errno.h"
using namespace std;
using namespace OpenZWave;

static long Ozw_NotificationInstNo = 0;

static const char * 
Ozw_NotificationTypeStrFromCode(int ntype, const char *ifunknown);

static void Ozw_NotificationInstDelProc(
    ClientData clientData
) {
    ckfree(clientData);
    return; 
}

#if 0
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
        Notification::NotificationType nt = ncd->notificationPtr->GetType();
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

        } else if (!strcmp(opt, "-homeid")) {
            Tcl_SetObjResult(interp, 
                Tcl_NewIntObj((int)ncd->notificationPtr->GetHomeId()));
            return TCL_OK;

        } else if (!strcmp(opt, "-nodeid")) {
            Tcl_SetObjResult(interp, 
                Tcl_NewIntObj((int)ncd->notificationPtr->GetNodeId()));
            return TCL_OK;

        } else if (!strcmp(opt, "-valueid")) {
            Tcl_AppendResult(interp, "TDB valueid not supported yet", NULL);
            return TCL_ERROR;

        } else if (!strcmp(opt, "-groupidx") && 
            (Notification::Type_Group == nt)
        ) {
            Tcl_SetObjResult(interp, 
                Tcl_NewIntObj((int)ncd->notificationPtr->GetGroupIdx()));
            return TCL_OK;

        } else if (!strcmp(opt, "-event") &&
            (ncd->notificationPtr->GetType() == Notification::Type_NodeEvent)) {
            Tcl_SetObjResult(interp, 
                Tcl_NewIntObj((int)ncd->notificationPtr->GetEvent()));
            return TCL_OK;

        } else if (!strcmp(opt, "-buttonid") &&
            (Notification::Type_CreateButton==nt || 
             Notification::Type_DeleteButton==nt || 
             Notification::Type_ButtonOn==nt || 
             Notification::Type_ButtonOff==nt)
d        ) { 
            Tcl_SetObjResult(interp, 
                Tcl_NewIntObj((int)ncd->notificationPtr->GetButtonId()));
            return TCL_OK;

        } else if (!strcmp(opt, "-errorcode") &&
            (Notification::Type_Error==nt)
        ) {
            Tcl_SetObjResult(interp, 
                Tcl_NewIntObj((int)ncd->notificationPtr->GetErrorCode()));
            return TCL_OK;

        } else if (!strcmp(opt, "-byte")) {
            Tcl_SetObjResult(interp, 
                Tcl_NewIntObj((int)ncd->notificationPtr->GetByte()));
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
#endif

static int
Ozw_NotificationBuildMsg(
    Tcl_DString *msgPtr,
    OpenZWave::Notification const* _notification
) {
    char buf[100];
    Notification::NotificationType nt =_notification->GetType();
    /* build the message */
    Tcl_DStringAppendElement(msgPtr, "notification");
    Tcl_DStringStartSublist(msgPtr);


    Tcl_DStringAppendElement(msgPtr, "type");
    Tcl_DStringAppendElement (msgPtr, 
        Ozw_NotificationTypeStrFromCode(_notification->GetType(), "???"));

    Tcl_DStringAppendElement(msgPtr, "nodeid");
    snprintf(buf, sizeof(buf), "%d", _notification->GetNodeId());
    Tcl_DStringAppendElement(msgPtr, buf);

    Tcl_DStringAppendElement(msgPtr, "homeid");
    snprintf(buf, sizeof(buf), "%d", _notification->GetHomeId());
    Tcl_DStringAppendElement(msgPtr, buf);

    /* valueid to be done */

    Tcl_DStringAppendElement(msgPtr, "valueid");
    snprintf(buf, sizeof(buf), "%llu", 
        _notification->GetValueID().GetId());
    Tcl_DStringAppendElement(msgPtr, buf);


    if (Notification::Type_Group == nt) {
        Tcl_DStringAppendElement(msgPtr, "groupidx");
        snprintf(buf, sizeof(buf), "%d", _notification->GetGroupIdx());
        Tcl_DStringAppendElement(msgPtr, buf);
    } 

    if (nt == Notification::Type_NodeEvent) {
        Tcl_DStringAppendElement(msgPtr, "event");
        snprintf(buf, sizeof(buf), "%d", _notification->GetEvent());
        Tcl_DStringAppendElement(msgPtr, buf);
    }

    if (Notification::Type_CreateButton==nt || 
        Notification::Type_DeleteButton==nt || 
        Notification::Type_ButtonOn==nt || 
        Notification::Type_ButtonOff==nt) {
        Tcl_DStringAppendElement(msgPtr, "buttonid");
        snprintf(buf, sizeof(buf), "%d", _notification->GetButtonId());
        Tcl_DStringAppendElement(msgPtr, buf);
    }

    if (Notification::Type_Error==nt) {
        Tcl_DStringAppendElement(msgPtr, "errorcode");
        snprintf(buf, sizeof(buf), "%d", _notification->GetErrorCode());
        Tcl_DStringAppendElement(msgPtr, buf);
    }

    if (1) {
        Tcl_DStringAppendElement(msgPtr, "byte");
        snprintf(buf, sizeof(buf), "%d", (int)_notification->GetByte());
        Tcl_DStringAppendElement(msgPtr, buf);
    }

    Tcl_DStringEndSublist(msgPtr);
    Tcl_DStringAppend(msgPtr, "\n", -1);
    return TCL_OK;
}

/* callback employed by ozw::manager addwatcher -command {xxx} */
/* which then in turn runs the tcl command {xxx} */
void Ozw_Watcher (
    OpenZWave::Notification const* _notification, 
    void* _context
) {

#define RETURN {Tcl_DStringFree(&msg); return;}

    OzwManagerClientData *mgrDataPtr = (OzwManagerClientData *) _context;
    Tcl_DString msg;
    Tcl_DStringInit(&msg);
    struct sockaddr_in serv_name;

    if (mgrDataPtr->notificationSendSocket == -1) {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            OpenZWave::Log::Write(OpenZWave::LogLevel_Fatal, "%s",
              "socket: failed to make send portion of notification channel");
            RETURN;
        };
        /* server address */ 
        serv_name.sin_family = AF_INET;
        inet_aton("127.0.0.1", &serv_name.sin_addr);
        serv_name.sin_port = htons(mgrDataPtr->notificationSendPort);
        /* connect to the server */
        if (connect(sock,(struct sockaddr*)&serv_name, sizeof(serv_name))==-1) {
            OpenZWave::Log::Write(OpenZWave::LogLevel_Fatal,
              "connect:%s:failed to make send portion of notification channel",
              strerror(errno));
            RETURN;
        }

        mgrDataPtr->notificationSendSocket = sock;
        mgrDataPtr->notificationSendChannel = 
            Tcl_MakeTcpClientChannel((void *)sock);

        OpenZWave::Log::Write(OpenZWave::LogLevel_Info, "%s",
            "creation of send side of notification channel complete");
#if 0
        if (Tcl_SetChannelOption((Tcl_Interp *)NULL, 
            mgrDataPtr->notificationSendChannel,
            optionName, 
            newValue) 
        != TCL_OK) {
            OpenZWave::Log::Write(OpenZWave::LogLevel_Fatal, "%s",
                "failed to make send (socket) portion of notification channel"
            RETURN;
        }
#endif
        
    }

    if (Ozw_NotificationBuildMsg( &msg, _notification) != TCL_OK) {
        OpenZWave::Log::Write(OpenZWave::LogLevel_Info, "%s",
            "creation of notification message failed");
        RETURN;
    }

    if (mgrDataPtr->notificationSendChannel != NULL) {
        int write_result = Tcl_Write(
            mgrDataPtr->notificationSendChannel, 
            Tcl_DStringValue(&msg),
            Tcl_DStringLength(&msg)
        );
        Tcl_Flush(mgrDataPtr->notificationSendChannel);
        OpenZWave::Log::Write(OpenZWave::LogLevel_Info,
            "------ WRITE: %s completed with code %d", 
                Tcl_DStringValue(&msg), write_result
        );
    }

    RETURN;
#undef RETURN
}

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

static const char * 
Ozw_NotificationTypeStrFromCode(int ntype, const char *ifunknown) {
    for(int x = 0; ntpairs[x].t != -1; x++) {
        if (ntpairs[x].t == ntype) {
            return ntpairs[x].s;
        }
    }
    return ifunknown;
}

int 
Ozw_NotificationInitArrays(Tcl_Interp *interp) {
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

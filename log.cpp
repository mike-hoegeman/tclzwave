/*
 *
 */
#include "ozw.h"
using namespace std;
using namespace OpenZWave;

int OzwLogLevelFromSymbol(
    Tcl_Interp *interp, 
    const char *symbol,
    enum OpenZWave::LogLevel *r)
{
#define EQ(A,B) !strcmp(A,B)
    int ok = TCL_OK;
    if (*symbol != 'L') {
        goto _err;
    }
    if (EQ(symbol, "LogLevel_Info")) {*r=LogLevel_Info; return ok;}
    if (EQ(symbol, "LogLevel_Error")) {*r=LogLevel_Error; return ok;}
    if (EQ(symbol, "LogLevel_Warning")) {*r=LogLevel_Warning; return ok;}
    if (EQ(symbol, "LogLevel_Alert")) {*r=LogLevel_Alert; return ok;}
    if (EQ(symbol, "LogLevel_Detail")) {*r=LogLevel_Detail; return ok;}
    if (EQ(symbol, "LogLevel_Debug")) {*r=LogLevel_Debug; return ok;}
    if (EQ(symbol, "LogLevel_Internal")) {*r=LogLevel_Internal; return ok;}
    if (EQ(symbol, "LogLevel_None")) {*r=LogLevel_None; return ok;}
    if (EQ(symbol, "LogLevel_Always")) {*r=LogLevel_Always; return ok;}
    if (EQ(symbol, "LogLevel_Fatal")) {*r=LogLevel_Fatal; return ok;}
_err:
    Tcl_AppendResult(interp, "\"", symbol,
        "\" is not a valid loglevel. should be one of: ",
        "LogLevel_None, LogLevel_Always, LogLevel_Fatal, LogLevel_Error, LogLevel_Warning, LogLevel_Alert, LogLevel_Info, LogLevel_Detail, LogLevel_Debug, LogLevel_Internal", 
        NULL); 
    return TCL_ERROR;
#undef EQ
}

static int OzwLogObjCmd(
    ClientData clientData, 
    Tcl_Interp *interp, 
    int objc, 
    Tcl_Obj *CONST objv[]
) {
    const char *subcommand = NULL;
    if (objc < 2) {
        Tcl_WrongNumArgs(interp, 1, objv, "subcommand ?arg ... arg?");
        return TCL_ERROR;
    }
    subcommand = Tcl_GetString(objv[1]);
    if (subcommand == NULL) {
        Tcl_SetObjResult(interp, 
            Tcl_NewStringObj("error getting subcommand", -1));
        return TCL_ERROR;
    }

    if (!strcmp(subcommand, "write")) {
        if (objc != 4) {
            Tcl_WrongNumArgs(interp, 2, objv, "LogLevel_Xxx messagetext");
            return TCL_ERROR;
        }
        const char *sym = Tcl_GetString(objv[2]);
        if (sym == NULL) {
            Tcl_AppendResult(interp, "error getting loglevel argument", NULL);
            return TCL_ERROR;
        }
        enum OpenZWave::LogLevel level;
        if (OzwLogLevelFromSymbol(interp, sym, &level) != TCL_OK) {
            return TCL_ERROR;
        }
        OpenZWave::Log::Write(level, "%s", Tcl_GetString(objv[3]));
        return TCL_OK;

    } else {
        Tcl_AppendResult(interp, 
            "illegal subcommand \"", subcommand, "\"", NULL);
        return TCL_ERROR;
    }

    return TCL_ERROR;
}

int OzwLog_Init(Tcl_Interp *interp) {
    Tcl_CreateObjCommand(interp, 
        "ozw::log", OzwLogObjCmd, (ClientData) NULL, NULL);
    return TCL_OK;
}
int OzwLog_SafeInit(Tcl_Interp *interp) {
    return OzwLog_Init(interp);
}

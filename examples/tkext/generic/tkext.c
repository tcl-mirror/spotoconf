/*
 * Copyright (c) 2019 Stuart Cassoff <stwo@users.sourceforge.net>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */


/*
 * Tk Extension
 */


#ifdef __cplusplus
extern "C" {
#endif


#include <tk.h>
#include <string.h>

typedef struct {
	Tk_Window tkwin;
	Display *display;
	Tcl_Interp *interp;
	Tcl_Command widgetCmd;
	Tk_OptionTable optionTable;
	XColor *background;
	int updatePending;
} Cow;

static const Tk_OptionSpec optionSpecs[] = {
	{TK_OPTION_COLOR, "-background", "background", "Background",
		"#d9d9d9", -1, offsetof(Cow, background),
		TK_OPTION_NULL_OK, "#101010", 0},
	{TK_OPTION_SYNONYM, "-bg", NULL, NULL, NULL, 0, -1, 0, "-background", 0},
	{TK_OPTION_END, NULL, NULL, NULL, NULL, 0, 0, 0, NULL, 0}
};

static void CowDeletedProc (ClientData clientData) {
	Cow *cowPtr = clientData;
	Tk_Window tkwin = cowPtr->tkwin;
	if (tkwin != NULL) {
		Tk_DestroyWindow(tkwin);
	}
}

static void CowDestroy (void *memPtr) {
	Cow *cowPtr = memPtr;
	ckfree(cowPtr);
}

static void CowDisplay (ClientData clientData) {
	Cow *cowPtr = clientData;
	Tk_Window tkwin = cowPtr->tkwin;

	cowPtr->updatePending = 0;
	if (!Tk_IsMapped(tkwin)) {
		return;
	}
}

static int CowConfigure (Tcl_Interp *interp, Cow *cowPtr) {
	Tk_SetWindowBackground(cowPtr->tkwin, cowPtr->background->pixel);
	if (!cowPtr->updatePending) {
		Tcl_DoWhenIdle(CowDisplay, cowPtr);
		cowPtr->updatePending = 1;
	}
	return TCL_OK;
}

static void CowObjEventProc (ClientData clientData, XEvent *eventPtr) {
	Cow *cowPtr = clientData;
	if (eventPtr->type == Expose) {
		if (!cowPtr->updatePending) {
			Tcl_DoWhenIdle(CowDisplay, cowPtr);
			cowPtr->updatePending = 1;
		}
	} else if (eventPtr->type == ConfigureNotify) {
		if (!cowPtr->updatePending) {
			Tcl_DoWhenIdle(CowDisplay, cowPtr);
			cowPtr->updatePending = 1;
		}
	} else if (eventPtr->type == DestroyNotify) {
		if (cowPtr->tkwin != NULL) {
			Tk_FreeConfigOptions((char *) cowPtr, cowPtr->optionTable, cowPtr->tkwin);
			cowPtr->tkwin = NULL;
			Tcl_DeleteCommandFromToken(cowPtr->interp, cowPtr->widgetCmd);
		}
		if (cowPtr->updatePending) {
			Tcl_CancelIdleCall(CowDisplay, cowPtr);
		}
		Tcl_EventuallyFree(cowPtr, (Tcl_FreeProc *) CowDestroy);
	}
}

static int CowWidgetCmd (ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]) {
	Cow *cowPtr = clientData;
	int result = TCL_OK;
	static const char *const cowOptions[] = {"cget", "configure", NULL};
	enum {
		COW_CGET, COW_CONFIGURE
	};
	Tcl_Obj *resultObjPtr;
	int index;

	if (objc < 2) {
		Tcl_WrongNumArgs(interp, 1, objv, "option ?arg arg...?");
		return TCL_ERROR;
	}

	if (Tcl_GetIndexFromObjStruct(interp, objv[1], cowOptions, sizeof(char *), "command", 0, &index) != TCL_OK) {
		return TCL_ERROR;
	}

	Tcl_Preserve(cowPtr);

	switch (index) {
	case COW_CGET:
		if (objc != 3) {
			Tcl_WrongNumArgs(interp, 2, objv, "option");
			goto error;
		}
		resultObjPtr = Tk_GetOptionValue(interp, (char *) cowPtr, cowPtr->optionTable, objv[2], cowPtr->tkwin);
		if (resultObjPtr == NULL) {
			result = TCL_ERROR;
		} else {
			Tcl_SetObjResult(interp, resultObjPtr);
		}
		break;
	case COW_CONFIGURE:
		resultObjPtr = NULL;
		if (objc == 2) {
			resultObjPtr = Tk_GetOptionInfo(interp, (char *) cowPtr, cowPtr->optionTable, NULL, cowPtr->tkwin);
			if (resultObjPtr == NULL) {
				result = TCL_ERROR;
			}
		} else if (objc == 3) {
			resultObjPtr = Tk_GetOptionInfo(interp, (char *) cowPtr, cowPtr->optionTable, objv[2], cowPtr->tkwin);
			if (resultObjPtr == NULL) {
				result = TCL_ERROR;
			}
		} else {
			result = Tk_SetOptions(interp, (char *) cowPtr, cowPtr->optionTable, objc - 2, objv + 2, cowPtr->tkwin, NULL, NULL);
			if (result == TCL_OK) {
				result = CowConfigure(interp, cowPtr);
			}
			if (!cowPtr->updatePending) {
				Tcl_DoWhenIdle(CowDisplay, cowPtr);
				cowPtr->updatePending = 1;
			}
		}
		if (resultObjPtr != NULL) {
			Tcl_SetObjResult(interp, resultObjPtr);
		}
	}

	Tcl_Release(cowPtr);

	return result;

  error:
	Tcl_Release(cowPtr);
	return TCL_ERROR;
}

static int Tkext_Cmd (ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]) {
	Cow *cowPtr;
	Tk_Window tkwin;
	Tk_OptionTable optionTable;

	if (objc < 2) {
		Tcl_WrongNumArgs(interp, 1, objv, "pathName ?-option value ...?");
		return TCL_ERROR;
	}

	if ((tkwin = Tk_CreateWindowFromPath(interp, Tk_MainWindow(interp), Tcl_GetString(objv[1]), NULL)) == NULL) { return TCL_ERROR; }

	Tk_SetClass(tkwin, "Cow");

	optionTable = Tk_CreateOptionTable(interp, optionSpecs);

	cowPtr = ckalloc(sizeof(Cow));
	memset(cowPtr, 0, sizeof(Cow));

	cowPtr->tkwin = tkwin;
	cowPtr->display = Tk_Display(tkwin);
	cowPtr->interp = interp;
	cowPtr->widgetCmd = Tcl_CreateObjCommand(interp, Tk_PathName(cowPtr->tkwin), CowWidgetCmd, cowPtr, CowDeletedProc);
	cowPtr->background = NULL;
	cowPtr->optionTable = optionTable;
	if (Tk_InitOptions(interp, (char *) cowPtr, optionTable, tkwin) != TCL_OK) {
		Tk_DestroyWindow(cowPtr->tkwin);
		ckfree(cowPtr);
		return TCL_ERROR;
	}
	Tk_CreateEventHandler(cowPtr->tkwin, ExposureMask|StructureNotifyMask, CowObjEventProc, cowPtr);
	if (Tk_SetOptions(interp, (char *) cowPtr, optionTable, objc - 2, objv + 2, tkwin, NULL, NULL) != TCL_OK) {
		goto error;
	}
	if (CowConfigure(interp, cowPtr) != TCL_OK) {
		goto error;
	}
	Tcl_SetObjResult(interp, Tcl_NewStringObj(Tk_PathName(cowPtr->tkwin), -1));
	return TCL_OK;

  error:
	Tk_DestroyWindow(cowPtr->tkwin);
	return TCL_ERROR;
}

EXTERN int
Tkext_Init (Tcl_Interp *interp) {
	if (Tcl_InitStubs       (interp,        TCL_VERSION, 0) == NULL) { return TCL_ERROR; }
	if (Tk_InitStubs        (interp,        TK_VERSION,  0) == NULL) { return TCL_ERROR; }
	if (Tcl_PkgRequire      (interp, "Tcl", TCL_VERSION, 0) == NULL) { return TCL_ERROR; }
	if (Tcl_PkgRequire      (interp, "Tk",  TK_VERSION,  0) == NULL) { return TCL_ERROR; }
	if (Tcl_CreateObjCommand(interp, "tkext", Tkext_Cmd, NULL, NULL) == NULL) { return TCL_ERROR; }
	if (Tcl_PkgProvide(interp, PACKAGE_NAME, PACKAGE_VERSION) != TCL_OK) { return TCL_ERROR; }
	return TCL_OK;
}


#ifdef __cplusplus
}
#endif


/* EOF */

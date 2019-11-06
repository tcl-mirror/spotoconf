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
 * Tcl or Jim Extension
 */


#ifdef __cplusplus
extern "C" {
#endif


#define EZT_NO_GETINT 1

#include "ezt_tj.h"

static
EZT_CMD(TJext_Cmd) {
	if (objc != 2) {
		Ezt_WrongNumArgs(interp, 1, objv, "string");
		return EZT_ERROR;
	}
	Ezt_SetResult(objv[1]);
	return EZT_OK;
}

#ifdef FOR_JIM
int
Jim_tjextInit (Jim_Interp *interp) {
	if (Jim_CreateCommand(interp, "tjext", TJext_Cmd, NULL, NULL) != JIM_OK) { return JIM_ERR; }
	if (Jim_PackageProvide(interp, PACKAGE_NAME, PACKAGE_VERSION, JIM_ERRMSG) != JIM_OK) { return JIM_ERR; }
	return JIM_OK;
}
#else
EXTERN int
Tjext_Init (Tcl_Interp *interp) {
	if (Tcl_InitStubs       (interp,        TCL_VERSION, 0) == NULL) { return TCL_ERROR; }
	if (Tcl_PkgRequire      (interp, "Tcl", TCL_VERSION, 0) == NULL) { return TCL_ERROR; }
	if (Tcl_CreateObjCommand(interp, "tjext", TJext_Cmd, NULL, NULL) == NULL) { return TCL_ERROR; }
	if (Tcl_PkgProvide(interp, PACKAGE_NAME, PACKAGE_VERSION) != TCL_OK) { return TCL_ERROR; }
	return TCL_OK;
}
#endif

#ifdef __cplusplus
}
#endif


/* EOF */

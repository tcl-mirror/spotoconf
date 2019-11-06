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
 * Tcl Extension (C)
 */


#ifdef __cplusplus
extern "C" {
#endif


#include <tcl.h>

static int Tclext_c_Cmd (ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]) {
	if (objc != 2) {
		Tcl_WrongNumArgs(interp, 1, objv, "string");
		return TCL_ERROR;
	}
	Tcl_SetObjResult(interp, objv[1]);
	return TCL_OK;
}

EXTERN int
Tclext_c_Init (Tcl_Interp *interp) {
	if (Tcl_InitStubs       (interp,        TCL_VERSION, 0) == NULL) { return TCL_ERROR; }
	if (Tcl_PkgRequire      (interp, "Tcl", TCL_VERSION, 0) == NULL) { return TCL_ERROR; }
	if (Tcl_CreateObjCommand(interp, "tclext_c", Tclext_c_Cmd, NULL, NULL) == NULL) { return TCL_ERROR; }
	if (Tcl_PkgProvide(interp, PACKAGE_NAME, PACKAGE_VERSION) != TCL_OK) { return TCL_ERROR; }
	return TCL_OK;
}


#ifdef __cplusplus
}
#endif


/* EOF */

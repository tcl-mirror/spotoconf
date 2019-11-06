
#undef BUILD_tcl
#undef STATIC_BUILD

#include <tcl.h>

extern Tcl_PackageInitProc PACKAGE_INIT;

int Tcl_AppInit (Tcl_Interp *interp) {
	if ((Tcl_Init)   (interp) == TCL_ERROR) { return TCL_ERROR; }
	if (PACKAGE_INIT (interp) == TCL_ERROR) { return TCL_ERROR; }
	Tcl_StaticPackage(interp, PACKAGE_NAME, PACKAGE_INIT, PACKAGE_INIT);

//puts(Tcl_GetString(Tcl_ObjPrintf("package ifneeded %s %s {load {} %s}", PACKAGE_NAME, PACKAGE_VERSION, INIT)));

	return TCL_OK;
}

int main (int argc, char *argv[]) {
	Tcl_Main(argc, argv, Tcl_AppInit);
	return 0;
}

/* EOF */

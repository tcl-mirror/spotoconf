# Tcl Module extension


namespace eval ::tmext {

proc statement {} { return "This is tmext" }

namespace ensemble create

}

package provide tmext 0.1


# EOF

# Tcl extension (Tcl Module)


namespace eval ::tmext { namespace eval string {

proc upper {string} { return [string toupper $string] }

proc lower {string} { return [string tolower $string] }

namespace export upper lower

namespace ensemble create

}}

package provide tmext_string 0.1


# EOF

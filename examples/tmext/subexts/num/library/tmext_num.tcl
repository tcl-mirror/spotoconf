# Tcl extension (Tcl Module)


namespace eval ::tmext { namespace eval num {

proc plusone {num} { return [incr num] }

proc minusone {num} { return [incr num -1] }

namespace export plusone minusone

namespace ensemble create

}}

package provide tmext::num 0.1


# EOF

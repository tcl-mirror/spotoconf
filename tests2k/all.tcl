# Run this to run all the tests
package prefer latest
package require Tcl 8.6
package require tcltest 2000-
tcltest::configure -testdir [file dirname [info script]] -tmpdir [pwd] {*}$argv
tcltest::runAllTests

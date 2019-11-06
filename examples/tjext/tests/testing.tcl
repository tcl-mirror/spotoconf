#
# Copyright 2005 Salvatore Sanfilippo <antirez@invece.org>
# Copyright 2005 Clemens Hintze <c.hintze@gmx.net>
# Copyright 2005 patthoyts - Pat Thoyts <patthoyts@users.sf.net>
# Copyright 2008 oharboe - Øyvind Harboe - oyvind.harboe@zylin.com
# Copyright 2008 Andrew Lunn <andrew@lunn.ch>
# Copyright 2008 Duane Ellis <openocd@duaneellis.com>
# Copyright 2008 Uwe Klein <uklein@klein-messgeraete.de>
# Copyright 2008 Steve Bennett <steveb@workware.net.au>
# Copyright 2009 Nico Coesel <ncoesel@dealogic.nl>
# Copyright 2009 Zachary T Welch zw@superlucidity.net
# Copyright 2009 David Brownell
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above
#    copyright notice, this list of conditions and the following
#    disclaimer in the documentation and/or other materials
#    provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE JIM TCL PROJECT ``AS IS'' AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
# PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# JIM TCL PROJECT OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
# INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
# STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# The views and conclusions contained in the software and documentation
# are those of the authors and should not be interpreted as representing
# official policies, either expressed or implied, of the Jim Tcl Project.
#

# Common code
set testinfo(verbose) 0
set testinfo(numpass) 0
set testinfo(stoponerror) 0
set testinfo(numfail) 0
set testinfo(numskip) 0
set testinfo(numtests) 0
set testinfo(failed) {}

set testdir [file dirname [info script]]
set bindir [file dirname [info nameofexecutable]]

if {[lsearch $argv "-verbose"] >= 0 || [info exists env(testverbose)]} {
	incr testinfo(verbose)
}
if {[lsearch $argv "-stoponerror"] >= 0 || [info exists env(stoponerror)]} {
	incr testinfo(stoponerror)
}

proc needs {type what {packages {}}} {
	if {$type eq "constraint"} {
		if {![info exists ::tcltest::testConstraints($what)]} {
			set ::tcltest::testConstraints($what) 0
		}
		if {![set ::tcltest::testConstraints($what)]} {
			skiptest " (constraint $what)"
		}
		return
	}
	if {$type eq "cmd"} {
		# Does it exist already?
		if {[info commands $what] ne ""} {
			return
		}
		if {$packages eq ""} {
			# e.g. exec command is in exec package
			set packages $what
		}
		foreach p $packages {
			catch {package require $p}
		}
		if {[info commands $what] ne ""} {
			return
		}
		skiptest " (command $what)"
	}
	error "Unknown needs type: $type"
}

proc skiptest {{msg {}}} {
	puts [format "%16s:   --- skipped$msg" $::argv0]
	exit 0
}

# If tcl, just use tcltest
if {[catch {info version}]} {
	package require Tcl 8.5
	package require tcltest 2.1
	namespace import tcltest::*

	if {$testinfo(verbose)} {
		configure -verbose bps
	}
	testConstraint utf8 1
	testConstraint tcl 1
	proc testreport {} {
		::tcltest::cleanupTests
	}
	return
}

lappend auto_path $testdir $bindir [file dirname [pwd]]

# For Jim, this is reasonable compatible tcltest
proc makeFile {contents name} {
	set f [open $name w]
	stdout puts "About to 'puts $f $contents'"
	puts $f $contents
	close $f
	return $name
}

proc removeFile {name} {
	file delete $name
}

# In case tclcompat is not selected
if {![exists -proc puts]} {
	proc puts {{-nonewline {}} {chan stdout} msg} {
		if {${-nonewline} ni {-nonewline {}}} {
			${-nonewline} puts $msg
		} else {
			$chan puts {*}${-nonewline} $msg
		}
	}
	proc close {chan args} {
		$chan close {*}$args
	}
	proc fileevent {args} {
		{*}$args
	}
}

proc script_source {script} {
	lassign [info source $script] f l
	if {$f ne ""} {
		puts "At      : $f:$l"
		return \t$f:$l
	}
}

proc error_source {} {
	lassign [info stacktrace] p f l
	if {$f ne ""} {
		puts "At      : $f:$l"
		return \t$f:$l
	}
}

proc package-or-skip {name} {
	if {[catch {
		package require $name
	}]} {
		puts [format "%16s:   --- skipped" $::argv0]
		exit 0
	}
}

proc testConstraint {constraint bool} {
	set ::tcltest::testConstraints($constraint) $bool
}

testConstraint {utf8} [expr {[string length "\xc2\xb5"] == 1}]
testConstraint {references} [expr {[info commands ref] ne ""}]
testConstraint {jim} 1
testConstraint {tcl} 0

proc bytestring {x} {
	return $x
}

# Note: We don't support -output or -errorOutput yet
proc test {id descr args} {
	set a [dict create -returnCodes {ok return} -match exact -result {} -constraints {} -body {} -setup {} -cleanup {}]
	if {[lindex $args 0] ni [dict keys $a]} {
		if {[llength $args] == 2} {
			lassign $args body result constraints
		} elseif {[llength $args] == 3} {
			lassign $args constraints body result
		} else {
			return -code error "$id: Wrong syntax for tcltest::test v1"
		}
		tailcall test $id $descr -body $body -result $result -constraints $constraints
	}
	# tcltest::test v2 syntax
	array set a $args

	incr ::testinfo(numtests)
	if {$::testinfo(verbose)} {
		puts -nonewline "$id "
	}

	foreach c $a(-constraints) {
		if {[info exists ::tcltest::testConstraints($c)]} {
			if {$::tcltest::testConstraints($c)} {
				continue
			}
			incr ::testinfo(numskip)
			if {$::testinfo(verbose)} {
				puts "SKIP"
			}
			return
		}
	}

	catch {uplevel 1 $a(-setup)}
	set rc [catch {uplevel 1 $a(-body)} result opts]
	catch {uplevel 1 $a(-cleanup)}

	if {[info return $rc] ni $a(-returnCodes) && $rc ni $a(-returnCodes)} {
		set ok 0
		set expected "rc=$a(-returnCodes) result=$a(-result)"
		set result "rc=[info return $rc] result=$result"
	} else {
		if {$a(-match) eq "exact"} {
			set ok [string equal $a(-result) $result]
		} elseif {$a(-match) eq "glob"} {
			set ok [string match $a(-result) $result]
		} elseif {$a(-match) eq "regexp"} {
			set ok [regexp $a(-result) $result]
		} else {
			return -code error "$id: unknown match type: $a(-match)"
		}
		set expected $a(-result)
	}

	if {$ok} {
		if {$::testinfo(verbose)} {
			puts "OK  $descr"
		}
		incr ::testinfo(numpass)
		return
	}

	if {!$::testinfo(verbose)} {
		puts -nonewline "$id "
	}
	puts "ERR $descr"
	if {$rc in {0 2}} {
		set source [script_source $a(-body)]
	} else {
		set source [error_source]
	}
	puts "Expected: '$expected'"
	puts "Got     : '$result'"
	puts ""
	incr ::testinfo(numfail)
	lappend ::testinfo(failed) [list $id $descr $source $expected $result]
	if {$::testinfo(stoponerror)} {
		exit 1
	}
}

proc ::tcltest::cleanupTests {} {
	tailcall testreport
}

proc testreport {} {
	if {$::testinfo(verbose)} {
		puts -nonewline "\n$::argv0"
	} else {
		puts -nonewline [format "%16s" $::argv0]
	}
	puts [format ": Total %5d   Passed %5d  Skipped %5d  Failed %5d" \
		$::testinfo(numtests) $::testinfo(numpass) $::testinfo(numskip) $::testinfo(numfail)]
	if {$::testinfo(numfail)} {
		puts [string repeat - 60]
		puts "FAILED: $::testinfo(numfail)"
		foreach failed $::testinfo(failed) {
			foreach {id descr source expected result} $failed {}
			puts "$source\t$id"
		}
		puts [string repeat - 60]
	}
	if {$::testinfo(numfail)} {
		exit 1
	}
}

proc testerror {} {
	error "deliberate error"
}

if {$testinfo(verbose)} {
	puts "==== $argv0 ===="
}

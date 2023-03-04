# Copyright 2023 Oğuz İsmail Uysal <oguzismailuysal@gmail.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <https://www.gnu.org/licenses/>.

# {{{
escape_for_printf() {
	eval "tmp=\$$1"
	
	case $tmp in
	-* )
		tmp="\055${tmp#-}"
	esac

	case $tmp in
	*%* )
		tmp=$(printf '%sx\n' "$tmp" | sed 's/%/%%/g')
		tmp=${tmp%x}
	esac

	eval "$1=\$tmp"
}

build_expected_result() {
	escape_for_printf expected_output
	printf "${expected_output}x%d\n" "$expected_status"
}

run_scenario() {
	escape_for_printf input
	printf "$input" | eval "$environment $binary $arguments"
	printf 'x%s\n' $?
}

run_test() {
	printf 'testing if %s is handled correctly... ' "$*"

	expected_result=$(build_expected_result | od)
	result=$(run_scenario | od)

	if test "$result" = "$expected_result"; then
		echo yes
	else
		echo no
		exit 1
	fi
}
# }}}

binary=./va
environment=

input=
arguments=x
expected_output=
expected_status=1
run_test an extra operand 2>/dev/null

input='x\n'
arguments=
expected_output=$input
expected_status=0
run_test an ordinary line

input='x'
arguments=
expected_output=$input
expected_status=0
run_test end of file as end of line

input='x\\\\\n'
arguments=
expected_output=$input
expected_status=0
run_test an escaped trailing backslash

input='x\\\n\t \t\\\n'
arguments=
expected_output='x \\\n  \\\n'
expected_status=0
run_test a continued line consisting of spaces and tabs only

input='x\\\n\tx\\\n'
arguments=
expected_output='x       \t\\\n\tx       \\\n'
expected_status=0
run_test a pair of continued lines with different levels

input='xxxxxxxxx\\\n\tx\\\n'
arguments=
expected_output='xxxxxxxxx       \t\\\n\tx               \\\n'
expected_status=0
run_test a pair of continued lines with different levels and widths

input='x\\\n'
arguments=
expected_output=$input
expected_status=0
run_test a single continued line

input='x\\\nxx\\\n'
arguments=
expected_output='x  \\\nxx \\\n'
expected_status=0
run_test a couple of continued lines with the same level

input='\txxxxxxxx\\\nx\\\n'
arguments=
expected_output='\txxxxxxxx \\\nx       \t \\\n'
expected_status=0
run_test a continued line with greater level and width than the rest

# vim: fdm=marker

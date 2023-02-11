run_test() {
	printf 'testing if %s is handled correctly... ' "$*"

	expected_result=$(get_expected_result | od)
	result=$(run_scenario | od)

	if test "$result" = "$expected_result"; then
		echo yes
	else
		echo no
		exit 1
	fi
}

get_expected_result() {
	escape_for_printf expected_output
	printf "${expected_output}x%d\n" "$expected_status"
}

run_scenario() {
	escape_for_printf input
	printf "$input" | eval "$environment ./va $arguments"
	printf 'x%d\n' $?
}

escape_for_printf() {
	eval "case \$$1 in -*)
		$1=\"\\055\${$1#-}\"
	esac

	case \$$1 in *%*)
		$1=\$(printf '%sx\n' \"\$$1\" | sed 's/%/%%/g')
		$1=\${$1%x}
	esac"
}

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
run_test a normal line

input='x\\\n\tx\\\n'
arguments=
expected_output='x       \t\\\n\tx       \\\n'
expected_status=0
run_test a couple of continued lines with different levels

input='xxxxxxxxx\\\n\tx\\\n'
arguments=
expected_output='xxxxxxxxx       \t\\\n\tx               \\\n'
expected_status=0
run_test a couple of continued lines with different levels and different widths

input='x\\\n'
arguments=
expected_output='x\\\n'
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

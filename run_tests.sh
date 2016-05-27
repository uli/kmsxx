#!/bin/sh
pass=0
fail=0
skip=0

for i in tests/*.py; do
	echo -en "${i#*/}  "
	PYTHONPATH=build/py/:py/:tests/lib/ python3 $i $1
	ret=$?
	if test $ret == 2 ; then
		echo SKIP
		skip=$((skip + 1))
	elif test $ret == 0 ; then
		echo PASS
		pass=$((pass + 1))
	else
		echo FAIL
		fail=$((fail + 1))
	fi
done

echo -e "\n$pass passed, $fail failed, $skip skipped"
test $fail -gt 0 && exit 1

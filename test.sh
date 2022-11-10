#!/bin/bash
assert() {
	expected="$1"
	input="$2"

	./ucc "$input" > tmp.s
	cc -o tmp tmp.s
	./tmp
	actual="$?"

	if [ "$actual" = "$expected" ]; then
		echo "$input => $actual"
	else
		echo "$input => $expected expected, but got $actual"
		exit 1
	fi
}

# num
assert 0 0
assert 42 42
# add/sub
assert 21 "5+20-4"
# space
assert 41 "12 + 34 - 5"
assert 30 "12 + 34 - 5 + 67      -78"
# mul/div/parenthesis
assert 47 "5+6*7"
assert 15 "5*(9-6)"
assert 4 "(3+5)/2"
# unary
assert 42 "+42"
assert 255 "-1"
assert 10 "+20-10"
# unary(is it ok?)
assert 30 "+20--10"
# eq
assert 1 "42==42"
assert 0 "42==41"
assert 0 "41==42"
# neq
assert 0 "42!=42"
assert 1 "42!=41"
assert 1 "41!=42"
# lt
assert 0 "42<42"
assert 0 "42<41"
assert 1 "41<42"
# lte
assert 1 "42<=42"
assert 0 "42<=41"
assert 1 "41<=42"
# gt
assert 0 "42>42"
assert 1 "42>41"
assert 0 "41>42"
# gte
assert 1 "42>=42"
assert 1 "42>=41"
assert 0 "41>=42"

echo OK

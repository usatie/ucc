#!/bin/bash
cat <<EOF | gcc -xc -c -o tmp2.o -
int ret_ft() { return 42; }
int ret_three() { return 3; }
int	add(int a, int b) { return a + b; }
int	mul(int a, int b) { return a * b; }
int	add6(int a, int b, int c, int d, int e, int f) { return a + b + c + d + e + f; }
EOF
assert() {
	expected="$1"
	input="$2"

	./ucc "$input" > tmp.s
	cc -o tmp tmp.s tmp2.o
	./tmp
	actual="$?"

	if [ "$actual" = "$expected" ]; then
		echo "$input => $actual"
	else
		echo "$input => $expected expected, but got $actual"
		exit 1
	fi
}

# function call
assert 8 "{ return add(3, 5); }"
assert 42 "{ return add(20, 22); }"
assert 15 "{ return mul(3, 5); }"
assert 42 "{ return mul(6, 7); }"
assert 21 "{ return add6(1,2,3,4,5,6); }"

# zero-arty function call
assert 42 "{ return ret_ft(); }"
assert 3 "{ return ret_three(); }"

# block
assert 42 "{ a = 1; b = 1; c = 40; return a + b + c; }"
assert 84 "j = 0; for (i = 0; i < 42; i = i + 1) { j = j + 2; } return j;"
assert 3 "{ {1; {2; } return 3; } }"

# null stmt
assert 42 "; ; ;; ;; ; return 42; ; ;"

# for
assert 21 "for (i = 0; i < 21; i = i+1) 1; return i;"
assert 42 "j = 0; for (i = 0; i < 21; i = i+1) j = j + 2; return j;"
assert 3 "for (;;) return 3; 1;"

# while
assert 42 "i = 0; while (i < 42) i = i + 1; return i;"

# if
assert 42 "if (1) return 42; return 1;"
assert 42 "if (0) return 1; return 42;"
assert 42 "if (0) return 1; if (0) return 2; if (1) return 42;"

# if else
assert 42 "if (0) return 1; else return 42; return 0;"
assert 42 "if (1) return 42; else return 1; return 0;"

# num
assert 0 "0;"
assert 42 "42;"
# add/sub
assert 21 "5+20-4;"
# space
assert 41 "12 + 34 - 5;"
assert 30 "12 + 34 - 5 + 67      -78;"
# mul/div/parenthesis
assert 47 "5+6*7;"
assert 15 "5*(9-6);"
assert 4 "(3+5)/2;"
# unary
assert 42 "+42;"
assert 255 "-1;"
assert 10 "+20-10;"
# unary(is it ok?)
assert 30 "+20--10;"
# eq
assert 1 "42==42;"
assert 0 "42==41;"
assert 0 "41==42;"
# neq
assert 0 "42!=42;"
assert 1 "42!=41;"
assert 1 "41!=42;"
# lt
assert 0 "42<42;"
assert 0 "42<41;"
assert 1 "41<42;"
# lte
assert 1 "42<=42;"
assert 0 "42<=41;"
assert 1 "41<=42;"
# gt
assert 0 "42>42;"
assert 1 "42>41;"
assert 0 "41>42;"
# gte
assert 1 "42>=42;"
assert 1 "42>=41;"
assert 0 "41>=42;"
# multiple statements
assert 3 "1;2;3;"
# one-letter local variables assignment
assert 3 "a=1;b=2;a+b;"
assert 5 "a=1;b=2;a=a+b;b=a+b;"
# multi-letter local variables assignment
assert 6 "foo = 1; bar = 2 + 3;foo + bar;"
assert 12 "foo = 1; bar = 2 + 3; foobar = foo + bar; foo + bar + foobar;"
# return
assert 42 "return 42;"
assert 1 "return 1;return 2; return 3;"

echo OK

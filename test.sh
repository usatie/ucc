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

# function declare with args
assert 6 "main() { add3(1, 2, 3); } add3(a, b, c) { return a + b + c; }"
assert 55 "main() { fib(10); } fib(n) { if (n <= 1) return n; else return fib(n-1) + fib(n-2); }"
assert 3 "main() { sub(10, 7); } sub(a, b) { return (a - b); }"
assert 0 "main() { isprime(10); } isprime(n) { i = 2; while (n >= i * i) { if (_mod(n, i) == 0) return 0; i = i + 1; } return 1; } _mod(n, m) { return (n - (n / m) * m); }"
assert 1 "main() { isprime(11); } isprime(n) { i = 2; while (n >= i * i) { if (_mod(n, i) == 0) return 0; i = i + 1; } return 1; } _mod(n, m) { return (n - (n / m) * m); }"
assert 251 "main() { maxprime(255); } maxprime(n) { max = 0; i = 2; while (i <= n) { if (isprime(i)) max = i; i = i + 1; } return max;} isprime(n) { i = 2; while (n >= i * i) { if (_mod(n, i) == 0) return 0; i = i + 1; } return 1; } _mod(n, m) { return (n - (n / m) * m); }"

# function declare
assert 42 "main() { return 42; }"
assert 42 "main() { ft(); } ft() { return 42; }"

# function call
assert 8 "main() { return add(3, 5); }"
assert 42 "main() { return add(20, 22); }"
assert 15 "main() { return mul(3, 5); }"
assert 42 "main() { return mul(6, 7); }"
assert 21 "main() { return add6(1,2,3,4,5,6); }"

# zero-arty function call
assert 42 "main() { return ret_ft(); }"
assert 3 "main() { return ret_three(); }"

# block
assert 42 "main() { a = 1; b = 1; c = 40; return a + b + c; }"
assert 84 "main() { j = 0; for (i = 0; i < 42; i = i + 1) { j = j + 2; } return j; }"
assert 3 "main() { {1; {2; } return 3; } }"

# null stmt
assert 42 "main() {; ; ;; ;; ; return 42; ; ;}"

# for
assert 21 "main() {for (i = 0; i < 21; i = i+1) 1; return i;}"
assert 42 "main() {j = 0; for (i = 0; i < 21; i = i+1) j = j + 2; return j;}"
assert 3 "main() {for (;;) return 3; 1;}"

# while
assert 42 "main() {i = 0; while (i < 42) i = i + 1; return i;}"

# if
assert 42 "main() {if (1) return 42; return 1;}"
assert 42 "main() {if (0) return 1; return 42;}"
assert 42 "main() {if (0) return 1; if (0) return 2; if (1) return 42;}"

# if else
assert 42 "main() {if (0) return 1; else return 42; return 0;}"
assert 42 "main() {if (1) return 42; else return 1; return 0;}"

# num
assert 0 "main() {0;}"
assert 42 "main() {42;}"
# add/sub
assert 21 "main() {5+20-4;}"
# space
assert 41 "main() {12 + 34 - 5;}"
assert 30 "main() {12 + 34 - 5 + 67      -78;}"
# mul/div/parenthesis
assert 47 "main() {5+6*7;}"
assert 15 "main() {5*(9-6);}"
assert 4 "main() {(3+5)/2;}"
# unary
assert 42 "main() {+42;}"
assert 255 "main() {-1;}"
assert 10 "main() {+20-10;}"
# unary(is it ok?)
assert 30 "main() {+20--10;}"
# eq
assert 1 "main() {42==42;}"
assert 0 "main() {42==41;}"
assert 0 "main() {41==42;}"
# neq
assert 0 "main() {42!=42;}"
assert 1 "main() {42!=41;}"
assert 1 "main() {41!=42;}"
# lt
assert 0 "main() {42<42;}"
assert 0 "main() {42<41;}"
assert 1 "main() {41<42;}"
# lte
assert 1 "main() {42<=42;}"
assert 0 "main() {42<=41;}"
assert 1 "main() {41<=42;}"
# gt
assert 0 "main() {42>42;}"
assert 1 "main() {42>41;}"
assert 0 "main() {41>42;}"
# gte
assert 1 "main() {42>=42;}"
assert 1 "main() {42>=41;}"
assert 0 "main() {41>=42;}"
# multiple statements
assert 3 "main() {1;2;3;}"
# one-letter local variables assignment
assert 3 "main() {a=1;b=2;a+b;}"
assert 5 "main() {a=1;b=2;a=a+b;b=a+b;}"
# multi-letter local variables assignment
assert 6 "main() {foo = 1; bar = 2 + 3;foo + bar;}"
assert 12 "main() {foo = 1; bar = 2 + 3; foobar = foo + bar; foo + bar + foobar;}"
# return
assert 42 "main() {return 42;}"
assert 1 "main() {return 1;return 2; return 3;}"

echo OK

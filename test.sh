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

# unary * and &
assert 42  "int main() { a = 42; return *(&a); }"
assert 42  "int main() { a = 42; b = &a; return *b; }"
assert 21  "int main() { a = 42; b = 21; return *(&b); }"
assert 42  "int main() { a = 42; b = 21; return *(&b + 8); }"

# function declare with args
assert 6   "int main() { add3(1, 2, 3); } int add3(int a, int b, int c) { return a + b + c; }"
assert 55  "int main() { fib(10); } int fib(int n) { if (n <= 1) return n; else return fib(n-1) + fib(n-2); }"
assert 3   "int main() { sub(10, 7); } int sub(int a, int b) { return (a - b); }"
assert 0   "int main() { isprime(10); } int isprime(int n) { i = 2; while (n >= i * i) { if (_mod(n, i) == 0) return 0; i = i + 1; } return 1; } int _mod(int n, int m) { return (n - (n / m) * m); }"
assert 1   "int main() { isprime(11); } int isprime(int n) { i = 2; while (n >= i * i) { if (_mod(n, i) == 0) return 0; i = i + 1; } return 1; } int _mod(int n, int m) { return (n - (n / m) * m); }"
assert 251 "int main() { maxprime(255); } int maxprime(int n) { max = 0; i = 2; while (i <= n) { if (isprime(i)) max = i; i = i + 1; } return max;} int isprime(int n) { i = 2; while (n >= i * i) { if (_mod(n, i) == 0) return 0; i = i + 1; } return 1; } int _mod(int n, int m) { return (n - (n / m) * m); }"

# function declare
assert 42  "int main() { return 42; }"
assert 42  "int main() { ft(); } int ft() { return 42; }"

# function call
assert 8   "int main() { return add(3, 5); }"
assert 42  "int main() { return add(20, 22); }"
assert 15  "int main() { return mul(3, 5); }"
assert 42  "int main() { return mul(6, 7); }"
assert 21  "int main() { return add6(1,2,3,4,5,6); }"

# zero-arty function call
assert 42  "int main() { return ret_ft(); }"
assert 3   "int main() { return ret_three(); }"

# block
assert 42  "int main() { a = 1; b = 1; c = 40; return a + b + c; }"
assert 84  "int main() { j = 0; for (i = 0; i < 42; i = i + 1) { j = j + 2; } return j; }"
assert 3   "int main() { {1; {2; } return 3; } }"

# null stmt
assert 42  "int main() {; ; ;; ;; ; return 42; ; ;}"

# for
assert 21  "int main() {for (i = 0; i < 21; i = i+1) 1; return i;}"
assert 42  "int main() {j = 0; for (i = 0; i < 21; i = i+1) j = j + 2; return j;}"
assert 3   "int main() {for (;;) return 3; 1;}"

# while
assert 42  "int main() {i = 0; while (i < 42) i = i + 1; return i;}"

# if
assert 42  "int main() {if (1) return 42; return 1;}"
assert 42  "int main() {if (0) return 1; return 42;}"
assert 42  "int main() {if (0) return 1; if (0) return 2; if (1) return 42;}"

# if else
assert 42  "int main() {if (0) return 1; else return 42; return 0;}"
assert 42  "int main() {if (1) return 42; else return 1; return 0;}"

# num
assert 0 "int main() {0;}"
assert 42 "int main() {42;}"
# add/sub
assert 21 "int main() {5+20-4;}"
# space
assert 41 "int main() {12 + 34 - 5;}"
assert 30 "int main() {12 + 34 - 5 + 67      -78;}"
# mul/div/parenthesis
assert 47 "int main() {5+6*7;}"
assert 15 "int main() {5*(9-6);}"
assert 4 "int main() {(3+5)/2;}"
# unary
assert 42 "int main() {+42;}"
assert 255 "int main() {-1;}"
assert 10 "int main() {+20-10;}"
# unary(is it ok?)
assert 30 "int main() {+20--10;}"
# eq
assert 1 "int main() {42==42;}"
assert 0 "int main() {42==41;}"
assert 0 "int main() {41==42;}"
# neq
assert 0 "int main() {42!=42;}"
assert 1 "int main() {42!=41;}"
assert 1 "int main() {41!=42;}"
# lt
assert 0 "int main() {42<42;}"
assert 0 "int main() {42<41;}"
assert 1 "int main() {41<42;}"
# lte
assert 1 "int main() {42<=42;}"
assert 0 "int main() {42<=41;}"
assert 1 "int main() {41<=42;}"
# gt
assert 0 "int main() {42>42;}"
assert 1 "int main() {42>41;}"
assert 0 "int main() {41>42;}"
# gte
assert 1 "int main() {42>=42;}"
assert 1 "int main() {42>=41;}"
assert 0 "int main() {41>=42;}"
# multiple statements
assert 3 "int main() {1;2;3;}"
# one-letter local variables assignment
assert 3 "int main() {a=1;b=2;a+b;}"
assert 5 "int main() {a=1;b=2;a=a+b;b=a+b;}"
# multi-letter local variables assignment
assert 6 "int main() {foo = 1; bar = 2 + 3;foo + bar;}"
assert 12 "int main() {foo = 1; bar = 2 + 3; foobar = foo + bar; foo + bar + foobar;}"
# return
assert 42 "int main() {return 42;}"
assert 1 "int main() {return 1;return 2; return 3;}"

echo OK

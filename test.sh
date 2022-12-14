#!/bin/bash
cat <<EOF | gcc -xc -c -o tmp2.o -
#include <stdlib.h>
int ret_ft() { return 42; }
int ret_three() { return 3; }
int	add(int a, int b) { return a + b; }
int	mul(int a, int b) { return a * b; }
int	add6(int a, int b, int c, int d, int e, int f) { return a + b + c + d + e + f; }
void alloc4(int **p, int n1, int n2, int n3, int n4) { *p = malloc(sizeof(int) * 4); (*p)[0] = n1; (*p)[1] = n2; (*p)[2] = n3; (*p)[3] = n4; }
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

# invalid type error
# asserterror "int main() { int a; int b; b = a; }"

# array
assert 63 "int main() { int a[2]; *a = 21; *(a + 1) = 42; int *p; p = a; return *p + *(p + 1); }"
assert 21 "int main() { int a[2]; *a = 21; *(a + 1) = 42; int *p; p = &a; return *p; }"

assert 42 "int main() { int arr[4]; arr[0] = 42; arr[1] = 2; arr[2] = 3; arr[3] = 4; return *arr; }"
assert  2 "int main() { int arr[4]; arr[0] = 42; arr[1] = 2; arr[2] = 3; arr[3] = 4; return *(arr+1); }"
assert  3 "int main() { int arr[4]; arr[0] = 42; arr[1] = 2; arr[2] = 3; arr[3] = 4; return *(arr+2); }"
assert  4 "int main() { int arr[4]; arr[0] = 42; arr[1] = 2; arr[2] = 3; arr[3] = 4; return *(arr+3); }"

assert 42 "int main() { int arr[4]; arr[0] = 42; arr[1] = 2; arr[2] = 3; arr[3] = 4; return arr[0]; }"
assert  2 "int main() { int arr[4]; arr[0] = 42; arr[1] = 2; arr[2] = 3; arr[3] = 4; return arr[1]; }"
assert  3 "int main() { int arr[4]; arr[0] = 42; arr[1] = 2; arr[2] = 3; arr[3] = 4; return arr[2]; }"
assert  4 "int main() { int arr[4]; arr[0] = 42; arr[1] = 2; arr[2] = 3; arr[3] = 4; return arr[3]; }"

assert 16 "int main() { int arr[4]; return sizeof(arr); }"
assert 48 "int main() { int arr[3][4]; return sizeof(arr); }"
assert 96 "int main() { int arr[2][3][4]; return sizeof(arr); }"

# sizeof
assert  4 "int main() { int x; return sizeof(x); }"
assert  4 "int main() { int x; return sizeof x; }"
assert  8 "int main() { int *x; return sizeof x; }"
assert  8 "int main() { int x; return sizeof(&x); }"
assert  8 "int main() { int x; return sizeof(&x + 1); }"
assert  4 'int main() { int x; x = 1; return sizeof(x=2); }'
assert  1 'int main() { int x; x = 1; sizeof(x=2); return x; }' # expression will not be evaluated. Only type of the expression matters.

# int size to 4
assert  4 "int main() { int *p; alloc4(&p, 1, 2, 4, 8); int *q; q = p + 2; return *q; }"
assert  8 "int main() { int *p; alloc4(&p, 1, 2, 4, 8); int *q; q = p + 3; return *q; }"

assert  1 "int main() { return f(1, 2, 3); } int f(int a, int b, int c) { return a; }"
assert  2 "int main() { return f(1, 2, 3); } int f(int a, int b, int c) { return b; }"
assert  3 "int main() { return f(1, 2, 3); } int f(int a, int b, int c) { return c; }"
assert  3 "int main() { return f(1, 2, 3); } int f(int a, int b, int c) { return a + b; }"
assert  6 "int main() { return f(1, 2, 3); } int f(int a, int b, int c) { return a + b + c; }"

assert  1 "int main() { int a; int b; int c; a = 1; b = 2; c = 3; return a; }"
assert  2 "int main() { int a; int b; int c; a = 1; b = 2; c = 3; return b; }"
assert  3 "int main() { int a; int b; int c; a = 1; b = 2; c = 3; return c; }"
assert  1 "int main() { int a; int b; int c; a = 1; b = 2; c = 3; *(&b) = 42; return a; }"
assert 42 "int main() { int a; int b; int c; a = 1; b = 2; c = 3; *(&b) = 42; return b; }"
assert  3 "int main() { int a; int b; int c; a = 1; b = 2; c = 3; *(&b) = 42; return c; }"

# function with many lvars
assert 1 "int main() { return f(1, 2, 3); } int f(int a, int b, int c) { int d; int e; int f; int g; int h; int i; return a; }"
assert 2 "int main() { return f(1, 2, 3); } int f(int a, int b, int c) { int d; int e; int f; int g; int h; int i; return b; }"
assert 3 "int main() { return f(1, 2, 3); } int f(int a, int b, int c) { int d; int e; int f; int g; int h; int i; return c; }"

# ptr arithmetic
assert 42 "int main() { int a; int b; int *p; a = 1; b = 2; p = &b; p = p + 1; *p = 42; return a; }"
assert 2 "int main() { int a; int b; int *p; a = 1; b = 2; p = &b; p = p + 1; *p = 42; return b; }"
assert 1 "int main() { int a; int b; int *p; a = 1; b = 2; p = &a; p = p - 1; *p = 42; return a; }"
assert 42 "int main() { int a; int b; int *p; a = 1; b = 2; p = &a; p = p - 1; *p = 42; return b; }"
assert 42 "int main() { int a; int b; int c; int *p; a = 1; b = 2; c = 3; p = &c; p = p + 2; *p = 42; return a; }"

# ptr to ptr to .... to int
assert 42 "int main() { int a; int *ap; int **app; ap = &a; app = &ap; a = 1; *ap = 2; **app = 42; return a; }"
assert 42 "int main() { int a; int *ap; int **app; ap = &a; app = &ap; a = 1; *ap = 42; return a; }"
assert 42 "int main() { int a; int *ap; int **app; ap = &a; app = &ap; a = 42; return a; }"

# same local var name
assert 42 "int main() { int a; a = 42; foo(1); return a; } int foo(int a){ a = a + 1; return a; }"
assert 42 "int main() { int a; a = 42; foo(); return a; } int foo(){ int a; a = 1; return a; }"
assert 42 "int main() { int a; int b; return foo(); } int foo(){ int b; int a; a = 1; b = 42; return b; }"
assert 1 "int main() { int a; int b; return foo(); } int foo(){ int b; int a; a = 1; b = 42; return a; }"

# unary * and &
assert 42  "int main() { int a; a = 42; return *(&a); }"
assert 42  "int main() { int a; int *b; a = 42; b = &a; return *b; }"
assert 21  "int main() { int a; int b; a = 42; b = 21; return *(&b); }"
assert 42  "int main() { int a; int b; a = 42; b = 21; return *(&b + 1); }"

# function declare with args
assert 6   "int main() { add3(1, 2, 3); } int add3(int a, int b, int c) { return a + b + c; }"
assert 55  "int main() { fib(10); } int fib(int n) { if (n <= 1) return n; else return fib(n-1) + fib(n-2); }"
assert 3   "int main() { sub(10, 7); } int sub(int a, int b) { return (a - b); }"
assert 0   "int main() { isprime(10); } int isprime(int n) { int i; i = 2; while (n >= i * i) { if (_mod(n, i) == 0) return 0; i = i + 1; } return 1; } int _mod(int n, int m) { return (n - (n / m) * m); }"
assert 1   "int main() { isprime(11); } int isprime(int n) { int i; i = 2; while (n >= i * i) { if (_mod(n, i) == 0) return 0; i = i + 1; } return 1; } int _mod(int n, int m) { return (n - (n / m) * m); }"
assert 251 "int main() { maxprime(255); } int maxprime(int n) { int max; int i; max = 0; i = 2; while (i <= n) { if (isprime(i)) max = i; i = i + 1; } return max;} int isprime(int n) { int i; i = 2; while (n >= i * i) { if (_mod(n, i) == 0) return 0; i = i + 1; } return 1; } int _mod(int n, int m) { return (n - (n / m) * m); }"

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
assert 42  "int main() { int a; int b; int c; a = 1; b = 1; c = 40; return a + b + c; }"
assert 84  "int main() { int j; int i; j = 0; for (i = 0; i < 42; i = i + 1) { j = j + 2; } return j; }"
assert 3   "int main() { {1; {2; } return 3; } }"

# null stmt
assert 42  "int main() {; ; ;; ;; ; return 42; ; ;}"

# for
assert 21  "int main() { int i; for (i = 0; i < 21; i = i+1) 1; return i;}"
assert 42  "int main() { int i; int j; j = 0; for (i = 0; i < 21; i = i+1) j = j + 2; return j;}"
assert 3   "int main() {for (;;) return 3; 1;}"

# while
assert 42  "int main() { int i; i = 0; while (i < 42) i = i + 1; return i;}"

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
assert 3 "int main() { int a; int b; int c; a=1;b=2;a+b;}"
assert 5 "int main() { int a; int b; a=1;b=2;a=a+b;b=a+b;}"
# multi-letter local variables assignment
assert 6 "int main() { int foo; int bar; foo = 1; bar = 2 + 3;foo + bar;}"
assert 12 "int main() { int foo; int bar; int foobar; foo = 1; bar = 2 + 3; foobar = foo + bar; foo + bar + foobar;}"
# return
assert 42 "int main() {return 42;}"
assert 1 "int main() {return 1;return 2; return 3;}"

echo OK

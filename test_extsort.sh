#!/bin/bash
echo "testing ... "
g++ -std=c++03 -Ofast extsort.cpp

echo "invalid args"
./a.exe

echo "invalid args"
./a.exe empty.txt out.txt 100000000 hello test test

echo "invalid args"
./a.exe empty.txt out.txt hello test test

echo "invalid args"
./a.exe empty.txt out.txt hello

echo "invalid buffer size"
./a.exe empty.txt out.txt -1024

echo "invalid buffer size"
./a.exe empty.txt out.txt 25000000000

echo "invalid args"
./a.exe non.valid.input.txt out.txt 100000000

echo "empty ext"
./a.exe empty.txt out.txt 100000000

echo "empty in memory"
./a.exe empty.txt out.txt 100000000
echo "----------------"

custom_testing() {
	echo "$1"
	time ./a.exe "$2" "$3" "$4"
	head "$3"
	tail "$3"
	if [[ $(diff <(sort "$2") <(cat "$3") | head) ]]; then
		echo "TEST FAILED !!!"
	else
		echo "TEST OK"
	fi
	echo "----------------"
}

custom_testing "normal ext" normal.txt out.normal.ext.txt 6000000
#custom_testing "normal in memory" normal.txt out.normal.in.txt 100000000
#custom_testing "numbers ext" numbers.txt out.numbers.ext.txt 2000
#custom_testing "numbers in memory" numbers.txt out.numbers.in.txt 100000000
#custom_testing "nonascii ext" nonascii.txt out.nonascii.ext.txt 2000
#custom_testing "nonascii in memory" nonascii.txt out.nonascii.in.txt 100000000

stress_testing() {
	base64 /dev/urandom | head -c "$1" > big.txt
	time ./a.exe big.txt out.big.ext.txt "$2"
	head out.big.ext.txt
	tail out.big.ext.txt
	rm out.*.txt big.txt
	echo "----------------"
}

#echo "stress testing... "
#stress_testing 1000000000 300000000
#stress_testing 2000000000 600000000
#stress_testing 5000000000 1000000000
#stress_testing 10000000000 1000000000

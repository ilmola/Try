# Try - An exceptionally simple test framework for C++11 and later

This is a header only library. There is only one header file and it's only a
couple of hundreds lines of code. This library has only two classes, no free 
functions and only one tiny tiny macro.

To use this library include `Try.hpp`.

~~~c++
#include <Try/Try.hpp>
~~~

First create a instance of `Try` (from name space `Try`).

~~~c++
Try::Try tr{};
~~~

You can optionally give a log stream. The default stream is 'std::cout'.

~~~c++
std::stringstream log{};
Try tr{log};
~~~

To run a test case use the `()`-operator.

~~~c++
template <typename F, typename... Args>
bool Try::operator()(SourceContext&& sc, F test, const Args&... args) noexcept;
~~~

To create a `SourceContext` use the macro `SC`. It is the *only* macro needed.
It creates a `SourceContext` so if a test case fails the line can be reported.

The parameter `test` is the test to run. Usually a lambda function. The 
return value of the test case is ignored. **To fail the test throw any exception**
with an appropriate message. 

`args` are the arguments forwarded to the test case when called. You can 
alternatively use lambda capture to pass arguments to the test case.

The return value is `true` if the test succeeds. If the test fails, the return 
value is `false` and the error message from the exception and the arguments are 
is logged to the stream. You can log additional messages to the log by calling 
`Try::os()` to get the stream.

~~~c++
Try::Try tr{};

tr.os() << "Testing mySquare().\n";

tr(SC, [] (int in, int out) {
	int result = mySquare(in);
	if (result != out) 
		throw std::runtime_error("Invalid result: "+std::to_string(result));
}, -2, 4);
~~~

Possible output:

~~~console
Testing mySquare().
Test failed: foo.cpp, line 14
Message: "Invalid result: -4"
Arguments:
"-2" (i)
"4" (i)
~~~

Another examble:

~~~c++
tr(SC, [] (std::size_t index, int value) {
	std::vector<int> data{1,2,3};
	data.at(index) = value;
 }, 4, 10);
~~~

Possible output:

~~~console
Test failed: foobar.cpp, line 12
Message: "vector::_M_range_check"
Arguments:
"4" (i)
"10" (i)
~~~

As testing for equality in the most common test case, there is a helper method 
`equal` to construct a test case for it.

~~~c++
template <typename T1, typename T2>
void Try::equal(SourceContext&& sc, const T1& a, const T2& b) noexcept;
~~~

Example:

~~~c++
std::string foobar("foobar");
tr.equal(SC, foobar.size(), 6);
~~~

Similarly there are also methods `notequal`, `less`, and `lequal`.

Another common case is to test if a function throws on invalid input. Helper
method `throws` will construct a test case that will fail if the test case does 
not throw.

~~~c++
template <typename F, typename... Args>
void Try::throws(SourceContext&& sc, F test, const Args&... args) noexcept;
~~~

Example:

~~~c++
tr.throws(SC, [] () {
	std::vector<int> data{1, 2, 3};
	data.at(data.size());
});
~~~

To nest tests capture the Try instance and use it inside another test.

~~~c++
tr(SC, [&tr] () {
	tr.os() << "test\n";
	tr(SC, [&tr] () {
		tr.os() << "subtest\n";
	});
});
~~~



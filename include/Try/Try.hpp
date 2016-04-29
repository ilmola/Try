#ifndef UUID_E65B69577BCE470086F19A6104A73D71
#define UUID_E65B69577BCE470086F19A6104A73D71

#include <cstdint>
#include <iostream>
#include <type_traits>
#include <stdexcept>
#include <typeinfo>



namespace Try {


/**
 * Helper class for tests to capture the source context.
 * Use the macro SC to create a SourceContext.
 */
class SourceContext {
public:

	SourceContext(const char* file, std::size_t line) noexcept :
		file{file}, line{line} { }

	const char* file;

	std::size_t line;

};

#define SC ::Try::SourceContext{__FILE__, __LINE__}

inline std::ostream& operator<<(std::ostream& os, const SourceContext& sc) noexcept {
	os << sc.file << ", line " << sc.line;
	return os;
}


namespace detail {


template<typename T>
class is_streamable {
public:
	template <typename U>
	static auto test(int) -> decltype(
		std::declval<std::ostream&>() << std::declval<U>(), std::true_type()
	);

	template<typename>
	static auto test(...) -> std::false_type;

	static constexpr bool value =
		std::is_same<decltype(test<T>(0)), std::true_type>::value;
};


template <typename T, typename std::enable_if<is_streamable<T>::value>::type* = nullptr>
void stream(std::ostream& os, const T& arg) {
	os << "\"" << arg << "\" (" << typeid(T).name() << ")";
}

template <typename T, typename std::enable_if<!is_streamable<T>::value>::type* = nullptr>
void stream(std::ostream& os, const T&) {
	os << "[Can't print'] (" << typeid(T).name() << ")";
}

inline void stream(std::ostream& os, const std::nullptr_t&) {
	os << "\"nullptr\" (nullptr_t)";
}

inline void streamArgs(std::ostream& os) noexcept { os << std::endl; }

template <typename T, typename... Args>
void streamArgs(std::ostream& os, const T& first, const Args&... args) noexcept {
	stream(os, first);
	os << std::endl;
	streamArgs(os, args...);
}

}


/**
 * A class for running test cases.
 * Use the ()-operator to run a test case. To fail the test throw any exception.
 */
class Try {
public:

	/// @param os A stream to log test errors to.
	explicit Try(std::ostream& os = std::cout) noexcept :
		mOs{&os},
		mSuccesCount{0},
		mFailCount{0}
	{ }

	/// @return The stream used for logging test errors.
	/// Can be used to log additional messages.
	std::ostream& os() const noexcept { return *mOs; }

	/// Runs the given test case with the given arguments. The test must throw
	/// on failure. The return value is ignored. The message from the exception
	/// along with the arguments given to the test are logged to the stream.
	/// @param sc Use the macro SC in place of this parameter.
	/// @param test A lambda function that performs the test.
	/// @param args Given to the test case function when called.
	/// @return false if test fails (throws) and true otherwise
	template <typename F, typename... Args>
	bool operator()(SourceContext&& sc, F test, const Args&... args) noexcept {
		try {
			test(args...);
			++mSuccesCount;
			return true;
		}
		catch (const std::exception& e) {
			os() << "Test failed: " << sc << std::endl;
			os() << "Message: \"" <<  e.what() << "\"" << std::endl;
		}
		catch (...) {
			os() << "Test failed: " << sc << std::endl;
			os() << "(no message)" << std::endl;
		}

		if (sizeof...(args) == 0) {
			os() << "(no arguments)" << std::endl << std::endl;
		}
		else {
			os() << "Arguments:" << std::endl;
			detail::streamArgs(os(), args...);
		}

		++mFailCount;
		return false;
	}

	/// Constructs and runs a test case that checks if a == b.
	/// The test will succeed if a == b and fails if not or if the "==" throws.
	/// @param sc Use the macro SC in place of this parameter.
	/// @param a,b Values to compare with the == operator.
	/// @return true if the test succeeds false if not.
	template <typename T1, typename T2>
	bool equal(SourceContext&& sc, const T1& a, const T2& b) noexcept {
		return (*this)(std::move(sc), [](const T1& a, const T2& b) {
			if (!(a == b)) {
				throw std::runtime_error{"Arguments are not equal!"};
			}
		}, a, b);
	}

	/// Constructs and runs a test case that checks if a != b.
	/// The test will succeed if a != b and fail if not or if "!=" throws.
	/// @param sc Use the macro SC in place of this parameter.
	/// @param a,b Values to compare with the != operator.
	/// @return true if the test succeeds false if not.
	template <typename T1, typename T2>
	bool notequal(SourceContext&& sc, const T1& a, const T2& b) noexcept {
		return (*this)(std::move(sc), [](const T1& a, const T2& b) {
			if (!(a != b)) {
				throw std::runtime_error{"Arguments are equal!"};
			}
		}, a, b);
	}

	/// Constructs and runs a test case that checks if a < b.
	/// The test will succeed if a < b and fails if not or if "<" throws.
	/// @param sc Use the macro SC in place of this parameter.
	/// @param a,b Values to compare with the < operator.
	/// @return true if the test succeeds false if not.
	template <typename T1, typename T2>
	bool less(SourceContext&& sc, const T1& a, const T2& b) noexcept {
		return (*this)(std::move(sc), [](const T1& a, const T2& b) {
			if (!(a < b)) {
				throw std::runtime_error{"The first argument is not less than the second!"};
			}
		}, a, b);
	}

	/// Constructs and runs a test case that checks if a <= b.
	/// The test will succeed if a <= b and fails if not or if "<=" throws.
	/// @param sc Use the macro SC in place of this parameter.
	/// @param a,b Values to compare with the <= operator.
	/// @return true if the test succeeds false if not.
	template <typename T1, typename T2>
	bool lequal(SourceContext&& sc, const T1& a, const T2& b) noexcept {
		return (*this)(std::move(sc), [](const T1& a, const T2& b){
			if (!(a <= b)) {
				throw std::runtime_error{"The first argument is not less than or equal to the second!"};
			}
		}, a, b);
	}

	/// Constructs and runs a test case that succeeds if it throws type T and 
	/// fails if it does not. The return value will make no difference.
	/// @tpraram T The type that needs to be thrown for the test to pass.
	/// @param sc Use the macro SC in place of this parameter.
	/// @param test A lambda function that runs the test.
	/// @param args The arguments given to the lambda when run.
	/// @return true if the test succeeds and false if it fails.
	template <typename T, typename F, typename... Args>
	bool throws(SourceContext&& sc, F test, const Args&... args) noexcept {
		return (*this)(std::move(sc), [test](const Args&... args) {
			try {
				test(args...);
			}
			catch (const T&) {
				return;
			}
			catch (const std::exception& e) {
				throw std::runtime_error{
					"Test throws a wrong exception ("+std::string{typeid(e).name()}+"): "+e.what()
				};
			}
			catch (...) {
				throw std::runtime_error{"Test throws a wrong non-exception!"};
			}

			throw std::runtime_error{"Test did not throw!"};
		}, args...);
	}

	/// @return The number of successful tests run.
	std::size_t succesCount() const noexcept { return mSuccesCount; }

	/// @return The number of failed tests run.
	std::size_t failCount() const noexcept { return mFailCount; }

private:

	std::ostream* mOs;

	std::size_t mSuccesCount;

	std::size_t mFailCount;

};


}


#endif

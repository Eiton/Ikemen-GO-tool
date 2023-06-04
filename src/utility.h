#ifndef HEADERFILE_UTILTIY
#define HEADERFILE_UTILTIY

#ifndef PI
#define PI 3.1415956535898
#endif
#include <string>
#include "linmath.h"
namespace Utility {
	void loadCustomSetting();
	struct CustomSetting {
		std::string filePath;
		vec3 clearColor = {197/255.0f,177/255.0f,197/255.0f};
		vec3 axisColor = {0,0,0};
	};
	template <class T>
	class ExecutionResult {
	public: 
		ExecutionResult(bool success = true, std::string message = "", T value = 0) : success(success), message(message), value(value) {}
		std::string getMessage() {
			return message;
		}
		std::string getValue() {
			return value;
		}
		explicit operator bool() const { return (this->success); }
	private:
		bool success;
		std::string message;
		T value;
	};

}
extern Utility::CustomSetting setting;
// Defer function call
// Code block borrowed from https://www.gingerbill.org/article/2015/08/19/defer-in-cpp/
template <typename F>
struct privDefer {
	F f;
	privDefer(F f) : f(f) {}
	~privDefer() { f(); }
};

template <typename F>
privDefer<F> defer_func(F f) {
	return privDefer<F>(f);
}

#define DEFER_1(x, y) x##y
#define DEFER_2(x, y) DEFER_1(x, y)
#define DEFER_3(x)    DEFER_2(x, __COUNTER__)
#define defer(code)   auto DEFER_3(_defer_) = defer_func([&](){code;})
#endif
#pragma once
#include <mono/jit/jit.h>
#include <string>
#include <span>

#ifdef COMPILE_DLL_RUNTIME
bool CompileToDLL(const std::string& output_file,
                  const std::vector<std::string>& source_files);
#endif

class MonoStringWrapper {
 public:
  MonoStringWrapper(const char* string) : string_(string) {}
  ~MonoStringWrapper() {}

  void Release() {
    mono_free((void*) string_);
  }

  operator const char*() { return string_; }

 private:
  const char* string_;
};

class ScopedMonoStringWrapper {
 public:
  ScopedMonoStringWrapper(const char* string) : string_(string) {}
  ScopedMonoStringWrapper(MonoStringWrapper string) : string_(string) {}

  ~ScopedMonoStringWrapper() {
    string_.Release();
  }

  operator const char*() { return string_; }

 private:
  MonoStringWrapper string_;
};
class MonoObjectWrapper {
 public:
  MonoObjectWrapper(MonoObject* object) : object_(object) {}
  ~MonoObjectWrapper() {}

  operator bool() { return object_ != nullptr; }

  std::string AsString() {
    MonoString* resultString = mono_object_to_string(object_, nullptr);
    const char* str = mono_string_to_utf8(resultString);
    std::string result(str);
    return result;
  }

  MonoStringWrapper AsWrappedString() {
    MonoString* resultString = mono_object_to_string(object_, nullptr);
    return {mono_string_to_utf8(resultString)};
  }

 private:
  MonoObject* object_;

};
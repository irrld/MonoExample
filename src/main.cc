#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>
#include <iostream>
#include "mono_util.h"

MonoString* GetName() {
  return mono_string_new(mono_domain_get(), "C++");
}

int main() {
  mono_jit_init("HelloWorld");

  MonoDomain *domain = mono_domain_get();
  mono_domain_set(domain, false);


  //mcs -target:library -out:assembly.dll HelloWorldClass.cs
  MonoAssembly* assembly = mono_domain_assembly_open(domain, "assembly.dll");
  if (!assembly) {
    std::cout << "Assembly not found!" << std::endl;
    return 1;
  }

  const char* entry_name = "Hello.Main::GetHelloWorld";
  MonoImage* image = mono_assembly_get_image(assembly);
  MonoMethodDesc* desc = mono_method_desc_new(entry_name, false);
  MonoMethod* method = mono_method_desc_search_in_image(desc, image);

  if (!method) {
    std::cout << "Method not found!" << std::endl;
    return 1;
  }

  // Namespace.Class::Method(params)
  mono_add_internal_call("Hello.Main::GetName", reinterpret_cast<void*>(GetName));

  MonoObjectWrapper result = mono_runtime_invoke(method, nullptr, nullptr, nullptr);
  if (result) {
    ScopedMonoStringWrapper string = result.AsWrappedString();
    std::cout << string << std::endl;
  }

  mono_jit_cleanup(domain);

  return 0;
}

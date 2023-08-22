#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>
#include <iostream>

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

  const char* entry_name = "HelloWorldNamespace.HelloWorldClass:GetHelloWorld";
  MonoImage* image = mono_assembly_get_image(assembly);
  MonoMethodDesc* desc = mono_method_desc_new(entry_name, false);
  MonoMethod* method = mono_method_desc_search_in_image(desc, image);

  if (!method) {
    std::cout << "Method not found!" << std::endl;
    return 1;
  }

  MonoObject *result = mono_runtime_invoke(method, nullptr, nullptr, nullptr);
  if (result) {
    MonoString* resultString = mono_object_to_string(result, nullptr);
    const char* str = mono_string_to_utf8(resultString);
    std::cout << "Returned string: " << str << std::endl;
    mono_free((void*) str);
  }

  mono_jit_cleanup(domain);

  return 0;
}

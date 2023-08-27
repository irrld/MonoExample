#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>
#include <iostream>
#include "mono_util.h"
#include <filesystem>
#include <thread>

MonoString* GetName() {
  return mono_string_new(mono_domain_get(), "C++");
}

bool MonitorFiles(bool load, std::vector<std::string>& files, std::vector<std::filesystem::file_time_type>& last_write_times) {
  for (int i = 0; i < files.size(); ++i) {
    const auto& item = files[i];
    auto current_file_last_write_time =
        std::filesystem::last_write_time(item);
    if (load) {
      last_write_times.push_back(current_file_last_write_time);
      continue;
    }
    if (last_write_times[i] != current_file_last_write_time) {
      return true;
    }
  }
  return false;
}

int main() {
  std::vector<std::string> files;
#ifdef COMPILE_DLL_RUNTIME
  files = {
      "csharp/HelloWorldClass.cs"
  };
#else
  files = {
      "obj/assembly.dll"
  };
#endif

  mono_jit_init("HelloWorld");

  MonoDomain * root_domain = mono_domain_get();
  mono_domain_set(root_domain, false);
  while (true) {
#ifdef COMPILE_DLL_RUNTIME
    //mcs -target:library -out:assembly.dll HelloWorldClass.cs
    CompileToDLL("obj/assembly.dll", {"csharp/HelloWorldClass.cs"});
#endif

    // Load assembly
    MonoDomain* domain = mono_domain_create_appdomain("HelloWorld", nullptr);
    mono_domain_set(domain, false);
    MonoAssembly* assembly = mono_domain_assembly_open(domain, "obj/assembly.dll");
    if (!assembly) {
      return 1;
    }

    const char* entry_name = "Hello.Main::GetHelloWorld";
    MonoImage* image = mono_assembly_get_image(assembly);
    MonoMethodDesc* desc = mono_method_desc_new(entry_name, false);
    MonoMethod* method = mono_method_desc_search_in_image(desc, image);

    if (!method) {
      std::cout << "Method not found!" << std::endl;
      return 2;
    }

    // Namespace.Class::Method(params)
    mono_add_internal_call("Hello.Main::GetName", reinterpret_cast<void*>(GetName));

    MonoObjectWrapper result = mono_runtime_invoke(method, nullptr, nullptr, nullptr);
    if (result) {
      ScopedMonoStringWrapper string = result.AsWrappedString();
      std::cout << string << std::endl;
    }

    // switch back to the root domain
    mono_domain_set(root_domain, false);
    // unload our appdomain
    mono_domain_unload(domain);

    bool first = true;
    std::vector<std::filesystem::file_time_type> last_write_times;
    while (true) {
      if (MonitorFiles(first, files, last_write_times)) {
        break;
      }

      if (first) {
        first = false;
        continue;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    std::cout << "Files changed!" << std::endl;
  }

  mono_jit_cleanup(root_domain);
  return 0;
}

#include "mono_util.h"
#include <numeric>
#include <vector>
#include <filesystem>
#include <iostream>

#ifdef COMPILE_DLL_RUNTIME
#ifdef WIN32
#error "Runtime compilation is added for Windows yet!"
#endif
std::pair<int, std::string> ExecuteCommandAndGetOutput(const char* command) {
  // Create a temporary file
  const char* temp_file_name = "temp_output.txt";

  // Construct the command to redirect output to the temporary file
  std::string new_command = std::string(command) + " > " + temp_file_name + " 2>&1";

  // Execute the command
  int result = std::system(new_command.c_str());

  // Read the temporary file into a string
  FILE* file = std::fopen(temp_file_name, "r");
  if (file) {
    std::string output;
    char buffer[128];
    while (std::fgets(buffer, sizeof(buffer), file) != nullptr) {
      output += buffer;
    }
    std::fclose(file);
    std::remove(temp_file_name); // Delete the temporary file
    return {result, output};
  }
  return {result, ""};
}

bool CompileToDLL(const std::string& output_file,
                  const std::vector<std::string>& source_files) {
  std::filesystem::path output_dir = std::filesystem::path(output_file).parent_path();
  if (!std::filesystem::exists(output_dir) && !std::filesystem::create_directory(output_dir)) {
    std::cout << "Failed to create output directory: " << output_dir << std::endl;
    return false;
  }
  std::string command_prefix = "mono/bin/mcs";
  std::string command = command_prefix + " -target:library -out:" + output_file + " " +
                        std::accumulate(source_files.begin(),
                                        source_files.end(),
                                        std::string(" "));

  std::pair result = ExecuteCommandAndGetOutput(command.c_str());
  if (result.first != 0) {
    std::cout << "Failed to compile C# sources to DLL (error code:" << result.first << ")" << std::endl << result.second << std::endl;
    return false;
  }
  return true;
}
#endif
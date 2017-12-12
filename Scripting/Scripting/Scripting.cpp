#include "stdafx.h"
#include "Scripting.h"

#include <string>
#include <iostream>
#include <windows.h>
#include <direct.h>

// Mono Include
#include <mono/metadata/assembly.h>
#include <mono/metadata/mono-config.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/jit/jit.h>

#pragma comment(lib, "mono-2.0-sgen.lib")

namespace ScriptingSystem
{
	// VARIABLES TO WORK WITH ------
	char my_path[FILENAME_MAX];
	std::string mono_path;
	MonoDomain* domain = nullptr;
	// -----------------------------

	SCRIPTING_MANAGER void ScriptingSystem::GetPath()
	{	
		// Fill my_path char array with the path of the .dll
		_getcwd(my_path, FILENAME_MAX);

		// Set where Mono Directory is placed in mono_path
		mono_path = my_path;
		mono_path += "/../../Scripting/Scripting/Mono";
	}

	SCRIPTING_MANAGER const char* ScriptingSystem::GetMonoPath()
	{
		return mono_path.c_str();
	}

	SCRIPTING_MANAGER bool ScriptingSystem::InitSystem()
	{
		// INIT MONO --------------------------------
		// Get the path of the project ----
		GetPath(); /*This path is saved in a variable until the program is closed, you only need to call this once*/

		// Use the standard configuration
		mono_config_parse(NULL);

		// Get the correct dirs
		std::string lib = mono_path;
		lib += "/lib";
		std::string etc = mono_path;
		etc += "/etc";

		// Setup the default directories for mono use here for now the directories of your Mono installation
		mono_set_dirs(lib.c_str(), etc.c_str());
		domain = mono_jit_init_version("Scripting", "v4.0.30319");
		
		return true;
	}

	SCRIPTING_MANAGER bool ScriptingSystem::CloseSystem()
	{
		// Shutdown mono
		if (domain != nullptr)
		{
			mono_jit_cleanup(domain);
		}
		return true;
	}

	SCRIPTING_MANAGER const char* ScriptingSystem::CompileFile(const char* path, const char* dest_path)
	{
		// Get the path of the project ----
		std::string script_path = path;

		// Get mono directory -------------
		std::string command = mono_path;

		// Save dll to Library Directory --------------------
		std::string libraryScripts = dest_path;
		std::string nameFile = path;
		size_t EndName = nameFile.find_last_of("/");
		nameFile = nameFile.substr(EndName + 1);
		EndName = nameFile.find_last_of(".");
		nameFile = nameFile.substr(0, EndName);
		nameFile += ".dll";
		libraryScripts += nameFile;

		// Compile the script -----------------------------
		command += "/bin/mcs " + script_path + " -target:library -out:" + libraryScripts;
		
		//ShowWindow(FindWindowA("ConsoleWindowClass", NULL), false); -> Hide console (good or bad?)
		system(command.c_str());

		return command.c_str();
	}
}


//// Open the Assembly with your scripts
//assembly = mono_domain_assembly_open(domain, "Example.dll");
//
//// Get the Image
//image = mono_assembly_get_image(assembly);
//
//// Find the Entity class in the image
//MonoClass* entityClass = mono_class_from_name(image, "Example", "Entity");
//
//// allocate memory for one Entity instance
//MonoObject* entityInstance = mono_object_new(domain, entityClass);
//
//// find the Entity class constructor method that takes one parameter
//MonoMethod* constructorMethod = mono_class_get_method_from_name(entityClass, ".ctor", 1);
//
//// create a MonoString that will be passed to the constructor as an argument
//MonoString* name = mono_string_new(mono_domain_get(), "Giorgos");
//void* args[1];
//args[0] = name;
//
//// finally, invoke the constructor
//MonoObject* exception = NULL;
//mono_runtime_invoke(constructorMethod, entityInstance, args, &exception);
//
//// find the Process method that takes zero parameters
//MonoMethod* processMethod = mono_class_get_method_from_name(entityClass, "Process", 0);
//exception = nullptr;
//
//// invoke the method
//// if invoking static methods, then the second argument must be NULL
//mono_runtime_invoke(processMethod, entityInstance, nullptr, &exception);
//
//// check for any thrown exception
//if (exception)
//{
//	std::cout << mono_string_to_utf8(mono_object_to_string(exception, nullptr)) << std::endl;
//}
//
//// find the GetName method
//MonoMethod* getNameMethod = mono_class_get_method_from_name(entityClass, "GetName", 0);
//exception = nullptr;
//MonoString* ret = (MonoString*)mono_runtime_invoke(getNameMethod, entityInstance, nullptr, &exception);
//char* c = mono_string_to_utf8(ret);
//std::cout << "Value of 'name' is " << c << std::endl;
//
//// free the memory allocated from mono_string_to_utf8 ()
//mono_free(c);
//
////// find the Id field in the Entity class
////MonoClassField* idField = mono_class_get_field_from_name(entityClass, "Id");
////int value = 42;
//
////// set the field's value
////mono_field_set_value(entityObject, idField, &value);
//
////int result;
////mono_field_get_value(entityObject, idField, &result);
////std::cout << "Value of 'Id' is " << result << std::endl;
//
//




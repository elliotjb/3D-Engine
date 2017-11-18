#include "ModuleFS.h"
#include "Application.h"
#include "ModuleInput.h"
#include "ModuleGUI.h"
#include "Application.h"
#include "ModuleResourceManager.h"
#include "JSONSerialization.h"

ModuleFS::ModuleFS(bool start_enabled) : Module(start_enabled)
{
	Start_enabled = true;
	preUpdate_enabled = true;

	name = "File System";
}


ModuleFS::~ModuleFS()
{
	allfilesAsstes.clear(); //
}

bool ModuleFS::Start()
{
	perf_timer.Start();

	// Will contain exe path
	HMODULE hModule = GetModuleHandle(NULL);
	if (hModule != NULL)
	{
		// When passing NULL to GetModuleHandle, it returns handle of exe itself
		GetModuleFileName(hModule, ownPth, (sizeof(ownPth)));
	}
	directory_Game = ownPth;

	//In release this code: -----------------------------------------
	//size_t EndName = directory_Game.find_last_of("\\");
	//directory_Game = directory_Game.substr(0, EndName);

	//Not release this: ---------------------------------------------
	size_t EndName = directory_Game.find_last_of("\\");
	directory_Game = directory_Game.substr(0, EndName);
	EndName = directory_Game.find_last_of("\\");
	directory_Game = directory_Game.substr(0, EndName);
	directory_Assets = directory_Game + "\\Game";
	directory_Game += "\\Game\\Assets"; // "\\Game\\Assets"

	// Check if Main Folders exist --------------------
	CreateFolder("Library");
	CreateFolder("Library\\Meshes");
	CreateFolder("Library\\Materials");
	CreateFolder("Library\\Animations");
	CreateFolder("Assets");
	// Get All Files From Assets
	GetAllFilesAssets(directory_Game, allfilesAsstes);
	checkAssets.Start();

	Start_t = perf_timer.ReadMs();
	return true;
}

update_status ModuleFS::PreUpdate(float dt)
{
	perf_timer.Start();

	if (App->input->GetKey(SDL_SCANCODE_N) == KEY_DOWN)
	{
		//App->resource_manager->resourcesToReimport.push_back(App->Json_seria->GetUUIDPrefab(allfilesAsstes[0].directory_name, 0));
		//char* buffer = nullptr;

		//// Loading File
		//uint size = LoadFile(files[1].c_str(), &buffer);
		//int x = 0;

		//CopyFileToAssets("C:\\Users\\Administrador\\Documents\\GitHub\\3D-Engine\\Engine\\Game\\Assets\\TEST_Time.png", "C:\\Users\\Administrador\\Documents\\GitHub\\3D-Engine\\Engine\\Game\\Library\\TEST_Time.png");
		//
		//uint temp = App->random->Int();
		//LOG("%i", temp);
	}

	if (checkAssets.ReadSec() > 20)
	{
		checkAssets.Start();
		//DeleteAllFilesAssets(allfilesAsstes);
		//GetAllFilesAssets(directory_Game, allfilesAsstes);

		// Need something...
		//AnyfileModificated(allfilesAsstes); 
	}

	preUpdate_t = perf_timer.ReadMs();
	return UPDATE_CONTINUE;
}

void ModuleFS::CopyFileToAssets(const char* fileNameFrom, const char* fileNameTo)
{
	//assert(fileExists(fileNameFrom));
	namespace fs = std::experimental::filesystem;
	std::string exits = fileNameTo;
	exits += "/" + FixName_directory(fileNameFrom);
	if (fs::exists(exits) == false)
	{
		fs::copy(fileNameFrom, fileNameTo);
	}
	exits.clear();
	// Copy Folders
	//std::filesystem::copy("/dir1", "/dir3", std::filesystem::copy_options::recursive);
}

bool ModuleFS::CheckAssetsIsModify()
{
	return true;
}

void ModuleFS::GetAllFolders(std::experimental::filesystem::path path, std::string folderActive, std::vector<FoldersNew>& folders)
{
	namespace stdfs = std::experimental::filesystem;

	DeleteFolders(folders);

	const stdfs::directory_iterator end{};

	if (path == "")
	{
		path = directory_Assets;
	}

	for (stdfs::directory_iterator iter{ path }; iter != end; ++iter)
	{
		if (stdfs::is_directory(*iter))
		{
			const char* isdirectA = ConverttoConstChar(iter->path().string());
			std::string isFolderAssets = FixName_directory(isdirectA);
			if (isFolderAssets == "Assets")
			{
				FoldersNew folder_temp;

				folder_temp.directory_name = ConverttoConstChar(iter->path().string());
				folder_temp.file_name = FixName_directory(iter->path().string()).c_str();
				if (folderActive == folder_temp.directory_name)
				{
					folder_temp.active = true;
				}
				GetAllFoldersChild(iter->path().string(), folderActive, folder_temp.folder_child);
				folders.push_back(folder_temp);
			}
		}
	}
}

void ModuleFS::GetAllFoldersChild(std::experimental::filesystem::path path, std::string folderActive, std::vector<FoldersNew>& folders)
{
	namespace stdfs = std::experimental::filesystem;

	const stdfs::directory_iterator end{};

	if (path == "")
	{
		path = directory_Game;
	}

	for (stdfs::directory_iterator iter{ path }; iter != end; ++iter)
	{
		if (stdfs::is_directory(*iter))
		{
			FoldersNew folder_temp;
			folder_temp.directory_name = ConverttoConstChar(iter->path().string());
			folder_temp.file_name = FixName_directory(iter->path().string()).c_str();
			if (folderActive == folder_temp.directory_name)
			{
				folder_temp.active = true;
			}
			GetAllFoldersChild(iter->path().string(), folderActive, folder_temp.folder_child);
			folders.push_back(folder_temp);
		}
	}
}

void ModuleFS::GetAllFiles(std::experimental::filesystem::path path, std::vector<FilesNew>& files)
{
	namespace stdfs = std::experimental::filesystem;

	DeleteFiles(files);

	const stdfs::directory_iterator end{};

	for (stdfs::directory_iterator iter{ path }; iter != end; ++iter)
	{
		std::string extension = GetExtension(iter->path().string());
		if (strcmp(extension.c_str(), "json") != 0)
		{
			FilesNew files_temp;
			files_temp.directory_name = ConverttoConstChar(iter->path().string());
			if (stdfs::is_directory(*iter))
			{
				files_temp.directory_name_next = ConverttoConstChar(iter->path().string() + "\\");
			}
			else
				files_temp.directory_name_next = nullptr;

			files_temp.file_name = FixName_directory(iter->path().string()).c_str();
			files_temp.file_type = ((Project*)App->gui->winManager[PROJECT])->SetType(files_temp.file_name);
			files.push_back(files_temp);
		}
	}
}

void ModuleFS::GetAllFilesAssets(std::experimental::filesystem::path path, std::vector<AllFiles>& files)
{
	namespace stdfs = std::experimental::filesystem;

	const stdfs::directory_iterator end{};

	for (stdfs::directory_iterator iter{ path }; iter != end; ++iter)
	{
		std::string extension = GetExtension(iter->path().string());
		for (std::string::iterator it = extension.begin(); it != extension.end(); it++)
		{
			*it = tolower(*it);
		}
		if (IsPermitiveExtension(extension.c_str()))
		{
			AllFiles files_temp;
			stdfs::file_time_type temp = stdfs::last_write_time(iter->path());
			std::time_t cftime = decltype(temp)::clock::to_time_t(temp);
			files_temp.ftime = cftime;
			files_temp.directory_name = ConverttoConstChar(iter->path().string());
			files_temp.file_name = FixName_directory(iter->path().string()).c_str();
			files.push_back(files_temp);
			if (stdfs::is_directory(*iter))
			{
				GetAllFilesAssets(iter->path().string(), files);
			}
		}
	}
}

void ModuleFS::GetAllFilesFromFolder(std::experimental::filesystem::path path, std::list<const char*>& files)
{
	namespace stdfs = std::experimental::filesystem;

	const stdfs::directory_iterator end{};

	for (stdfs::directory_iterator iter{ path }; iter != end; ++iter)
	{
		std::string extension = GetExtension(iter->path().string());
		for (std::string::iterator it = extension.begin(); it != extension.end(); it++)
		{
			*it = tolower(*it);
		}
		if (IsPermitiveExtension(extension.c_str()))
		{
			if (App->resource_manager->CheckFileType(extension.c_str()) == Resource::Type::MESH)
			{
				files.push_back(ConverttoConstChar(iter->path().string()));
			}
			else if (App->resource_manager->CheckFileType(extension.c_str()) == Resource::Type::MATERIAL)
			{
				files.push_front(ConverttoConstChar(iter->path().string()));
			}
		}
		else
		{
			std::string isFolder = ConverttoConstChar(iter->path().string());
			if (App->resource_manager->CheckFileType(isFolder.c_str()) == Resource::Type::FOLDER)
			{
				files.push_back(ConverttoConstChar(iter->path().string()));
			}
		}
	}
}

bool ModuleFS::AnyfileModificated(std::vector<AllFiles>& files)
{
	namespace stdfs = std::experimental::filesystem;

	const stdfs::directory_iterator end{};
	std::experimental::filesystem::path path = directory_Game;
	int i = 0;
	int count = 0;
	for (stdfs::directory_iterator iter{ path }; iter != end; ++iter)
	{
		if (stdfs::is_directory(*iter) == false)
		{
			std::string extension = GetExtension(iter->path().string());
			//Set lowercase the extension to normalize it
			for (std::string::iterator it = extension.begin(); it != extension.end(); it++)
			{
				*it = tolower(*it);
			}
			if (IsPermitiveExtension(extension.c_str()))
			{
				stdfs::file_time_type temp = stdfs::last_write_time(iter->path());
				std::time_t cftime = decltype(temp)::clock::to_time_t(temp);
				if (files[i++].ftime == cftime)
				{
					// No Modificated
				}
				else
				{
					// LOG("MODIFICATED");
					switch (App->resource_manager->CheckFileType(extension.c_str()))
					{
					case Resource::Type::MESH:
					{
						bool finish = false; int id = 0;
						while (finish == false)
						{
							ReImport temp = App->Json_seria->GetUUIDPrefab(files[i - 1].directory_name, id++);
							if (temp.uuid != 0)
							{
								App->resource_manager->resourcesToReimport.push_back(temp);
							}
							else
							{
								finish = true;
							}
						}
						break;
					}
					case Resource::Type::MATERIAL:
					{
						App->resource_manager->resourcesToReimport.push_back(App->Json_seria->GetUUIDMaterial(files[i - 1].directory_name));
						break;
					}
					}
				}
			}
		}
		count++;
	}
	LOG("%i", count);
	return true;
}

bool ModuleFS::IsPermitiveExtension(const char* extension)
{
	if (strcmp(extension, "png") == 0 || strcmp(extension, "jpg") == 0 ||
		strcmp(extension, "fbx") == 0 || strcmp(extension, "obj") == 0 || 
		strcmp(extension, "tga") == 0)
	{
		return true;
	}
	return false;
}

// If we Import a new File!
void ModuleFS::UpdateFilesAsstes()
{
	checkAssets.Start();
	DeleteAllFilesAssets(allfilesAsstes);
	GetAllFilesAssets(directory_Game, allfilesAsstes);
}

// If focused window check if have a file was modificated and reimport.-
void ModuleFS::CheckNowfilesAssets()
{
	checkAssets.Start();
	AnyfileModificated(allfilesAsstes);
}

void ModuleFS::DeleteFiles(std::vector<FilesNew>& files)
{
	for (int i = 0; i < files.size(); i++)
	{
		RELEASE_ARRAY(files[i].directory_name);
		RELEASE_ARRAY(files[i].directory_name_next)
		RELEASE_ARRAY(files[i].file_name)
	}
	files.clear();
}

void ModuleFS::DeleteFolders(std::vector<FoldersNew>& folders)
{
	for (int i = 0; i < folders.size(); i++)
	{
		RELEASE_ARRAY(folders[i].directory_name);
		RELEASE_ARRAY(folders[i].file_name);
		DeleteFolders(folders[i].folder_child);
	}
	folders.clear();
}

void ModuleFS::DeleteAllFilesAssets(std::vector<AllFiles>& filesAssets)
{
	for (int i = 0; i < filesAssets.size(); i++)
	{
		RELEASE_ARRAY(filesAssets[i].directory_name);
		RELEASE_ARRAY(filesAssets[i].file_name);
	}
	filesAssets.clear();
}

bool ModuleFS::DeleteFileLibrary(const char* file, DIRECTORY_IMPORT directory)
{
	std::string temp = file;
	switch (directory)
	{
	case IMPORT_DEFAULT:
	{
		break;
	}
	case IMPORT_DIRECTORY_ASSETS:
	{
		temp = DIRECTORY_ASSETS + temp;
		break;
	}
	case IMPORT_DIRECTORY_LIBRARY:
	{
		temp = DIRECTORY_LIBRARY + temp;
		break;
	}
	case IMPORT_DIRECTORY_LIBRARY_MESHES:
	{
		temp = DIRECTORY_LIBRARY_MESHES + temp;
		break;
	}
	case IMPORT_DIRECTORY_LIBRARY_MATERIALS:
	{
		temp = DIRECTORY_LIBRARY_MATERIALS + temp;
		break;
	}
	}
	if (std::remove(temp.c_str()) == 0)
	{
		LOG("File Removed");
		return true;
	}
	return false;
}

uint ModuleFS::LoadFile(const char* file, char** buffer, DIRECTORY_IMPORT directory)
{
	// First apply direcotry
	std::string temp = file;
	switch (directory)
	{
	case IMPORT_DEFAULT:
	{
		break;
	}
	case IMPORT_DIRECTORY_ASSETS:
	{
		temp = DIRECTORY_ASSETS + temp;
		break;
	}
	case IMPORT_DIRECTORY_LIBRARY:
	{
		temp = DIRECTORY_LIBRARY + temp;
		break;
	}
	case IMPORT_DIRECTORY_LIBRARY_MESHES:
	{
		temp = DIRECTORY_LIBRARY_MESHES + temp;
		break;
	}
	case IMPORT_DIRECTORY_LIBRARY_MATERIALS:
	{
		temp = DIRECTORY_LIBRARY_MATERIALS + temp;
		break;
	}
	}
	std::ifstream is(temp, std::ifstream::binary);
	int length = 0;
	if (is)
	{
		// get length of file:
		is.seekg(0, is.end);
		length = is.tellg();
		is.seekg(0, is.beg);

		*buffer = new char[length];

		is.read(*buffer, length);

		if (is)
		{
			LOG("File Loaded.")
		}
		else
		{
			LOG("Error %s", is.gcount());
		}

		is.close();
	}
	else
	{
		LOG("Error to Load File -> %s", file);
	}
	return length;
}

bool ModuleFS::SaveFile(const char* data, std::string name, uint size, DIRECTORY_IMPORT directory)
{
	// First apply direcotry
	switch (directory)
	{
	case IMPORT_DEFAULT:
	{
		break;
	}
	case IMPORT_DIRECTORY_ASSETS:
	{
		name = DIRECTORY_ASSETS + name;
		break;
	}
	case IMPORT_DIRECTORY_LIBRARY:
	{
		name = DIRECTORY_LIBRARY + name;
		break;
	}
	case IMPORT_DIRECTORY_LIBRARY_MESHES:
	{
		name = DIRECTORY_LIBRARY_MESHES + name;
		break;
	}
	case IMPORT_DIRECTORY_LIBRARY_MATERIALS:
	{
		name = DIRECTORY_LIBRARY_MATERIALS + name;
		break;
	}
	}
	// Open or Created ----------------------------------------
	std::ofstream outfile(name, std::ofstream::binary);

	if (outfile.good())
	{
		// write to outfile
		outfile.write(data, size);
		LOG("Save Completed");
	}
	else
	{
		LOG("Failed to write the file %s", name.c_str());
	}

	outfile.close();
	// ------------------------------------------------------------
	return false;
}

bool ModuleFS::CheckIsFileExist(const std::string& name) {
	std::ifstream fil(name.c_str());
	return fil.good();
}

std::string ModuleFS::GetMainDirectory()
{
	return directory_Game;
}

void ModuleFS::FixNames_directories(std::vector<std::string>& files)
{
	for (int i = 0; i < files.size(); i++)
	{
		size_t EndName = files[i].find_last_of("\\");
		files[i] = files[i].substr(EndName + 1);
	}
}

std::string ModuleFS::FixName_directory(std::string file)
{
	size_t EndName = file.find_last_of("\\");
	file = file.substr(EndName + 1);
	return file;
}

// If newExtension == nullptr, only return name not extension
std::string ModuleFS::FixExtension(std::string file, const char* newExtension)
{
	size_t EndName = file.find_last_of(".");
	file = file.substr(0, EndName);
	if(newExtension != nullptr)
		file += newExtension;
	return file;
}

std::string ModuleFS::GetExtension(std::string file)
{
	size_t EndName = file.find_last_of(".");
	file = file.substr(EndName + 1);
	return file;
}

const char* ModuleFS::ConverttoConstChar(std::string name)
{
	char* temp = new char[name.length() + 1];
	strcpy(temp, name.c_str());
	temp[name.length()] = '\0';
	return temp;
}

std::string ModuleFS::GetAssetsDirecotry()
{
	return std::string();
}

std::string ModuleFS::AddDirectorybyType(std::string name, DIRECTORY_IMPORT directory)
{
	std::string temp = name;
	switch (directory)
	{
	case IMPORT_DEFAULT:
	{
		break;
	}
	case IMPORT_DIRECTORY_ASSETS:
	{
		temp = DIRECTORY_ASSETS + temp;
		break;
	}
	case IMPORT_DIRECTORY_LIBRARY:
	{
		temp = DIRECTORY_LIBRARY + temp;
		break;
	}
	case IMPORT_DIRECTORY_LIBRARY_MESHES:
	{
		temp = DIRECTORY_LIBRARY_MESHES + temp;
		break;
	}
	case IMPORT_DIRECTORY_LIBRARY_MATERIALS:
	{
		temp = DIRECTORY_LIBRARY_MATERIALS + temp;
		break;
	}
	}
	return temp;
}

std::string ModuleFS::CreateFolder(const char* file_name, bool forceCreate)
{
	namespace fs = std::experimental::filesystem;

	if (!fs::exists(file_name)) // Check if src folder exists
	{ 
		fs::create_directory(file_name); // create src folder
		return file_name;
	}
	else
	{
		if (forceCreate)
		{
			bool stop = false;
			int i = 1;
			std::string force;
			while (stop == false)
			{
				force = file_name;
				force += " " + std::to_string(i);
				if (!fs::exists(force.c_str()))
				{
					fs::create_directory(force.c_str());
					return force;
				}
				i++;
			}
		}
	}
}

void ModuleFS::CreateFolder(const char* file_name)
{
	namespace fs = std::experimental::filesystem;

	if (!fs::exists(file_name)) { // Check if src folder exists
		fs::create_directory(file_name); // create src folder
	}
}

// Special JSON Array Float3
JSON_Status ModuleFS::json_array_dotset_float3(JSON_Object *object, std::string name, float3 transform)
{
	JSON_Value* value = json_value_init_array();
	if (value == NULL) {
		return JSONFailure;
	}
	JSON_Array* arry = json_value_get_array(value);
	if (json_object_dotset_value(object, name.c_str(), value) == JSONFailure)
	{
		json_value_free(value);
		return JSONFailure;
	}
	json_array_append_number(arry, transform.x);
	json_array_append_number(arry, transform.y);
	json_array_append_number(arry, transform.z);
	return JSONSuccess;
}

JSON_Status ModuleFS::json_array_dotset_float2(JSON_Object *object, std::string name, float2 transform)
{
	JSON_Value* value = json_value_init_array();
	if (value == NULL) {
		return JSONFailure;
	}
	JSON_Array* arry = json_value_get_array(value);
	if (json_object_dotset_value(object, name.c_str(), value) == JSONFailure)
	{
		json_value_free(value);
		return JSONFailure;
	}
	json_array_append_number(arry, transform.x);
	json_array_append_number(arry, transform.y);
	return JSONSuccess;
}

JSON_Status ModuleFS::json_array_dotset_float4(JSON_Object *object, std::string name, float4 transform)
{
	JSON_Value* value = json_value_init_array();
	if (value == NULL) {
		return JSONFailure;
	}
	JSON_Array* arry = json_value_get_array(value);
	if (json_object_dotset_value(object, name.c_str(), value) == JSONFailure)
	{
		json_value_free(value);
		return JSONFailure;
	}
	json_array_append_number(arry, transform.x);
	json_array_append_number(arry, transform.y);
	json_array_append_number(arry, transform.z);
	json_array_append_number(arry, transform.w);
	return JSONSuccess;
}

float3 ModuleFS::json_array_dotget_float3_string(const JSON_Object* object, std::string name)
{
	JSON_Array* array = json_object_dotget_array(object, name.c_str());
	float3 transform;
	transform.x = (float)json_value_get_number(json_array_get_value(array, 0));
	transform.y = (float)json_value_get_number(json_array_get_value(array, 1));
	transform.z = (float)json_value_get_number(json_array_get_value(array, 2));

	return transform;
}
float2 ModuleFS::json_array_dotget_float2_string(const JSON_Object* object, std::string name)
{
	JSON_Array* array = json_object_dotget_array(object, name.c_str());
	float2 transform;
	transform.x = (float)json_value_get_number(json_array_get_value(array, 0));
	transform.y = (float)json_value_get_number(json_array_get_value(array, 1));

	return transform;
}
float4 ModuleFS::json_array_dotget_float4_string(const JSON_Object* object, std::string name)
{
	JSON_Array* array = json_object_dotget_array(object, name.c_str());
	float4 transform;
	transform.x = (float)json_value_get_number(json_array_get_value(array, 0));
	transform.y = (float)json_value_get_number(json_array_get_value(array, 1));
	transform.z = (float)json_value_get_number(json_array_get_value(array, 2));
	transform.w = (float)json_value_get_number(json_array_get_value(array, 3));

	return transform;
}
#include "ModuleImporter.h"
#include "ImportMesh.h"
#include "ModuleFS.h"
#include "GameObject.h"
#include "ImportMaterial.h"
#include "CompMesh.h"
#include "ResourceMesh.h"
#include "CompMaterial.h"
#include "CompTransform.h"
#include "ModuleTextures.h"

#include <filesystem>
#include <iostream>
#include <experimental/filesystem>
#include <fstream>

ImportMesh::ImportMesh()
{
}


ImportMesh::~ImportMesh()
{
}

bool ImportMesh::Load(const char* exported_file, Texture* resource)
{
	return false;
}

bool ImportMesh::Import(const aiScene* scene, const aiMesh* mesh, GameObject* obj, const char* name)
{
	bool ret = true;
	uint num_vertices = 0;
	uint num_indices = 0;
	uint num_normals = 0;
	uint num_textures = 0;

	float3* vertices = nullptr;
	uint* indices = nullptr;
	float3* vert_normals = nullptr;
	float2* tex_coords = nullptr;

	CompMesh* meshComp = (CompMesh*)obj->AddComponent(C_MESH);
	//ResourceMesh* resourceMesh = App->resource_manager->CreateNewResource(Resource::Type::MESH);

	if (mesh != nullptr)
	{
		LOG("Importing Mesh %s", name);
		
		// SET VERTEX DATA -------------------------------
		num_vertices = mesh->mNumVertices;
		vertices = new float3[num_vertices];
		memcpy(vertices, mesh->mVertices, sizeof(float3) * num_vertices);
		LOG("- Imported all vertex from data, total vertex: %i", num_vertices);

		// SET INDEX DATA -----------------------------------------
		if (mesh->HasFaces())
		{
			num_indices = mesh->mNumFaces * 3;
			indices = new uint[num_indices];
			for (uint i = 0; i < mesh->mNumFaces; i++)
			{
				if (mesh->mFaces[i].mNumIndices != 3)
				{
					LOG("WARNING, Geometry face with != 3 indices!");
				}
				else
				{
					memcpy(&indices[i * 3], mesh->mFaces[i].mIndices, sizeof(uint) * 3);
				}
			}
			LOG("- Imported all index from data, total indices: %i", num_indices);
		}
		else
		{
			LOG("- Mesh %s hasn't got Faces", mesh->mName.C_Str());
		}

		// SET NORMAL DATA -------------------------------
		if (mesh->HasNormals())
		{
			num_normals = num_vertices;
			vert_normals = new float3[num_normals];
			memcpy(vert_normals, mesh->mNormals, sizeof(float3) * num_normals);
			LOG("- Imported all Normals from data");
		}
		else
		{
			num_normals = 0;
			LOG("- Mesh %s hasn't got Normals", mesh->mName.C_Str());
		}

		// SET TEX COORD DATA -------------------------------
		//num_tex_coords = num_vertices;
		tex_coords = new float2[num_vertices];
		if (mesh->mTextureCoords[0])
		{
			for (uint i = 0; i < num_vertices; i++)
			{
				tex_coords[i].x = mesh->mTextureCoords[0][i].x;
				tex_coords[i].y = mesh->mTextureCoords[0][i].y;
			}
			//memcpy(tex_coords, mesh->mTextureCoords[0], sizeof(float2) * num_tex_coords);
			LOG("- Imported all Tex Coords from data");
		}
		else
		{
			for (uint i = 0; i < num_vertices; i++)
			{
				tex_coords[i] = float2(0, 0);
			}

			LOG("- Mesh %s hasn't got Tex Coords", mesh->mName.C_Str());
		}
		LOG("Imported all data");
	}
	else
	{
		LOG("Can't Import Mesh");
		ret = false;
	}

	
	// SET MATERIAL DATA -----------------------------------------
	if (mesh->mMaterialIndex >= 0)
	{
		CompMaterial* materialComp = (CompMaterial*)obj->AddComponent(C_MATERIAL);
		
		std::vector<Texture> text_t;
		aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];
		std::vector<Texture> diffuseMaps = LoadMaterialTextures(mat, aiTextureType_DIFFUSE, "texture_diffuse");
		text_t.insert(text_t.end(), diffuseMaps.begin(), diffuseMaps.end());

		// For the moment, we can only see textures on the diffuse channel, but we can load the specular ones
		std::vector<Texture> specularMaps = LoadMaterialTextures(mat, aiTextureType_SPECULAR, "texture_specular");
		text_t.insert(text_t.end(), specularMaps.begin(), specularMaps.end());

		materialComp->SetTexture(text_t);
		num_textures = text_t.size();
	}
	

	//meshComp->InitRanges(num_vertices, num_indices, num_normals);
	//meshComp->Init(vertices, indices, vert_normals, tex_coords);
	//meshComp->SetupMesh();
	meshComp->Enable();

	// Create Resource ----------------------
	ResourceMesh* res_mesh = (ResourceMesh*)App->resource_manager->CreateNewResource(Resource::Type::MESH);
	meshComp->SetResource(res_mesh);


	// ALLOCATING DATA INTO BUFFER ------------------------
	uint ranges[3] = { num_vertices, num_indices, num_normals }; //,num_tex_coords };

	uint size = sizeof(ranges) + sizeof(float3) *  num_vertices + sizeof(uint) * num_indices + sizeof(float3) *  num_normals + sizeof(float2) *  num_vertices;

	// Allocating all data 
	char* data = new char[size];
	char* cursor = data;

	// Storing Ranges
	uint bytes = sizeof(ranges);
	memcpy(cursor, ranges, bytes);

	// Storing Vertices
	cursor += bytes;
	bytes = sizeof(float3) * num_vertices;
	memcpy(cursor, vertices, bytes);

	// Storing Indices
	cursor += bytes;
	bytes = sizeof(uint) * num_indices;
	memcpy(cursor, indices, bytes);

	// Storing Normals
	cursor += bytes;
	bytes = sizeof(float3) * num_normals;
	memcpy(cursor, vert_normals, bytes);

	// Storing Tex Coords
	cursor += bytes;
	bytes = sizeof(float2) * num_vertices; //num_tex_coords;
	memcpy(cursor, tex_coords, bytes);

	// Release all pointers
	RELEASE_ARRAY(vertices);
	RELEASE_ARRAY(indices);
	RELEASE_ARRAY(vert_normals);
	RELEASE_ARRAY(tex_coords);
	//RELEASE(texture);

	uint uid_mesh = App->random->Int();
	std::string fileName = std::to_string(uid_mesh);
	res_mesh->InitInfo(uid_mesh, name);
	res_mesh->name = App->GetCharfromConstChar(name);
	//res_mesh->uuid_directory = fileName.c_str();
	//meshComp->SetUUIDMesh(uid_mesh);
	//Save Mesh
	App->fs->SaveFile(data, fileName, size, IMPORT_DIRECTORY_LIBRARY_MESHES);

	RELEASE_ARRAY(data);


	return ret;
}

void ImportMesh::Import(uint num_vertices, uint num_indices, uint num_normals, std::vector<uint> indices, std::vector<float3> vertices, uint uid)
{
	// ALLOCATING DATA INTO BUFFER ------------------------
	uint ranges[3] = { num_vertices, num_indices, num_normals }; //,num_tex_coords };

	uint size = sizeof(ranges) + sizeof(float3) *  num_vertices + sizeof(uint) * num_indices + sizeof(float3) *  num_normals + sizeof(float2) *  num_vertices;


	float3* vert_normals = nullptr;
	// Allocating all data 
	char* data = new char[size];
	char* cursor = data;

	// Storing Ranges
	uint bytes = sizeof(ranges);
	memcpy(cursor, ranges, bytes);

	// Storing Vertices
	float3* vertices_ = new float3[num_vertices];
	memcpy(vertices_, vertices.data(), sizeof(float3) * num_vertices);
	cursor += bytes;
	bytes = sizeof(float3) * num_vertices;
	memcpy(cursor, vertices_, bytes);

	// Storing Indices
	uint* indices_ = new uint[num_indices];
	memcpy(indices_, indices.data(), sizeof(uint) * num_indices);
	cursor += bytes;
	bytes = sizeof(uint) * num_indices;
	memcpy(cursor, indices_, bytes);

	// Storing Normals
	//cursor += bytes;
	//bytes = sizeof(float3) * num_normals;
	//memcpy(cursor, vert_normals, bytes);

	//// Storing Tex Coords
	//cursor += bytes;
	//bytes = sizeof(float2) * num_vertices; //num_tex_coords;
	//memcpy(cursor, tex_coords, bytes);

	// Release all pointers
	RELEASE_ARRAY(vertices_);
	RELEASE_ARRAY(indices_);
	RELEASE_ARRAY(vert_normals);
	indices.clear();
	vertices.clear();
	//RELEASE_ARRAY(tex_coords);
	//RELEASE(texture);

	// Create Resource ----------------------
	ResourceMesh* res_mesh = (ResourceMesh*)App->resource_manager->CreateNewResource(Resource::Type::MESH, uid);

	uint uid_mesh = App->random->Int();
	std::string fileName = std::to_string(uid_mesh);
	res_mesh->InitInfo(uid_mesh, "Cube");
	res_mesh->name = "Cube";

	//Save Mesh
	App->fs->SaveFile(data, fileName, size, IMPORT_DIRECTORY_LIBRARY_MESHES);

	RELEASE_ARRAY(data);
}

bool ImportMesh::LoadResource(const char* file, ResourceMesh* resourceMesh)
{
	char* buffer = nullptr;
	uint num_vertices = 0;
	uint num_indices = 0;
	uint num_normals = 0;
	//uint num_tex_coords = 0;

	float3* vertices = nullptr;
	uint* indices = nullptr;
	float3* vert_normals = nullptr;
	float2* tex_coords = nullptr;
	//Texture* texture = nullptr;
	// Loading File
	uint size = App->fs->LoadFile(file, &buffer, IMPORT_DIRECTORY_LIBRARY_MESHES);

	if (buffer != nullptr && size > 0)
	{
		char* cursor = buffer;

		// Amount vertices, amount indices, amount normals
		uint ranges[3];
		uint bytes = sizeof(ranges);
		memcpy(ranges, cursor, bytes);

		// Set Amounts
		num_vertices = ranges[0];
		num_indices = ranges[1];
		num_normals = ranges[2];
		//num_tex_coords = ranges[3];

		//Load Vertices
		cursor += bytes;
		bytes = sizeof(float3) * num_vertices;
		vertices = new float3[num_vertices];
		memcpy(vertices, cursor, bytes);

		//Load Indices
		cursor += bytes;
		bytes = sizeof(uint) * num_indices;
		indices = new uint[num_indices];
		memcpy(indices, cursor, bytes);

		//Load Normals
		if (num_normals > 0)
		{
			cursor += bytes;
			bytes = sizeof(float3) * num_normals;
			vert_normals = new float3[num_normals];
			memcpy(vert_normals, cursor, bytes);
		}

		//Load Tex Coords
		cursor += bytes;
		bytes = sizeof(float2) * num_vertices; //num_tex_coords;
		tex_coords = new float2[num_vertices];
		memcpy(tex_coords, cursor, bytes);

		resourceMesh->InitRanges(num_vertices, num_indices, num_normals);
		resourceMesh->Init(vertices, indices, vert_normals, tex_coords);
		resourceMesh->LoadToMemory();
		resourceMesh->isLoaded = true;

		LOG("Mesh %s Loaded!", file);
	}
	return true;
}

bool ImportMesh::Load(const char* file, CompMesh* mesh)
{
	char* buffer = nullptr;
	uint num_vertices = 0;
	uint num_indices = 0;
	uint num_normals = 0;
	//uint num_tex_coords = 0;

	float3* vertices = nullptr;
	uint* indices = nullptr;
	float3* vert_normals = nullptr;
	float2* tex_coords = nullptr;
	//Texture* texture = nullptr;
	// Loading File
	uint size = App->fs->LoadFile(file, &buffer, IMPORT_DIRECTORY_LIBRARY_MESHES);

	if (buffer != nullptr && size > 0)
	{
		char* cursor = buffer;

		// Amount vertices, amount indices, amount normals
		uint ranges[3];
		uint bytes = sizeof(ranges);
		memcpy(ranges, cursor, bytes);

		// Set Amounts
		num_vertices = ranges[0];
		num_indices = ranges[1];
		num_normals = ranges[2];
		//num_tex_coords = ranges[3];
		
		//Load Vertices
		cursor += bytes;
		bytes = sizeof(float3) * num_vertices;
		vertices = new float3[num_vertices];
		memcpy(vertices, cursor, bytes);
		
		//Load Indices
		cursor += bytes;
		bytes = sizeof(uint) * num_indices;
		indices = new uint[num_indices];
		memcpy(indices, cursor, bytes);

		//Load Normals
		cursor += bytes;
		bytes = sizeof(float3) * num_normals;
		vert_normals = new float3[num_normals];
		memcpy(vert_normals, cursor, bytes);

		//Load Tex Coords
		cursor += bytes;
		bytes = sizeof(float2) * num_vertices; //num_tex_coords;
		tex_coords = new float2[num_vertices];
		memcpy(tex_coords, cursor, bytes);
		
		//GameObject* gameobject = new GameObject();
		//CompMesh* mesh = (CompMesh*)gameobject->AddComponent(C_MESH);
		
		//mesh->InitRanges(num_vertices, num_indices, num_normals);
		//mesh->Init(vertices, indices, vert_normals, tex_coords);
		//mesh->SetupMesh();
		mesh->Enable();
		
		LOG("Mesh %s Loaded!", file);

		//App->scene->gameobjects.push_back(gameobject);
		
	}
	return true;
}

void ImportMesh::PrepareToImport()
{
	for (int i = 0; i < materialImpoted.size(); i++)
	{
		materialImpoted[i].name.clear();
		materialImpoted[i].path.clear();
		materialImpoted[i].name.clear();
	}
	materialImpoted.clear();
}

std::vector<Texture> ImportMesh::LoadMaterialTextures(aiMaterial* mat, aiTextureType type, const char* typeName)
{
	std::vector<Texture> textures;
	std::vector<Texture> l_tex;
	bool skip = false;
	for (uint i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString str;
		mat->GetTexture(type, i, &str);

		for (uint j = 0; j < l_tex.size(); j++)
		{
			if (std::strcmp(l_tex[j].path.c_str(), str.C_Str()) == 0)
			{
				textures.push_back(l_tex[j]);
				skip = true;
				break;
			}
		}

		if (skip == false)
		{
			// Check if the texture has been imported.
			// if so, only put the information (Dont import again).
			bool noImport = false;
			int num_texture = -1;
			if (materialImpoted.size() > 0)
			{
				for (int ds = 0; ds < materialImpoted.size(); ds++)
				{
					if (materialImpoted[ds].path.compare(str.C_Str()) == 0)
					{
						LOG("The texture was already imported!");
						noImport = true;
						num_texture = ds;
					}
				}
			}

			if (noImport == false)
			{
				//if Not imported, just import
				Texture tex;
				tex.id = App->textures->LoadTexture(str.C_Str());
				tex.type = typeName;
				tex.path = str.C_Str();

				uint uid_material = App->random->Int();
				tex.name = std::to_string(uid_material);

				App->importer->iMaterial->Import(tex.path.c_str(), tex.name.c_str());
				tex.name += ".dds";

				textures.push_back(tex);
				l_tex.push_back(tex);
				//delete importmaterial;

				materialImpoted.push_back(tex);
			}
			else
			{
				textures.push_back(materialImpoted[num_texture]);
				l_tex.push_back(materialImpoted[num_texture]);
			}
		}
	}

	return textures;
}
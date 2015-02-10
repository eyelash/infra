/*

Copyright © 2012-2015 Elias Aebi

All rights reserved.

*/

#include "infra.hpp"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <stdio.h>

namespace infra {

// Material
static void print_properties (aiMaterial* material) {
	for (int i=0; i<material->mNumProperties; i++) {
		aiMaterialProperty* property = material->mProperties[i];
		aiString key = property->mKey;
		printf ("%d: %s\n", i, key.C_Str());
	}
}
Material::Material (aiMaterial* material): colormap(NULL), normalmap(NULL) {
//	printf ("Material::Material: material properties:\n");
//	print_properties (material);
	if (material->GetTextureCount(aiTextureType_DIFFUSE)) {
		aiString texture_path;
		material->GetTexture (aiTextureType_DIFFUSE, 0, &texture_path);
		printf ("Material::Material(): found diffuse texture: %s\n", texture_path.C_Str());
		colormap = new Texture (texture_path.C_Str());
	}
	else {
		aiColor3D diffuse_color;
		material->Get (AI_MATKEY_COLOR_DIFFUSE, diffuse_color);
	//	printf ("Material::Material(): diffuse color: {%f, %f, %f}\n", diffuse_color.r, diffuse_color.g, diffuse_color.b);
		color = Color (diffuse_color);
		// emit
		material->Get (AI_MATKEY_COLOR_EMISSIVE, diffuse_color);
	//	printf ("Material::Material(): emissive color: {%f, %f, %f}\n", diffuse_color.r, diffuse_color.g, diffuse_color.b);
		color.r += diffuse_color.r * color.r;
		color.g += diffuse_color.g * color.g;
		color.b += diffuse_color.b * color.b;
	//	printf ("Material::Material(): final color: {%f, %f, %f}\n", color.r, color.g, color.b);
	}
	// normal map
	if (material->GetTextureCount(aiTextureType_NORMALS)) {
		aiString texture_path;
		material->GetTexture (aiTextureType_NORMALS, 0, &texture_path);
		printf ("Material::Material(): found normals texture: %s\n", texture_path.C_Str());
		normalmap = new Texture (texture_path.C_Str());
	}
	// create the program
	Shader* vs = new Shader ("shaders/vertex_shader.glsl", GL_VERTEX_SHADER);
	Shader* fs;
	if (colormap || normalmap)
		fs = new Shader ("shaders/material_texture.glsl", GL_FRAGMENT_SHADER);
	else
		fs = new Shader ("shaders/material.glsl", GL_FRAGMENT_SHADER);
	program = new Program (vs, fs);
}
Material::~Material () {
	delete colormap;
	delete normalmap;
	delete program;
}
void Material::activate () {
	program->use ();
	if (colormap) {
		colormap->bind (0);
		program->set_uniform_int ("colormap", 0);
	}
	else
		color.use ();
	if (normalmap) {
		normalmap->bind (1);
		program->set_uniform_int ("normalmap", 1);
	}
}
void Material::deactivate () {
	if (normalmap) {
		normalmap->unbind ();
	}
	if (colormap) {
		colormap->unbind ();
	}
}

// Mesh
Mesh::Mesh (aiMesh* mesh, const aiScene* scene): vertex_count(mesh->mNumVertices), buffer(vertex_count*4*sizeof(aiVector3D)), material(scene->mMaterials[mesh->mMaterialIndex]) {
	if (mesh->mPrimitiveTypes & ~aiPrimitiveType_TRIANGLE) {
		printf ("Mesh::Mesh: the mesh contains faces that are not triangles\n");
	}
	printf ("Mesh::Mesh: the mesh contains %d vertices and %d faces\n", vertex_count, mesh->mNumFaces);
	buffer.set_data (0, vertex_count*sizeof(aiVector3D), mesh->mVertices);
	buffer.set_data (vertex_count*sizeof(aiVector3D), vertex_count*sizeof(aiVector3D), mesh->mNormals);
	buffer.set_data (vertex_count*2*sizeof(aiVector3D), vertex_count*sizeof(aiVector3D), mesh->mTextureCoords[0]);
	buffer.set_data (vertex_count*3*sizeof(aiVector3D), vertex_count*sizeof(aiVector3D), mesh->mTangents);
}
void Mesh::draw () {
	material.activate ();
	buffer.bind ();
	
	// get the indices
	//int position_index = material.program->get_attribute_location ("in_position");
	int tangent_index = material.program->get_attribute_location ("in_tangent");
	//int normal_index = material.program->get_attribute_location ("in_normal");
	//int texcoord_index = material.program->get_attribute_location ("in_texcoord");
	
	// enable vertex arrays and set the sources
	glEnableClientState (GL_VERTEX_ARRAY);
	glEnableClientState (GL_NORMAL_ARRAY);
	glEnableClientState (GL_TEXTURE_COORD_ARRAY);
	glEnableVertexAttribArray (tangent_index);
	glVertexPointer (3, GL_FLOAT, 0, NULL);
	glNormalPointer (GL_FLOAT, 0, (void*)(vertex_count*sizeof(aiVector3D)));
	glTexCoordPointer (3, GL_FLOAT, 0, (void*)(vertex_count*2*sizeof(aiVector3D)));
	glVertexAttribPointer (tangent_index, 3, GL_FLOAT, GL_FALSE, 0, (void*)(vertex_count*3*sizeof(aiVector3D)));
	
	// do the actual drawing
	glDrawArrays (GL_TRIANGLES, 0, vertex_count);
	
	// disable vertex arrays
	glDisableClientState (GL_VERTEX_ARRAY);
	glDisableClientState (GL_NORMAL_ARRAY);
	glDisableClientState (GL_TEXTURE_COORD_ARRAY);
	glDisableVertexAttribArray (tangent_index);
	
	buffer.unbind ();
	material.deactivate ();
}

// Object
Object::Object (const char* obj_file) {
	// load the file
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile (obj_file, aiProcess_CalcTangentSpace|aiProcess_Triangulate);
	if (!scene) {
		fprintf (stderr, "Object::Object: an error occurred reading %s: %s\n", obj_file, importer.GetErrorString());
		return;
	}
	
	// create the meshes
	printf ("Object::Object: the file %s contains %d meshes\n", obj_file, scene->mNumMeshes);
	for (int i=0; i<scene->mNumMeshes; i++) {
		meshes.append (new Mesh(scene->mMeshes[i], scene));
	}
}

void Object::draw () {
	for (int i=0; i<meshes.count(); i++)
		meshes[i]->draw ();
}

// Instance
Instance::Instance (Object* object): object(object), position(0.0f,0.0f,0.0f), rotation(0.0f,0.0f,0.0f) {
	
}
Instance::Instance (Object* object, vec3 position): object(object), position(position), rotation(0.0f,0.0f,0.0f) {
	
}
void Instance::draw () {
	glPushMatrix ();
	glTranslatef (position.x, position.y, position.z);
	glRotatef (rotation.z, 0, 0, 1);
	glRotatef (rotation.y, 0, 1, 0);
	glRotatef (rotation.x, 1, 0, 0);
	object->draw ();
	glPopMatrix ();
}

// Scene
void Scene::draw () {
	for (int i=0; i<instances.count(); i++)
		instances[i]->draw ();
}

// Camera
Camera::Camera (Scene* scene, int width, int height): scene(scene), width(width), height(height), position(0.0f,0.0f,0.0f), track(NULL), max_distance(0.0f) {
	
}
static float get_angle (float x, float y) {
	if (y==0.0) {
		if (x==0) return 0.0f;
		else if (x>0) return M_PI * 0.5;
		else if (x<0) return M_PI * 1.5;
	}
	else if (y>0) return atan (x/y);
	else if (y<0) return atan (x/y) + M_PI;
}
void Camera::take_a_picture () {
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	Projection::perspective (-0.4, 0.4, -0.4/width*height, 0.4/width*height, 1, 1000);
	
	//glLoadIdentity ();
	if (track) {
		if (max_distance > 0.0f && length(track->position-position) > max_distance) {
			vec3 direction = track->position - position;
			float factor = (length(direction) - max_distance) / length(direction);
			position += direction * factor;
		}
		float dx = track->position.x - position.x;
		float dy = track->position.y - position.y;
		float dz = track->position.z - position.z;
		float tilt = get_angle (-dy, sqrt(dx*dx+dz*dz)) * (180.0/M_PI); // down
		float rotation = get_angle (dx, -dz) * (180.0/M_PI); // to the right
		glRotatef (tilt, 1, 0, 0);
		glRotatef (rotation, 0, 1, 0);
	}
	glTranslatef (-position.x, -position.y, -position.z);
	
	scene->draw ();
}
void Camera::set_resolution (int width, int height) {
	this->width = width;
	this->height = height;
}
void Camera::look_at (float x, float y, float z) {
	
}

}

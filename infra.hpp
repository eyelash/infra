/*

Copyright © 2012-2015 Elias Aebi

All rights reserved.

*/

#ifndef INFRA_HPP
#define INFRA_HPP

#include <assimp/scene.h>
#include "foundation.hpp"
#include "vlist.hpp"

namespace infra {

class ResourceManager {
	~ResourceManager ();
};

class Material {
	public:
	Color color;
	Texture* colormap;
	Texture* normalmap;
	Program* program;
	float hardness;
	float light_size;
	Material (aiMaterial* material);
	~Material ();
	void activate ();
	void deactivate ();
};

class Mesh {
	public:
	unsigned int vertex_count;
	Buffer buffer;
	Material material;
	Mesh (aiMesh* mesh, const aiScene* scene);
	void draw ();
};

class Object {
	static Program* material_program;
public:
	List<Mesh*> meshes;
	GLuint buffer;
	GLuint index_buffer;
	GLuint vertex_count;
	GLuint face_count;
	Texture* colormap;
	Texture* normalmap;
	Object (const char* filename);
	void draw ();
};

class Instance {
	Object* object;
protected:
	Instance () {}
public:
	Instance (Object* object);
	Instance (Object* object, vec3 position);
	vec3 position;
	vec3 rotation;
	virtual void draw ();
};

class Light {
	static Program* program;
	float size;
	Color color;
public:
	vec3 position;
	Light (float x, float y, float z);
	void draw (Texture* color, Texture* normal, Texture* positionmap);
};

class Scene {
	public:
	List<Instance*> instances;
	List<Light> lights;
	void draw ();
};

class Window {
public:
	Window ();
	~Window ();
};

class Camera {
protected:
	bool direct_rendering;
	int width, height;
public:
	vec3 position;
	Scene* scene;
	Instance* track;
	float max_distance;
	bool lock_y_axis;
	
	Camera (Scene* scene, int width, int height);
	virtual void take_a_picture ();
	void set_resolution (int width, int height);
	
	void look_at (float x, float y, float z);
};

class DeferredRenderingCamera: public Camera {
	FramebufferObject* target;
	Texture* normal_texture;
	Texture* position_texture;
	FramebufferObject* result;
public:
	DeferredRenderingCamera (Scene* scene, int width, int height);
	~DeferredRenderingCamera ();
	virtual void take_a_picture ();
};

class BloomEffect {
	public:
	FramebufferObject* intermediate_result;
	FramebufferObject* result;
	static Program* program;
	BloomEffect (int width, int height);
	void apply (Texture* input);
};

class DeferredRendering {
	FramebufferObject* target;
public:
	DeferredRendering ();
	~DeferredRendering ();
};

}

#endif // INFRA_HPP

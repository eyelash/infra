/*

Copyright © 2012-2014 Elias Aebi

All rights reserved.

*/

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <math.h>

#ifndef FOUNDATION_HPP
#define FOUNDATION_HPP

class Position {
	public:
	float x, y, z;
	Position (): x(0.0f), y(0.0f), z(0.0f) {}
	Position (float x, float y, float z): x(x), y(y), z(z) {}
};
float distance (const Position& p1, const Position& p2);

class vec3 {
	public:
	float x, y, z;
	vec3 () {}
	vec3 (float x, float y, float z): x(x), y(y), z(z) {}
	vec3 operator + (const vec3& v) {
		return vec3 (x+v.x, y+v.y, z+v.z);
	}
	vec3 operator - (const vec3& v) {
		return vec3 (x-v.x, y-v.y, z-v.z);
	}
	vec3 operator * (float s) {
		return vec3 (x*s, y*s, z*s);
	}
	float length () {
		return sqrt (x*x + y*y + z*z);
	}
	void operator += (const vec3& v) {
		x += v.x;
		y += v.y;
		z += v.z;
	}
};

class Color {
	public:
	float r, g, b, a;
	Color () {}
	Color (float r, float g, float b, float a = 1.0f): r(r), g(g), b(b), a(a) {}
	Color (aiColor3D c): r(c.r), g(c.g), b(c.b), a(1.0f) {}
	void use () {
		//printf ("setting the color to {%f, %f, %f, %f}\n", r, g, b, a);
		glColor4f (r, g, b, a);
	}
};

class Projection {
	public:
	static void perspective (double left, double right, double bottom, double top, double near, double far);
	static void orthographic (double left, double right, double bottom, double top, double near, double far);
};

class Program;
class Texture {
	int texture_unit;
public:
	GLuint identifier;
	int width, height;
	static Program* program;
	Texture (const char* filename);
	Texture (int width, int height, GLenum format);
	~Texture ();
	void bind (int texture_unit = 0);
	void unbind ();
	void draw (Program* program = NULL);
	void draw (float x, float y, float w, float h = 0.0f);
	void get_data (void* data);
	void debug_print ();
};

void draw_2_textures (Texture* t1, Texture* t2, Program* p);
void draw_3_textures (Texture* t1, Texture* t2, Texture* t3, Program* p);

class FramebufferObject {
	public:
	int width, height;
	GLuint identifier;
	GLuint depth_renderbuffer;
	Texture* color_texture;
	int color_attachments_count;
	Texture* depth_texture;
	//FramebufferObject (Texture* texture = NULL, bool depth = false);
	//FramebufferObject (Texture* color = NULL, Texture* depth = NULL);
	FramebufferObject (int width, int height, GLenum texture_format = GL_RGBA32F);
	~FramebufferObject ();
	void bind ();
	void unbind ();
	void attach_texture (Texture* texture);
};

class Shader {
	public:
	GLuint identifier;
	Shader (const char* filename, GLenum type);
	~Shader ();
};

class Program {
	public:
	GLuint identifier;
	Program (Shader* vertex_shader, Shader* fragment_shader);
	Program (const char* vertex_shader, const char* fragment_shader);
	Program ();
	~Program ();
	void attach_shader (Shader* shader);
	void link ();
	void use ();
	void set_uniform_int (const char* name, int value);
	void set_uniform_vec3 (const char* name, const vec3& value);
	int get_attribute_location (const char* name);
};

class Error {
	public:
	static void print (const char* origin);
};

#endif // FOUNDATION_HPP

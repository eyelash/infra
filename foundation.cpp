/*

Copyright © 2012-2014 Elias Aebi

All rights reserved.

*/

#include "infra.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <SOIL/SOIL.h>

// Distance
float distance (const Position& p1, const Position& p2) {
	return sqrt ((p2.x-p1.x)*(p2.x-p1.x) + (p2.y-p1.y)*(p2.y-p1.y) + (p2.z-p1.z)*(p2.z-p1.z));
}

// Projection
void Projection::perspective (double left, double right, double bottom, double top, double near, double far) {
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	glFrustum (left, right, bottom, top, near, far);
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();
}
void Projection::orthographic (double left, double right, double bottom, double top, double near, double far) {
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	glOrtho (left, right, bottom, top, near, far);
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();
}

// Texture
Program* Texture::program = NULL;
Texture::Texture (const char* filename) {
	identifier = SOIL_load_OGL_texture (filename, SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y|SOIL_FLAG_TEXTURE_REPEATS);
	if (identifier==0)
		fprintf (stderr, "Texture::Texture(): failed to load %s: %s\n", filename, SOIL_last_result());
	// anisotropic filtering
	glBindTexture (GL_TEXTURE_2D, identifier);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 4.0f);
	glBindTexture (GL_TEXTURE_2D, 0);
}
Texture::Texture (int width, int height, GLenum format): width(width), height(height), texture_unit(0) {
	// formats: GL_RGB8 (GL_RGB), GL_RGBA8 (GL_RGBA), GL_RGBA16F, GL_RGBA32F
	if (!program) {
		program = new Program ("shaders/vertex_shader.glsl", "shaders/texture_passthrough.glsl");
	}
	Error::print ("before Texture::Texture");
	glGenTextures (1, &identifier);
	glBindTexture (GL_TEXTURE_2D, identifier);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	// RGB
	if (format == GL_RGB)
		glTexImage2D (GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	// RGBA
	else if (format == GL_RGBA)
		glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	// 32 bit floating point
	else if (format == GL_RGB32F)
		glTexImage2D (GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	else if (format == GL_RGBA32F)
		glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	// 16 bit floating point
	// depth
	else if (format == GL_DEPTH_COMPONENT)
		glTexImage2D (GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
	else if (format == GL_DEPTH_COMPONENT32F)
		glTexImage2D (GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	else
		printf ("Texture::Texture: this format is not yet supported\n");
	glBindTexture (GL_TEXTURE_2D, 0);
	Error::print ("Texture::Texture");
}
Texture::~Texture () {
	glDeleteTextures (1, &identifier);
}
void Texture::bind (int texture_unit) {
	this->texture_unit = texture_unit;
	glActiveTexture (GL_TEXTURE0 + texture_unit);
	glBindTexture (GL_TEXTURE_2D, identifier);
}
void Texture::unbind () {
	glActiveTexture (GL_TEXTURE0 + texture_unit);
	glBindTexture (GL_TEXTURE_2D, 0);
}
void Texture::draw (Program* p) {
	if (!program) {
		program = new Program ("shaders/vertex_shader.glsl", "shaders/texture_passthrough.glsl");
	}
	
	GLint vertices[] = {
		0, 0,
		1, 0,
		1, 1,
		0, 1
	};
	GLint tex_coords[] = {
		0, 0,
		1, 0,
		1, 1,
		0, 1
	};
	
	Projection::orthographic (0, 1, 0, 1, -1, 1);
	glLoadIdentity ();
	
	// enable stuff
	if (p) {
		p->use ();
		p->set_uniform_int ("texture", 0);
	}
	else {
		program->use ();
		program->set_uniform_int ("texture", 0);
	}
	glEnableClientState (GL_VERTEX_ARRAY);
	glEnableClientState (GL_TEXTURE_COORD_ARRAY);
	bind ();
	glVertexPointer (2, GL_INT, 0, vertices);
	glTexCoordPointer (2, GL_INT, 0, tex_coords);
	glDisable (GL_DEPTH_TEST);
	
	// draw
	glDrawArrays (GL_QUADS, 0, 4);
	
	// disable stuff:
	unbind ();
	glDisableClientState (GL_VERTEX_ARRAY);
	glDisableClientState (GL_TEXTURE_COORD_ARRAY);
	glEnable (GL_DEPTH_TEST);
}
void Texture::draw (float x, float y, float w, float h) {
	if (h == 0.0f)
		h = w;
	
	if (!program) {
		program = new Program ("shaders/vertex_shader.glsl", "shaders/texture_passthrough.glsl");
	}
	
	GLfloat vertices[] = {
		x, y,
		x+w, y,
		x+w, y+h,
		x, y+h
	};
	GLfloat tex_coords[] = {
		0, 0,
		1, 0,
		1, 1,
		0, 1
	};
	
	Projection::orthographic (0, 1, 0, 1, -1, 1);
	//glLoadIdentity ();
	
	// enable stuff
	program->use ();
	program->set_uniform_int ("texture", 0);
	glEnableClientState (GL_VERTEX_ARRAY);
	glEnableClientState (GL_TEXTURE_COORD_ARRAY);
	bind ();
	glVertexPointer (2, GL_FLOAT, 0, vertices);
	glTexCoordPointer (2, GL_FLOAT, 0, tex_coords);
	glDisable (GL_DEPTH_TEST);
	
	// draw
	glDrawArrays (GL_QUADS, 0, 4);
	
	// disable stuff:
	unbind ();
	glDisableClientState (GL_VERTEX_ARRAY);
	glDisableClientState (GL_TEXTURE_COORD_ARRAY);
	glEnable (GL_DEPTH_TEST);
}
void Texture::get_data (void* data) {
	glGetTexImage (GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, data);
}
void Texture::debug_print () {
	GLfloat* buffer = (GLfloat*) malloc (width*height*4*sizeof(GLfloat));
	glBindTexture (GL_TEXTURE_2D, identifier);
	glGetTexImage (GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, buffer);
	glBindTexture (GL_TEXTURE_2D, 0);
	int index = height*3/4*width + width/2;
	printf ("Texture::debug_print: {%f, %f, %f, %f}\n", buffer[index], buffer[index+1], buffer[index+2], buffer[index+3]);
	free (buffer);
}

void draw_2_textures (Texture* t1, Texture* t2, Program* p) {
	GLint vertices[] = {
		0, 0,
		1, 0,
		1, 1,
		0, 1
	};
	GLint tex_coords[] = {
		0, 0,
		1, 0,
		1, 1,
		0, 1
	};
	
	Projection::orthographic (0, 1, 0, 1, -1, 1);
	
	// enable stuff
	if (p) p->use ();
	glEnableClientState (GL_VERTEX_ARRAY);
	glEnableClientState (GL_TEXTURE_COORD_ARRAY);
	glVertexPointer (2, GL_INT, 0, vertices);
	glTexCoordPointer (2, GL_INT, 0, tex_coords);
	glDisable (GL_DEPTH_TEST);
	
	// bind textures
	t1->bind (0);
	p->set_uniform_int ("t1", 0);
	t2->bind (1);
	p->set_uniform_int ("t2", 1);
	
	// draw
	glDrawArrays (GL_QUADS, 0, 4);
	
	// unbind textures
//	glActiveTexture (GL_TEXTURE1);
	t2->unbind ();
//	glActiveTexture (GL_TEXTURE0);
	t1->unbind ();
	
	// disable stuff:
	glDisableClientState (GL_VERTEX_ARRAY);
	glDisableClientState (GL_TEXTURE_COORD_ARRAY);
	glEnable (GL_DEPTH_TEST);
}
void draw_3_textures (Texture* t1, Texture* t2, Texture* t3, Program* p) {
	GLint vertices[] = {
		0, 0,
		1, 0,
		1, 1,
		0, 1
	};
	GLint tex_coords[] = {
		0, 0,
		1, 0,
		1, 1,
		0, 1
	};
	
	Projection::orthographic (0, 1, 0, 1, -1, 1);
	
	// enable stuff
	if (p) p->use ();
	glEnableClientState (GL_VERTEX_ARRAY);
	glEnableClientState (GL_TEXTURE_COORD_ARRAY);
	glVertexPointer (2, GL_INT, 0, vertices);
	glTexCoordPointer (2, GL_INT, 0, tex_coords);
	glDisable (GL_DEPTH_TEST);
	
	// bind textures
	t1->bind (0);
	p->set_uniform_int ("t1", 0);
	t2->bind (1);
	p->set_uniform_int ("t2", 1);
	t3->bind (2);
	p->set_uniform_int ("t3", 2);
	
	// draw
	glDrawArrays (GL_QUADS, 0, 4);
	
	// unbind textures
//	glActiveTexture (GL_TEXTURE2);
	t3->unbind ();
//	glActiveTexture (GL_TEXTURE1);
	t2->unbind ();
//	glActiveTexture (GL_TEXTURE0);
	t1->unbind ();
	
	// disable stuff:
	glDisableClientState (GL_VERTEX_ARRAY);
	glDisableClientState (GL_TEXTURE_COORD_ARRAY);
	glEnable (GL_DEPTH_TEST);
}

// FramebufferObject
/*
FramebufferObject::FramebufferObject (Texture* texture, bool depth) {
	glGenFramebuffers (1, &identifier);
	glBindFramebuffer (GL_FRAMEBUFFER, identifier);
	if (texture) {
		glFramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture->identifier, 0);
		width = texture->width;
		height = texture->height;
	}
	if (depth) {
		glGenRenderbuffers (1, &depth_renderbuffer);
		glBindRenderbuffer (GL_RENDERBUFFER, depth_renderbuffer);
		glRenderbufferStorage (GL_RENDERBUFFER, GL_DEPTH_COMPONENT, texture->width, texture->height);
		
		glFramebufferRenderbuffer (GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_renderbuffer);
	}
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		printf ("FramebufferObject::FramebufferObject: error\n");
	glBindFramebuffer (GL_FRAMEBUFFER, 0);
}
FramebufferObject::FramebufferObject (Texture* texture, Texture* depth) {
	glGenFramebuffers (1, &identifier);
	glBindFramebuffer (GL_FRAMEBUFFER, identifier);
	if (texture) {
		glFramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture->identifier, 0);
		width = texture->width;
		height = texture->height;
	}
	if (depth) {
		glFramebufferTexture2D (GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth->identifier, 0);
	}
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		printf ("FramebufferObject::FramebufferObject: error\n");
	glBindFramebuffer (GL_FRAMEBUFFER, 0);
}
*/
FramebufferObject::FramebufferObject (int width, int height, GLenum texture_format): width(width), height(height), color_attachments_count(1) {
	glGenFramebuffers (1, &identifier);
	glBindFramebuffer (GL_FRAMEBUFFER, identifier);
	color_texture = new Texture (width, height, texture_format);
	glFramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_texture->identifier, 0);
	depth_texture = new Texture (width, height, GL_DEPTH_COMPONENT);
	glFramebufferTexture2D (GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_texture->identifier, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		printf ("FramebufferObject::FramebufferObject: error\n");
	glBindFramebuffer (GL_FRAMEBUFFER, 0);
}
FramebufferObject::~FramebufferObject () {
	glDeleteFramebuffers (1, &identifier);
}
void FramebufferObject::bind () {
	glBindFramebuffer (GL_FRAMEBUFFER, identifier);
	glViewport (0, 0, width, height);
	glClear (GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
//	glLoadIdentity ();
//	printf ("FramebufferObject::bind: color_attachments_count == %d\n", color_attachments_count);
	if (color_attachments_count > 4)
		printf ("FramebufferObject::bind: more than 4 attachments are not yet supported\n");
	else if (color_attachments_count > 1) {
		GLenum buffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
		glDrawBuffers (color_attachments_count, buffers);
	}
	/*else if (color_attachments_count == 1) {
		GLenum buffers[] = {GL_BACK_BUFFER};
		glDrawBuffers (1, buffers);
	}*/
}
void FramebufferObject::unbind () {
	glBindFramebuffer (GL_FRAMEBUFFER, 0);
}
void FramebufferObject::attach_texture (Texture* texture) {
	glBindFramebuffer (GL_FRAMEBUFFER, identifier);
	glFramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + color_attachments_count++, GL_TEXTURE_2D, texture->identifier, 0);
	glBindFramebuffer (GL_FRAMEBUFFER, 0);
}

// Shader
Shader::Shader (const char* filename, GLenum type) {
	FILE* file = fopen (filename, "r");
	if (!file) {
		fprintf (stderr, "Shader::Shader(): could not find the file %s\n", filename);
		return;
	}
	// determine the length
	fseek (file, 0, SEEK_END);
	int length = ftell (file);
	rewind (file);
	// allocate memory and read the file
	char* source = (GLchar*) malloc (length);
	fread (source, 1, length, file);
	
	identifier = glCreateShader (type);
	const GLchar* const_source = source;
	glShaderSource (identifier, 1, &const_source, &length);
	glCompileShader (identifier);
	GLint compile_status;
	glGetShaderiv (identifier, GL_COMPILE_STATUS, &compile_status);
	if (compile_status == GL_FALSE) {
		GLint log_length;
		glGetShaderiv (identifier, GL_INFO_LOG_LENGTH, &log_length);
		char* log = (char*) malloc (log_length);
		glGetShaderInfoLog (identifier, log_length, NULL, log);
		printf ("the following errors occurred during the compilation of %s:\n%s\n", filename, log);
		free (log);
	}
	
	free (source);
	fclose (file);
}
Shader::~Shader () {
	glDeleteShader (identifier);
}

// Program
Program::Program (Shader* vertex_shader, Shader* fragment_shader) {
	identifier = glCreateProgram ();
	glAttachShader (identifier, vertex_shader->identifier);
	glAttachShader (identifier, fragment_shader->identifier);
	glLinkProgram (identifier);
	// add error handling here
}
Program::Program (const char* vertex_shader, const char* fragment_shader) {
	identifier = glCreateProgram ();
	Shader* v = new Shader (vertex_shader, GL_VERTEX_SHADER);
	Shader* f = new Shader (fragment_shader, GL_FRAGMENT_SHADER);
	glAttachShader (identifier, v->identifier);
	glAttachShader (identifier, f->identifier);
	glLinkProgram (identifier);
	// add error handling here
}
Program::Program () {
	identifier = glCreateProgram ();
}
Program::~Program () {
	glDeleteProgram (identifier);
}
void Program::attach_shader (Shader* shader) {
	glAttachShader (identifier, shader->identifier);
}
void Program::link () {
	glLinkProgram (identifier);
	// add error handling here
}
void Program::use () {
	glUseProgram (identifier);
}
void Program::set_uniform_int (const char* name, int value) {
	GLint location = glGetUniformLocation (identifier, name);
	glUniform1i (location, value);
}
void Program::set_uniform_vec3 (const char* name, const vec3& value) {
	GLint location = glGetUniformLocation (identifier, name);
	glUniform3f (location, value.x, value.y, value.z);
}
int Program::get_attribute_location (const char* name) {
	return glGetAttribLocation (identifier, name);
}

// Error
void Error::print (const char* origin) {
	GLenum error = glGetError ();
	if (error == GL_NO_ERROR)
		return;
	else if (error == GL_INVALID_ENUM)
		fprintf (stderr, "%s GL_INVALID_ENUM\n", origin);
	else if (error == GL_INVALID_VALUE)
		fprintf (stderr, "%s GL_INVALID_VALUE\n", origin);
	else if (error == GL_INVALID_OPERATION)
		fprintf (stderr, "%s GL_INVALID_OPERATION\n", origin);
	else if (error == GL_INVALID_FRAMEBUFFER_OPERATION)
		fprintf (stderr, "%s GL_INVALID_FRAMEBUFFER_OPERATION\n", origin);
	else if (error == GL_OUT_OF_MEMORY)
		fprintf (stderr, "%s GL_OUT_OF_MEMORY\n", origin);
	else if (error == GL_STACK_UNDERFLOW)
		fprintf (stderr, "%s GL_STACK_UNDERFLOW\n", origin);
	else if (error == GL_STACK_OVERFLOW)
		fprintf (stderr, "%s GL_STACK_OVERFLOW\n", origin);
	else
		fprintf (stderr, "%s unknown error\n", origin);
}

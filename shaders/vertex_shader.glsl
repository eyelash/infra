/*

Copyright © 2012-2014 Elias Aebi

All rights reserved.

*/

attribute vec3 tangent;
varying mat3 TBN;
varying vec4 real_position;
//varying vec4 screen_position;

void main () {
	gl_Position = ftransform ();
	gl_FrontColor = gl_Color;
	gl_BackColor = gl_Color;
	gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
	
	// TBN
	vec3 normal = normalize (gl_NormalMatrix * gl_Normal);
	vec3 _tangent = normalize (gl_NormalMatrix * tangent);
	vec3 binormal = cross (normal, _tangent);
	TBN = mat3 (_tangent, binormal, normal);
	
	// real position
	real_position = gl_ModelViewMatrix * gl_Vertex;
	
	// screen position
	//screen_position = ftransform() / ftransform().w;
	//screen_position *= (1.0/screen_position.z);
	//screen_position.z = (gl_ModelViewMatrix * gl_Vertex).z;
}

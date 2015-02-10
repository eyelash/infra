/*

Copyright © 2012-2014 Elias Aebi

All rights reserved.

*/

attribute vec3 in_tangent;
varying mat3 TBN;
varying vec4 real_position;

void main () {
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	gl_FrontColor = gl_Color;
	gl_BackColor = gl_Color;
	gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
	
	// TBN
	vec3 normal = normalize (gl_NormalMatrix * gl_Normal);
	vec3 tangent = normalize (gl_NormalMatrix * in_tangent);
	vec3 binormal = cross (normal, tangent);
	TBN = mat3 (tangent, binormal, normal);
	
	// real position
	real_position = gl_ModelViewMatrix * gl_Vertex;
}

/*

Copyright © 2012-2015 Elias Aebi

All rights reserved.

*/

varying mat3 TBN;
varying vec4 real_position;
uniform sampler2D colormap;
uniform sampler2D normalmap;

float angle (const in vec3 v1, const in vec3 v2) {
	return acos( dot(v1,v2) / (length(v1)*length(v2)) );
}

vec4 diffuse (const in vec4 color, const in vec3 normal, const in vec3 light) {
	return color * (dot (normal, light) * 0.5 + 0.75);
}

vec4 specular (const in vec4 color, const in vec3 normal, const in vec3 light, const in vec3 eye) {
	float a = angle (reflect(light,normal), eye);
	if (degrees(a) < 30.0)
		return color * (cos (6.0*a) * 0.5 + 0.5);
	else
		return vec4 (0.0);
}

vec4 fog (const in vec4 color, const in float distance, const in vec4 fog_color) {
	return mix (fog_color, color, pow(0.99,distance));
}

void main () {
	vec4 color = texture2D (colormap, gl_TexCoord[0].st);
	// split the normalmap
	vec4 normalmap_sample = texture2D (normalmap, gl_TexCoord[0].st);
	float specular_coefficient = normalmap_sample.a;
	vec3 normal = normalize (TBN * (normalmap_sample.rgb * 2.0 - 1.0));
	// diffuse
	gl_FragColor = diffuse (color, normal, vec3(0.0,0.0,1.0));
	// specular
	gl_FragColor += specular (color, normal, vec3(0.0,0.0,1.0), real_position.xyz) * specular_coefficient;
	// fog
	gl_FragColor = fog (gl_FragColor, -real_position.z, vec4(0.9,0.9,0.9,1.0));
}

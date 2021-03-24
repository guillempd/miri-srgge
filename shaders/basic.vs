#version 330 core

in vec3 mPos;
in vec3 mNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalMatrix;

out vec3 eNormal;

void main()
{
  // Transform matrix to viewspace
  eNormal = normalMatrix * mNormal;
	gl_Position = projection * view * model * vec4(mPos, 1.0);
}


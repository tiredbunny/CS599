
#version 450

vec3 points[3] = vec3[]( vec3(-1,-1,0), vec3(-1,3,0), vec3(3,-1,0) );

void main()
{
  gl_Position.xyz = points[gl_VertexIndex];
}

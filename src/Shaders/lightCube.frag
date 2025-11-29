#version 330 core
out vec4 FragColor;
in vec3 FragPos;

uniform vec3 color1;
uniform vec3 color2;

void main()
{
  if (FragPos.x > 0) { 
    FragColor = vec4(color1, 1.0);
  }
  else {
    FragColor = vec4(color2, 1.0);
  }
}

#version 330 core
out vec4 FragColor;

in vec2 nuestrasTexturas;

//ejemplos de texturas
uniform sampler2D textura1;
uniform sampler2D textura2;

void main()
{
	FragColor = mix(texture(textura1, nuestrasTexturas), texture(textura2, nuestrasTexturas), 0.4);
}
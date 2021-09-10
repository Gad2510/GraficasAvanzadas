#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <glm/glm.hpp> //operaciones avanzadas
#include <glm/gtc/matrix_transform.hpp> //transformaciones matriciales
#include <glm/gtc/type_ptr.hpp> //transformaciones de tipos de datos

#include <shader/shader.h>
#include <camara/camara.h>

#include <iostream>
#include <vector>

using namespace std;

/*
Tenemos luz:
	-Ambiental: luz homogénea, que se va reflejar en un ángulo de 90° que va a iluminar todos los objetos de nuestro shader
	-Especular: es un punto de luz concentrado dentro de la figura. Entre más grande sea el valor, se concentrará más en un punto
	-combinada: Para crear efectos mejores
*/

void framebuffer_size_callback(GLFWwindow* window, int width, int height); //almacenar nuestra ventana y configs
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window); //acciones para nuestra ventana
unsigned int loadCubemap(vector<string> faces);
unsigned int loadTexture(string path, int type);

//medidas de la pantalla
const unsigned int WIDTH = 800;
const unsigned int HEIGHT = 600;

//camara
Camara camera(glm::vec3(0.0f, 1.0f, 6.0f));
float lastX = WIDTH / 2.0f;
float lastY = HEIGHT / 2.0f;
bool firstMouse = true;

//timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

//Punto donde estará posicionada nuestra luz
glm::vec3 posLuz(0.0f, 4.0f, 5.0f);

int main() {
	//inicializar glfw
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	//creamos nuestra ventana
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "TAMAL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Fallo en gcrear GLFW y la ventana" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	//decirle a glfw que va a recibir señales de mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	//comprobar que glad se este corriendo o se haya inicializado correctamente
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "no se esta ejecutando el alegre" << std::endl;
		return -1;
	}

	glEnable(GL_DEPTH_TEST); //comprobacion del buffer z

	Shader nuestroShader("luz.vs", "luz.fs");
	Shader luzShader("cubito.vs", "cubito.fs");
	Shader skyboxShader("skybox.vs", "skybox.fs");

	float vertices[]{
		//posiciones		//cómo afecta
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
	};

	unsigned int indices[]
	{
		 0, 1, 2,
		3, 4, 5,
		6, 7, 8,
		9, 10, 11,
		12, 13, 14,
		14, 16, 17,
		18, 19, 20,
		21, 22, 23,
		24, 25, 26,
		27, 28, 29,
		30, 31, 32,
		33, 34, 35
	};

	float verticesSkybox[] = {
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};

	//configuramos nuestro skybox
	unsigned int sVAO, sVBO;
	glGenVertexArrays(1, &sVAO);
	glGenBuffers(1, &sVBO);
	glBindVertexArray(sVAO);
	glBindBuffer(GL_ARRAY_BUFFER, sVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verticesSkybox), &verticesSkybox, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);


	

	vector<string> faces
	{
		("skybox/right.jpg"),
		("skybox/left.jpg"),
		("skybox/top.jpg"),
		("skybox/bottom.jpg"),
		("skybox/front.jpg"),
		("skybox/back.jpg")
	};

	unsigned int cubemapTexture = loadCubemap(faces);

	//configurar el shader del skybox
	skyboxShader.use();
	skyboxShader.setInt("skybox", 0);

	unsigned int VBO, VAO, EBO; //Vertex Buffer Object, Vertex Array Object y Extendet Array Object
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	//unir o linkear
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	//posiciones
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	//luces
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	//segundo shader
	unsigned int VAO2;
	glGenVertexArrays(1, &VAO2);
	glBindVertexArray(VAO2);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);

	glEnableVertexAttribArray(0);

	float vertices_Granero[]{
		//posiciones		//cómo afecta
		-2.0f,  2.0f, 2.5f,  0.0f,  0.0f, 1.0f, 0.0f,   0.5f, //0
		-2.0f, -2.0f, 2.5f,  0.0f,  0.0f, 1.0f, 0.0f,   0.0f, //1
		 2.0f,  2.0f, 2.5f,  0.0f,  0.0f, 1.0f, 0.5f,   0.5f, //2
		 2.0f, -2.0f, 2.5f,  0.0f,  0.0f, 1.0f, 0.5f,   0.0f, //3
		-1.7f,  2.5f, 2.5f,  0.0f,  0.0f, 1.0f, 0.062f, 0.66f,//4
		 1.7f,  2.5f, 2.5f,  0.0f,  0.0f, 1.0f, 0.438f, 0.66f,//5
		-1.2f,  3.0f, 2.5f,  0.0f,  0.0f, 1.0f, 0.125f, 0.82f,//6
		 1.2f,  3.0f, 2.5f,  0.0f,  0.0f, 1.0f, 0.375f, 0.82f,//7
		 0.0f,  3.2f, 2.5f,  0.0f,  0.0f, 1.0f, 0.25f,  1.0f,  //8

		-2.0f,  2.0f,-2.5f,  0.0f,  0.0f, -1.0f, 0.0f,   0.5f, //9
		-2.0f, -2.0f,-2.5f,  0.0f,  0.0f, -1.0f, 0.0f,   0.0f, //10
		 2.0f,  2.0f,-2.5f,  0.0f,  0.0f, -1.0f, 0.5f,   0.5f, //11
		 2.0f, -2.0f,-2.5f,  0.0f,  0.0f, -1.0f, 0.5f,   0.0f, //12
		-1.7f,  2.5f,-2.5f,  0.0f,  0.0f, -1.0f, 0.062f, 0.66f,//13
		 1.7f,  2.5f,-2.5f,  0.0f,  0.0f, -1.0f, 0.438f, 0.66f,//14
		-1.2f,  3.0f,-2.5f,  0.0f,  0.0f, -1.0f, 0.125f, 0.82f,//15
		 1.2f,  3.0f,-2.5f,  0.0f,  0.0f, -1.0f, 0.375f, 0.82f,//16
		 0.0f,  3.2f,-2.5f,  0.0f,  0.0f, -1.0f, 0.25f,  1.0f,  //17

		-2.0f,  2.0f, 2.5f,  1.0f,  0.0f, 0.0f, 0.5f,   0.5f, //18
		-2.0f, -2.0f, 2.5f,  1.0f,  0.0f, 0.0f, 0.5f,   0.0f, //19
		 2.0f,  2.0f, 2.5f, -1.0f,  0.0f, 0.0f, 0.5f,   0.5f, //20
		 2.0f, -2.0f, 2.5f, -1.0f,  0.0f, 0.0f, 0.5f,   0.0f, //21
		-1.7f,  2.5f, 2.5f,  1.0f,  0.0f, 0.0f, 0.5f,   0.66f,//22
		 1.7f,  2.5f, 2.5f, -1.0f,  0.0f, 0.0f, 0.5f,   0.66f,//23
		-1.2f,  3.0f, 2.5f,  1.0f,  0.0f, 0.0f, 0.5f,   0.82f,//24
		 1.2f,  3.0f, 2.5f, -1.0f,  0.0f, 0.0f, 0.5f,   0.82f,//25
		 0.0f,  3.2f, 2.5f,  1.0f,  0.0f, 0.0f, 0.5f,   1.0f, //26
									  
		-2.0f,  2.0f,-2.5f,  1.0f,  0.0f, 0.0f, 1.0f,   0.5f, //27
		-2.0f, -2.0f,-2.5f,  1.0f,  0.0f, 0.0f, 1.0f,   0.0f, //28
		 2.0f,  2.0f,-2.5f, -1.0f,  0.0f, 0.0f, 1.0f,   0.5f, //29
		 2.0f, -2.0f,-2.5f, -1.0f,  0.0f, 0.0f, 1.0f,   0.0f, //30
		-1.7f,  2.5f,-2.5f,  1.0f,  0.0f, 0.0f, 1.0f,   0.66f,//31
		 1.7f,  2.5f,-2.5f, -1.0f,  0.0f, 0.0f, 1.0f,   0.66f,//32
		-1.2f,  3.0f,-2.5f,  1.0f,  0.0f, 0.0f, 1.0f,   0.82f,//33
		 1.2f,  3.0f,-2.5f, -1.0f,  0.0f, 0.0f, 1.0f,   0.82f,//34
		 0.0f,  3.2f,-2.5f,  1.0f,  0.0f, 0.0f, 1.0f,   1.0f, //35
	};

	unsigned int indices_Granero[]
	{
		//Front
		0, 3, 1,
		0, 3, 2,
		4, 2, 0,
		4, 2, 5,
		6, 5, 4,
		6, 5, 7,
		6, 7, 8,

		//Back
		9,12,10,
		9,12,11,
		13,11,9,
		13,11,14,
		15,14,13,
		15,14,16,
		15,16,17,

		//Left
		18,27,28,
		18,28,19,
		22,31,27,
		22,27,18,
		24,33,31,
		24,31,22,
		26,35,33,
		26,33,24,

		//Right
		20,29,30,
		20,30,21,
		23,32,29,
		23,29,20,
		25,34,32,
		25,32,23,
		26,35,34,
		26,34,25
	};

	unsigned int VBO_Granero, VAO_Granero, EBO_Granero; //Vertex Buffer Object, Vertex Array Object y Extendet Array Object
	glGenVertexArrays(1, &VAO_Granero);
	glGenBuffers(1, &VBO_Granero);
	glGenBuffers(1, &EBO_Granero);
	//unir o linkear
	glBindVertexArray(VAO_Granero);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_Granero);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_Granero), vertices_Granero, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_Granero);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices_Granero), indices_Granero, GL_STATIC_DRAW);
	//posiciones
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	//luces
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	//uv
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	//segundo shader
	unsigned int VAO2_Granero;
	glGenVertexArrays(1, &VAO2_Granero);
	glBindVertexArray(VAO2_Granero);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_Granero);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_Granero);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);

	glEnableVertexAttribArray(0);

	float vertices_GraneroAla[]{
		//posiciones		//cómo afecta
		//Frente
		-0.75f,  0.0f, 2.0f,  0.0f,  0.0f, 1.0f,   0.5f ,0.0f,//0
		 0.75f,  0.0f, 2.0f,  0.0f,  0.0f, 1.0f,   0.0f ,0.0f,//1
		 0.75f,  1.5f, 2.0f,  0.0f,  0.0f, 1.0f,   0.5f ,0.5f,//2
		-0.75f,  2.0f, 2.0f,  0.0f,  0.0f, 1.0f,   0.0f ,0.5f,//3
		//Atras
		-0.75f,  0.0f, -2.0f,  0.0f,  0.0f, -1.0f, 0.5f ,0.0f,//4
		 0.75f,  0.0f, -2.0f,  0.0f,  0.0f, -1.0f, 0.0f ,0.0f,//5
		 0.75f,  1.5f, -2.0f,  0.0f,  0.0f, -1.0f, 0.5f ,0.5f,//6
		-0.75f,  2.0f, -2.0f,  0.0f,  0.0f, -1.0f, 0.0f ,0.5f,//7
		//Arriba
		-0.75f,  2.0f, 2.0f,  0.0f,  1.0f, 0.0f,  1.0f ,0.5f,//8
		 0.75f,  1.5f, 2.0f,  0.0f,  1.0f, 0.0f,  0.5f ,0.5f,//9
		-0.75f,  2.0f,-2.0f,  0.0f,  1.0f, 0.0f,  1.0f ,1.0f,//10
		 0.75f,  1.5f,-2.0f,  0.0f,  1.0f, 0.0f,  0.5f ,1.0f,//11
		 //Derecha
		 0.75f,  0.0f, 2.0f,  -1.0f,  0.0f, 0.0f, 0.5f ,0.0f,//12
		 0.75f,  1.5f, 2.0f,  -1.0f,  0.0f, 0.0f, 0.0f ,0.0f,//13
		 0.75f,  0.0f, -2.0f, -1.0f,  0.0f, 0.0f, 0.5f ,0.5f,//14
		 0.75f,  1.5f, -2.0f, -1.0f,  0.0f, 0.0f, 0.0f ,0.5f,//15
	};

	unsigned int indices_GraneroAla[]
	{
		//Front
		0, 1, 3,
		3,1,2,
		//Back
		4,5,7,
		7,5,6,
		//UP
		8,9,10,
		10,9,11,
		//Right
		12,14,13,
		13,14,15
	};

	unsigned int VBO_GraneroAla, VAO_GraneroAla, EBO_GraneroAla; //Vertex Buffer Object, Vertex Array Object y Extendet Array Object
	glGenVertexArrays(1, &VAO_GraneroAla);
	glGenBuffers(1, &VBO_GraneroAla);
	glGenBuffers(1, &EBO_GraneroAla);
	//unir o linkear
	glBindVertexArray(VAO_GraneroAla);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_GraneroAla);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_GraneroAla), vertices_GraneroAla, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_GraneroAla);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices_GraneroAla), indices_GraneroAla, GL_STATIC_DRAW);
	//posiciones
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	//luces
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	//uv
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	//segundo shader
	unsigned int VAO2_GraneroAla;
	glGenVertexArrays(1, &VAO2_GraneroAla);
	glBindVertexArray(VAO2_GraneroAla);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_GraneroAla);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_GraneroAla);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);

	glEnableVertexAttribArray(0);


	float vertices_Cubo[]{
		//posiciones		//cómo afecta		//Tex
		//Frente
		 0.5f, -0.5f, 0.5f,  0.0f,  0.0f, 1.0f, 1.0f ,0.0f,//0
		-0.5f, -0.5f, 0.5f,  0.0f,  0.0f, 1.0f, 0.0f ,0.0f,//1
		 0.5f,  0.5f, 0.5f,  0.0f,  0.0f, 1.0f, 1.0f ,1.0f,//2
		-0.5f,  0.5f, 0.5f,  0.0f,  0.0f, 1.0f, 0.0f ,1.0f,//3
		//Atras									
		 0.5f, -0.5f,-0.5f,  0.0f,  0.0f,-1.0f, 1.0f ,0.0f,//4
		-0.5f, -0.5f,-0.5f,  0.0f,  0.0f,-1.0f, 0.0f ,0.0f,//5
		 0.5f,  0.5f,-0.5f,  0.0f,  0.0f,-1.0f, 1.0f ,1.0f,//6
		-0.5f,  0.5f,-0.5f,  0.0f,  0.0f,-1.0f, 0.0f ,1.0f,//7
		//Arriba							
		 0.5f,  0.5f,-0.5f,  0.0f,  1.0f, 0.0f, 1.0f ,0.0f,//8
		-0.5f,  0.5f,-0.5f,  0.0f,  1.0f, 0.0f, 0.0f ,0.0f,//9
		 0.5f,  0.5f, 0.5f,  0.0f,  1.0f, 0.0f, 1.0f ,1.0f,//10
		-0.5f,  0.5f, 0.5f,  0.0f,  1.0f, 0.0f, 0.0f ,1.0f,//11
		//Abajo							
		 0.5f, -0.5f,-0.5f,  0.0f, -1.0f, 0.0f, 1.0f ,0.0f,//12
		-0.5f, -0.5f,-0.5f,  0.0f, -1.0f, 0.0f, 0.0f ,0.0f,//13
		 0.5f, -0.5f, 0.5f,  0.0f, -1.0f, 0.0f, 1.0f ,1.0f,//14
		-0.5f, -0.5f, 0.5f,  0.0f, -1.0f, 0.0f, 0.0f ,1.0f,//15
		 //Derecha							
		 0.5f,  0.5f,-0.5f,  1.0f,  0.0f, 0.0f, 1.0f ,0.0f,//16
		 0.5f, -0.5f,-0.5f,  1.0f,  0.0f, 0.0f, 0.0f ,0.0f,//17
		 0.5f,  0.5f, 0.5f,  1.0f,  0.0f, 0.0f, 1.0f ,1.0f,//18
		 0.5f, -0.5f, 0.5f,  1.0f,  0.0f, 0.0f, 0.0f ,1.0f,//19
		 //Izquierda						
		-0.5f,  0.5f,-0.5f, -1.0f,  0.0f, 0.0f, 1.0f ,0.0f,//20
		-0.5f, -0.5f,-0.5f, -1.0f,  0.0f, 0.0f, 0.0f ,0.0f,//21
		-0.5f,  0.5f, 0.5f, -1.0f,  0.0f, 0.0f, 1.0f ,1.0f,//22
		-0.5f, -0.5f, 0.5f, -1.0f,  0.0f, 0.0f, 0.0f ,1.0f//23
	};

	unsigned int indices_Cubo[]
	{
		//Front
		0,2,3,
		0,3,1,
		//Back
		4,6,7,
		4,7,5,
		//Up
		8,10,11,
		8,11,9,
		//Down
		12,14,15,
		12,15,13,
		//Right
		16,18,19,
		16,19,17,
		//Left
		20,22,23,
		20,23,21
	};

	unsigned int VBO_Cubo, VAO_Cubo, EBO_Cubo; //Vertex Buffer Object, Vertex Array Object y Extendet Array Object
	glGenVertexArrays(1, &VAO_Cubo);
	glGenBuffers(1, &VBO_Cubo);
	glGenBuffers(1, &EBO_Cubo);
	//unir o linkear
	glBindVertexArray(VAO_Cubo);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_Cubo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_Cubo), vertices_Cubo, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_Cubo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices_Cubo), indices_Cubo, GL_STATIC_DRAW);
	//posiciones
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	//luces
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	//uv
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	//segundo shader
	unsigned int VAO2_Cubo;
	glGenVertexArrays(1, &VAO2_Cubo);
	glBindVertexArray(VAO2_Cubo);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_Cubo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_Cubo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);

	glEnableVertexAttribArray(0);

	float vertices_Trap[]{
		//posiciones		//cómo afecta		//Tex
		//Frente
		 0.5f, -0.5f, 0.5f,  0.0f,  0.0f, 1.0f, 1.0f ,0.0f,//0
		-0.5f, -0.5f, 0.5f,  0.0f,  0.0f, 1.0f, 0.0f ,0.0f,//1
		 0.3f,  0.5f, 0.3f,  0.0f,  0.0f, 1.0f, 1.0f ,1.0f,//2
		-0.3f,  0.5f, 0.3f,  0.0f,  0.0f, 1.0f, 0.0f ,1.0f,//3
		//Atras									
		 0.5f, -0.5f,-0.5f,  0.0f,  0.0f,-1.0f, 1.0f ,0.0f,//4
		-0.5f, -0.5f,-0.5f,  0.0f,  0.0f,-1.0f, 0.0f ,0.0f,//5
		 0.3f,  0.5f,-0.3f,  0.0f,  0.0f,-1.0f, 1.0f ,1.0f,//6
		-0.3f,  0.5f,-0.3f,  0.0f,  0.0f,-1.0f, 0.0f ,1.0f,//7
		//Arriba							
		 0.3f,  0.5f,-0.3f,  0.0f,  1.0f, 0.0f, 1.0f ,0.0f,//8
		-0.3f,  0.5f,-0.3f,  0.0f,  1.0f, 0.0f, 0.0f ,0.0f,//9
		 0.3f,  0.5f, 0.3f,  0.0f,  1.0f, 0.0f, 1.0f ,1.0f,//10
		-0.3f,  0.5f, 0.3f,  0.0f,  1.0f, 0.0f, 0.0f ,1.0f,//11
		//Abajo							
		 0.5f, -0.5f,-0.5f,  0.0f, -1.0f, 0.0f, 1.0f ,0.0f,//12
		-0.5f, -0.5f,-0.5f,  0.0f, -1.0f, 0.0f, 0.0f ,0.0f,//13
		 0.5f, -0.5f, 0.5f,  0.0f, -1.0f, 0.0f, 1.0f ,1.0f,//14
		-0.5f, -0.5f, 0.5f,  0.0f, -1.0f, 0.0f, 0.0f ,1.0f,//15
		 //Derecha							
		 0.3f,  0.5f,-0.3f,  1.0f,  0.0f, 0.0f, 0.0f ,1.0f,//16
		 0.5f, -0.5f,-0.5f,  1.0f,  0.0f, 0.0f, 0.0f ,0.0f,//17
		 0.3f,  0.5f, 0.3f,  1.0f,  0.0f, 0.0f, 1.0f ,1.0f,//18
		 0.5f, -0.5f, 0.5f,  1.0f,  0.0f, 0.0f, 1.0f ,0.0f,//19
		 //Izquierda						
		-0.3f,  0.5f,-0.3f, -1.0f,  0.0f, 0.0f, 0.0f ,1.0f,//20
		-0.5f, -0.5f,-0.5f, -1.0f,  0.0f, 0.0f, 0.0f ,0.0f,//21
		-0.3f,  0.5f, 0.3f, -1.0f,  0.0f, 0.0f, 1.0f ,1.0f,//22
		-0.5f, -0.5f, 0.5f, -1.0f,  0.0f, 0.0f, 1.0f ,0.0f //23
	};

	unsigned int indices_Trap[]
	{
		//Front
		0,2,3,
		0,3,1,
		//Back
		4,6,7,
		4,7,5,
		//Up
		8,10,11,
		8,11,9,
		//Down
		12,14,15,
		12,15,13,
		//Right
		16,18,19,
		16,19,17,
		//Left
		20,22,23,
		20,23,21
	};

	unsigned int VBO_Trap, VAO_Trap, EBO_Trap; //Vertex Buffer Object, Vertex Array Object y Extendet Array Object
	glGenVertexArrays(1, &VAO_Trap);
	glGenBuffers(1, &VBO_Trap);
	glGenBuffers(1, &EBO_Trap);
	//unir o linkear
	glBindVertexArray(VAO_Trap);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_Trap);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_Trap), vertices_Trap, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_Trap);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices_Trap), indices_Trap, GL_STATIC_DRAW);
	//posiciones
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	//luces
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	//uv
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	//segundo shader
	unsigned int VAO2_Trap;
	glGenVertexArrays(1, &VAO2_Trap);
	glBindVertexArray(VAO2_Trap);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_Trap);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_Trap);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);

	glEnableVertexAttribArray(0);

	unsigned int ventanas = loadTexture("ventana.jpg", GL_RGB);
	unsigned int pasto = loadTexture("pasto.jpg", GL_RGB);
	unsigned int sample = loadTexture("sample_tex.jpg", GL_RGB);
	unsigned int rancho = loadTexture("rancho_tex.jpg", GL_RGB);
	unsigned int puerta = loadTexture("puerta_gr.jpg", GL_RGB);
	unsigned int base_molino = loadTexture("molino_tex.png", GL_RGBA);
	unsigned int helice_molino = loadTexture("helice_tex.png", GL_RGBA);

	float angle = 0.0f;

	while (!glfwWindowShouldClose(window))
	{
		//calculo para el mouse para que pueda captar los movimeintos en tiempo real
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		processInput(window);

		//Renderizado
		glClearColor(0.9f, 0.1f, 0.1f, 1.0f);

		glEnable(GL_DEPTH_TEST);


		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //limpieza del buffer z
		

		float radio = 4.0f;
		float multipVelocidad = 2.0f;
		//posLuz = glm::vec3(radio * cos(glfwGetTime() * multipVelocidad), posLuz.y, radio * sin(glfwGetTime() * multipVelocidad));

		//Propiedades de la luz
		nuestroShader.use();
		nuestroShader.setVec3("light.position", posLuz);
		nuestroShader.setVec3("viewPos", camera.Position);

		//propiedades avanzadas de la luz
		glm::vec3 lightColor= glm::vec3(1.0f);
		/*lightColor.x = sin(glfwGetTime() * 2.0f);
		lightColor.y = sin(glfwGetTime() * 0.7f);
		lightColor.z = sin(glfwGetTime() * 1.3f);*/

		angle = glfwGetTime()/10.0f;

		glm::vec3 diffuseColor = lightColor * glm::vec3(4.5f); //potencia de la luz difusa
		glm::vec3 ambientColor = diffuseColor * glm::vec3(0.2f);
		nuestroShader.setVec3("light.ambient", ambientColor);
		nuestroShader.setVec3("light.diffuse", diffuseColor);
		nuestroShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);

		//propiedades de los materiales
		nuestroShader.setVec3("material.ambient", 1.0f, 1.0f, 1.0f);
		nuestroShader.setVec3("material.diffuse", 0.0f, 0.0f, 0.0f);
		nuestroShader.setVec3("material.specular", 0.7f, 0.6f, 0.6f);
		nuestroShader.setFloat("material.shininess", 32.0f);// intensidad o enfoque

		//mvp
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();

		nuestroShader.setMat4("projection", projection);
		nuestroShader.setMat4("view", view);

		glBindTexture(GL_TEXTURE_2D, rancho);
		glBindVertexArray(VAO_GraneroAla);

		glm::mat4 model = glm::mat4(1.0f);
		model = translate(model, glm::vec3(2.5f, 0.0f, 0.0f));
		nuestroShader.setMat4("model", model);

		//renderizar cubo
		glDrawElements(GL_TRIANGLES, 100, GL_UNSIGNED_INT, 0);

		model = glm::mat4(1.0f);
		model = translate(model, glm::vec3(-2.5f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(-1.0f,1.0f,1.0f));
		nuestroShader.setMat4("model", model);

		//renderizar cubo
		glDrawElements(GL_TRIANGLES, 100, GL_UNSIGNED_INT, 0);

		
		glBindVertexArray(VAO_Granero);

		model = glm::mat4(1.0f);
		model = translate(model, glm::vec3(0.0f, 2.0f, 0.0f));
		nuestroShader.setMat4("model", model);

		//renderizar cubo
		glDrawElements(GL_TRIANGLES, 100, GL_UNSIGNED_INT, 0);

		//Puerta
		glBindTexture(GL_TEXTURE_2D, puerta);

		model = glm::mat4(1.0f);
		model = glm::scale(model, glm::vec3(0.4f, 0.4f, 0.1f));
		model = translate(model, glm::vec3(0.0f, 2.0f, 26.0f));
		nuestroShader.setMat4("model", model);

		//renderizar cubo
		glDrawElements(GL_TRIANGLES, 100, GL_UNSIGNED_INT, 0);


		glBindTexture(GL_TEXTURE_2D, ventanas);
		glBindVertexArray(VAO_Cubo);

		model = glm::mat4(1.0f);
		model = glm::scale(model, glm::vec3(0.9f, 0.9f, 0.1f));
		model = translate(model, glm::vec3(0.0f, 4.0f, 25.0f));
		nuestroShader.setMat4("model", model);

		//renderizar cubo
		glDrawElements(GL_TRIANGLES, 100, GL_UNSIGNED_INT, 0);

		model = glm::mat4(1.0f);
		model = glm::scale(model, glm::vec3(0.9f, 0.9f, 0.1f));
		model = translate(model, glm::vec3(1.2f, 4.0f, 25.0f));
		nuestroShader.setMat4("model", model);

		//renderizar cubo
		glDrawElements(GL_TRIANGLES, 100, GL_UNSIGNED_INT, 0);

		model = glm::mat4(1.0f);
		model = glm::scale(model, glm::vec3(0.9f, 0.9f, 0.1f));
		model = translate(model, glm::vec3(-1.2f, 4.0f, 25.0f));
		nuestroShader.setMat4("model", model);

		//renderizar cubo
		glDrawElements(GL_TRIANGLES, 100, GL_UNSIGNED_INT, 0);

		glBindTexture(GL_TEXTURE_2D, pasto);
		model = glm::mat4(1.0f);
		model = glm::scale(model, glm::vec3(20.0f, 0.1f, 20.0f));
		model = translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
		nuestroShader.setMat4("model", model);

		//renderizar cubo
		glDrawElements(GL_TRIANGLES, 100, GL_UNSIGNED_INT, 0);

		

		//Configurar punto de luz
		luzShader.use();
		luzShader.setMat4("projection", projection);
		luzShader.setMat4("view", view);
		model = glm::mat4(1.0f);
		model = translate(model, posLuz);
		model = glm::scale(model, glm::vec3(0.2f));
		//model = translate(model, vec3(posLuz.x, posLuz.y, -glfwGetTime()));
		model = glm::rotate(model, 45.0f, glm::vec3(0.0f, 1.0f, 0.0f));
		luzShader.setMat4("model", model);

		//renderizar luz
		glBindVertexArray(VAO2);
		glDrawElements(GL_TRIANGLES, 50, GL_UNSIGNED_INT, 0);


		

		//dibujado del skybox
		glDepthFunc(GL_LEQUAL); //funcion de profundidad encargada del calculo entre el usuario y la profundidad del skybox para la generacion de infinidad
		skyboxShader.use();
		view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
		skyboxShader.setMat4("view", view);
		skyboxShader.setMat4("projection", projection);

		glBindVertexArray(sVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);


		glDepthFunc(GL_LESS);

		nuestroShader.use();
		nuestroShader.setVec3("light.position", posLuz);
		nuestroShader.setVec3("viewPos", camera.Position);

		//Trap
		glBindVertexArray(VAO_Trap);
		glBindTexture(GL_TEXTURE_2D, base_molino);

		model = glm::mat4(1.0f);
		model = glm::scale(model, glm::vec3(2.0f, 10.0f, 2.0f));
		model = translate(model, glm::vec3(2.3f, 0.5f, 0.0f));
		nuestroShader.setMat4("model", model);

		//renderizar cubo
		glDrawElements(GL_TRIANGLES, 100, GL_UNSIGNED_INT, 0);
		
		glBindTexture(GL_TEXTURE_2D, helice_molino);
		glBindVertexArray(VAO_Cubo);

		model = glm::mat4(1.0f);
		model = translate(model, glm::vec3(4.6f, 9.0f, 1.0f));
		model = glm::scale(model, glm::vec3(4.0f, 4.0f, 0.1f));
		model = glm::rotate(model, angle, glm::vec3(0.0f, 0.0f, 1.0f));
		nuestroShader.setMat4("model", model);

		//renderizar cubo
		glDrawElements(GL_TRIANGLES, 100, GL_UNSIGNED_INT, 0);


		//detecte eventos de IO
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &VAO);
	glDeleteVertexArrays(1, &VAO2);
	glDeleteVertexArrays(1, &sVAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
	glDeleteBuffers(1, &sVBO);

	glDeleteVertexArrays(1, &VAO_Granero);
	glDeleteVertexArrays(1, &VAO2_Granero);
	glDeleteBuffers(1, &VBO_Granero);
	glDeleteBuffers(1, &EBO_Granero);

	glDeleteVertexArrays(1, &VAO_GraneroAla);
	glDeleteVertexArrays(1, &VAO2_GraneroAla);
	glDeleteBuffers(1, &VBO_GraneroAla);
	glDeleteBuffers(1, &EBO_GraneroAla);

	glDeleteVertexArrays(1, &VAO_Trap);
	glDeleteVertexArrays(1, &VAO2_Trap);
	glDeleteBuffers(1, &VBO_Trap);
	glDeleteBuffers(1, &EBO_Trap);

	glDeleteVertexArrays(1, &VAO_Cubo);
	glDeleteVertexArrays(1, &VAO2_Cubo);
	glDeleteBuffers(1, &VBO_Cubo);
	glDeleteBuffers(1, &EBO_Cubo);


	glfwTerminate();
	return 0;
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	//nuestra primera configuracion de la camara
	glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

unsigned int loadCubemap(vector<string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else
		{
			cout << "fallo en cargar las texturas de nuestro skybox, la textura que fallo es: " << faces[i] << endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}

unsigned int loadTexture(string path, int type) {
	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load and generate the texture
	int width, height, nrChannels;
	unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, type, width, height, 0, type, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);

	return texture;
}
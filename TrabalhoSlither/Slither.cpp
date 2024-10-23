/* Segue o Mouse - código adaptado de https://learnopengl.com/Getting-started/Hello-Triangle
 *
 * Adaptado por Rossana Baptista Queiroz
 * para a disciplina de Fundamentos de Computação Gráfica - Unisinos
 * Versão inicial: 05/10/2024 (ver gravação da aula)
 * Última atualização em 17/10/2024
 *
 * Este programa desenha um triângulo que segue o cursor do mouse
 * usando OpenGL e GLFW.
 * A posição e a rotação do triângulo são calculadas com base no movimento do mouse.
 */

#include <iostream>
#include <string>
#include <assert.h>

// Bibliotecas GLAD para carregar funções OpenGL
#include <glad/glad.h>

// Biblioteca GLFW para criar janelas e gerenciar entrada de teclado/mouse
#include <GLFW/glfw3.h>

// GLM para operações matemáticas (vetores, matrizes)
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cmath>
#include <vector>

using namespace std;
using namespace glm;

// Constantes
const float Pi = 3.14159265;
const GLuint WIDTH = 800, HEIGHT = 600; // Dimensões da janela

// Estrutura para armazenar informações sobre as geometrias da cena
struct Geometry {
    GLuint VAO;        // Vertex Array Geometry
    vec3 position;     // Posição do objeto
    float angle;       // Ângulo de rotação
    vec3 dimensions;   // Escala do objeto (largura, altura)
    vec3 color;        // Cor do objeto
    int nVertices;     // Número de vértices a desenhar
};

// Variáveis globais
bool keys[1024];   // Estados das teclas (pressionadas/soltas)
vec2 mousePos;     // Posição do cursor do mouse
vec3 dir = vec3(0.0, -1.0, 0.0); // Vetor direção (do objeto para o mouse)

// Protótipos das funções
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);
int setupShader();           // Função para configurar os shaders
int setupGeometry();         // Função para configurar a geometria (triângulo)
void drawGeometry(GLuint shaderID, GLuint VAO, int nVertices, vec3 position, vec3 dimensions, float angle, vec3 color, GLuint drawingMode = GL_TRIANGLES, vec3 axis = vec3(0.0, 0.0, 1.0));

// Vetor que armazena todos os segmentos da cobrinha,
// incluindo a cabeça
std::vector<Geometry> snake;
Geometry createSegment(int i, vec3 dir);
int createCircle(int nPoints, float radius);

//Váriaveis da cobrinha
float minDistance = 1;
float maxDistance = 10;
float smoothFactor = 0.1;
bool addNew = false;

// Objeto Geometry que representa os olhos da cabeça da cobrinha
Geometry eyes;
int createEyes(int nPoints, float radius);


int main() {
    // Inicializa GLFW e configurações de versão do OpenGL
    glfwInit();
    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Seguindo o Mouse", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);

    // Inicializa GLAD para carregar todas as funções OpenGL
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Falha ao inicializar GLAD" << std::endl;
        return -1;
    }
    // Informações sobre o Renderer e a versão OpenGL
    const GLubyte *renderer = glGetString(GL_RENDERER);
    const GLubyte *version = glGetString(GL_VERSION);
    cout << "Renderer: " << renderer << endl;
    cout << "Versão OpenGL suportada: " << version << endl;

    // Configurações de viewport
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);


    // Compila o shader e cria a geometria (triângulo)
    GLuint shaderID = setupShader();
	/*
    Geometry triangle;
    triangle.VAO = setupGeometry();
    triangle.position = vec3(0.0, 0.0, 0.0);
    triangle.color = vec3(1.0, 1.0, 0.0);  // Amarelo
    triangle.dimensions = vec3(50.0, 50.0, 1.0);
    triangle.nVertices = 3;  // Triângulo
	*/

	Geometry head = createSegment(0, dir);
	snake.push_back(head);
	snake[0].VAO =  createCircle(32, 0.5);
	snake[0].nVertices = 34;
    snake[0].position = vec3(0.0, 0.0, 0.0);
    snake[0].color = vec3(1.0, 1.0, 0.0);  // Amarelo
    snake[0].dimensions = vec3(50.0, 50.0, 1.0);


	//Criação da geometria dos olhos
	Geometry eyes = createSegment(0, dir);
	eyes.VAO = createEyes(32, 0.25);
	eyes.nVertices = 34;
	eyes.position = vec3(400, 300, 0);
	eyes.dimensions = vec3(50, 50, 1.0);
	eyes.angle = 0.0;
	eyes.color = vec3(1.0, 1.0, 1.0);


    // Ativa o teste de profundidade
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_ALWAYS); // Sempre passa no teste de profundidade (desnecessário se não houver profundidade)

    glUseProgram(shaderID);

    // Matriz de projeção ortográfica (usada para desenhar em 2D)
    mat4 projection = ortho(0.0f, 800.0f, 0.0f, 600.0f, -1.0f, 1.0f);
    glUniformMatrix4fv(glGetUniformLocation(shaderID, "projection"), 1, GL_FALSE, value_ptr(projection));

    // Loop da aplicação
    while (!glfwWindowShouldClose(window)) {
        // Processa entradas (teclado e mouse)
        glfwPollEvents();

        // Limpa a tela
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Pega a posição do mouse e calcula a direção
        double xPos, yPos;
        glfwGetCursorPos(window, &xPos, &yPos);
        mousePos = vec2(xPos, height - yPos);  // Inverte o eixo Y para se alinhar à tela
        vec3 dir = normalize(vec3(mousePos, 0.0) - snake[0].position);
        float angle = atan2(dir.y, dir.x);

        // Move a cobrinha suavemente na direção do mouse
        if (distance(snake[0].position, vec3(mousePos, 0.0)) > 0.01f) {
            snake[0].position += 0.75f * dir;  // Aumente ou diminua 0.5f para controlar a 
			eyes.position = snake[0].position;
        }

        // Atualiza o ângulo de rotação do triângulo
        snake[0].angle = angle + radians(-90.0f); // Rotaciona para que a ponta aponte para o mouse
		eyes.angle = snake[0].angle;
	
		// Atualiza a posição dos segmentos da cobrinha para seguir a cabeça
		for (int i = 1; i < snake.size(); i++){
			vec3 dir = normalize(snake[i - 1].position - snake[i].position);
			float distance = length(snake[i - 1].position - snake[i].position);
			// Calcula a nova posição do segmento com suavidade, respeitando as distâncias mínima e máxima
			vec3 targetPosition = snake[i].position;
			float dynamicSmoothFactor = smoothFactor * (distance / maxDistance);
			if (distance < minDistance){
				targetPosition = snake[i].position + (distance - minDistance) * dir;
				}
			else if (distance > maxDistance){
				targetPosition = snake[i].position + (distance - maxDistance) * dir;
				}
			// Interpolação suave para a nova posição do segmento
			snake[i].position = mix(snake[i].position, targetPosition, dynamicSmoothFactor);
		}

		// Adiciona novos segmentos à cobrinha quando solicitado
		if (addNew){
			snake.push_back(createSegment(snake.size(), -dir));
			addNew = false;
			}

        // Desenha o triângulo e o cursor
        //drawGeometry(shaderID, snake[0].VAO, snake[0].nVertices, snake[0].position, snake[0].dimensions, snake[0].angle, snake[0].color);
        //drawGeometry(shaderID, snake[0].VAO, 3, vec3(mousePos, 0.0), vec3(10.0, 10.0, 1.0), 0.0f, vec3(1.0, 0.0, 1.0));

		// Desenha os segmentos da cobrinha e os olhos
	for (int i = snake.size() - 1; i >= 0; i--){
		drawGeometry(shaderID, snake[i].VAO, snake[i].nVertices, snake[i].position, snake[i].dimensions, snake[i].angle, snake[i].color, GL_TRIANGLE_FAN);
		if (i == 0){ // cabeça
			// Desenha as escleras dos olhos
			drawGeometry(shaderID, eyes.VAO, eyes.nVertices, eyes.position, eyes.dimensions, eyes.angle, eyes.color, GL_TRIANGLE_FAN);
			drawGeometry(shaderID, eyes.VAO, eyes.nVertices, eyes.position, eyes.dimensions, eyes.angle, eyes.color, GL_TRIANGLE_FAN);
			// Desenha as pupilas dos olhos
			drawGeometry(shaderID, eyes.VAO, eyes.nVertices, eyes.position, eyes.dimensions, eyes.angle, vec3(0.0, 0.0, 0.0), GL_TRIANGLE_FAN);
			drawGeometry(shaderID, eyes.VAO, eyes.nVertices, eyes.position, eyes.dimensions, eyes.angle, vec3(0.0, 0.0, 0.0), GL_TRIANGLE_FAN);
			}
		}

        // Troca os buffers da tela
        glfwSwapBuffers(window);
    }

    // Limpa a memória alocada pelos buffers
    glDeleteVertexArrays(1, &snake[0].VAO);
    glfwTerminate();
    return 0;
}

// Callback de teclado
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if (action == GLFW_PRESS)
        keys[key] = true;
    if (action == GLFW_RELEASE)
        keys[key] = false;
	// Adiciona um novo segmento à cobrinha quando a tecla Espaço é pressionada
	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS){
		addNew = true;
		}
}


// Configura e compila os shaders
int setupShader() {
    // Código do vertex shader
    const GLchar *vertexShaderSource = R"(
    #version 400
    layout (location = 0) in vec3 position;
    uniform mat4 projection;
    uniform mat4 model;
    void main() {
        gl_Position = projection * model * vec4(position, 1.0);
    })";

    // Código do fragment shader
    const GLchar *fragmentShaderSource = R"(
    #version 400
    uniform vec4 inputColor;
    out vec4 color;
    void main() {
        color = inputColor;
    })";

    // Compilação do vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // Verificando erros de compilação do vertex shader
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Compilação do fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // Verificando erros de compilação do fragment shader
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Linkando os shaders no programa
    GLuint shaderID = glCreateProgram();
    glAttachShader(shaderID, vertexShader);
    glAttachShader(shaderID, fragmentShader);
    glLinkProgram(shaderID);
    // Verificando erros de linkagem do programa de shaders
    glGetProgramiv(shaderID, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderID, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    // Limpando os shaders compilados após o link
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderID;
}

// Configura a geometria do triângulo
int setupGeometry() {
    GLfloat vertices[] = {
        -0.5f, -0.5f, 0.0f, // Vértice 1
         0.5f, -0.5f, 0.0f, // Vértice 2
         0.0f,  0.5f, 0.0f  // Vértice 3
    };

    GLuint VBO, VAO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    // Desvincula o VAO e o VBO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return VAO;
}

// Função para desenhar o objeto
void drawGeometry(GLuint shaderID, GLuint VAO, int nVertices, vec3 position, vec3 dimensions, float angle, vec3 color, GLuint drawingMode, vec3 axis) {
    glBindVertexArray(VAO); // Vincula o VAO

    // Aplica as transformações de translação, rotação e escala
    mat4 model = mat4(1.0f);
    model = translate(model, position);
    model = rotate(model, angle, axis);
    model = scale(model, dimensions);
    // Envia a matriz de modelo ao shader
    glUniformMatrix4fv(glGetUniformLocation(shaderID, "model"), 1, GL_FALSE, value_ptr(model));
    
    // Envia a cor do objeto ao shader
    glUniform4f(glGetUniformLocation(shaderID, "inputColor"), color.r, color.g, color.b, 1.0f);

    // Desenha o objeto
    glDrawArrays(drawingMode, 0, nVertices);

    // Desvincula o VAO
    glBindVertexArray(0);
}

Geometry createSegment(int i, vec3 dir){
	std::cout << "Criando segmento " << i << std::endl;
	// Inicializa um objeto Geometry para armazenar as informações do segmento
	Geometry segment;
	segment.VAO = createCircle(32, 0.5); // Cria a geometria do segmento como um círculo
	segment.nVertices = 34; // Número de vértices do círculo
	// Define a posição inicial do segmento
	if (i == 0) { // Cabeça
		segment.position = vec3(400.0, 300.0, 0.0); // Posição inicial no centro da tela
	} 
	else {
		// Ajusta a direção com base na posição dos segmentos anteriores para evitar sobreposição
		if (i >= 2)
			dir = normalize(snake[i - 1].position - snake[i - 2].position);
// Posiciona o novo segmento com uma distância mínima do segmento anterior
		segment.position = snake[i - 1].position + minDistance * dir;
	}
	// Define as dimensões do segmento (tamanho do círculo)
	segment.dimensions = vec3(50, 50, 1.0);
	segment.angle = 0.0; // Ângulo inicial (sem rotação)
	// Alterna a cor do segmento entre azul e amarelo, dependendo do índice
	if (i % 2 == 0) {
		segment.color = vec3(0, 0, 1); // Azul para segmentos de índice par
	} 
	else {
		segment.color = vec3(1, 1, 0); // Amarelo para segmentos de índice ímpar
	}
	return segment;
	}

int createCircle(int nPoints, float radius)
{
	vector <GLfloat> vertices;

	float angle = 0.0;
	float slice = 2 * Pi / (float)nPoints;

	//Adicionar o ponto da origem (0.0,0.0,0.0) como sendo o centro do círculo
	vertices.push_back(0.0); // Xc
	vertices.push_back(0.0); // Yc
	vertices.push_back(0.0); // Zc

	for (int i = 0; i < nPoints+1; i++)
	{
		float x = radius * cos(angle);
		float y = radius * sin(angle);
		float z = 0.0;

		vertices.push_back(x); // x
		vertices.push_back(y); // y
		vertices.push_back(z); // z

		angle = angle + slice;
	}

	//Configuração dos buffer VBO e VAO
	GLuint VBO, VAO;
	//Geração do identificador do VBO
	glGenBuffers(1, &VBO);
	//Faz a conexão (vincula) do buffer como um buffer de array
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	//Envia os dados do array de floats para o buffer da OpenGl
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);

	//Geração do identificador do VAO (Vertex Array Object)
	glGenVertexArrays(1, &VAO);
	// Vincula (bind) o VAO primeiro, e em seguida  conecta e seta o(s) buffer(s) de vértices
	// e os ponteiros para os atributos 
	glBindVertexArray(VAO);
	//Para cada atributo do vertice, criamos um "AttribPointer" (ponteiro para o atributo), indicando: 
	// Localização no shader * (a localização dos atributos devem ser correspondentes no layout especificado no vertex shader)
	// Numero de valores que o atributo tem (por ex, 3 coordenadas xyz) 
	// Tipo do dado
	// Se está normalizado (entre zero e um)
	// Tamanho em bytes 
	// Deslocamento a partir do byte zero 
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	// Observe que isso é permitido, a chamada para glVertexAttribPointer registrou o VBO como o objeto de buffer de vértice 
	// atualmente vinculado - para que depois possamos desvincular com segurança
	glBindBuffer(GL_ARRAY_BUFFER, 0); 

	// Desvincula o VAO (é uma boa prática desvincular qualquer buffer ou array para evitar bugs medonhos)
	glBindVertexArray(0); 

	return VAO;

}

// Cria a geometria dos olhos da cabeça da cobrinha, retornando o identificador do VAO
// nPoints: Número de pontos usados para aproximar os círculos que compõem os olhos
// radius: Raio das escleras dos olhos
int createEyes(int nPoints, float radius) {
	// Vetor para armazenar os vértices dos olhos (escleras e pupilas)
	std::vector<GLfloat> vertices;
	// Ângulo inicial e incremento para cada ponto do círculo
	float angle = 0.0;
	float slice = 2 * Pi / static_cast<float>(nPoints);
	// Posições iniciais para os círculos dos olhos (escleras e pupilas)
	float xi = 0.125f; // Posição inicial X das escleras
	float yi = 0.3f; // Posição inicial Y das escleras
	radius = 0.225f; // Raio das escleras

	// Olho esquerdo (esclera)
	vertices.push_back(xi); // Xc
	vertices.push_back(yi); // Yc
	vertices.push_back(0.0f); // Zc
	for (int i = 0; i < nPoints + 1; i++) {
		float x = xi + radius * cos(angle);
		float y = yi + radius * sin(angle);
		float z = 0.0f;
		vertices.push_back(x); // Coordenada X
		vertices.push_back(y); // Coordenada Y
		vertices.push_back(z); // Coordenada Z
		angle += slice; // Incrementa o ângulo para o próximo ponto
	}

	// Olho direito (esclera)
	angle = 0.0;
	vertices.push_back(xi); // Xc
	vertices.push_back(-yi); // Yc
	vertices.push_back(0.0f); // Zc
	for (int i = 0; i < nPoints + 1; i++) {
	float x = xi + radius * cos(angle);
	float y = -yi + radius * sin(angle);
	float z = 0.0f;
	vertices.push_back(x); // Coordenada X
	vertices.push_back(y); // Coordenada Y
	vertices.push_back(z); // Coordenada Z
	angle += slice;
	}

	// Olho esquerdo (pupila)
	radius = 0.18f; // Raio das pupilas
	xi += 0.09f; // Ajuste de posição para as pupilas
	angle = 0.0;
	vertices.push_back(xi); // Xc
	vertices.push_back(yi); // Yc
	vertices.push_back(0.0f); // Zc
	for (int i = 0; i < nPoints + 1; i++) {
		float x = xi + radius * cos(angle);
		float y = yi + radius * sin(angle);
		float z = 0.0f;
		vertices.push_back(x); // Coordenada X
		vertices.push_back(y); // Coordenada Y
		vertices.push_back(z); // Coordenada Z
		angle += slice;
	}

	// Olho direito (pupila)
	angle = 0.0;
	vertices.push_back(xi); // Xc
	vertices.push_back(-yi); // Yc
	vertices.push_back(0.0f); // Zc
	for (int i = 0; i < nPoints + 1; i++) {
		float x = xi + radius * cos(angle);
		float y = -yi + radius * sin(angle);
		float z = 0.0f;
		vertices.push_back(x); // Coordenada X
		vertices.push_back(y); // Coordenada Y
		vertices.push_back(z); // Coordenada Z
		angle += slice;
	}

	// Identificadores para o VBO (Vertex Buffer Object) e VAO (Vertex Array Object)
	GLuint VBO, VAO;
	// Geração do identificador do VBO e vinculação
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// Envia os dados do vetor de vértices para a GPU
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);
	// Geração do identificador do VAO e vinculação
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	// Configuração do ponteiro de atributos para os vértices
	// layout (location = 0) no Vertex Shader, 3 componentes por vértice (x, y, z)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	// Desvincula o VBO e o VAO para evitar modificações acidentais
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	// Retorna o identificador do VAO, que será utilizado para desenhar os olhos (escleras e pupilas)
	return VAO;
}
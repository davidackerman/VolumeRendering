// GLEW
#define GLEW_STATIC //use static library
#include <GL/glew.h> //Manages OpenGL function calls; used to access the modern OpenGL API functions

// GLFW
#include <GLFW/glfw3.h> //Gives us a window and OpenGL context to render in, and handles keyboard input; OPENGL DOES NOT HANDLE WINDOW CREATION OR INPUT
#include <SOIL/SOIL.h>
#include <iostream>
#include <string>

#include <sstream>

#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>
#include <glm/glm/gtc/type_ptr.hpp>



// Shaders
//Starts with version declaration and say we are using core functionality
const GLchar* vertexShaderSource = "#version 330 core\n"
"uniform float offset;\n"
"uniform mat4 transform;\n"
"uniform mat4 view; uniform mat4 projection;\n"
"layout (location = 0) in vec3 position;\n" // Set layout (location = 0) and say that our input ("in" keyword) position to the shader is of type vec3. layout (location = 0) means that the position attribute is at location 0
"layout (location = 1) in vec3 color;\n"    // The color variable has attribute position 1
"layout (location = 2) in vec2 textureCoordinate;\n" //location 2 has the texture coordinates
"out vec4 ourPosition;\n"
"out vec3 ourColor;\n"// Output a color to the fragment shader
"out vec2 ourTextureCoordinate ;\n"
"void main()\n"
"{\n"
"gl_Position = projection * view * transform*vec4(position[0], position[1], position[2], 1.0f);\n" //vec3 to a vec4 constructor
"ourColor = color;\n" //Set output variabell to a dark-red color
"ourPosition = gl_Position;\n"
"ourTextureCoordinate = textureCoordinate;\n"
"}\0";
const GLchar* fragmentShaderSource = "#version 330 core\n"
"uniform float whichDirection, alphaFactor;\n"
"in vec3 ourColor;\n" //Input defined with "in" keyword
"in vec4 ourPosition;\n"
"in vec2 ourTextureCoordinate;\n"
"uniform sampler2D ourTexture1;\n"//uniform 2D sampler for passing the texture to the fragment shader
"uniform sampler2D ourTexture2;\n"//second texture
"out vec4 color;\n" // out color
"void main()\n"
"{\n"
"color = texture(ourTexture1,ourTextureCoordinate);\n"//will return 80% first texture, 20% first texture
"color = vec4(sqrt(color.x), sqrt(color.y), sqrt(color.z),alphaFactor*sqrt(sqrt((color.x+color.y+color.z)/3.0)));\n"
"//color.w=(color.x+color.y+color.z)/3.0;\n"
"}\0";

void validate(GLuint ID, char order){
	GLint success;
	GLchar infoLog[512];
	if (order < 4){
		glGetShaderiv(ID, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(ID, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::" << order << "::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
		}
	}
	else{
		glGetProgramiv(ID, GL_LINK_STATUS, &success);
		if (!success) {
			glGetProgramInfoLog(ID, 512, NULL, infoLog);
			std::cout << "Program loading failed: " << infoLog << std::endl;
		}
	}
}

double windowWidth = 800, windowHeight = 800;
GLfloat rotateAboutZ = 0, previousRotateAboutZ = 0, xposPrevious = 0, xposCurrent;
GLfloat rotateAboutX = 0, previousRotateAboutX = 0, yposPrevious = 0, yposCurrent;
GLfloat cameraRadius = 1; GLfloat alphaFactor=1.0;
bool leftButtonState = false;
static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (leftButtonState){
		rotateAboutZ = previousRotateAboutZ + (((xpos - xposPrevious) + .01*windowWidth) / (1.02*windowWidth)) * 2 * 3.1415926; //1.02 because of border and adding 1.01 so it starts at 9
		rotateAboutX = previousRotateAboutX + (((ypos - yposPrevious) + .01*windowHeight) / (1.01*windowHeight)) * 2 * 3.1415926; //1.02 because of border and adding 1.01 so it starts at 9
	}
	xposCurrent = xpos;
	yposCurrent = ypos;
	//printf("%f %f %f %f \n", xpos, ypos, rotateAboutX, rotateAboutZ);
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == 0){ //left click
		if (action){
			leftButtonState = true;
			xposPrevious = xposCurrent;
			yposPrevious = yposCurrent;
		}
		else{
			leftButtonState = false;
			previousRotateAboutZ = rotateAboutZ;
			previousRotateAboutX = rotateAboutX;
		}
	}
}

static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{	cameraRadius = cameraRadius - 0.05*yoffset;
}


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) //Function to be registered with KeyCallback function
{
	// When a user presses the escape key, we set the WindowShouldClose property to true, 
	// closing the application
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	else if (key==333) alphaFactor = alphaFactor-.05;
	else if (key==334) alphaFactor = alphaFactor+.05;
}

//typedef void(* GLFWscrollfun)(GLFWwindow *, double, double)
int main()
{
	bool simple = false; const int numberOfTextures = 22;

	//First instantiate the GLFW window
	//Initialize glfw, then start to configure GLFW with glfwWindowHint(option, value to set option equal to)
	glfwInit();
	//Tell OpenGL which versions to run
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);



	GLFWwindow* window = glfwCreateWindow(800, 800, "LearnOpenGL", nullptr, nullptr);
	if (window == nullptr)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	//Make the context of our window the main context on the current thread
	glfwMakeContextCurrent(window);

	//Set up glew so that we can make OpnenGL calls
	//Glew manages OpenGL function pointers, so we need to initialize it before calling any OpenGL stuff. Setting experimental to true ensures it uses more modern functionality
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		std::cout << "Failed to initialize GLEW" << std::endl;
		return -1;
	}


	int width, height;
	glfwGetFramebufferSize(window, &width, &height);

	//First two parameters for lower left corner
	glViewport(0, 0, width, height);


	glfwSetKeyCallback(window, key_callback);  //Actually register the function with the proper callback
	//	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	/* VERTEX SHADER */
	//For OpenGL to use the shader it has to dynamically compile it at runtime. So first we create the vertex shader object
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	//Attach the shader source code to the shader object and compile the shader. glShaderSource(shader object to compile, how many strings we are passing, the address of the source code, NULL)
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	//Check if compilation was successful
	validate(vertexShader, '1');

	/* FRAGMENT SHADER */
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	validate(fragmentShader, '2');

	/* PROGRAM OBJECT */
	//Create shader program and attach shaders to it, and linnk the shader program shaders (output of one goes to input of next)
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	validate(shaderProgram, '4');

	//Delete the shaders since they are already in the program
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);


	/* WILL BE USING SOIL TO LOAD IMAGE */
	int imageWidth, imageHeight;
	int numberOfSlices;
	unsigned char* image;
	simple ? numberOfSlices = 22 : numberOfSlices = 22;
	GLuint texture[numberOfTextures]; //Create ID
	glGenTextures(numberOfTextures, texture); //glGenTextures(number of textures (storing them in GLuint array) given as second argument, location to store textures)
	//glBindTexture(GL_TEXTURE_2D, texture[numberOfSlices]); //bind the texture so that any subsequent commands will be executed on this texutre
	for (int i = 0; i <= numberOfTextures - 1; i++){
		if (simple) {
			i % 2 == 0 ? image = SOIL_load_image("cylinderTopBottom.jpg", &imageWidth, &imageHeight, 0, SOIL_LOAD_RGBA) : //loads in the image and stores the value
				image = SOIL_load_image("cylinderCrossSection.jpg", &imageWidth, &imageHeight, 0, SOIL_LOAD_RGBA);
		}
		else{
			std::ostringstream oss;
			if (i + 1 < 10){
				oss << "IM-0001-000" << i + 1 << ".jpg";
			}
			else{
				oss << "IM-0001-00" << i + 1 << ".jpg";
			}
			std::string var = oss.str();
			image = SOIL_load_image(var.c_str(), &imageWidth, &imageHeight, 0, SOIL_LOAD_RGBA);
		}
		//glActiveTexture(GL_TEXTURE0+i); //DGA: Activate the texture unit first before binding the texture
		glBindTexture(GL_TEXTURE_2D, texture[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, //specifies texture target (for now, it is a two dimensional texture, as opposed to 1D or 3D)
			0,               //mipmap stuff, but we'll set to base level which is 0
			GL_RGBA,          //how we want to store our image (RGB)
			imageWidth,      //texture width
			imageHeight,     //texture height
			0,               //always 0, legacy stuff
			GL_RGBA,          //(format) loaded RGB values
			GL_UNSIGNED_BYTE,//(datatype) and stored them as chars
			image);			 //image data
		//At this point, texture object has the texture image attached to it
		//	glGenerateMipmap(GL_TEXTURE_2D); //generates all the required mipmaps for the currently bound texture
		//free the image
		SOIL_free_image_data(image);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	GLfloat vertices[] = {
		// Positions          // Colors           // Texture Coords
		0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,   // Top Right
		0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,   // Bottom Right
		-0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,   // Bottom Left
		-0.5f, 0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,    // Top Left 

	};
	GLuint indices[] = {
		0, 1, 3, //First Triangle
		1, 2, 3,  //Second Triangle

	};

	//create VBO, EBO and VAO etc
	GLuint VBO, VAO, EBO;
	glGenBuffers(1, &VBO); //generate vertex buffer object
	glGenVertexArrays(1, &VAO); //generate vertex array object
	glGenBuffers(1, &EBO); //generate EBO
	glBindVertexArray(VAO); //bind vertex array object

	//Copy vertices array into vertex buffer object for OpenGL to use and set vertex attribute pointers
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(0)); //attribute 0 is position, with length 3, size of float, stride 8 *sizeof float and starts at position 3
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat))); //attribute 1 is color, with length 3, size of float, stride 8 *sizeof float and starts at position 3
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat))); //attribute 1 is color, with length 2, size of float, stride 8 *sizeof float and starts at position 6
	glEnableVertexAttribArray(2);
	//Bind and set EBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	//Unbind the VAO
	glBindVertexArray(0);

	//	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); //Wireframe mode
	//Game loop so that the window doesnt close
	float whichDirectionCurrent = 1;
	glm::vec3 newCenters, oldCenters;
	int whichToPlot;
	GLfloat pos;

	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetScrollCallback(window, scroll_callback);
	while (!glfwWindowShouldClose(window))
	{
		//Checks if any events are triggered and call events
		glfwPollEvents();

		//Rendering commands here
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f); //Always want to clear the screen at the start of an iteration so that whatever was on the screen isnt there anymore. State setting function
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//Clear the screen with the color we set. State using function (uses the current state to retrieve the clearing color from)
		// Draw a triangle, updating the uniform color
		GLfloat timeValue = glfwGetTime(); //Running time (in seconds)
		/*GLfloat greenValue = (sin(timeValue) / 2) + 0.5; //Vary color*/
		GLfloat transformLocation = glGetUniformLocation(shaderProgram, "transform"); //Get location of ourColor uniform
		GLfloat whichDirectionLocation = glGetUniformLocation(shaderProgram, "whichDirection"); //Get location of whichDirection uniform
				GLfloat alphaFactorLocation = glGetUniformLocation(shaderProgram, "alphaFactor"); //Get location of whichDirection uniform
		glUseProgram(shaderProgram);

		glActiveTexture(GL_TEXTURE0); //DGA: Activate the texture unit first before binding the texture

		// glm::mat4 view;
		glm::mat4 projection;
		projection = glm::perspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);

		glm::mat4 view;

		if (cos(rotateAboutX) > 0){
			printf("up\n");
			view = glm::lookAt(glm::vec3(0, cameraRadius*sin(rotateAboutX), cameraRadius*cos(rotateAboutX)), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
		}
		else{
			printf("down\n");
			view = glm::lookAt(glm::vec3(0, cameraRadius*sin(rotateAboutX), cameraRadius*cos(rotateAboutX)), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0));
		}


		// Get their uniform location
		GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
		GLint projLoc = glGetUniformLocation(shaderProgram, "projection");
		// Pass the matrices to the shader
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		// Note: currently we set the projection matrix each frame, but since the projection matrix rarely changes it's often best practice to set it outside the main loop only once.
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
		glBindVertexArray(VAO);
		for (int i = 0; i < numberOfSlices; i++){//i<numberOfSlices; i++){
			glm::mat4 trans;
			cos(rotateAboutX) < 0 ? whichToPlot = numberOfSlices - i : whichToPlot = i;
			pos = 0.2f - 0.4f* i*1.0 / (numberOfSlices - 1);
			glBindTexture(GL_TEXTURE_2D, texture[i]);

			trans = glm::rotate(trans, rotateAboutZ, glm::vec3(0.0f, 0.0f, 1.0f));

			trans = glm::translate(trans, glm::vec3(0.0f, 0.0f, pos));//glm::vec3(-sliceCenters[i].x, -sliceCenters[i].y, -sliceCenters[i].z)); 
			glUniform1f(alphaFactorLocation,alphaFactor);
			glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(trans)); //Must use shader first before setting uniform since it sets it on the currently active shader
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); //glDrawElements(mode, count of elements (6 vertices in total), type of indices, an offset or pass an index array when not using EBO- but we are so set it to 0); mill use currently bound EBO indices

		}
		glBindVertexArray(0);


		//Swaps the front and back buffer (where the comands are rendered to)
		glfwSwapBuffers(window);
	}


	glfwTerminate(); //Properly deletes and cleans everything
	return 0;

}


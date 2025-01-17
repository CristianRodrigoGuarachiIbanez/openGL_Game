#include <iostream>
#include <cmath>
#define GLEW_STATIC
#include <GL/glew.h>
#define SDL_MAIN_HANDLED


#ifdef _WIN32
#include <SDL.h>
#pragma comment(lib, "SDL2.lib")
#pragma comment(lib, "glew32s.lib")
#pragma comment(lib, "opengl32.lib")
#else
#include <SDL2/SDL.h>
#endif

#include "libs/glm/glm.hpp"
#include "libs/glm/ext/matrix_transform.hpp"
#include "libs/glm/gtc/matrix_transform.hpp"

#include "dataTypes.h"
#include "vertexBuffer.h"
#include "indexBuffer.h"
#include "shader.h"

#define STB_IMAGE_IMPLEMENTATION // define nur in cpp datei
#include "libs/stb_image.h"

void openGLDebugCallBack(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei len, const GLchar*msg, const void*userPar){
    //if(severity== GL_DEBUG_SEVERITY_HIGH)
    std::cout<< "[Opengl Error]: "<< msg<<std::endl;
}

#define GLCALL(call) call


int main(int argc, char**argv){
    SDL_Window* window;
    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);// for alpha channel sind 8 bits
    SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE,32); //jede Pixel ist 32 bit groß
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1); //dobbel buffern 
    
    #ifdef _DEBUG 
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
    #endif
    
    uint32 flags= SDL_WINDOW_OPENGL | SDL_WINDOW_BORDERLESS;
    window = SDL_CreateWindow("Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, flags);


    SDL_GLContext glContext = SDL_GL_CreateContext(window);

    GLenum err = glewInit();
    if(err!=GLEW_OK){
        std::cout<<"[Error]: " << glewGetErrorString(err)<<std::endl;
        std::cout<<std::cin.get();
        return -1;
    }
    #ifdef _DEBUG
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(openGLDebugCallBack, 0);
    #endif
    Vertex vertices[] = {
        Vertex{-0.5f, -0.5f, 0.0f, 
        0.0f, 0.0f,
        1.0f, 0.0f, 0.0f, 1.0f},

        Vertex{-0.5f, 0.5f, 0.0f, 
        0.0f, 1.0f, 
        0.0f, 1.0f, 0.0f, 1.0f},

        Vertex{0.5f, -0.5f, 0.0f, 
        1.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 1.0f},

        Vertex{0.5f, 0.5f, 0.0f, 
        1.0f, 1.0f, 
        0.0f, 0.0f, 0.0f, 1.0f},
    };
    uint32 numVertices = 4;

    // INDEX BUFFER
    uint32 indices[] = {0,1,2,1,2,3}; //indeces 
    uint32 numIndices = 6;

    IndexBuffer indexBuffer(indices, numIndices, sizeof(indices[0]));
    // VERTEX BUFFER
    VertexBuffer vertexBuffer(vertices, numVertices);
    //bind();

    // Texture
    int32 textureWidth = 0;
    int32 textureHeight = 0;
    int32 bitsPerPixel = 0;
    stbi_set_flip_vertically_on_load(true); // lade die Daten anders rum 
    auto textureBuffer = stbi_load("img/awesomeface.png", &textureWidth, &textureHeight, &bitsPerPixel, 4);
    GLuint textureId; // ID 
	GLCALL(glGenTextures(1, &textureId));
	GLCALL(glBindTexture(GL_TEXTURE_2D, textureId));

	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR)); //
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR)); // wenn nah, texture vergrößert werden
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

	GLCALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, textureWidth, textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureBuffer));
	GLCALL(glBindTexture(GL_TEXTURE_2D, 0));

    if(textureBuffer){
        stbi_image_free(textureBuffer);
    }
    ///////////////////// SHADER ///////////////////////
    SHADER shader("shaders/basic.vs", "shaders/basic.fs");
    shader.bind();
    //FDS theta
    uint64 perFrameCounterFrequency = SDL_GetPerformanceFrequency();
    uint64 lastCounter = SDL_GetPerformanceCounter();
    float32 delta = 0.0f;

    ////////////////// MATRIX /////////////////////
    // MATRIX MODEL
    glm::mat4 model = glm::mat4(1.5f);
	model = glm::scale(model, glm::vec3(2.0f, 1.0f, 1.0f));
    //load das MODEL 
    int modelViewProjMatrixLocation = GLCALL(glGetUniformLocation(shader.getShaderID(), "u_model"));
	
    int colorUniformLocation = GLCALL(glGetUniformLocation(shader.getShaderID(), "u_color"));
    
    if(!colorUniformLocation!=-1){
        GLCALL(glUniform4f(colorUniformLocation, 1.0f, 0.1f, 0.1f, 0.1f));
    }

    int textureUniformLocation = GLCALL(glGetUniformLocation(shader.getShaderID(), "u_texture"));
    if(! textureUniformLocation!=-1){
        GLCALL(glUniform1i(textureUniformLocation, 0));
    }
    // time variable
    float time = 0.0f;

    bool close = false;
    while(!close){
        glClearColor(1.0f, 0.0f,1.0f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        time+=delta;
        // p: Model, WieViel Rotiert , achse
		model = glm::rotate(model, 1.0f*delta, glm::vec3(0, 1, 0)); 

        if(!colorUniformLocation!=-1){
            float sin_n =  sinf(time)*sinf(time);
            //std::cout<<"Time: "<< sin_n<<std::endl;
            GLCALL(glUniform4f(colorUniformLocation,sin_n, 0.0f, 1.0f, 1.0f));
        }
        //wire frame
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        vertexBuffer.Bind();
        //glDrawArrays(GL_TRIANGLE_STRIP, 0, numVertices);
        indexBuffer.Bind();

        GLCALL(glActiveTexture(GL_TEXTURE0)); // zusammenhängend mit dem Uniform oben 
        GLCALL(glBindTexture(GL_TEXTURE_2D, textureId));

        GLCALL(glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0));
        indexBuffer.Unbind();
        vertexBuffer.Unbind();
       
        SDL_GL_SwapWindow(window);
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if(event.type == SDL_QUIT){
                close = true;
            }
        }
        uint64 endCounter = SDL_GetPerformanceCounter();
        uint64 counterElapsed = endCounter -lastCounter;
        delta = ((float32)counterElapsed)/ ((float32)perFrameCounterFrequency); // time needed for the frame
        uint32 FPS = (uint32)((float32)perFrameCounterFrequency /(float32)counterElapsed);
        //std::cout<< "FPS: "<<FPS<<std::endl;
        lastCounter = endCounter;        
        
    } 
	GLCALL(glDeleteTextures(1, &textureId));
    return 0;
}
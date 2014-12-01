// Include standard headers
#include <stdio.h>
#include <stdlib.h>

// Include GLEW
#include "GL/glew.h"

// Include GLFW
#include <glfw/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
using namespace glm;

#include "common/shader.hpp"

#include "RgbImage.h"
#include <windows.h>
#include <string>
#include <stdio.h>  /* defines FILENAME_MAX */
#ifdef WIN32
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif


// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "common/shader.hpp""
#include "common/texture.hpp"
#include "FBOManager.h"


extern "C" char cCurrentPath[FILENAME_MAX];

#define  NO_FBO

int main( int agc, char ** argv )
{
    GLfloat _Arr1[56] = {41.0,10.0,22.0,8.0,112.0,13.0,58.0,10.0,22.0,4.0,14.0,4.0,33.0,6.0,23.0,5.0,
        175.0,6.0,175.0,6.0,173.0,5.0,174.0,6.0,178.0,3.0,173.0,3.0,162.0,3.0,170.0,3.0,132.0,9.0,
        91.0,8.0,76.0,8.0,10.0,8.0,126.0,6.0,89.0,6.0,71.0,5.0,95.0,5.0,130.0,14.0,92.0,12.0,70.0,10.0,97.0,11.0};
    
    int _Arr1Size[2] = {7,8};

    // Initialise GLFW
    if( !glfwInit() )
    {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open a window and create its OpenGL context
    window = glfwCreateWindow( 1024, 768, "Tutorial 05 - Textured Cube", NULL, NULL);
    if( window == NULL ){
        fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return -1;
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    // Dark blue background
    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

    // Enable depth test
#ifdef NO_FBO
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS); 

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);
#endif // NO_FBO

    char  cPathToFrag[FILENAME_MAX], 
        cPathToVert[FILENAME_MAX], cPathToTexture[FILENAME_MAX];

    if (!GetCurrentDir(cCurrentPath, sizeof(cCurrentPath)))
    {
        return errno;
    }
    sprintf(cPathToFrag, "%s%s", cCurrentPath,  "/Shader/DrawSpecial_frag.hpp");
    sprintf(cPathToVert, "%s%s", cCurrentPath,  "/Shader/DrawSpecial_vert.hpp");
    sprintf(cPathToTexture, "%s%s", cCurrentPath,  "/sample_watermark.png");
    
    // Create and compile our GLSL program from the shaders
    // Create and compile our GLSL program from the shaders
    GLuint programID = LoadShaders( cPathToVert, cPathToFrag );


    // Get a handle for our "MVP" uniform
    GLuint MatrixID = glGetUniformLocation(programID, "MVP");

    // Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
    glm::mat4 Projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
    // Camera matrix
    glm::mat4 View       = glm::lookAt(
        glm::vec3(0,0,10), // Camera is at (4,3,3), in World Space
        glm::vec3(0,0,0), // and looks at the origin
        glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
        );
    // Model matrix : an identity matrix (model will be at the origin)
    glm::mat4 Model      = glm::mat4(1.0f);
    // Our ModelViewProjection : multiplication of our 3 matrices
    glm::mat4 MVP        = Projection * View * Model; // Remember, matrix multiplication is the other way around
   
    // Load the texture using any two methods
    //GLuint Texture = loadBMP_custom("uvtemplate.bmp");
    int pwidth, pheight;
    GLuint Texture = png_texture_loadA(cPathToTexture, &pwidth, &pheight);
#ifndef NO_FBO
    FBOManager fbo;
    fbo.Initialize(1024, 768);
    fbo.Perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
    fbo.LookAt(0,0,10,0,0,0,0,1,0);
    fbo.SetDrawMode(true, Texture);
#endif 

   // fbo.SetRotate(50, 0, 0);
  //  fbo.SetTranslate(0, 10.1f, -20);
    // Get a handle for our "imageTexture" uniform
#ifdef NO_FBO
    GLuint TextureID  = glGetUniformLocation(programID, "imageTexture");
    GLuint gltextID  = glGetUniformLocation(programID, "grTexture");


    // Our vertices. Tree consecutive floats give a 3D vertex; Three consecutive vertices give a triangle.
    // A cube has 6 faces with 2 triangles each, so this makes 6*2=12 triangles, and 12*3 vertices
    static const GLfloat g_vertex_buffer_data[] = { 
        -16.0f,-8.3f, -20.0f,
        16.0f, -8.3f, -20.0f,
        -16.0f, 8.3f, -20.0f,
       
        16.0f, 8.3f, -20.0f,
        -16.0f, 8.3f, -20.0f,
        16.0f, -8.3f, -20.0f

    };

    // Two UV coordinatesfor each vertex. They were created withe Blender.
    static const GLfloat g_uv_buffer_data[] = { 
        0.0f,0.0f, 
        1.0f, 0.0f, 
        0.0f, 1.0f,
        1.0f, 1.0f, 
        0.0f, 1.0f,
       1.0f, 0.0f
};
    float normControlRect[4];
    normControlRect[0] = 1218.f / 2560.f;
    normControlRect[1] = 99.f   / 1080.f;
    normControlRect[2] = 2394.f / 2560.f;
    normControlRect[3] = 1003.f / 1080.f;

    GLuint vertexbuffer;
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

    GLuint uvbuffer;
    glGenBuffers(1, &uvbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_uv_buffer_data), g_uv_buffer_data, GL_STATIC_DRAW);
#endif
   
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
  // glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);


    do{

#ifndef NO_FBO
        fbo.Draw();
#else
        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
       
        // Use our shader
        glUseProgram(programID);

        // Send our transformation to the currently bound shader, 
        // in the "MVP" uniform
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

        // Bind our texture in Texture Unit 0
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, Texture);
        // Set our "imageTexture" sampler to user Texture Unit 0
        glUniform1i(TextureID, 0);
        glUniform1i(gltextID, 0);

        ///////////Pass some pararmeters/////////////////////////////////////////////
        glUniform1fv( glGetUniformLocation(programID, "Arr1"), _Arr1Size[0]*_Arr1Size[1], _Arr1);
        glUniform2i(glGetUniformLocation(programID, "Arr1Size"), _Arr1Size[0], _Arr1Size[1]);
        //////////////////////////////////////////////////////////////////////////////////////

        // 1rst attribute buffer : vertices
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glVertexAttribPointer(
            0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
            3,                  // size
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void*)0            // array buffer offset
            );

        // 2nd attribute buffer : UVs
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
        glVertexAttribPointer(
            1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
            2,                                // size : U+V => 2
            GL_FLOAT,                         // type
            GL_FALSE,                         // normalized?
            0,                                // stride
            (void*)0                          // array buffer offset
            );
        
        // Draw the triangle !
        glDrawArrays(GL_TRIANGLES, 0, 2*3); // 12*3 indices starting at 0 -> 12 triangles
        
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
#endif

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

    }  // Check if the ESC key was pressed or the window was closed
    while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
        glfwWindowShouldClose(window) == 0 );

    // Cleanup VBO and shader
   /* glDeleteBuffers(1, &vertexbuffer);
    glDeleteBuffers(1, &uvbuffer);
    glDeleteProgram(programID);
    glDeleteVertexArrays(1, &VertexArrayID);*/

    // Close OpenGL window and terminate GLFW
    glfwTerminate();
    return 0;
}
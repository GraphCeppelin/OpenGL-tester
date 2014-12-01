#include "FBOManager.h"
#include "common/shader.hpp"
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>

extern "C" char cCurrentPath[FILENAME_MAX] = {0};

FBOManager::FBOManager(void)
{
    m_vertexArrayID = 0;

    m_framebufferName = 0;
    m_depthrenderbuffer = 0;
    m_fbo_texture = 0;
    m_quad_vertexbuffer = 0;
    m_quad_programID = 0;
    m_MVPUniformID = 0;
    m_renderedTexID = 0;

    m_screenResolution = vec2(1024, 768);
    m_drawBuffers[0] = GL_COLOR_ATTACHMENT0;

    m_translationMatrix = translate(mat4(), vec3(0.f, 0.f, 0.f));
    m_scalingMatrix = scale(mat4(), vec3(1.0f, 1.0f, 1.0f));
    m_rotationMatrix = eulerAngleYXZ(0.f, 0.f, 0.f);
    m_modelMatrix = m_translationMatrix * m_rotationMatrix * m_scalingMatrix;

    m_drawSpecial = false;
}

void FBOManager::LookAt (
    GLfloat eyex, 
    GLfloat eyey, 
    GLfloat eyez, 
    GLfloat centerx, 
    GLfloat centery, 
    GLfloat centerz, 
    GLfloat upx, 
    GLfloat upy, 
    GLfloat upz)
{
    m_viewMatrix = glm::lookAt(
        glm::vec3(eyex,eyey,eyez), // Camera is at (4,3,3), in World Space
        glm::vec3(centerx,centery,centerz), // and looks at the origin
        glm::vec3(upx,upy,upz)  // Head is up (set to 0,-1,0 to look upside-down)
        );
    UpdateMVP();
}


void FBOManager::Perspective(
    GLfloat fovy, 
    GLfloat aspect, 
    GLfloat zNear, 
    GLfloat zFar)
{
    m_projectionMatrix = glm::perspective(fovy, aspect, zNear, zFar);
    UpdateMVP();
}

void FBOManager::UpdateMVP()
{
    m_MVP = m_projectionMatrix * m_viewMatrix * m_modelMatrix; // Remember, matrix multiplication is the other way around
}

void FBOManager::SetResolution(int width, int height)
{
    m_screenResolution = vec2(width, height);
    glBindTexture(GL_TEXTURE_2D, m_fbo_texture);
    // Give an empty image to OpenGL ( the last "0" means "empty" )
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_screenResolution.x, m_screenResolution.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    // Poor filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindRenderbuffer(GL_RENDERBUFFER, m_depthrenderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_screenResolution.x, m_screenResolution.y);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthrenderbuffer);

    // Set "fbo_texture" as our colour attachement #0
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_fbo_texture, 0);
}

void FBOManager::InitGeometryObjects()
{
#pragma region Init special drawing geometry
    static const GLfloat g_vertex_buffer_data[] = { 
        -16.0f,-8.3f, -20.0f,
        16.0f, -8.3f, -20.0f,
        -16.0f, 8.3f, -20.0f,

        16.0f, 8.3f, -20.0f,
        -16.0f, 8.3f, -20.0f,
        16.0f, -8.3f, -20.0f
    };

    static const GLfloat g_uv_buffer_data[] = { 
        0.0f,0.0f, 
        1.0f, 0.0f, 
        0.0f, 1.0f,
        1.0f, 1.0f, 
        0.0f, 1.0f,
        1.0f, 0.0f
    };

    glGenBuffers(1, &m_specialGO.vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_specialGO.vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

    glGenBuffers(1, &m_specialGO.uvbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_specialGO.uvbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_uv_buffer_data), g_uv_buffer_data, GL_STATIC_DRAW);

    char  cPathToFrag[FILENAME_MAX],  cPathToVert[FILENAME_MAX];

    sprintf(cPathToFrag, "%s%s", cCurrentPath,  "/Shader/DrawSpecial_frag.hpp");
    sprintf(cPathToVert, "%s%s", cCurrentPath,  "/Shader/DrawSpecial_vert.hpp");

    m_specialGO.programID = LoadShaders( cPathToVert, cPathToFrag);
    m_specialGO.MVPUniformID = glGetUniformLocation(m_specialGO.programID, "MVP");
    m_specialGO.ModelMatrixID = glGetUniformLocation(m_specialGO.programID, "M");
    m_specialGO.ViewMatrixID = glGetUniformLocation(m_specialGO.programID, "V");
    m_specialGO.textureUniformID = glGetUniformLocation(m_specialGO.programID, "textureSample");

#pragma endregion Init special drawing geometry
}

int FBOManager::Initialize(int screenWidth, int screenHeight)
{
    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return -1;
    }

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS); 
    // Cull triangles which normal is not towards the camera
 //   glEnable(GL_CULL_FACE);

    glGenVertexArrays(1, &m_vertexArrayID);
    glBindVertexArray(m_vertexArrayID);

    // ---------------------------------------------
    // Render to Texture - specific code begins here
    // ---------------------------------------------
    glGenFramebuffers(1, &m_framebufferName);
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferName);
    
    glGenTextures(1, &m_fbo_texture);
    glGenRenderbuffers(1, &m_depthrenderbuffer);

    SetResolution(screenWidth, screenHeight);

    // Set the list of draw buffers.
    glDrawBuffers(1, m_drawBuffers); // "1" is the size of DrawBuffers

    // Always check that our framebuffer is ok
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        return -1;

    // The fullscreen quad's FBO
    static const GLfloat g_quad_vertex_buffer_data[] = { 
        -1.0f, -1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        1.0f,  1.0f, 0.0f,
    };

    glGenBuffers(1, &m_quad_vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_quad_vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, GL_STATIC_DRAW);


    char  cPathToFrag[FILENAME_MAX],  cPathToVert[FILENAME_MAX];

    sprintf(cPathToFrag, "%s%s", cCurrentPath,  "/Shader/Frame_frag.hpp");
    sprintf(cPathToVert, "%s%s", cCurrentPath,  "/Shader/Frame_vert.hpp");

    // Create and compile our GLSL program from the shaders
    m_quad_programID = LoadShaders( cPathToVert, cPathToFrag);
    m_renderedTexID = glGetUniformLocation(m_quad_programID, "fbo_texture");
    // Get a handle for our "MVP" uniform
    m_MVPUniformID = glGetUniformLocation(m_quad_programID, "MVP");
    InitGeometryObjects();
}

void FBOManager::DrawSpecial()
{
    // Render to our framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferName);
    glViewport(0,0,m_screenResolution.x,m_screenResolution.y); // Render on the whole framebuffer, complete from the lower left corner to the upper right
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Use our shader
    glUseProgram(m_specialGO.programID);

    // Send our transformation to the currently bound shader, 
    // in the "MVP" uniform
    glUniformMatrix4fv(m_specialGO.MVPUniformID, 1, GL_FALSE, &m_MVP[0][0]);
    glUniformMatrix4fv(m_specialGO.ModelMatrixID, 1, GL_FALSE, &m_modelMatrix[0][0]);
    glUniformMatrix4fv(m_specialGO.ViewMatrixID, 1, GL_FALSE, &m_viewMatrix[0][0]);

    // Bind our texture in Texture Unit 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_specialGO.textureHandleID);
    // Set our "myTextureSampler" sampler to user Texture Unit 0
    glUniform1i(m_specialGO.textureUniformID, 0);

    // 1rst attribute buffer : vertices
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, m_specialGO.vertexbuffer);
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
    glBindBuffer(GL_ARRAY_BUFFER, m_specialGO.uvbuffer);
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
}

void FBOManager::SetRotate(float x, float y, float z)
{
    m_rotationMatrix = eulerAngleYXZ(y, x, z);
    m_modelMatrix = m_translationMatrix * m_rotationMatrix * m_scalingMatrix;
    UpdateMVP();
}

void FBOManager::SetTranslate(float x, float y, float z)
{
    m_translationMatrix = translate(mat4(), vec3(x, y, z));
    m_modelMatrix = m_translationMatrix * m_rotationMatrix * m_scalingMatrix;
    UpdateMVP();
}

void FBOManager::Draw()
{
    glDisable( GL_BLEND );
    if (m_drawSpecial)
    {
        DrawSpecial();
    }

    // Render to the screen
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0,0,m_screenResolution.x,m_screenResolution.y); // Render on the whole framebuffer, complete from the lower left corner to the upper right
    // Clear the screen
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Use our shader
    glUseProgram(m_quad_programID);

    // Bind our texture in Texture Unit 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_fbo_texture);
    // Set our "fbo_texture" sampler to user Texture Unit 0
    glUniform1i(m_renderedTexID, 0);


    // 1rst attribute buffer : vertices
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, m_quad_vertexbuffer);
    glVertexAttribPointer(
        0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
        3,                  // size
        GL_FLOAT,           // type
        GL_FALSE,           // normalized?
        0,                  // stride
        (void*)0            // array buffer offset
        );
    glEnable( GL_BLEND );
    // Draw the triangles !
    glDrawArrays(GL_TRIANGLES, 0, 6); // 2*3 indices starting at 0 -> 2 triangles

    glDisableVertexAttribArray(0);
}


FBOManager::~FBOManager(void)
{
}

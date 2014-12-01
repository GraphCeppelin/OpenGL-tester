#pragma once
// Include GLEW
#include "GL/glew.h"
// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;


class FBOManager
{
typedef struct GeometryObject 
{
    GLuint uvbuffer;
    GLuint vertexbuffer;
    GLuint normalbuffer;
    GLuint elementbuffer;
    GLuint textureHandleID;
    GLuint textureUniformID;
    GLuint programID;
    GLuint MVPUniformID;
    GLuint ModelMatrixID;
    GLuint ViewMatrixID;
} GeometryObject;

private:
    GLuint m_vertexArrayID;

    GLuint m_fbo_texture; // The texture we're going to render to
    GLuint m_framebufferName;// The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
    GLuint m_depthrenderbuffer;// The depth buffer
    vec2 m_screenResolution;
    GLenum m_drawBuffers[1];
    GLuint m_quad_vertexbuffer;
    GLuint m_quad_programID;
    GLuint m_MVPUniformID;
    GLuint m_renderedTexID;

    //Space matrices
    glm::mat4 m_projectionMatrix; //projection matrix
    glm::mat4 m_viewMatrix; // Camera matrix
    glm::mat4 m_modelMatrix;// Model matrix : an identity matrix (model will be at the origin)
    glm::mat4 m_MVP; // Matrix multiplication is the other way around
    glm::mat4 m_rotationMatrix; // Rotation matrix
    glm::mat4 m_translationMatrix; // Translation matrix
    glm::mat4 m_scalingMatrix; // scalling matrix

    //Geomtery objects data
    GeometryObject m_specialGO;

    bool m_drawSpecial;//Draw the special geometry

public:
    FBOManager(void);
    ~FBOManager(void);

    int Initialize(int screenWidth, int screenHeight);
    void SetResolution(int width, int height);
    int* GetResolution() {int res[2]={m_screenResolution.x, m_screenResolution.y}; return res;};

    void LookAt (
        GLfloat eyex, 
        GLfloat eyey, 
        GLfloat eyez, 
        GLfloat centerx, 
        GLfloat centery, 
        GLfloat centerz, 
        GLfloat upx, 
        GLfloat upy, 
        GLfloat upz);

    void Perspective(
        GLfloat fovy, 
        GLfloat aspect, 
        GLfloat zNear, 
        GLfloat zFar);

    void Draw();
    void SetDrawMode(bool drawSpecial, GLuint textureHandle) 
    { 
        m_drawSpecial = drawSpecial;
        m_specialGO.textureHandleID = textureHandle;
    };
    void SetRotate(float x, float y, float z);
    void SetTranslate(float x, float y, float z);


private:
    void UpdateMVP();
    void InitGeometryObjects();
    void DrawSpecial();



};


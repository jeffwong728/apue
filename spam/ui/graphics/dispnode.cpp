#include "dispnode.h"
#include <numeric>
#include <algorithm>
#include <epoxy/gl.h>

SPDispNode GLDispNode::MakeNew(const WPDispNode &parent,
    const std::vector<glm::vec3> &vPositions,
    const std::vector<GLuint> &faceIdxs,
    const std::vector<GLuint> &edgeIdxs)
{
    std::vector<GLuint> vertIdxs(vPositions.size());
    std::iota(vertIdxs.begin(), vertIdxs.end(), 0);

    GLuint VBO = 0, VAO = 0, EBO = 0;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    GLsizeiptr vboPosSize  = sizeof(vPositions.front())*vPositions.size();
    GLsizeiptr eboFaceSize = sizeof(faceIdxs.front())*(faceIdxs.size());
    GLsizeiptr eboEdgeSize = sizeof(edgeIdxs.front())*(edgeIdxs.size());
    GLsizeiptr eboVertSize = sizeof(vertIdxs.front())*(vertIdxs.size());


    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vboPosSize, vPositions.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, eboFaceSize + eboEdgeSize + eboVertSize, nullptr, GL_STATIC_DRAW);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, eboFaceSize, faceIdxs.data());
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, eboFaceSize, eboEdgeSize, edgeIdxs.data());
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, eboFaceSize + eboEdgeSize, eboVertSize, vertIdxs.data());

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    if (VBO && VAO && EBO)
    {
        return std::make_shared<GLDispNode>(this_is_private{ 0 }, parent, VBO, VAO, EBO);
    }
    else
    {
        return SPDispNode();
    }
}

GLDispNode::GLDispNode(const this_is_private&, const WPDispNode &parent, const GLuint VBO, const GLuint VAO, const GLuint EBO)
    : parent_(parent), VBO_(VBO), VAO_(VAO), EBO_(EBO)
    , faceColor_( 1.0f, 0.85f, 0.35f, 1.0f )
    , edgeColor_( 0.0f, 0.0f, 0.0f, 1.0f )
    , vertColor_( 0.0f, 0.0f, 1.0f, 1.0f )
{
}

GLDispNode::~GLDispNode()
{
    glDeleteVertexArrays(1, &VAO_);
    glDeleteBuffers(1, &VBO_);
    glDeleteBuffers(1, &EBO_);
}

void GLDispNode::RenderMe() const
{

}

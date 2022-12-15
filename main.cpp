#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "tiny_obj_loader.h"
#include "Shader.h"
#include "glm/glm.hpp"
#include "glm/ext.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

glm::mat4 GetScaleMatrix(glm::vec3& scale)
{
    float scaleMat[16] = {
            scale.x,     0.0f,       0.0f,       0.0f,
            0.0f,        scale.y,    0.0f,       0.0f,
            0.0f,        0.0f,      scale.z,    0.0f,
            0.0f,       0.0f,      0.0f,       1.f,
    };

    return glm::make_mat4(scaleMat);
}

glm::mat4 GetTranslationMatrix(glm::vec3& translation)
{
    float translateMat[16] = {
            1.0f,        0.0f,       0.0f,       translation.x,
            0.0f,        1.0f,       0.0f,       translation.y,
            0.0f,        0.0f,      1.0f,       translation.z,
            0.0f,       0.0f,      0.0f,       1.f,
    };

    return glm::make_mat4(translateMat);
}

glm::mat4 GetRotationMatrix(glm::vec3& rotInDegree)
{
    glm::vec3 rot = glm::radians(rotInDegree);
    float rotXMat[16] = {
            1.0f,        0.0f,                  0.0f,                    0.0f,
            0.0f,        cos(rot.x),     -sin(rot.x),      0.0f,
            0.0f,        sin(rot.x),     cos(rot.x),       0.0f,
            0.0f,       0.0f,                  0.0f,                   1.f,
    };

    float rotYMat[16] = {
            cos(rot.y),   0.f,             -sin(rot.y),       0.0f,
            0.0f,                1.0f,            0.0f,                     0.0f,
            sin(rot.y),   0.0f,            cos(rot.y),       0.0f,
            0.0f,               0.0f,            0.0f,                   1.f,
    };

    float rotZMat[16] = {
            cos(rot.z),   -sin(rot.z),    0.0f,                  0.0f,
            sin(rot.z),   cos(rot.z),     0.0f,                  0.0f,
            0.0f,                0.0f,                 1.0f,                 0.0f,
            0.0f,               0.0f,                 0.0f,                  1.f,
    };

    return glm::make_mat4(rotXMat) * glm::make_mat4(rotYMat) * glm::make_mat4(rotZMat);
}

int main(int argc, char *argv[])
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    const char* glsl_version = "#version 100";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    const float windowWidth = 640.f;
    const float windowHeight = 480.f;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(windowWidth,  windowHeight, "Mesh Viewer", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    if (glewInit() != GLEW_OK)
        std::cout << "Error!" << std::endl;

    std::cout << glGetString(GL_VERSION) << std::endl;

    std::string inputfile = "res/mesh/teapot.obj";
//    std::string inputfile = *argv;
    tinyobj::ObjReaderConfig reader_config;
    reader_config.mtl_search_path = "./"; // Path to material files

    tinyobj::ObjReader reader;

    if (!reader.ParseFromFile(inputfile, reader_config)) {
        if (!reader.Error().empty()) {
            std::cerr << "TinyObjReader: " << reader.Error();
        }
        exit(1);
    }

    if (!reader.Warning().empty()) {
        std::cout << "TinyObjReader: " << reader.Warning();
    }

    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();
    auto& materials = reader.GetMaterials();

    std::vector<float> vertices;
    std::vector<float> normals;
    std::vector<float> uvs;
    std::vector<int> indices;

    normals.resize(attrib.vertices.size());
    // Loop over shapes
    for (size_t s = 0; s < shapes.size(); s++) {
        // Loop over faces(polygon)
        size_t index_offset = 0;
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
            size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

            // P0
            tinyobj::index_t idx0 = shapes[s].mesh.indices[index_offset];
            tinyobj::real_t vx0 = attrib.vertices[3*size_t(idx0.vertex_index)+0];
            tinyobj::real_t vy0 = attrib.vertices[3*size_t(idx0.vertex_index)+1];
            tinyobj::real_t vz0 = attrib.vertices[3*size_t(idx0.vertex_index)+2];

            // P1
            tinyobj::index_t idx1 = shapes[s].mesh.indices[index_offset + 1];
            tinyobj::real_t vx1 = attrib.vertices[3*size_t(idx1.vertex_index)+0];
            tinyobj::real_t vy1 = attrib.vertices[3*size_t(idx1.vertex_index)+1];
            tinyobj::real_t vz1 = attrib.vertices[3*size_t(idx1.vertex_index)+2];

            // P1
            tinyobj::index_t idx2 = shapes[s].mesh.indices[index_offset + 2];
            tinyobj::real_t vx2 = attrib.vertices[3*size_t(idx2.vertex_index)+0];
            tinyobj::real_t vy2 = attrib.vertices[3*size_t(idx2.vertex_index)+1];
            tinyobj::real_t vz2 = attrib.vertices[3*size_t(idx2.vertex_index)+2];

            glm::vec3 e10(vx1 - vx0, vy1 - vy0, vz1 - vz0);
            glm::vec3 e21(vx2 - vx1, vy2 - vy1, vz2 - vz1);
            glm::vec3 fn = glm::cross(e10, e21);

            for (size_t v = 0; v < fv; v++) {
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                normals[3*size_t(idx.vertex_index)+0] += fn.x;
                normals[3*size_t(idx.vertex_index)+1] += fn.y;
                normals[3*size_t(idx.vertex_index)+2] += fn.z;
            }
            index_offset += fv;

            // per-face material
            shapes[s].mesh.material_ids[f];
        }
    }

    for (int i = 0; i < attrib.vertices.size(); i++) {
        vertices.push_back(attrib.vertices[i]);
    }

    for (auto& shape : shapes) {
        for (int i = 0; i < shape.mesh.indices.size(); i++) {
            indices.push_back(shape.mesh.indices[i].vertex_index);
        }
    }

    GLuint VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO);

    // position attribute
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // normal attribute
    GLuint VNB;
    glGenBuffers(1, &VNB);
    glBindBuffer(GL_ARRAY_BUFFER, VNB);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), normals.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);

    GLuint IBO;
    glGenBuffers(1, &IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO); // IBO: index buffer
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), indices.data(), GL_STATIC_DRAW);

    Shader shader("res/shaders/vertex_shader", "res/shaders/fragment_shader");

    glm::vec3 position(0.0f, 0.0f, 0.0f);
    glm::vec3 rot(0.f, 0.f, 0.0f);
    glm::vec3 scale(1.f, 1.f, 1.0f);
    glm::vec3 cameraPos(0.f, 4.0f, -8.f);
    glm::vec3 cameraLookat(0.f, 0.f, 0.f);
    glm::vec3 cameraRot(0.f, 0.f, 0.f);
    glm::vec3 lightDir(0.3f, -1.0f, 0.f);

    float aspectRatio = windowWidth / windowHeight;
    float zNear = 0.1f;
    float zFar = 1000.f;
    float zRange = zNear - zFar;
    float FOV = 70.f; // field of view

    float time = 0.f;
    bool autoRotate = true;

    glEnable(GL_DEPTH_TEST);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shader.Use();

        glBindVertexArray(VAO);
        glm::vec3 right(1.0f, 0.f, 0.f);
        glm::vec3 up(0.f, 1.f, 0.f);
        glm::vec3 forward(0.f, 0.f, 1.f);
//        glm::vec3 cameraTransformedPos = glm::vec4(cameraPos.x, cameraPos.y, cameraPos.z, 1.f) * glm::transpose(GetRotationMatrix(cameraRot));
        forward = glm::normalize( cameraPos - cameraLookat);
        right = glm::normalize(glm::cross(glm::vec3 (0.f, 1.f, 0.f), forward));
        up = glm::normalize(glm::cross(forward, right));

        float viewMat[16] = {
                right.x,   right.y,   right.z,    -cameraPos.x,
                up.x,      up.y,      up.z,       -cameraPos.y,
                forward.x, forward.y, forward.z, -cameraPos.z,
                0.f,      0.f,       0.f,       1.f,
        };

        float tanHalfFOV = tan(glm::radians(FOV/2.0f));
        float projectionMat[16] = {
                1.0f / (tanHalfFOV * aspectRatio), 0.f,               0.f,                       0.f,
                0.0f,                               1.0f / tanHalfFOV, 0.0f,                      0.f,
                0.f,                                0.f,               (-zNear - zFar) / zRange, 2.0f * zFar * zNear / zRange,
                0.f,                               0.f,               1.f,                      0.f
        };

        if (autoRotate)
        {
            rot.y += 0.5f;
        }

        lightDir = glm::normalize(lightDir);

        glm::mat4 projection = glm::make_mat4(projectionMat);
        glm::mat4 view = glm::transpose(glm::make_mat4(viewMat));


//        glm::mat4 view = glm::transpose(glm::lookAt(cameraPos, cameraLookat, glm::vec3(0.f, 1.f, 0.f)));
        // convert from row-based to col-based matrix
//        glm::mat4 wvp = glm::transpose(world * view * projection);
        glm::mat4 world = GetScaleMatrix(scale) * GetRotationMatrix(rot) * GetTranslationMatrix(position);

        glm::mat4 rotX = glm::rotate(glm::mat4(1.f), glm::radians(rot.x), glm::vec3(1.f, 0.f, 0.f));
        glm::mat4 rotY = glm::rotate(glm::mat4(1.f), glm::radians(rot.y), glm::vec3(0.f, 1.f, 0.f));
        glm::mat4 rotZ = glm::rotate(glm::mat4(1.f), glm::radians(rot.z), glm::vec3(0.f, 0.f, 1.f));
        world = glm::scale(glm::mat4(1.f), scale) * glm::translate(glm::mat4(1.f), position) * rotX * rotY * rotZ;
        view = glm::lookAt(cameraPos, cameraLookat, glm::vec3(0.f, 1.f, 0.f));
        projection = glm::perspective(FOV, aspectRatio, zNear, zFar);
        glm::mat4 wvp = projection * view * world;
        shader.SetUniformMatrix4f("u_wvp", wvp);
        shader.SetUniformFloat3("u_viewDir", forward);
        shader.SetUniformFloat3("u_lightDir", lightDir);

        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        time += 0.01f;

        /* Poll for and process events */
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        {
            ImGui::Begin("Debug Menu");
            ImGui::SliderFloat3("Light Direction", &lightDir.x, -1.0f, 1.0f);
            ImGui::InputFloat3("Camera Pos", &cameraPos.x);
            ImGui::InputFloat3("Camera Lookat", &cameraLookat.x);
            ImGui::InputFloat3("Object Position", &position.x);
            ImGui::SliderFloat3("Object Rotation", &rot.x, -360.f, 360.f);
            ImGui::SliderFloat3("Object Scale", &scale.x, 0.f, 10.f);

            ImGui::Checkbox("Auto Rotate", &autoRotate);
            ImGui::SliderFloat("FOV", &FOV, 10.f, 150.f);
            ImGui::SliderFloat("Aspect Ratio", &aspectRatio,  0.5f, 2.0f);

            ImGui::End();
        }
        // Rendering
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        /* Swap front and back buffers */
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}
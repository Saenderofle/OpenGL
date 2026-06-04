#include <iostream>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// Шейдери для кубів (з підтримкою текстури)
const char* cubeVertexShader = R"glsl(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec2 aTexCoord;
    out vec2 TexCoord;
    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;
    void main() {
        gl_Position = projection * view * model * vec4(aPos, 1.0);
        TexCoord = aTexCoord;
    }
)glsl";

const char* cubeFragmentShader = R"glsl(
    #version 330 core
    out vec4 FragColor;
    in vec2 TexCoord;
    uniform sampler2D texture1;
    void main() {
        FragColor = texture(texture1, TexCoord);
    }
)glsl";

// Шейдер для рамки (однотонний помаранчево-жовтий колір)
const char* singleColorFragmentShader = R"glsl(
    #version 330 core
    out vec4 FragColor;
    void main() {
        FragColor = vec4(1.0, 0.6, 0.0, 1.0);
    }
)glsl";

// Налаштування камери
glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f, 5.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f, 0.0f);

bool firstMouse = true;
float yaw   = -90.0f;
float pitch = 0.0f;
float lastX = 400.0f;
float lastY = 300.0f;

// Логіка вибору активного об'єкта
int activeCubeIndex = 0; // 0, 1 або 2
float rotationAngle = 0.0f;

// Параметри обмеження FPS
double lastTime = 0.0;
const double limitFPS = 1.0 / 60.0;

// Обробка руху миші для орієнтації камери у просторі
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = (float)xpos;
        lastY = (float)ypos;
        firstMouse = false;
    }

    float xoffset = (float)xpos - lastX;
    float yoffset = lastY - (float)ypos;
    lastX = (float)xpos;
    lastY = (float)ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw   += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)  pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

// Обробка натискання клавіш 1, 2, 3 для зміни активного куба
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_1) activeCubeIndex = 0;
        if (key == GLFW_KEY_2) activeCubeIndex = 1;
        if (key == GLFW_KEY_3) activeCubeIndex = 2;
    }
}

// Переміщення камери (W, A, S, D)
void processCameraInput(GLFWwindow* window, float deltaTime) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float cameraSpeed = 2.5f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
}

// Функція для завантаження текстури
unsigned int loadTexture(const char* path) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);
    if (data) {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        std::cout << "Successfully loaded texture: " << path << std::endl;
    } else {
        std::cerr << "Failed to load texture: " << path << std::endl;
    }
    stbi_image_free(data);
    return textureID;
}

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "3D Cubes with Stencil Outline", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // Захоплюємо курсор миші у вікно

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Налаштування тестів глибини та трафарету
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    // Компіляція шейдерних програм
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &cubeVertexShader, nullptr);
    glCompileShader(vertexShader);

    unsigned int fragmentShaderCube = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShaderCube, 1, &cubeFragmentShader, nullptr);
    glCompileShader(fragmentShaderCube);

    unsigned int fragmentShaderOutline = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShaderOutline, 1, &singleColorFragmentShader, nullptr);
    glCompileShader(fragmentShaderOutline);

    unsigned int cubeProgram = glCreateProgram();
    glAttachShader(cubeProgram, vertexShader);
    glAttachShader(cubeProgram, fragmentShaderCube);
    glLinkProgram(cubeProgram);

    unsigned int outlineProgram = glCreateProgram();
    glAttachShader(outlineProgram, vertexShader);
    glAttachShader(outlineProgram, fragmentShaderOutline);
    glLinkProgram(outlineProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShaderCube);
    glDeleteShader(fragmentShaderOutline);

    // Геометрія куба (36 вершин: координати X,Y,Z + текстурні U,V)
    float vertices[] = {
        // Задня грань
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

        // Передня грань
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

        // Ліва грань
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

        // Права грань
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

        // Нижня грань
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

        // Верхня грань
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
    };

    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    unsigned int texture = loadTexture("square_texture.jpg");

    // Конфігурація світових позицій та масштабів для 3 кубів
    glm::vec3 cubePositions[] = {
        glm::vec3( 0.0f,  0.0f,  0.0f),
        glm::vec3( 2.0f,  1.0f, -2.0f),
        glm::vec3(-2.0f, -1.0f, -1.0f)
    };
    glm::vec3 cubeScales[] = {
        glm::vec3(1.0f),
        glm::vec3(0.6f),
        glm::vec3(1.4f)
    };

    lastTime = glfwGetTime();

    // Головний 3D цикл рендерингу
    while (!glfwWindowShouldClose(window)) {
        double now = glfwGetTime();
        double deltaTime = now - lastTime;

        if (deltaTime >= limitFPS) {
            lastTime = now;

            // Обробка W,A,S,D
            processCameraInput(window, (float)deltaTime);

            // --- КРИТИЧНЕ ВИПРАВЛЕННЯ БАГУ ЗМАЗУВАННЯ ---
            // Скидаємо маску перед очищенням кадру, щоб glClear стер абсолютно все полотно
            glStencilMask(0xFF);

            glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

            // Кут обертання тільки для активного об'єкта
            rotationAngle += 50.0f * (float)deltaTime;

            // Матриці проекції та камери
            glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
            glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

            glBindVertexArray(VAO);
            glBindTexture(GL_TEXTURE_2D, texture);

            // Рендеримо сцену
            for (int i = 0; i < 3; i++) {
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, cubePositions[i]);
                model = glm::scale(model, cubeScales[i]);

                if (i == activeCubeIndex) {
                    model = glm::rotate(model, glm::radians(rotationAngle), glm::vec3(0.5f, 1.0f, 0.0f));

                    // Налаштування запису трафарету для активного елемента (записуємо 1)
                    glStencilFunc(GL_ALWAYS, 1, 0xFF);
                    glStencilMask(0xFF);
                } else {
                    // Неактивні куби не змінюють буфер трафарету
                    glStencilMask(0x00);
                }

                glUseProgram(cubeProgram);
                glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
                glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
                glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

                glDrawArrays(GL_TRIANGLES, 0, 36);

                // Малюємо рамку навколо активного об'єкта за трафаретом
                if (i == activeCubeIndex) {
                    // Малюємо рамку лише там, де значення трафарету НЕ дорівнює 1
                    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
                    glStencilMask(0x00);
                    glDisable(GL_DEPTH_TEST); // Робимо рамку видимою навіть крізь об'єкти

                    glUseProgram(outlineProgram);

                    // Матриця моделі для рамки (збільшена на 5%)
                    glm::mat4 outlineModel = glm::mat4(1.0f);
                    outlineModel = glm::translate(outlineModel, cubePositions[i]);
                    outlineModel = glm::scale(outlineModel, cubeScales[i] * 1.05f);
                    outlineModel = glm::rotate(outlineModel, glm::radians(rotationAngle), glm::vec3(0.5f, 1.0f, 0.0f));

                    glUniformMatrix4fv(glGetUniformLocation(outlineProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
                    glUniformMatrix4fv(glGetUniformLocation(outlineProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
                    glUniformMatrix4fv(glGetUniformLocation(outlineProgram, "model"), 1, GL_FALSE, glm::value_ptr(outlineModel));

                    glDrawArrays(GL_TRIANGLES, 0, 36);

                    // Відновлюємо налаштування для наступних ітерацій кадру
                    glStencilMask(0xFF);
                    glEnable(GL_DEPTH_TEST);
                }
            }

            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(cubeProgram);
    glDeleteProgram(outlineProgram);
    glfwTerminate();
    return 0;
}
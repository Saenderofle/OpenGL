#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

const char* vertexShaderSource = R"glsl(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec2 aTexCoord;
    out vec2 TexCoord;
    void main() {
        gl_Position = vec4(aPos, 1.0);
        TexCoord = aTexCoord;
    }
)glsl";

const char* fragmentShaderSource = R"glsl(
    #version 330 core
    out vec4 FragColor;
    in vec2 TexCoord;
    uniform sampler2D texture1;
    void main() {
        FragColor = texture(texture1, TexCoord);
    }
)glsl";

// Функція для завантаження текстури
unsigned int loadTexture(const char* path) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Налаштування обгортки та фільтрації
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true); // Перевертаємо по осі Y
    unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);
    if (data) {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
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

    GLFWwindow* window = glfwCreateWindow(800, 600, "Three Textured Rectangles", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Компіляція шейдерів
    unsigned int vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertex);

    unsigned int fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragment);

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertex);
    glAttachShader(shaderProgram, fragment);
    glLinkProgram(shaderProgram);

    // Видаляємо шейдери, бо вони вже прилінковані
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    // Геометрія: 3 окремі прямокутники (кожен з 4 вершин)
    float vertices[] = {
        // Позиції X, Y, Z      // Текстурні UV
        // Прямокутник 1 (Лівий верхній)
        -0.8f,  0.8f, 0.0f,    0.0f, 1.0f,
        -0.2f,  0.8f, 0.0f,    1.0f, 1.0f,
        -0.2f,  0.2f, 0.0f,    1.0f, 0.0f,
        -0.8f,  0.2f, 0.0f,    0.0f, 0.0f,

        // Прямокутник 2 (Правий верхній)
         0.2f,  0.8f, 0.0f,    0.0f, 1.0f,
         0.8f,  0.8f, 0.0f,    1.0f, 1.0f,
         0.8f,  0.2f, 0.0f,    1.0f, 0.0f,
         0.2f,  0.2f, 0.0f,    0.0f, 0.0f,

        // Прямокутник 3 (Нижній по центру)
        -0.3f, -0.2f, 0.0f,    0.0f, 1.0f,
         0.3f, -0.2f, 0.0f,    1.0f, 1.0f,
         0.3f, -0.8f, 0.0f,    1.0f, 0.0f,
        -0.3f, -0.8f, 0.0f,    0.0f, 0.0f
    };

    unsigned int indices[] = {
        0, 1, 2,  0, 2, 3,       // Індекси для 1-го
        4, 5, 6,  4, 6, 7,       // Індекси для 2-го
        8, 9, 10, 8, 10, 11      // Індекси для 3-го
    };

    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Завантажуємо 3 різні текстури
    unsigned int tex1 = loadTexture("texture1.jpg");
    unsigned int tex2 = loadTexture("texture2.jpg");
    unsigned int tex3 = loadTexture("texture3.jpg");

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);

        // Малюємо 1-й прямокутник
        glBindTexture(GL_TEXTURE_2D, tex1);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0);

        // Малюємо 2-й прямокутник
        glBindTexture(GL_TEXTURE_2D, tex2);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(6 * sizeof(unsigned int)));

        // Малюємо 3-й прямокутник
        glBindTexture(GL_TEXTURE_2D, tex3);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(12 * sizeof(unsigned int)));

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);
    
    glfwTerminate();
    return 0;
}
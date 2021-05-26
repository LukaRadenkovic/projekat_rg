#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <iostream>


void framebuffer_size_callback(GLFWwindow *window, int width, int height);

void mouse_callback(GLFWwindow *window, double xpos, double ypos);

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

void processInput(GLFWwindow *window);

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

unsigned int loadTexture(const char *path);
unsigned int loadCubemap(vector<std::string> faces);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

//blin

bool blinn = false;
bool blinnKeyPressed = false;

// camera

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

struct PointLight {
    glm::vec3 position;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

struct MovingObject{
    int lopta = -1;
};

struct ProgramState {
    glm::vec3 clearColor = glm::vec3(0);
    Camera camera;
    bool CameraMouseMovementUpdateEnabled = true;
    bool gameStart = false;
    double startTime;
    PointLight pointLight;
    ProgramState()
            : camera(glm::vec3(-2.3,0.5,5.9)) {}

    void SaveToFile(std::string filename);

    void LoadFromFile(std::string filename);
};

void ProgramState::SaveToFile(std::string filename) {
    std::ofstream out(filename);
    out << clearColor.r << '\n'
        << clearColor.g << '\n'
        << clearColor.b << '\n'
        << camera.Position.x << '\n'
        << camera.Position.y << '\n'
        << camera.Position.z << '\n'
        << camera.Front.x << '\n'
        << camera.Front.y << '\n'
        << camera.Front.z << '\n';
}

void ProgramState::LoadFromFile(std::string filename) {
    std::ifstream in(filename);
    if (in) {
        in >> clearColor.r
           >> clearColor.g
           >> clearColor.b
           >> camera.Position.x
           >> camera.Position.y
           >> camera.Position.z
           >> camera.Front.x
           >> camera.Front.y
           >> camera.Front.z;
    }
}

ProgramState *programState;
MovingObject movingObject;


int main() {
    // glfw: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);
    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    programState = new ProgramState;
    programState->LoadFromFile("resources/program_state.txt");


    // configure global opengl state
    glEnable(GL_DEPTH_TEST);

    // build and compile shaders
    Shader ourShader("resources/shaders/2.model_lighting.vs", "resources/shaders/2.model_lighting.fs");
    Shader skyboxShader("resources/shaders/6.1.skybox.vs", "resources/shaders/6.1.skybox.fs");
    Shader transpShader("resources/shaders/transparentobj.vs", "resources/shaders/transparentobj.fs");

    //lightcube
    Shader lightCubeShader("resources/shaders/light_cube.vs", "resources/shaders/light_cube.fs");
    glEnable(GL_DEPTH_TEST);
    // TODO adv
    Shader advShader("resources/shaders/advanced_lighting.vs", "resources/shaders/advanced_lighting.fs");

    // load models
    Model ourModelSuncobran("resources/objects/suncobran/13518_Beach_Umbrella_v1_L3.obj");
    Model ourModelLopta("resources/objects/lopta/13517_Beach_Ball_v2_L3.obj");
    Model ourModelKokos("resources/objects/kokos2/10175_CoconutHalf_L3.obj");

    glEnable(GL_DEPTH_TEST);

    // setting coordinates:

    // transparent background square
    float transparentVertices[] = {
            // positions         // texture coordinates (non-swapped because stbi flips picture when loading)
            0.0f,  0.5f,  0.0f,  0.0f,  1.0f,
            0.0f, -0.5f,  0.0f,  0.0f,  0.0f,
            1.0f, -0.5f,  0.0f,  1.0f,  0.0f,

            0.0f,  0.5f,  0.0f,  0.0f,  1.0f,
            1.0f, -0.5f,  0.0f,  1.0f,  0.0f,
            1.0f,  0.5f,  0.0f,  1.0f,  1.0f
    };


    // skybox
    float skyboxVertices[] = {
            // positions
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            -1.0f,  1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f,  1.0f
    };


    float peskirVertices[] = {
            1.0f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top right
            1.0f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right
            -1.0f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left
            -1.0f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // top left
    };

    unsigned int indices[] = {
            0,1,3,
            1,2,3
    };

    float CubeVertices[] = {
            // positions          // normals           // texture coords
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
            0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
            0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
            0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
            0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
            -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
            -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

            0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
            0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
            0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
            0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
            0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
            0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
            0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
            0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
            0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
            0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
    };


        // TODO adv
    float planeVertices[] = {

//
//            10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,  10.0f,  0.0f,
//            -10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
//            -10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,   0.0f, 10.0f,
//
//            10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,  10.0f,  0.0f,
//            -10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,   0.0f, 10.0f,
//            10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,  10.0f, 10.0f


            5.0f, -0.25f,  5.0f,  0.0f, 1.0f, 0.0f,  10.0f,  0.0f,
            -5.0f, -0.25f,  5.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
            -5.0f, -0.25f, -5.0f,  0.0f, 1.0f, 0.0f,   0.0f, 10.0f,

            5.0f, -0.25f,  5.0f,  0.0f, 1.0f, 0.0f,  10.0f,  0.0f,
            -5.0f, -0.25f, -5.0f,  0.0f, 1.0f, 0.0f,   0.0f, 10.0f,
            5.0f, -0.25f, -5.0f,  0.0f, 1.0f, 0.0f,  10.0f, 10.0f

    };


    unsigned int lightCubeVAO, lightCubeVBO;
    glGenVertexArrays(1, &lightCubeVAO);
    glBindVertexArray(lightCubeVAO);

    glGenBuffers(1, &lightCubeVBO);
    glBindBuffer(GL_ARRAY_BUFFER, lightCubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(CubeVertices), CubeVertices, GL_STATIC_DRAW);

    glBindVertexArray(lightCubeVAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

//    glBindBuffer(GL_ARRAY_BUFFER, lightCubeVBO);
    // note that we update the lamp's position attribute's stride to reflect the updated buffer data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // TODO plane VAO adv
    unsigned int planeVAO, planeVBO;
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);
    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glBindVertexArray(0);


    // skybox VAO
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);


    // TRAVA VAO
    unsigned int transparentTravaVAO, transparentTravaVBO;
    glGenVertexArrays(1, &transparentTravaVAO);
    glGenBuffers(1, &transparentTravaVBO);
    glBindVertexArray(transparentTravaVAO);
    glBindBuffer(GL_ARRAY_BUFFER, transparentTravaVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(transparentVertices), transparentVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);


    //peskir EBO

    unsigned int peskirVBO, peskirVAO, peskirEBO;
    glGenVertexArrays(1, &peskirVAO);
    glGenBuffers(1, &peskirVBO);
    glGenBuffers(1, &peskirEBO);
    glBindVertexArray(peskirVAO);
    glBindBuffer(GL_ARRAY_BUFFER, peskirVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(peskirVertices), peskirVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, peskirEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);


    // texture loading
    stbi_set_flip_vertically_on_load(true);

    unsigned int floorTexture = loadTexture(FileSystem::getPath("resources/textures/pexels-sharon-mccutcheon-3711238.jpg").c_str());
    unsigned int transparentTravaTexture = loadTexture(FileSystem::getPath("resources/textures/grass.png").c_str());
    unsigned int peskirTexture = loadTexture(FileSystem::getPath("resources/textures/pexels-sharon-mccutcheon-3711238.jpg").c_str());

    //lightcube svetlo
    glm::vec3 pointLightPositions[] = {
            glm::vec3(-3.32,0.54,6.8),
            glm::vec3(-1.0f, 3.0f, 4.0f)
    };


    vector<glm::vec3> vegetation
            {
                    //    glm::vec3(-3.0f, 0.0f, 3.5f)
//                    glm::vec3(-3.75f, 0.0f, 3.5f),
                    glm::vec3(10.0f,-8.5f,3.5f),
                    glm::vec3(10.0f,-7.0f,3.5f),
                    glm::vec3(10.0f,-5.5f,3.5f),
                    glm::vec3(10.0f,-8.5f,6.5f),
                    glm::vec3(10.0f,-7.0f,6.5f),
                    glm::vec3(10.0f,-5.5f,6.5f),
            };

    stbi_set_flip_vertically_on_load(false);


    vector<std::string> faces
            {
                    FileSystem::getPath("resources/textures/PalmTrees/posx.jpg"),
                    FileSystem::getPath("resources/textures/PalmTrees/negx.jpg"),
                    FileSystem::getPath("resources/textures/PalmTrees/posy.jpg"),
                    FileSystem::getPath("resources/textures/PalmTrees/negy.jpg"),
                    FileSystem::getPath("resources/textures/PalmTrees/posz.jpg"),
                    FileSystem::getPath("resources/textures/PalmTrees/negz.jpg")
            };

    unsigned int cubemapTexture = loadCubemap(faces);

    transpShader.use();
    transpShader.setInt("texture1", 0);

    advShader.use();
    advShader.setInt("floorTexture", 0);

    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);

    glm::vec3 lightPos(-3.75f, 0.0f, 3.5f);
    // render loop
    while (!glfwWindowShouldClose(window)) {
        // per-frame time logic
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        processInput(window);


        // render
        glClearColor(programState->clearColor.r, programState->clearColor.g, programState->clearColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // point lights
        PointLight& pointLight = programState->pointLight;
        pointLight.position = glm::vec3(-0.2f, -1.0f, -0.3f);
        pointLight.ambient = glm::vec3(0.35, 0.35, 0.35);
        pointLight.diffuse = glm::vec3(0.84, 0.84, 0.84);
        pointLight.specular = glm::vec3(1.0, 1.0, 1.0);


        pointLight.constant = 0.1f;
        pointLight.linear = 0.03f;
        pointLight.quadratic = 0.032f;

        ourShader.use();

        // pointLight1

//        camera(glm::vec3(-2.32,0.54,5.87)
        pointLight.position = glm::vec3(-2.32,0.54,6.8);

        ourShader.setVec3("pointLight.position", pointLight.position);
        ourShader.setVec3("pointLight.ambient", pointLight.ambient);
        ourShader.setVec3("pointLight.diffuse", pointLight.diffuse);
        ourShader.setVec3("pointLight.specular", pointLight.specular);
        ourShader.setFloat("pointLight.constant", pointLight.constant);
        ourShader.setFloat("pointLight.linear", pointLight.linear);
        ourShader.setFloat("pointLight.quadratic", pointLight.quadratic);
        ourShader.setVec3("viewPosition", programState->camera.Position);
        ourShader.setFloat("material.shininess", 32.0f);

        // pointLight2
        //glm::vec3(-2.5f,-1.2f,10.5f)
        //pointLight.position = glm::vec3(8.0f, -5.0f, 30.0f);
        //svetlo
        pointLight.position = glm::vec3(-1.0f, 3.0f, 4.0f);


        ourShader.setVec3("pointLight1.position", pointLight.position);
        ourShader.setVec3("pointLight1.ambient", pointLight.ambient);
        ourShader.setVec3("pointLight1.diffuse", pointLight.diffuse);
        ourShader.setVec3("pointLight1.specular", pointLight.specular);
        ourShader.setFloat("pointLight1.constant", pointLight.constant);
        ourShader.setFloat("pointLight1.linear", pointLight.linear);
        ourShader.setFloat("pointLight1.quadratic", pointLight.quadratic);
        ourShader.setVec3("viewPosition", programState->camera.Position);
        ourShader.setFloat("material.shininess", 32.0f);


        //spotlight:
        ourShader.setVec3("spotLight.position", programState->camera.Position);
        ourShader.setVec3("spotLight.direction", programState->camera.Front);
        ourShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
        ourShader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
        ourShader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
        ourShader.setFloat("spotLight.constant", 0.5f);
        ourShader.setFloat("spotLight.linear", 0.03);
        ourShader.setFloat("spotLight.quadratic", 0.032);
        ourShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
        ourShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));


        glm::mat4 projection = glm::perspective(glm::radians(programState->camera.Zoom), (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = programState->camera.GetViewMatrix();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);




        // rendering loaded models

        //SUNCOBRAN
        glm::mat4 model = glm::mat4(1.0f);

        //glm::vec3(-2.75f, -0.30f, 3.5f)
        model = glm::translate(model,glm::vec3(-2.5f,-1.2f,10.5f));
        model = glm::rotate(model,glm::radians(80.0f),glm::vec3(0.85,0,1.0));
        model = glm::rotate(model,glm::radians(150.0f),glm::vec3(0,1.0,0));
        model = glm::scale(model, glm::vec3(0.017f,0.017f,0.017f));    // it's a bit too big for our scene, so scale it down
        ourShader.setMat4("model", model);
        ourModelSuncobran.Draw(ourShader);

        //LOPTA
        model = glm::mat4(1.0f);
        model = glm::translate(model,glm::vec3(8.0f, -15.0f, 35.0f) + (float)movingObject.lopta * glm::vec3(0.0f, sin(glfwGetTime() * 4) * 4, 2.0f));
        model = glm::rotate(model,glm::radians(30.0f),glm::vec3(0.0,1.0,0.0));
        model = glm::scale(model, glm::vec3(0.2f));
        ourShader.setMat4("model", model);
        ourModelLopta.Draw(ourShader);


        //KOKOS
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-13.0f,-8.0f,3.5f));
        model = glm::rotate(model,glm::radians(-90.0f),glm::vec3(1.0,0,0.0));
        model = glm::scale(model, glm::vec3(0.008f,0.008f,0.008f));
        ourShader.setMat4("model", model);
        ourModelKokos.Draw(ourShader);

        //peskir
        glBindTexture(GL_TEXTURE_2D, peskirTexture);
        transpShader.use();
        projection = glm::perspective(glm::radians(programState->camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        view = programState->camera.GetViewMatrix();

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-3.5f, -7.6f, 25.0f));
        model = glm::rotate(model,glm::radians(90.0f),glm::vec3(0.4,0.0,0.0));
        model = glm::scale(model,glm::vec3(3.0f));

        transpShader.setMat4("model", model);
        transpShader.setMat4("projection", projection);
        transpShader.setMat4("view", view);
        glBindVertexArray(peskirVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        lightCubeShader.use();
        lightCubeShader.setMat4("projection", projection);
        lightCubeShader.setMat4("view", view);

        // we now draw as many light bulbs as we have point lights.
        glBindVertexArray(lightCubeVAO);
        for (unsigned int i = 0; i < 2; i++)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, pointLightPositions[i]);
            model = glm::scale(model, glm::vec3(0.2f)); // Make it a smaller cube
            lightCubeShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // TODO adv
        advShader.use();
        glm::mat4 projectionAdv = glm::perspective(glm::radians(programState->camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 viewAdv = programState->camera.GetViewMatrix();
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-2.5f, -5.6f, 25.0f));
        model = glm::rotate(model,glm::radians(90.0f),glm::vec3(0.4,0.0,0.0));
        model = glm::scale(model,glm::vec3(3.0f));

        transpShader.setMat4("model", model);
        advShader.setMat4("projection", projectionAdv);
        advShader.setMat4("view", viewAdv);

        advShader.setVec3("viewPos", programState->camera.Position);
        advShader.setVec3("lightPos", lightPos);
        advShader.setInt("blinn", blinn);

        glBindVertexArray(planeVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, floorTexture);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        //TRAVA

            transpShader.use();
            projection = glm::perspective(glm::radians(programState->camera.Zoom),
                                          (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 100.0f);

            glBindVertexArray(transparentTravaVAO);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, transparentTravaTexture);

            for (unsigned int i = 0; i < vegetation.size(); i++)
            {
                model = glm::mat4(1.0f);
                model = glm::translate(model, vegetation[i]);
                model = glm::scale(model, glm::vec3(2.5f, 2.5f, 2.5f));
                model = glm::rotate(model,glm::radians(90.0f),glm::vec3(0.0,1.0,0.0));
                transpShader.setMat4("model", model);
                transpShader.setMat4("projection", projection);
                transpShader.setMat4("view", view);
                glDrawArrays(GL_TRIANGLES, 0, 6);
            }



        // drawing skybox as last
        glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
        skyboxShader.use();
        view = glm::mat4(glm::mat3(programState->camera.GetViewMatrix())); // remove translation from the view matrix
        skyboxShader.setMat4("view", view);
        skyboxShader.setMat4("projection", projection);

        // skybox cube
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); // set depth function back to default

        std::cout << (blinn ? "Blinn-Phong" : "Phong") << std::endl;

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    programState->SaveToFile("resources/program_state.txt");
    delete programState;
    // glfw: terminate, clearing all previously allocated GLFW resources.




    glDeleteVertexArrays(1, &peskirVAO);
    //adv
    glDeleteVertexArrays(1, &planeVAO);
    glDeleteBuffers(1, &peskirVBO);
    glDeleteBuffers(1, &peskirEBO);


    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);



//    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
//        programState->camera.ProcessKeyboard(FORWARD, deltaTime);
//    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
//        programState->camera.ProcessKeyboard(BACKWARD, deltaTime);
//    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
//        programState->camera.ProcessKeyboard(LEFT, deltaTime);
//    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
//        programState->camera.ProcessKeyboard(RIGHT, deltaTime);


    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS && !blinnKeyPressed)
    {
        blinn = !blinn;
        blinnKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_RELEASE)
    {
        blinnKeyPressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
        programState->camera.Position = glm::vec3(-2.32,0.54,5.87);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    if (programState->CameraMouseMovementUpdateEnabled)
        programState->camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    programState->camera.ProcessMouseScroll(yoffset);
}



void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_S && action == GLFW_PRESS) {
        if (!programState->gameStart) {
            programState->startTime = glfwGetTime();
        }
        programState->gameStart = true;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }


}


unsigned int loadCubemap(vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}


unsigned int loadTexture(char const * path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}



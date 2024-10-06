/**
* Author: Tanzia Nur
* Assignment: Pong Clone
* Date due: 2024-10-12, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define LOG(argument) std::cout << argument << '\n'

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"
#include "cmath"
#include <ctime>

enum AppStatus { RUNNING, TERMINATED };

constexpr int WINDOW_WIDTH = 640,
WINDOW_HEIGHT = 480;

constexpr float BG_RED = 0.2f,
BG_GREEN = 0.3f,
BG_BLUE = 0.5f,
BG_OPACITY = 1.0f;

constexpr int VIEWPORT_X = 0,
VIEWPORT_Y = 0,
VIEWPORT_WIDTH = WINDOW_WIDTH,
VIEWPORT_HEIGHT = WINDOW_HEIGHT;

constexpr char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

constexpr float MILLISECONDS_IN_SECOND = 1000.0;

constexpr char BROOM_SPRITE_FILEPATH[] = "broom.png",
FIRE_SPRITE_FILEPATH[] = "fireball.png";

constexpr float MINIMUM_COLLISION_DISTANCE = 1.0f;
constexpr float ANGLE = glm::radians(90.0f);
constexpr float ROT_SPEED = 200.0f;

constexpr glm::vec3 INIT_SCALE_BROOM = glm::vec3(0.5f, 1.3611f, 0.0f),
INIT_SCALE_BROOM1 = glm::vec3(0.5f, 1.3611f, 0.0f),
INIT_SCALE_FIRE = glm::vec3(0.33f, 0.4f, 0.0f),
INIT_POS_BROOM = glm::vec3(-4.0f, 0.0f, 0.0f),
INIT_POS_BROOM1 = glm::vec3(4.0f, 0.0f, 0.0f),
INIT_POS_FIRE = glm::vec3(-2.0f, 0.0f, 0.0f);

SDL_Window* g_display_window;

AppStatus g_app_status = RUNNING;
ShaderProgram g_shader_program = ShaderProgram();
glm::mat4 g_view_matrix, g_broom_matrix, g_projection_matrix, g_fire_matrix, g_broom1_matrix;

float g_previous_ticks = 0.0f;

GLuint g_broom_texture_id;
GLuint g_broom1_texutre_id;
GLuint g_fire_texture_id;


glm::vec3 g_broom_position = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_broom1_position = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_broom_movement = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_broom1_movement = glm::vec3(0.0f, 0.0f, 0.0f);

glm::vec3 g_fire_position = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_fire_movement = glm::vec3(0.0f, 0.0f, 0.0f);

float g_broom_speed = 4.0f;  // move 2 units per second
float g_broom1_speed = 4.0f;  // move 2 units per second

void initialise();
void process_input();
void update();
void render();
void shutdown();

constexpr GLint NUMBER_OF_TEXTURES = 1;  
constexpr GLint LEVEL_OF_DETAIL = 0;  
constexpr GLint TEXTURE_BORDER = 0; 

GLuint load_texture(const char* filepath)
{
    // STEP 1: Loading the image file
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);

    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }

    // STEP 2: Generating and binding a texture ID to our image
    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);

    // STEP 3: Setting our texture filter parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // STEP 4: Releasing our file from memory and returning our texture id
    stbi_image_free(image);

    return textureID;
}

void initialise()
{
    SDL_Init(SDL_INIT_VIDEO);
    g_display_window = SDL_CreateWindow("Halloween Pong!",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);


    if (g_display_window == nullptr)
    {
        shutdown();
    }
#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);

    g_broom_matrix = glm::mat4(1.0f);
    g_broom1_matrix = glm::mat4(1.0f);
    g_fire_matrix = glm::mat4(1.0f);
    g_fire_matrix = glm::translate(g_fire_matrix, glm::vec3(1.0f, 1.0f, 0.0f));
    g_fire_position += g_fire_movement;

    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_shader_program.get_program_id());

    glClearColor(BG_RED, BG_GREEN, BG_BLUE, BG_OPACITY);

    g_broom_texture_id = load_texture(BROOM_SPRITE_FILEPATH);
    g_broom1_texutre_id = load_texture(BROOM_SPRITE_FILEPATH);
    g_fire_texture_id = load_texture(FIRE_SPRITE_FILEPATH);

    // enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{
    // VERY IMPORTANT: If nothing is pressed, we don't want to go anywhere
    g_broom_movement = glm::vec3(0.0f);
	g_broom1_movement = glm::vec3(0.0f);

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            // End game
        case SDL_QUIT:
        case SDL_WINDOWEVENT_CLOSE:
            g_app_status = TERMINATED;
            break;

        case SDL_KEYDOWN:
            switch (event.key.keysym.sym)
            {
            case SDLK_q:
                // Quit the game with a keystroke
                g_app_status = TERMINATED;
                break;

            default:
                break;
            }

        default:
            break;
        }
    }


    const Uint8* key_state = SDL_GetKeyboardState(NULL);

    if (key_state[SDL_SCANCODE_UP])
    {
        g_broom1_movement.y = 1.0f;
    }
    else if (key_state[SDL_SCANCODE_DOWN])
    {
        g_broom1_movement.y = -1.0f;
    }

	if (key_state[SDL_SCANCODE_W])
	{
		g_broom_movement.y = 1.0f;
	}
	else if (key_state[SDL_SCANCODE_S])
	{
		g_broom_movement.y = -1.0f;
	}

    // This makes sure that the player can't "cheat" their way into moving
    // faster
    if (glm::length(g_broom1_movement) > 1.0f)
    {
        g_broom1_movement = glm::normalize(g_broom1_movement);
    }
}

void update()
{
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND; // get the current number of ticks
    float delta_time = ticks - g_previous_ticks; // the delta time is the difference from the last frame
    g_previous_ticks = ticks;

    // Add direction * units per second * elapsed time
    g_broom_position += g_broom_movement * g_broom_speed * delta_time;
    g_broom1_position += g_broom1_movement * g_broom1_speed * delta_time;

    g_broom_matrix = glm::mat4(1.0f);
    g_broom_matrix = glm::translate(g_broom_matrix, INIT_POS_BROOM);
    g_broom_matrix = glm::translate(g_broom_matrix, g_broom_position);

	g_broom1_matrix = glm::mat4(1.0f);
	g_broom1_matrix = glm::translate(g_broom1_matrix, INIT_POS_BROOM1);
	g_broom1_matrix = glm::translate(g_broom1_matrix, g_broom1_position);

    g_fire_matrix = glm::mat4(1.0f);
    g_fire_matrix = glm::translate(g_fire_matrix, INIT_POS_FIRE);

    g_broom_matrix = glm::scale(g_broom_matrix, INIT_SCALE_BROOM);
	g_broom1_matrix = glm::scale(g_broom1_matrix, INIT_SCALE_BROOM1);
    g_fire_matrix = glm::scale(g_fire_matrix, INIT_SCALE_FIRE);

    float x_distance = fabs(g_broom_position.x + INIT_POS_BROOM.x - INIT_POS_FIRE.x) -
        ((INIT_SCALE_FIRE.x + INIT_SCALE_BROOM.x) / 2.0f);

    float y_distance = fabs(g_broom_position.y + INIT_POS_BROOM.y - INIT_POS_FIRE.y) -
        ((INIT_SCALE_FIRE.y + INIT_SCALE_BROOM.y) / 2.0f);

    if (x_distance < -0.19f && y_distance < 0.0f)
    {
       // std::cout << std::time(nullptr) << ": Collision.\n";
        g_app_status = TERMINATED;
    } 
}
void draw_object(glm::mat4& object_model_matrix, GLuint& object_texture_id)
{
    g_shader_program.set_model_matrix(object_model_matrix);
    glBindTexture(GL_TEXTURE_2D, object_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6); // we are now drawing 2 triangles, so we use 6 instead of 3
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Vertices
    float vertices[] = {
        -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,  // triangle 1
        -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f   // triangle 2
    };

    // Textures
    float texture_coordinates[] = {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,     // triangle 1
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,     // triangle 2
    };

    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());

    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    // Bind texture
    draw_object(g_broom_matrix, g_broom_texture_id);
	draw_object(g_broom1_matrix, g_broom_texture_id);
    draw_object(g_fire_matrix, g_fire_texture_id);

    // We disable two attribute arrays now
    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    SDL_GL_SwapWindow(g_display_window);
}

void shutdown() { SDL_Quit(); }


int main(int argc, char* argv[])
{
    initialise();

    while (g_app_status == RUNNING)
    {
        process_input();
        update();
        render();
    }

    shutdown();
    return 0;
}
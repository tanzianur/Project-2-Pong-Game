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

constexpr glm::vec3 INIT_SCALE_BROOM = glm::vec3(0.5f, 1.3611f, 0.0f),
INIT_SCALE_BROOM1 = glm::vec3(0.5f, 1.3611f, 0.0f),
INIT_SCALE_FIRE = glm::vec3(0.33f, 0.4f, 0.0f),
INIT_POS_BROOM = glm::vec3(-4.5f, 0.0f, 0.0f),
INIT_POS_BROOM1 = glm::vec3(4.5f, 0.0f, 0.0f),
INIT_POS_FIRE = glm::vec3(0.0f, 0.0f, 0.0f);

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

const float g_broom_speed = 4.0f;
const float g_broom1_speed = 4.0f;

bool broom_upper = false;
bool broom_lower = false;

bool broom1_upper = false;
bool broom1_lower = false;

float ROT_ANGLE = 0.0f;
float ROT_FIRE_SPEED = 45.0f;

bool left_collision = false,
right_collision = false,
upper_collision = false,
bottom_collision = false;

float x_speed = 0.0f;
float y_speed = 0.0f;

bool single_player = false;
bool game_end = false;

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

// Check if ball hits the broomsticks
bool collision(glm::vec3& position_1, glm::vec3 &position_2, const glm::vec3 &init_pos) {
    float x_distance = fabs(position_1.x - init_pos.x) - ((INIT_SCALE_FIRE.x + INIT_SCALE_BROOM.x) / 2.0f);

    float y_distance = fabs(position_1.y - position_2.y) - ((INIT_SCALE_FIRE.y + INIT_SCALE_BROOM.y) / 2.0f);

    if (x_distance < -0.19 && y_distance <= -0.1) { // had to adjust from 0 because there's invisible padding on the paddles
        return true;
    }
    else {
        return false;
    }
}

// Check if ball hits left or right walls
bool is_out_of_bounds(glm::vec3 fire_position) {
    float left_bound = -5.0f;
    float right_bound = 5.0f;

    if (fire_position.x < left_bound || fire_position.x > right_bound) {
        return true;
    }
    return false;
}

// Fireball movement
void fire_direction() {
    g_fire_movement = glm::vec3(0.0f);
    static float speed_scale = 1.0f;

    if (collision(g_fire_position, g_broom_position, INIT_POS_BROOM)) {
        right_collision = false;
        left_collision = true;
        // using random numbers for x speed and y speed of ball
        x_speed = ((rand() % 100) / 100.0) + 2.5f;
        y_speed = ((rand() % 100) / 100.0) / 2.5f;

        double random_dir = (rand() % 100) / 100.0;
        if (random_dir >= 0.5) {
            upper_collision = true;
            bottom_collision = false;
        }
        else {
            upper_collision = false;
            bottom_collision = true;
        }
        speed_scale += 0.05; // slight speed boost upon collision with paddle
    }
    else if (collision(g_fire_position, g_broom1_position, INIT_POS_BROOM1)) {
        right_collision = true;
        left_collision = false;
        x_speed = ((rand() % 100) / 100.0) + 2.5f;
        y_speed = ((rand() % 100) / 100.0) / 2.5f;

        double random_dir = (rand() % 100) / 100.0;
        if (random_dir >= 0.5) {
            upper_collision = true;
            bottom_collision = false;
        }
        else {
            upper_collision = false;
            bottom_collision = true;
        }
        speed_scale += 0.05f;
    }
    if (g_fire_position.y > 3.5f) {
        upper_collision = false;
        bottom_collision = true;
    }
    else if (g_fire_position.y < -3.5f) {
        upper_collision = true;
        bottom_collision = false;
    }

    if (left_collision) {
        g_fire_movement.x = x_speed;
    }
    else if (right_collision) {
        g_fire_movement.x = -x_speed;
    }
    else {
        g_fire_movement.x = 1;
    }
    if (upper_collision) {
        g_fire_movement.y = y_speed;
    }
    else if (bottom_collision) {
        g_fire_movement.y = -y_speed;
    }
    else {
        g_fire_movement.y = 0.0f;
    }
    if (g_fire_position.x < -5.0) {
        std::cout << "player 2 wins\n";

    }
    else if (g_fire_position.x > 5.0) {
        std::cout << "player 1 wins\n";
    }
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
            case SDLK_t:
                single_player = false;
                break;

            default:
                break;
            }

        default:
            break;
        }
    }


    const Uint8* key_state = SDL_GetKeyboardState(NULL);

    if (key_state[SDL_SCANCODE_UP] and !broom1_upper)
    {
        g_broom1_movement.y = 1.0f;
    }
    else if (key_state[SDL_SCANCODE_DOWN] and !broom1_lower)
    {
        g_broom1_movement.y = -1.0f;
    }

	if (key_state[SDL_SCANCODE_W] and !broom_upper)
	{
		g_broom_movement.y = 1.0f;
	}
	else if (key_state[SDL_SCANCODE_S] and !broom_lower)
	{
		g_broom_movement.y = -1.0f;
	}
    if (key_state[SDL_SCANCODE_T]) {
        single_player = !single_player;
    }

    // setting broom1 bounds
    if (g_broom1_position.y > 3.0f) {
        broom1_upper = true;
    }
    else {
        broom1_upper = false;
    }
    if (g_broom1_position.y < -3.0f) {
        broom1_lower = true;
    }
    else {
        broom1_lower = false;
    }

    // setting broom bounds
    if (g_broom_position.y > 3.0f) {
        broom_upper = true;
    }
    else {
        broom_upper = false;
    }
    if (g_broom_position.y < -3.0f) {
        broom_lower = true;
    }
    else {
        broom_lower = false;
    }

    if (glm::length(g_broom1_movement) > 1.0f)
    {
        g_broom1_movement = glm::normalize(g_broom1_movement);
    }
    if (glm::length(g_broom_movement) > 1.0f)
    {
        g_broom_movement = glm::normalize(g_broom_movement);
    }
}

void update()
{
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - g_previous_ticks; 
    g_previous_ticks = ticks;

    // Add direction * units per second * elapsed time
    g_broom_position += g_broom_movement * g_broom_speed * delta_time;
    g_broom1_position += g_broom1_movement * g_broom1_speed * delta_time;
    g_fire_position += g_fire_movement * delta_time;

    if (is_out_of_bounds(g_fire_position)) {
        game_end = true;
        g_fire_position = glm::vec3(0.0f, 0.0f, 0.0f);
        g_fire_movement = glm::vec3(0.5f, 0.5f, 0.0f);
        ROT_ANGLE = 0.0f;

        g_broom_position = glm::vec3(0.0f, 0.0f, 0.0f);
        g_broom1_position = glm::vec3(0.0f, 0.0f, 0.0f);
    }
    
    //game_end = false;
    g_broom_matrix = glm::mat4(1.0f);
    g_broom_matrix = glm::translate(g_broom_matrix, INIT_POS_BROOM);
    g_broom_matrix = glm::translate(g_broom_matrix, g_broom_position);

	g_broom1_matrix = glm::mat4(1.0f);
	g_broom1_matrix = glm::translate(g_broom1_matrix, INIT_POS_BROOM1);
	g_broom1_matrix = glm::translate(g_broom1_matrix, g_broom1_position);

    g_fire_matrix = glm::mat4(1.0f);
    g_fire_matrix = glm::translate(g_fire_matrix, INIT_POS_FIRE);
    g_fire_matrix = glm::translate(g_fire_matrix, g_fire_position);

    g_broom_matrix = glm::scale(g_broom_matrix, INIT_SCALE_BROOM);
	g_broom1_matrix = glm::scale(g_broom1_matrix, INIT_SCALE_BROOM1);
    g_fire_matrix = glm::scale(g_fire_matrix, INIT_SCALE_FIRE);

    ROT_ANGLE += ROT_FIRE_SPEED * delta_time;


}


void draw_object(glm::mat4& object_model_matrix, GLuint& object_texture_id)
{
    g_shader_program.set_model_matrix(object_model_matrix);
    glBindTexture(GL_TEXTURE_2D, object_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6); 
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
        if (game_end == false) {
            fire_direction();
            update();
        }

        render();
    }

    shutdown();
    return 0;
}
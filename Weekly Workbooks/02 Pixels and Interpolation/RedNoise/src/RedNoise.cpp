#include <CanvasTriangle.h>
#include <DrawingWindow.h>
#include <Utils.h>
#include <fstream>
#include <vector>
#include "glm/vec3.hpp"

#define WIDTH 320
#define HEIGHT 240


std::vector<glm::vec3> interpolateThreeElementValues(const glm::vec3& from, const glm::vec3& to, int numberOfValues) {
    if (numberOfValues <= 1) {
        std::cerr << "Error: numberOfValues must be greater than 1." << std::endl;
        return {}; // Return an empty vector in case of error
    }

    std::vector<glm::vec3> result;
    glm::vec3 step = (to - from) / static_cast<float>(numberOfValues - 1);

    for (int i = 0; i < numberOfValues; ++i) {
        result.push_back(from + glm::vec3(i * step.x, i * step.y, i * step.z));
    }

    return result;
}

// return an evenly spaced list of size numberOfValues
std::vector<float> interpolateSingleFloats(float from, float to, int numberOfValues){

    if (numberOfValues <= 1) {
        std::cerr << "Error: numberOfValues must be greater than 1." << std::endl;
        return {}; // Return an empty vector in case of error
    }

    std::vector<float> result;
    float step = (to - from) / (float(numberOfValues) - 1);

    for (int i = 0; i < numberOfValues; ++i) {
        result.push_back(from + i * step);
    }

    return result;
}

void draw(DrawingWindow &window) {
    window.clearPixels();
    // create a vector of floats from 255 to 0 left to right
    std::vector<float> interpolationValues = interpolateSingleFloats(255.f, 0.0f, window.width);

    // create 4 corner values
    glm::vec3 topLeft(255, 0, 0);        // red
    glm::vec3 topRight(0, 0, 255);       // blue
    glm::vec3 bottomRight(0, 255, 0);    // green
    glm::vec3 bottomLeft(255, 255, 0);   // yellow

    // Create vectors
    std::vector<glm::vec3> interpolationValuesLeft = interpolateThreeElementValues(topLeft, bottomLeft, window.height);
    std::vector<glm::vec3> interpolationValuesRight = interpolateThreeElementValues(topRight, bottomRight, window.height);

    for (size_t y = 0; y < window.height; y++) {
        // Create a vector of interpolated RGB colors for the current row
        std::vector<glm::vec3> interpolationValuesRow = interpolateThreeElementValues(interpolationValuesLeft[y], interpolationValuesRight[y], window.width);

        for (size_t x = 0; x < window.width; x++) {
            glm::vec3 color = interpolationValuesRow[x];

            // Convert RGB color to uint32_t
            uint32_t red = static_cast<uint32_t>(color.r);
            uint32_t green = static_cast<uint32_t>(color.g);
            uint32_t blue = static_cast<uint32_t>(color.b);
            uint32_t alpha = 255;
            uint32_t pixelColor = (alpha << 24) + (red << 16) + (green << 8) + blue;

            window.setPixelColour(x, y, pixelColor);
        }
    }
}

void handleEvent(SDL_Event event, DrawingWindow &window) {
    if (event.type == SDL_KEYDOWN) {
        if (event.key.keysym.sym == SDLK_LEFT) std::cout << "LEFT" << std::endl;
        else if (event.key.keysym.sym == SDLK_RIGHT) std::cout << "RIGHT" << std::endl;
        else if (event.key.keysym.sym == SDLK_UP) std::cout << "UP" << std::endl;
        else if (event.key.keysym.sym == SDLK_DOWN) std::cout << "DOWN" << std::endl;
    } else if (event.type == SDL_MOUSEBUTTONDOWN) {
        window.savePPM("output.ppm");
        window.saveBMP("output.bmp");
    }
}

int main(int argc, char *argv[]) {
    DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);
    SDL_Event event;

    // test for interpolateSingleFloats
    std::vector<float> result;
    result = interpolateSingleFloats(2.2, 8.5, 7);
    for(size_t i=0; i<result.size(); i++) std::cout << result[i] << " ";
    std::cout << std::endl;

    //test for interpolateThreeElementValues
    glm::vec3 from(1.0f, 4.0f, 9.2f);
    glm::vec3 to(4.0f, 1.0f, 9.8f);
    int numberOfValues = 4; // Adjust the number of values as needed

    std::vector<glm::vec3> interpolatedValues = interpolateThreeElementValues(from, to, numberOfValues);

    for (const glm::vec3& value : interpolatedValues) {
        std::cout << "(" << value.x << ", " << value.y << ", " << value.z << ") ";
    }
    std::cout << std::endl;

    while (true) {
        // We MUST poll for events - otherwise the window will freeze !
        if (window.pollForInputEvents(event)) handleEvent(event, window);
        draw(window);
        // Need to render the frame at the end, or nothing actually gets shown on the screen !
        window.renderFrame();
    }
}
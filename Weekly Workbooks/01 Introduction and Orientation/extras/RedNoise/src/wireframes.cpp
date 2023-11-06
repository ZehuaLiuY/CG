#include "wireframes.h"
#include "triangle.h"
#include "interpolation.h"

// Task 2: read obj file
std::vector<ModelTriangle> loadOBJ(std::string filename, std::map<std::string, Colour> palette, float scale) {
    std::ifstream objFile(filename);
    if (!objFile.is_open()) {
        std::cerr << "Failed to open the OBJ file: " << filename << std::endl;
        return {};
    }

    std::string line;
    std::vector<glm::vec3> vertices;
    std::vector<ModelTriangle> modelTriangles;
    std::string index;

    // get each line of the file
    while (std::getline(objFile, line)) {
        std::vector<std::string> values = split(line, ' ');
        // if the line is a new vertex, add it to the vertices, if it's a new triangle, add it to the modelTriangles
        if (values[0] == "usemtl") {
            index = values[1];
        } else if (values[0] == "v") {
            vertices.push_back(scale * glm::vec3 (std::stof(values[1]), std::stof(values[2]), std::stof(values[3])));
        } else if (values[0] == "f") {
            glm::vec3 vertex1 = vertices[std::stoi(values[1].substr(0, values[1].find('/'))) - 1];
            glm::vec3 vertex2 = vertices[std::stoi(values[2].substr(0, values[2].find('/'))) - 1];
            glm::vec3 vertex3 = vertices[std::stoi(values[3].substr(0, values[3].find('/'))) - 1];
            modelTriangles.push_back(ModelTriangle(vertex1, vertex2, vertex3, palette[index]));
            // std::cout << "Assigned color for triangle: " << palette[index] << std::endl;

        }
    }

    objFile.close();
    return modelTriangles;
}

// Task 3: read mtl file
std::map<std::string, Colour> loadMTL(std::string filename) {
    std::ifstream mtlFile(filename);
    if (!mtlFile.is_open()) {
        std::cerr << "Failed to open the OBJ file: " << filename << std::endl;
        return {};
    }

    std::string line;
    std::map<std::string, Colour> palette;
    std::vector<std::string> index;
    std::vector<Colour> colours;

    // get each line of the file
    while (std::getline(mtlFile, line)) {
        std::vector<std::string> values = split(line, ' ');
        // if the line is a new colour, add it to the palette
        if (values[0] == "newmtl") {
            index.push_back(values[1]);
        } else if (values[0] == "Kd") {
            Colour tempColour = Colour(std::stof(values[1]) * 255, std::stof(values[2]) * 255, std::stof(values[3]) * 255);
            colours.push_back(tempColour);
        }
    }
    // add the colours to the palette
    for (unsigned int i = 0; i < index.size(); i++) {
        palette[index[i]] = colours[i];
    }

    mtlFile.close();
    return palette;
}

// files reader wrapper
std::vector<ModelTriangle> readFiles(const std::string& objFilename, const std::string& mtlFilename, float scalingFactor) {
    std::map<std::string, Colour> palette = loadMTL(mtlFilename);
    std::vector<ModelTriangle> modelTriangles = loadOBJ(objFilename, palette, scalingFactor);

    return modelTriangles;
}

// Task 4 & 5 : render a graphical representation of the sample mode

// New function for return the 2D CanvasPoint position
CanvasPoint getCanvasIntersectionPoint (glm::vec3 &cameraPosition, glm::mat3 &cameraOrientation, glm::vec3 vertexPosition, float focalLength) {
    glm::vec3 cameraCoordinate = vertexPosition - cameraPosition;
    // update the cameraCoordinate by multiplying the cameraOrientation
    cameraCoordinate =  cameraCoordinate * cameraOrientation;
    // position on the image plane (ui, vi)
    // multiplier fot 160 looks good, if set to 240, many points will be out of scope 180 for bigger image to ob the ray tracing
    float ui = focalLength * ((cameraCoordinate.x) / abs(cameraCoordinate.z)) * 160 + (WIDTH / 2);
    // top left corner is (0,0) to bottom right corner is (WIDTH, HEIGHT)
    float vi = HEIGHT - (focalLength * ((cameraCoordinate.y) / abs(cameraCoordinate.z)) * 160 + (HEIGHT / 2));
    float depthValue = cameraCoordinate.z; // Set the depth value
    CanvasPoint intersectionPoint = CanvasPoint(ui, vi, depthValue); // Pass the depth value to the CanvasPoint constructor
    return intersectionPoint;
}


// Task 6
void drawPoints(DrawingWindow &window, glm::vec3 &cameraPosition, glm::mat3 &cameraOrientation, const std::vector<ModelTriangle> modelTriangles, uint32_t colour) {
//    glm::vec3 cameraPosition = glm::vec3 (0.0, 0.0, 4.0);
    float focalLength = 2.0;
    for (ModelTriangle modelTriangle : modelTriangles) {
        for (glm::vec3 points3d : modelTriangle.vertices) {
            CanvasPoint point = getCanvasIntersectionPoint(cameraPosition, cameraOrientation, points3d, focalLength);
            window.setPixelColour(round(point.x), round(point.y), colour);
        }
    }
}

// Task 7: Wireframe Render
// function for transform 3D ModelTriangle to 2D CanvasTriangle
// add new attribute "colour" to each triangle
std::vector<std::pair<CanvasTriangle, Colour>> triangleTransformer(const std::vector<ModelTriangle> modelTriangles, glm::vec3 &cameraPosition, glm::mat3 &cameraOrientation) {
    std::vector<std::pair<CanvasTriangle, Colour>> canvasTriangles;
    // add assigned colour
    std::vector<Colour> colour;

    float focalLength = 2.0;

    for (ModelTriangle modelTriangle : modelTriangles) {
        std::vector<CanvasPoint> canvasPoint;
        Colour colour = modelTriangle.colour;
        for (glm::vec3 points3d : modelTriangle.vertices) {
            CanvasPoint point = getCanvasIntersectionPoint(cameraPosition,cameraOrientation, points3d, focalLength);
            canvasPoint.push_back(point);
        }
        CanvasTriangle triangle = CanvasTriangle(canvasPoint[0], canvasPoint[1], canvasPoint[2]);
        canvasTriangles.push_back({triangle, colour});
    }

    return canvasTriangles;
}

// Task 9: find the weights of the point
std::vector<std::vector<float>> depthBuffer(HEIGHT, std::vector<float> (WIDTH, 0));

// week 5
// function for reset the depth buffer to 0
void resetDepthBuffer () {
    std::vector<std::vector<float>> init (HEIGHT, std::vector<float>(WIDTH, 0));
    depthBuffer = init;

    // 2 for loop, set all the value to INT32_MIN smaller than 0
}

float findDepth(float x, float y, CanvasTriangle triangle) {
    CanvasPoint top = triangle.vertices[0];
    CanvasPoint middle = triangle.vertices[1];
    CanvasPoint bottom = triangle.vertices[2];

    float ratio = (middle.y - bottom.y) * (top.x - bottom.x) + (bottom.x - middle.x) * (top.y - bottom.y);
    float a = ((middle.y - bottom.y) * (x - bottom.x) + (bottom.x - middle.x) * (y - bottom.y)) / ratio;
    float b = ((bottom.y - top.y) * (x - bottom.x) + (top.x - bottom.x) * (y - bottom.y)) / ratio;
    float c = 1.0f - a - b;
    float z = abs(a * top.depth + b * middle.depth + c * bottom.depth);
//    std::cout << "z: " << z << std::endl;
//    std::cout << "depthBuffer: " << depthBuffer[round(x)][round(y)] << std::endl;
//    std::cout << "1/z: " << 1/z << std::endl;

    return 1/z;
}
// avoid some point out of the window
std::pair<CanvasPoint, CanvasPoint> clipLine(DrawingWindow &window, CanvasPoint from, CanvasPoint to) {
    int xMin = 0;
    int xMax = WIDTH - 1;
    int yMin = 0;
    int yMax = HEIGHT - 1;

    bool fromInside = (from.x >= xMin && from.x <= xMax && from.y >= yMin && from.y <= yMax);
    bool toInside = (to.x >= xMin && to.x <= xMax && to.y >= yMin && to.y <= yMax);

    if (fromInside && toInside) {
        return {from, to};
    }

    float m = (to.y - from.y) / (to.x - from.x);

    if (!fromInside) {
        if (from.x < xMin) {
            from.y += m * (xMin - from.x);
            from.x = xMin;
        } else if (from.x > xMax) {
            from.y += m * (xMax - from.x);
            from.x = xMax;
        }

        if (from.y < yMin) {
            from.x += (yMin - from.y) / m;
            from.y = yMin;
        } else if (from.y > yMax) {
            from.x += (yMax - from.y) / m;
            from.y = yMax;
        }
    }

    if (!toInside) {
        if (to.x < xMin) {
            to.y += m * (xMin - to.x);
            to.x = xMin;
        } else if (to.x > xMax) {
            to.y += m * (xMax - to.x);
            to.x = xMax;
        }

        if (to.y < yMin) {
            to.x += (yMin - to.y) / m;
            to.y = yMin;
        } else if (to.y > yMax) {
            to.x += (yMax - to.y) / m;
            to.y = yMax;
        }
    }

    return {from, to};
}


void drawLineWithDepth(DrawingWindow &window, CanvasPoint from, CanvasPoint to, Colour colour, CanvasTriangle triangle) {
//    auto clipped = clipLine(window, from, to);
//    from = clipped.first;
//    to = clipped.second;

    float xDiff = to.x - from.x;
    float yDiff = to.y - from.y;
    float numberOfSteps = std::max(abs(xDiff), abs(yDiff));
    float xStepSize = xDiff / numberOfSteps;
    float yStepSize = yDiff / numberOfSteps;
    uint32_t uIntColour = colourConverter(colour);
    for (float i = 0.0; i <= numberOfSteps; i++) {
        float x = from.x + (xStepSize * i);
        float y = from.y + (yStepSize * i);
        float z = findDepth(x, y, triangle);
        if (depthBuffer[int(round(y))][int(round(x))] < z) {
            depthBuffer[int(round(y))][int(round(x))] = z;
            window.setPixelColour(int(round(x)), int(round(y)), uIntColour);
        }
    }
}

// Task 8 new function for drawing filled triangle, not the same as the one in week3.cpp
// using interpolation instead of slope
// new function for drawing filled triangle by interpolation
void drawFilledTriangles(DrawingWindow &window, const CanvasTriangle &triangle, Colour &fillColour) {
    std::array<CanvasPoint, 4> sortedPoints = calculateExtraPoint(triangle);

    CanvasPoint top = sortedPoints[0];
    CanvasPoint left = sortedPoints[1];
    CanvasPoint right = sortedPoints[2];
    CanvasPoint bottom = sortedPoints[3];

    if (left.x > right.x) {
        std::swap(left, right);
    }

    // Calculate the interpolation step for left and right edges of the top part of the triangle
    float topToMidY = abs(top.y - left.y);
    float xStepRT = (right.x - top.x) / topToMidY;
    float xStepLT = (left.x - top.x) / topToMidY;
    // Calculate the interpolation step for left and right edges of the bottom part of the triangle
    float midToBottomY = abs((bottom.y - left.y));
    float xStepBR = (bottom.x - right.x) / midToBottomY;
    float xStepBL = (bottom.x - left.x) / midToBottomY;

    // Draw the top part of the triangle
    for (float i = 0; i < topToMidY; i++) {
        CanvasPoint start = CanvasPoint(int(round(top.x + xStepRT * i)), top.y + i);
        CanvasPoint end = CanvasPoint(int(round(top.x + xStepLT * i)), top.y + i);
        drawLineWithDepth(window, start, end, fillColour, triangle);
    }
    // Draw the bottom part of the triangle
    for (float i = 0; i < midToBottomY; i++) {
        CanvasPoint start = CanvasPoint(int(round(right.x + xStepBR * i)), right.y + i);
        CanvasPoint end = CanvasPoint(int(round(left.x + xStepBL * i)), left.y + i);
        drawLineWithDepth(window, start, end, fillColour, triangle);
    }
}
// render the wireframe
void renderWireframe(DrawingWindow &window, glm::vec3 &cameraPosition, glm::mat3 &cameraOrientation) {
    std::vector<ModelTriangle> modelTriangles = readFiles("../src/files/cornell-box.obj", "../src/files/cornell-box.mtl", 0.35);
//    for (const ModelTriangle& modelTriangle : modelTriangles) {
//        std::cout << modelTriangle << std::endl;
//    }
    // Task 6:
//    Colour pointsColour = Colour(255,255,255);
//    uint32_t colour = colourConverter(pointsColour);
//
//    drawPoints(window, modelTriangles, colour);
    // Task 7:
    std::vector<std::pair<CanvasTriangle, Colour>> triangles = triangleTransformer(modelTriangles, cameraPosition, cameraOrientation);

    for (const auto& trianglePair : triangles) {
        CanvasTriangle triangle = trianglePair.first;
        Colour fillColour = trianglePair.second;
        // works good for draw stroked triangle with colour
        // drawStrokedTriangle(window, triangle, fillColour);
        drawFilledTriangles(window, triangle, fillColour);
    }

}

// week 5 tasks:
// camera position and orientation
// translate the camera position in 3 dimensions (up/down, left/right, forwards/backwards)
void translateCamera(glm::vec3 &cameraPosition, float x, float y, float z) {
    cameraPosition.x += x;
    cameraPosition.y += y;
    cameraPosition.z += z;
}

// rotate the camera position about the centre of the scene
// rotate by x-axis (UP/DOWN)
void rotateCameraByX(glm::vec3 &cameraPosition) {
    float angle = 0.01;
    glm::mat3 rotationMatrix(
            1, 0, 0,
            0, std::cos(angle), -1 * std::sin(angle),
            0, std::sin(angle), std::cos(angle));
    cameraPosition = rotationMatrix * cameraPosition;
}

// rotate by y-axis (LEFT/RIGHT)
void rotateCameraByY(glm::vec3 &cameraPosition) {
    float angle = 0.01;
    glm::mat3 rotationMatrix(
            std::cos(angle), 0, std::sin(angle),
            0,  1, 0,
            -1 * std::sin(angle), 0, std::cos(angle));
    cameraPosition = rotationMatrix * cameraPosition;
}

void rotateUp(glm::vec3 &cameraPosition, glm::mat3 &cameraOrientation, float angle) {
    glm::mat3 rotationMatrix(
            1, 0, 0,
            0, std::cos(angle), -1 * std::sin(angle),
            0, std::sin(angle), std::cos(angle));
    cameraPosition = rotationMatrix * cameraPosition;
    lookAt(cameraPosition, cameraOrientation);
}

void rotateClock(glm::vec3& cameraPosition, glm::mat3& cameraOrientation, float angle) {
    glm::mat3 rotationMatrix(
            std::cos(angle), 0, std::sin(angle),
            0,  1, 0,
            -1 * std::sin(angle), 0, std::cos(angle));
    cameraPosition = rotationMatrix * cameraPosition;
    lookAt(cameraPosition, cameraOrientation);
}

void lookAt(glm::vec3 &cameraPosition, glm::mat3 &cameraOrientation) {
    // (0,0,0) is the centre of the scene
    glm::vec3 forward = glm::normalize(cameraPosition - glm::vec3(0, 0, 0));
    glm::vec3 right = glm::normalize(glm::cross(glm::vec3(0, 1, 0), forward));
    glm::vec3 up = glm::normalize(glm::cross(forward, right));

    cameraOrientation[0] = right;
    cameraOrientation[1] = up;
    cameraOrientation[2] = forward;
}

void orbitClockwise(DrawingWindow &window, glm::vec3 &cameraPosition, glm::mat3 &cameraOrientation, float step) {
    rotateClock(cameraPosition, cameraOrientation, step);
}
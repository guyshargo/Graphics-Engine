#include <iostream>
#include <glm/glm.hpp>
#include <fstream>
#include "WorldModel.h"
#include "Model.h"

int main() {
    // Image dimensions
    const int image_width = 256;
    const int image_height = 256;

    Model sceneModel;
    sceneModel.fovXdegree = 90.0f;

    // Create WorldModel instance
    WorldModel worldModel(image_width, image_height, sceneModel.fovXdegree);

    // Create and open a text file to write our image data
    std::ofstream imageFile("output.ppm");

    // PPM Header: P3 means ASCII colors, followed by width, height, and max color value (255)
    imageFile << "P3\n" << image_width << " " << image_height << "\nMax Color Value: 255\n\n";

    // Write the pixel data
    for (int j = image_height - 1; j >= 0; --j) {
        for (int i = 0; i < image_width; ++i) {

            glm::vec3 pixelColor = worldModel.renderPixel(i, j);

            // Convert 0.0-1.0 range to 0-255 integer range
            int ir = static_cast<int>(255.999 * pixelColor.r);
            int ig = static_cast<int>(255.999 * pixelColor.g);
            int ib = static_cast<int>(255.999 * pixelColor.b);

            // Write the RGB values to the file
            imageFile << ir << ' ' << ig << ' ' << ib << '\n';
        }
    }

    imageFile.close();
    std::cout << "Success: output.ppm has been generated in your build directory!\n";
    
    return 0;
}
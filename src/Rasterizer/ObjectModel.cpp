#include "ObjectModel.h"
#include "RasterizerWorld.h"
#include "BarycentricCoordinates.h"
#include "Utilities.h"
#include <iostream>
#include <algorithm>

RasterizationExerciseEnum ObjectModel::exercise;

// Constructor
ObjectModel::ObjectModel(RasterizerWorld* rasterizerWorld, int imageWidth, int imageHeight)
    : rasterizerWorld(rasterizerWorld), imageWidth(imageWidth), imageHeight(imageHeight) {
}

void ObjectModel::initTransformations() {
    modelM = glm::mat4{1.0f};
    lookatM = glm::mat4{1.0f};
    projectionM = glm::mat4{1.0f};
    viewportM = glm::mat4{1.0f};
}

void ObjectModel::setModelM(const glm::mat4& m) { modelM = m; }
void ObjectModel::setLookatM(const glm::mat4& m) { lookatM = m; }
void ObjectModel::setProjectionM(const glm::mat4& m) { projectionM = m; }
void ObjectModel::setViewportM(const glm::mat4& m) { viewportM = m; }

glm::vec3 ObjectModel::getBoundingBoxDimensions() const { return boundingBoxDimensions; }
glm::vec3 ObjectModel::getBoundingBoxCenter() const { return boundingBoxCenter; }

void ObjectModel::setTextureCallbacks(TextureGetPixel getPixel, TextureGetDimension getWidth, TextureGetDimension getHeight) {
    textureGetPixel = getPixel;
    textureGetWidth = getWidth;
    textureGetHeight = getHeight;
    hasTexture = true;
}

bool ObjectModel::load(const std::string& fileName) {

    OBJLoader objLoader;

    try {
        objLoader.loadOBJ(fileName);
        verticesData = objLoader.getVertices();
        faces = objLoader.getFaces();
        boundingBoxDimensions = objLoader.getBoundingBoxDimensions();
        boundingBoxCenter = objLoader.getBoundingBoxCenter();
        return true;

    } catch (const std::exception& e) {
        std::cerr << "Failed to load object model '" << fileName
                  << "': " << e.what() << std::endl;
        return false;
    }
}

bool ObjectModel::objectHasTexture() const { return hasTexture; }

void ObjectModel::render(const SetPixelCallback& setPixel) {

    exercise = rasterizerWorld -> exercise;

    // homogenic vector of light position for transformations
    glm::vec4 homoLightPos(rasterizerWorld -> lightPositionWorldCoordinates, 1.0f);

    // lookat transformation: light source will change position according to eye\camera position
    homoLightPos = lookatM * homoLightPos;

    // actual update of light source position vector with non-homo vector
    lightPositionEyeCoordinates = glm::vec3(homoLightPos);

    if (!verticesData.empty()) {

        // going through all vertices to figure out where they are on the screen
        for (VertexData& vertexData : verticesData) {
            vertexProcessing(setPixel, vertexData);
        }

        // going through all faces to paint pixels in them on screen
        for (const TriangleFace& face : faces) {
            rasterization(setPixel, 
                          verticesData[face.indices[0]],
                          verticesData[face.indices[1]],
                          verticesData[face.indices[2]],
                          face.color);
        }
    }
}

void ObjectModel::vertexProcessing(const SetPixelCallback& setPixel, VertexData& vertex) {

    // converting vertex to homogeneous coordinates for transformations with 4x4 matrixes
    glm::vec4 homoPointObj(vertex.pointObjectCoordinates, 1.0f);

    // model transform: original object coordinates -> world coordinates
    homoPointObj = modelM * homoPointObj;

    // changing view point: world coordinates -> eye(camera) coordinates
    homoPointObj = lookatM * homoPointObj;
    vertex.pointEyeCoordinates = glm::vec3(homoPointObj);

    // projection transform: orthographic\perspective -> clip coordinates
    homoPointObj = projectionM * homoPointObj;

    if (homoPointObj.w == 0.0f) {
        std::cerr << "W coordinate value is 0" << std::endl;
    }
    // w devision to all homoPointObj values: creating the depth element
    else {
        homoPointObj /= homoPointObj.w;
    }

    // viewport transform to pixels: window coordinates
    homoPointObj = viewportM * homoPointObj;

    // transformation normal from object coordinates to eye coordinates v->normal
    transformNormalFromObjectCoordToEyeCoordAndDrawIt(setPixel, vertex);

    // calculate lighting for a vertex for 'gourard shading'
    if (rasterizerWorld -> displayType == DisplayTypeEnum::LIGHTING_GOURARD) {
        float vertexLighting = Utilities::lightingEquation(vertex.pointEyeCoordinates, vertex.normalEyeCoordinates, lightPositionEyeCoordinates, 
                                                rasterizerWorld -> lighting_Diffuse, rasterizerWorld -> lighting_Specular, 
                                                rasterizerWorld -> lighting_Ambient, rasterizerWorld -> lighting_sHininess);

        vertex.lightingIntensity0to1 = vertexLighting;
    }
}

void ObjectModel::transformNormalFromObjectCoordToEyeCoordAndDrawIt(const SetPixelCallback& setPixel, VertexData& vertex) {

    // transformation normal from object coordinates to eye coordinates v->normal
    // --> v->NormalEyeCoordinates
    glm::mat4 modelviewM(lookatM * modelM);
    glm::mat3 modelviewM3x3(modelviewM);
    vertex.normalEyeCoordinates = modelviewM3x3 * vertex.normalObjectCoordinates;

    if (rasterizerWorld -> displayNormals) {
        // drawing normals
        glm::vec4 t2(vertex.pointEyeCoordinates + vertex.normalEyeCoordinates * 0.1f, 1);
        
        // projection transform
        t2 = projectionM * t2;
        if (t2.w != 0) {
            t2 /= t2.w;
        } else {
        std::cerr << "Division by w == 0 in vertexProcessing normal transformation" << std::endl;
        }

        t2 = viewportM * t2;
        glm::vec3 point_plusNormal_screen(t2);
        drawLineDDA(setPixel, vertex.pointWindowCoordinates, point_plusNormal_screen, 0, 0, 1.0f);
    }
}

void ObjectModel::rasterization(const SetPixelCallback& setPixel, const VertexData& vertex1, const VertexData& vertex2, 
                                const VertexData& vertex3, const glm::vec3& faceColor) {
    
    // normal for entire polygon face for 'flat shading'
    glm::vec3 faceNormal = glm::normalize(glm::cross(vertex2.pointEyeCoordinates - vertex1.pointEyeCoordinates, 
                                                    vertex3.pointEyeCoordinates - vertex1.pointEyeCoordinates));
    
    // lines rasterization: draw white lines between polygon vertices
    if (rasterizerWorld -> displayType == DisplayTypeEnum::FACE_EDGES) {
        drawLineDDA(setPixel, vertex1.pointWindowCoordinates, vertex2.pointWindowCoordinates, 1.0f, 1.0f, 1.0f);
        drawLineDDA(setPixel, vertex2.pointWindowCoordinates, vertex3.pointWindowCoordinates, 1.0f, 1.0f, 1.0f);
        drawLineDDA(setPixel, vertex3.pointWindowCoordinates, vertex1.pointWindowCoordinates, 1.0f, 1.0f, 1.0f);
    
    // Polygon faces
    } else {
        glm::ivec4 boundingBox = calcBoundingBox(vertex1.pointWindowCoordinates, vertex2.pointWindowCoordinates, 
                                                 vertex3.pointWindowCoordinates, imageWidth, imageHeight);
    
        BarycentricCoordinates bc(vertex1.pointWindowCoordinates, 
					vertex2.pointWindowCoordinates, vertex3.pointWindowCoordinates);

        // for flat shading
        float polygonLighting = Utilities::lightingEquation(vertex1.pointEyeCoordinates, faceNormal, lightPositionEyeCoordinates,
                                                 rasterizerWorld -> lighting_Diffuse, rasterizerWorld -> lighting_Specular,
                                                 rasterizerWorld -> lighting_Ambient, rasterizerWorld -> lighting_sHininess);

        // going over every pixel in bounding box
        for (int x = boundingBox.x; x <= boundingBox.y; x++) {
            for (int y = boundingBox.z; y <= boundingBox.w; y++) {

                // calc and set Barycentric Coordinates for current pixel
                bc.calcCoordinatesForPoint(x, y);

                // if pixel is inside polygon
                if (bc.isPointInsideTriangle()) {

                    // interpolate z components from vertices' z component
                    float zDepth = bc.interpolate(vertex1.pointWindowCoordinates.z, vertex2.pointWindowCoordinates.z,
                                                  vertex3.pointWindowCoordinates.z);
                
                    // Flattened 1D index for the Z-buffer (y * width + x)
                    int zBufferIndex = y * imageWidth + x;

                    // draw pixel only if its closest and not hidden
                    if (zDepth < rasterizerWorld -> zBuffer[zBufferIndex]) {
                        
                        // create fragment data object
                        FragmentData fragmentData;

                        if (rasterizerWorld -> displayType == DisplayTypeEnum::FACE_COLOR) {
                            // color all pixels in face color
                            fragmentData.pixelColor = faceColor;
                        
                        } else if (rasterizerWorld -> displayType == DisplayTypeEnum::INTERPOlATED_VERTEX_COLOR) {
                            // calculating interpolated color for pixel inside polygon
                            glm::vec3 interpolatedColor = bc.interpolate(vertex1.color, vertex2.color, vertex3.color);
                            fragmentData.pixelColor = interpolatedColor;

                        } else if (rasterizerWorld -> displayType == DisplayTypeEnum::LIGHTING_FLAT) {
                            // pixel light intensity for all pixels in current polygon
                            fragmentData.pixelIntensity0to1 = polygonLighting;

                        } else if (rasterizerWorld -> displayType == DisplayTypeEnum::LIGHTING_GOURARD) {
                            // pixel light intensity with interpolation with vertices
                            float interpolatedLight = bc.interpolate(vertex1.lightingIntensity0to1, vertex2.lightingIntensity0to1, 
                                                                     vertex3.lightingIntensity0to1);
                            fragmentData.pixelIntensity0to1 = interpolatedLight;

                        } else if (rasterizerWorld -> displayType == DisplayTypeEnum::LIGHTING_PHONG) {
                            // interpolation of point and normal in eye coordinates
                            glm::vec3 interpolatedEyePoint = bc.interpolate(vertex1.pointEyeCoordinates, vertex2.pointEyeCoordinates,
                                                                            vertex3.pointEyeCoordinates);
                            glm::vec3 interpolatedEyeNormal = bc.interpolate(vertex1.normalEyeCoordinates, vertex2.normalEyeCoordinates,
                                                                            vertex3.normalEyeCoordinates);
                            
                            fragmentData.pointEyeCoordinates = interpolatedEyePoint;
                            fragmentData.normalEyeCoordinates = interpolatedEyeNormal;
                        
                        } else if (rasterizerWorld -> displayType == DisplayTypeEnum::TEXTURE) {
                            // texture coordinates interpolation
                            glm::vec2 interpolatedTexture = bc.interpolate(vertex1.textureCoordinates, vertex2.textureCoordinates,
                                                                            vertex3.textureCoordinates);
                                                    
                            fragmentData.textureCoordinates = interpolatedTexture;

                            // interpolation of phong lighting
                            glm::vec3 interpolatedEyePoint = bc.interpolate(vertex1.pointEyeCoordinates, vertex2.pointEyeCoordinates,
                                                                            vertex3.pointEyeCoordinates);
                            glm::vec3 interpolatedEyeNormal = bc.interpolate(vertex1.normalEyeCoordinates, vertex2.normalEyeCoordinates,
                                                                            vertex3.normalEyeCoordinates);
                            
                            fragmentData.pointEyeCoordinates = interpolatedEyePoint;
                            fragmentData.normalEyeCoordinates = interpolatedEyeNormal;
                        }

                        // get pixel color
                        glm::vec3 pixelColor = fragmentProcessing(fragmentData);

                        // paint pixel and set zBuffer to current pixel's
                        setPixel(x, y, pixelColor);
                        rasterizerWorld -> zBuffer[zBufferIndex] = zDepth;
                    }
                }
            }
        }
    }
    
}

glm::vec3 ObjectModel::fragmentProcessing(const FragmentData& fragmentData) {
    
    if (rasterizerWorld -> displayType == DisplayTypeEnum::FACE_COLOR) {
        return fragmentData.pixelColor;

    } else if (rasterizerWorld -> displayType == DisplayTypeEnum::INTERPOlATED_VERTEX_COLOR) {
        return fragmentData.pixelColor;

    } else if (rasterizerWorld -> displayType == DisplayTypeEnum::LIGHTING_FLAT) {
        // return vector3f of gray scale lighting for polygon
        return glm::vec3(fragmentData.pixelIntensity0to1);

    } else if (rasterizerWorld -> displayType == DisplayTypeEnum::LIGHTING_GOURARD) {
        // return vector3f of gray scale lighting for polygon
        return glm::vec3(fragmentData.pixelIntensity0to1);

    } else if (rasterizerWorld -> displayType == DisplayTypeEnum::LIGHTING_PHONG) {
        // calculate light for every pixel with its unique location and normal
        float pixelLighting = Utilities::lightingEquation(fragmentData.pointEyeCoordinates, fragmentData.normalEyeCoordinates,
                                                lightPositionEyeCoordinates,
                                                rasterizerWorld -> lighting_Diffuse,
                                                rasterizerWorld -> lighting_Specular,
                                                rasterizerWorld -> lighting_Ambient,
                                                rasterizerWorld -> lighting_sHininess);

        return glm::vec3(pixelLighting);

    } else if (rasterizerWorld -> displayType == DisplayTypeEnum::TEXTURE) {
        glm::vec2 textureCoordinates(fragmentData.textureCoordinates);

        // multiply with width and height to get actual coordinates from texture image
        glm::vec2 textureImgCoordinates((textureCoordinates.x * (textureGetWidth() -1)), 
                                         (textureCoordinates.y * (textureGetHeight() -1)));

        // round result to get to Nearest pixel in texture image
        return textureGetPixel(static_cast<int>(std::round(textureImgCoordinates.x)),
                                static_cast<int>(std::round(textureImgCoordinates.y)));

    } else if (rasterizerWorld -> displayType == DisplayTypeEnum::TEXTURE_LIGHTING) {

        float pixelLighting = Utilities::lightingEquation(fragmentData.pointEyeCoordinates, fragmentData.normalEyeCoordinates,
                                                lightPositionEyeCoordinates,
                                                rasterizerWorld -> lighting_Diffuse,
                                                rasterizerWorld -> lighting_Specular,
                                                rasterizerWorld -> lighting_Ambient,
                                                rasterizerWorld -> lighting_sHininess);

        glm::vec2 textureCoordinates(fragmentData.textureCoordinates);

        // multiply with width and height to get actual coordinates from texture image
        glm::vec2 textureImgCoordinates((textureCoordinates.x * (textureGetWidth() -1)), 
                                         (textureCoordinates.y * (textureGetHeight() -1)));

        // round result to get to Nearest pixel in texture image
        glm::vec3 textureColor = textureGetPixel(static_cast<int>(std::round(textureImgCoordinates.x)),
                                static_cast<int>(std::round(textureImgCoordinates.y)));
        
        return textureColor * pixelLighting;
    }

    return glm::vec3();
}

void ObjectModel::drawLineDDA(const SetPixelCallback& setPixel, const glm::vec3& p1, const glm::vec3& p2, float r, float g, float b) {

    int x1round = static_cast<int>(std::round(p1.x));
    int x2round = static_cast<int>(std::round(p1.x));
    int y1round = static_cast<int>(std::round(p1.x));
    int y2round = static_cast<int>(std::round(p1.x));

    int dx = x2round - x1round;
    int dy = y2round - y1round;

    // First check if switching points is needed
    if ((dy < -dx) || ((dy == -dx) && (dx < 0))) {

        // Switch between p1 and p2
        int tempX1 = x1round;
        x1round = x2round;
        x2round = tempX1;
        int tempY1 = y1round;
        y1round = y2round;
        y2round = tempY1;

        // Recalculate dx, dy with switched points
        dx = x2round - x1round;
        dy = y2round - y1round;
    }

    // Second check to implement via X axis
    if (std::abs(dy) <= std::abs(dx)) {
        float a = (float) dy/dx;
        float y = y1round;
        
        for (int x = x1round; x <= x2round; x++) {
            setPixel(x, std::round(y), glm::vec3(r, g, b));
            y += a;
        }
    }

    // Third check to implement via Y axis
    else {
        float a = static_cast<float>(dx) / dy;  // only here 'a' is calculated dx/dy
        float x = x1round;

        for (int y = y1round; y <= y2round; y++) {
            setPixel(std::round(x), y, glm::vec3(r, g, b));
            x += a;
        }
    }
}

// calc 4 vertices of bounding box of polygon
glm::ivec4 ObjectModel::calcBoundingBox(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, 
                                        int imageWidth, int imageHeight) {

    int minX = static_cast<int>(glm::floor(glm::max(0.0f, glm::min(p1.x, glm::min(p2.x, p3.x)))));

    int maxX = static_cast<int>(glm::ceil(glm::min(static_cast<float>(imageWidth - 1), glm::max(p1.x, glm::max(p2.x, p3.x)))));

    int minY = static_cast<int>(glm::floor(glm::max(0.0f, glm::min(p1.y, glm::min(p2.y, p3.y)))));

    int maxY = static_cast<int>(glm::ceil(glm::min(static_cast<float>(imageHeight - 1), glm::max(p1.y, glm::max(p2.y, p3.y)))));

    return glm::ivec4(minX, maxX, minY, maxY);
}
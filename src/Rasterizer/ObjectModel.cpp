#include "ObjectModel.h"
#include "WorldModel.h"
#include "BarycentricCoordinates.h"
#include <iostream>
#include <algorithm>

// Constructor
ObjectModel::ObjectModel(WorldModel* worldModel, int imageWidth, int imageHeight)
    : worldModel(worldModel), imageWidth(imageWidth), imageHeight(imageHeight) {
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

    exercise = worldModel -> exercise;

    // homogenic vector of light position for transformations
    glm::vec4 homoLightPos(worldModel -> lightPositionWorldCoordinates, 1.0f);

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
    if (worldModel -> displayType == DisplayTypeEnum::LIGHTING_GOURARD) {
        float vertexLighting = lightingEquation(vertex.pointEyeCoordinates, vertex.normalEyeCoordinates, lightPositionEyeCoordinates, 
                                                worldModel -> lighting_Diffuse, worldModel -> lighting_Specular, 
                                                worldModel -> lighting_Ambient, worldModel -> lighting_sHininess);

        vertex.lightingIntensity0to1 = vertexLighting;
    }
}

void ObjectModel::transformNormalFromObjectCoordToEyeCoordAndDrawIt(const SetPixelCallback& setPixel, VertexData& vertex) {

    // transformation normal from object coordinates to eye coordinates v->normal
    // --> v->NormalEyeCoordinates
    glm::mat4 modelviewM(lookatM * modelM);
    glm::mat3 modelviewM3x3(modelviewM);
    vertex.normalEyeCoordinates = modelviewM3x3 * vertex.normalObjectCoordinates;

    if (worldModel -> displayNormals) {
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

void ObjectModel::rasterization(const SetPixelCallback& setPixel, const VertexData& vertex1, const VertexData& vertex2, const VertexData& vertex3, const glm::vec3& faceColor) {
    
    // normal for entire polygon face for 'flat shading'
    glm::vec3 faceNormal = glm::normalize(glm::cross(vertex2.pointEyeCoordinates - vertex1.pointEyeCoordinates, vertex3.pointEyeCoordinates - vertex1.pointEyeCoordinates));
    
    // lines rasterization: draw white lines between polygon vertices
    if (worldModel -> displayType == DisplayTypeEnum::FACE_EDGES) {
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
        float polygonLighting = lightingEquation(vertex1.pointEyeCoordinates, faceNormal, lightPositionEyeCoordinates,
                                                 worldModel -> lighting_Diffuse, worldModel -> lighting_Specular,
                                                 worldModel -> lighting_Ambient, worldModel -> lighting_sHininess);

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
                    if (zDepth < worldModel -> zBuffer[zBufferIndex]) {
                        
                        // create fragment data object
                        FragmentData fragmentData;

                        if (worldModel -> displayType == DisplayTypeEnum::FACE_COLOR) {
                            // color all pixels in face color
                            fragmentData.pixelColor = faceColor;
                        
                        } else if (worldModel -> displayType == DisplayTypeEnum::INTERPOlATED_VERTEX_COLOR) {
                            // calculating interpolated color for pixel inside polygon
                            glm::vec3 interpolatedColor = bc.interpolate(vertex1.color, vertex2.color, vertex3.color);
                            fragmentData.pixelColor = interpolatedColor;

                        } else if (worldModel -> displayType == DisplayTypeEnum::LIGHTING_FLAT) {
                            // pixel light intensity for all pixels in current polygon
                            fragmentData.pixelIntensity0to1 = polygonLighting;

                        } else if (worldModel -> displayType == DisplayTypeEnum::LIGHTING_GOURARD) {
                            // pixel light intensity with interpolation with vertices
                            float interpolatedLight = bc.interpolate(vertex1.lightingIntensity0to1, vertex2.lightingIntensity0to1, 
                                                                     vertex3.lightingIntensity0to1);
                            fragmentData.pixelIntensity0to1 = interpolatedLight;

                        } else if (worldModel -> displayType == DisplayTypeEnum::LIGHTING_PHONG) {
                            // interpolation of point and normal in eye coordinates
                            glm::vec3 interpolatedEyePoint = bc.interpolate(vertex1.pointEyeCoordinates, vertex2.pointEyeCoordinates,
                                                                            vertex3.pointEyeCoordinates);
                            glm::vec3 interpolatedEyeNormal = bc.interpolate(vertex1.normalEyeCoordinates, vertex2.normalEyeCoordinates,
                                                                            vertex3.normalEyeCoordinates);
                            
                            fragmentData.pointEyeCoordinates = interpolatedEyePoint;
                            fragmentData.normalEyeCoordinates = interpolatedEyeNormal;
                        
                        } else if (worldModel -> displayType == DisplayTypeEnum::TEXTURE) {
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
                        worldModel -> zBuffer[zBufferIndex] = zDepth;
                    }
                }
            }
        }
    }
    
}
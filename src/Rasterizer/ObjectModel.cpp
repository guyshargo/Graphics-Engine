#include "ObjectModel.h"
#include "RasterizerWorld.h"
#include "BarycentricCoordinates.h"
#include "Utilities.h"
#include "PlaneData.h"
#include "DefaultParams.h"
#include "YourUtilities.h"
#include "VertexData.h"

#include <iostream>
#include <algorithm>
#include <glm/gtc/matrix_inverse.hpp>

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
        
        // --- Load Companion Texture Directly into ObjectModel ---
        size_t lastDot = fileName.find_last_of('.');
        if (lastDot != std::string::npos) {
            std::string texPath = fileName.substr(0, lastDot) + ".bmp";
            SDL_Surface* surf = SDL_LoadBMP(texPath.c_str());
            
            if (surf) {
                SDL_Surface* formatted = SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_ARGB8888, 0);
                SDL_FreeSurface(surf);
                
                if (formatted) {
                    textureWidth = formatted->w;
                    textureHeight = formatted->h;
                    textureData.resize(textureWidth * textureHeight);
                    
                    uint32_t* pixels = static_cast<uint32_t*>(formatted->pixels);
                    int pitch = formatted->pitch / 4;
                    
                    for (int y = 0; y < textureHeight; ++y) {
                        for (int x = 0; x < textureWidth; ++x) {
                            uint32_t argb = pixels[y * pitch + x];
                            float r = static_cast<float>((argb >> 16) & 0xFF) / 255.0f;
                            float g = static_cast<float>((argb >> 8) & 0xFF) / 255.0f;
                            float b = static_cast<float>(argb & 0xFF) / 255.0f;
                            textureData[y * textureWidth + x] = glm::vec3(r, g, b);
                        }
                    }
                    SDL_FreeSurface(formatted);
                    
                    // Bind callbacks to this ObjectModel's own memory
                    setTextureCallbacks(
                        [this](int x, int y) {
                            int wrapX = ((x % textureWidth) + textureWidth) % textureWidth;
                            int wrapY = ((y % textureHeight) + textureHeight) % textureHeight;
                            wrapY = textureHeight - 1 - wrapY;
                            return textureData[wrapY * textureWidth + wrapX];
                        },
                        [this]() { return textureWidth; },
                        [this]() { return textureHeight; }
                    );
                }
            }
        }

        return true;

    } catch (const std::exception& e) {
        std::cerr << "Failed to load object model '" << fileName
                  << "': " << e.what() << std::endl;
        return false;
    }
}

bool ObjectModel::objectHasTexture() const { return hasTexture; }

void ObjectModel::render(const PlotPixelCallback& plotPixel) {

    exercise = rasterizerWorld -> exercise;

    std::vector<Plane> viewingPlanes = YourUtilities::getViewPlanes(
        DefaultParams::HORIZONTAL_FOV,
        DefaultParams::ASPECT_RATIO, 
        DefaultParams::PROJ_NEAR_PLANE, 
        DefaultParams::PROJ_FAR_PLANE
    );

    // homogenic vector of light position for transformations
    glm::vec4 homoLightPos(rasterizerWorld -> lightPositionWorldCoordinates, 1.0f);

    // lookat transformation: light source will change position according to eye\camera position
    homoLightPos = lookatM * homoLightPos;

    // actual update of light source position vector with non-homo vector
    lightPositionEyeCoordinates = glm::vec3(homoLightPos);

    if (!verticesData.empty()) {

        // going through all vertices to figure out where they are on the screen
        for (VertexData& vertexData : verticesData) {
            vertexProcessing(plotPixel, vertexData);
        }

        std::vector<std::array<VertexData, 3>> survivingTriangles;
        std::vector<std::array<VertexData, 3>> nextTriangles;

        // safe maximum for a single triangle cut by 6 planes
        survivingTriangles.reserve(64); 
        nextTriangles.reserve(64);

        // going through all faces to paint pixels in them on screen
        for (const TriangleFace& face : faces) {

            survivingTriangles.clear();
            
            survivingTriangles.push_back({
                verticesData[face.indices[0]],
                verticesData[face.indices[1]],
                verticesData[face.indices[2]]
            });

            // Pass the triangles through Near\Far clipping planes
            for (const Plane& plane : viewingPlanes) {

                nextTriangles.clear();
                                
                for (const auto& triangle : survivingTriangles) {
                    // Buffer to hold up to 2 triangles returned by the clipping function
                    std::array<std::array<VertexData, 3>, 2> clippedTriangles;
                    int numClipped = clipTriangleAgainstPlane(plane.pointOnPlane, plane.normal, triangle, clippedTriangles);
                    
                    for (int i = 0; i < numClipped; ++i) {
                        nextTriangles.push_back(clippedTriangles[i]);
                    }
                }
                
                survivingTriangles = nextTriangles;
                
                // If triangle is entirely outside the viewing volume, stop checking planes
                if (survivingTriangles.empty()) {
                    break;
                }
            }

            // Finish processing and rasterize all surviving triangles
            for (auto& clippedTriangle : survivingTriangles) {
                
                finalizeVertex(plotPixel, clippedTriangle[0]);
                finalizeVertex(plotPixel, clippedTriangle[1]);
                finalizeVertex(plotPixel, clippedTriangle[2]);

                rasterization(plotPixel, 
                              clippedTriangle[0],
                              clippedTriangle[1],
                              clippedTriangle[2],
                              face.color);
            }

        }
    }
}

void ObjectModel::vertexProcessing(const PlotPixelCallback& plotPixel, VertexData& vertex) {

    // converting vertex to homogeneous coordinates for transformations with 4x4 matrixes
    glm::vec4 homoPointObj(vertex.pointObjectCoordinates, 1.0f);

    // model transform: original object coordinates -> world coordinates
    homoPointObj = modelM * homoPointObj;

    // changing view point: world coordinates -> eye(camera) coordinates
    homoPointObj = lookatM * homoPointObj;
    vertex.pointEyeCoordinates = glm::vec3(homoPointObj);

    // Calculate Normal in Eye Coordinates
    glm::mat4 modelviewM(lookatM * modelM);
    glm::mat3 normalMatrix = glm::inverseTranspose(glm::mat3(modelviewM));
    vertex.normalEyeCoordinates = glm::normalize(normalMatrix * vertex.normalObjectCoordinates);

    // calculate lighting for a vertex for 'gourard shading'
    if (rasterizerWorld -> displayType == DisplayTypeEnum::LIGHTING_GOURARD) {
        float vertexLighting = Utilities::lightingEquation(vertex.pointEyeCoordinates, vertex.normalEyeCoordinates, lightPositionEyeCoordinates, 
                                                rasterizerWorld -> lighting_Diffuse, rasterizerWorld -> lighting_Specular, 
                                                rasterizerWorld -> lighting_Ambient, rasterizerWorld -> lighting_sHininess);

        vertex.lightingIntensity0to1 = vertexLighting;
    }
}

void ObjectModel::finalizeVertex(const PlotPixelCallback& plotPixel, VertexData& vertex) {
    
    glm::vec4 homoPointObj(vertex.pointEyeCoordinates, 1.0f);

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
    vertex.pointWindowCoordinates = glm::vec3(homoPointObj);

    // draw normals
    if (rasterizerWorld -> displayNormals) {
        glm::vec4 t2(vertex.pointEyeCoordinates + vertex.normalEyeCoordinates * 0.1f, 1);
        
        t2 = projectionM * t2;
        if (t2.w != 0) {
            t2 /= t2.w;
        } else {
            std::cerr << "Division by w == 0 in normal transformation" << std::endl;
        }

        t2 = viewportM * t2;
        glm::vec3 point_plusNormal_screen(t2);
        drawLineBresenham(plotPixel, vertex.pointWindowCoordinates, point_plusNormal_screen, 0, 0, 1.0f);
    }
}

void ObjectModel::drawLineBresenham(const PlotPixelCallback& plotPixel, const glm::vec3& p1, const glm::vec3& p2, float r, float g, float b) {

    int x1 = static_cast<int>(std::round(p1.x));
    int x2 = static_cast<int>(std::round(p2.x));
    int y1 = static_cast<int>(std::round(p1.y));
    int y2 = static_cast<int>(std::round(p2.y));

    int dx = x2 - x1;
    int dy = y2 - y1;

    // First check if switching points is needed
    if (dy < -dx) {
        std::swap(x1, x2);
        std::swap(y1, y2);

        dx = x2 - x1;
        dy = y2 - y1;
    }

    if (std::abs(dy) <= std::abs(dx)) {
        int yInceremnt = 1;

        if (dy < 0) {
            yInceremnt = -1;
            dy = -dy;
        }
        int y = y1;
        int diff = 2*dy - dx;

        for (int stepX = x1; stepX <= x2; stepX++) {
            plotPixel(stepX, y, glm::vec3(r, g, b));

            if (diff < 0) {
                diff += 2*dy;

            } else {
                y += yInceremnt;
                diff += 2*dy - 2*dx;
            }
        }

    } else {
        int xInceremnt = 1;

        if (dx < 0) {
            xInceremnt = -1;
            dx = -dx;
        }

        int x = x1;
        int diff = 2*dx - dy;

        for (int stepY = y1; stepY <= y2; stepY++) {
            plotPixel(x, stepY, glm::vec3(r, g, b));

            if (diff < 0) {
                diff += 2*dx;
            } else {
                x += xInceremnt;
                diff += 2*dx - 2*dy;
            }
        }
    }   
}

void ObjectModel::rasterization(const PlotPixelCallback& plotPixel, const VertexData& vertex1, const VertexData& vertex2, 
                                const VertexData& vertex3, const glm::vec3& faceColor) {

    // normal for entire polygon face for Culling and 'flat shading'
    glm::vec3 faceNormal = glm::normalize(glm::cross(vertex2.pointEyeCoordinates - vertex1.pointEyeCoordinates, 
                                                    vertex3.pointEyeCoordinates - vertex1.pointEyeCoordinates));
    
    // Back-Face Culling for both camera types
    if (rasterizerWorld->projectionType == ProjectionTypeEnum::PERSPECTIVE) {
        // In perspective, check if the face points in the same direction as the camera ray
        if (glm::dot(faceNormal, vertex1.pointEyeCoordinates) >= 0.0f) {
            return; 
        }
    } else {
        // In orthographic, rays are perfectly parallel down the -Z axis
        if (faceNormal.z <= 0.0f) {
            return;
        }
    }
    
    // lines rasterization: draw white lines between polygon vertices
    if (rasterizerWorld -> displayType == DisplayTypeEnum::FACE_EDGES) {
        drawLineBresenham(plotPixel, vertex1.pointWindowCoordinates, vertex2.pointWindowCoordinates, 1.0f, 1.0f, 1.0f);
        drawLineBresenham(plotPixel, vertex2.pointWindowCoordinates, vertex3.pointWindowCoordinates, 1.0f, 1.0f, 1.0f);
        drawLineBresenham(plotPixel, vertex3.pointWindowCoordinates, vertex1.pointWindowCoordinates, 1.0f, 1.0f, 1.0f);
    
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
                            float interpolatedLight = bc.interpolate(vertex1.lightingIntensity0to1,
                                                                     vertex2.lightingIntensity0to1, 
                                                                     vertex3.lightingIntensity0to1);

                            fragmentData.pixelIntensity0to1 = interpolatedLight;

                        } else if (rasterizerWorld -> displayType == DisplayTypeEnum::LIGHTING_PHONG) {
                            // interpolation of point and normal in eye coordinates
                            glm::vec3 interpolatedEyePoint = bc.interpolate(vertex1.pointEyeCoordinates, 
                                                                            vertex2.pointEyeCoordinates,
                                                                            vertex3.pointEyeCoordinates);

                            glm::vec3 interpolatedEyeNormal = bc.interpolate(vertex1.normalEyeCoordinates, 
                                                                             vertex2.normalEyeCoordinates,
                                                                             vertex3.normalEyeCoordinates);
                            
                            fragmentData.pointEyeCoordinates = interpolatedEyePoint;
                            fragmentData.normalEyeCoordinates = interpolatedEyeNormal;
                        
                        } else if (rasterizerWorld -> displayType == DisplayTypeEnum::TEXTURE) {

                            if (!hasTexture) {
                                // No texture available
                                glm::vec3 magenta(1.0f, 0.0f, 1.0f);
                                plotPixel(x, y, magenta);
                                rasterizerWorld->zBuffer[zBufferIndex] = zDepth;
                                continue;
                            }
                            
                            // texture coordinates interpolation
                            glm::vec2 interpolatedTexture = bc.interpolate(vertex1.textureCoordinates,
                                                                            vertex2.textureCoordinates,
                                                                            vertex3.textureCoordinates);
                            
                            fragmentData.textureCoordinates = interpolatedTexture;

                        } else if (rasterizerWorld -> displayType == DisplayTypeEnum::TEXTURE_LIGHTING) {

                            if (!hasTexture) {
                                // No texture available
                                glm::vec3 magenta(1.0f, 0.0f, 1.0f);
                                plotPixel(x, y, magenta);
                                rasterizerWorld->zBuffer[zBufferIndex] = zDepth;
                                continue;
                            }

                            // texture coordinates interpolation
                            glm::vec2 interpolatedTexture = bc.interpolate(vertex1.textureCoordinates, 
                                                                           vertex2.textureCoordinates,
                                                                           vertex3.textureCoordinates);
                            
                            fragmentData.textureCoordinates = interpolatedTexture;

                            // interpolation of phong lighting
                            glm::vec3 interpolatedEyePoint = bc.interpolate(vertex1.pointEyeCoordinates, 
                                                                            vertex2.pointEyeCoordinates,
                                                                            vertex3.pointEyeCoordinates);

                            glm::vec3 interpolatedEyeNormal = bc.interpolate(vertex1.normalEyeCoordinates, 
                                                                            vertex2.normalEyeCoordinates,
                                                                            vertex3.normalEyeCoordinates);
                            
                            fragmentData.pointEyeCoordinates = interpolatedEyePoint;
                            fragmentData.normalEyeCoordinates = interpolatedEyeNormal;
                        }

                        // get pixel color
                        glm::vec3 pixelColor = fragmentProcessing(fragmentData);

                        // paint pixel and set zBuffer to current pixel's
                        plotPixel(x, y, pixelColor);
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

        // Calculate the integer coordinates of the top-left texel (x0, y0)
        int x0 = static_cast<int>(std::floor(textureImgCoordinates.x));
        int y0 = static_cast<int>(std::floor(textureImgCoordinates.y));

        // Calculate the coordinates of the bottom-right texel (x1, y1), modulo wraps around the texture edges safely
        int x1 = (x0 + 1) % textureGetWidth();
        int y1 = (y0 + 1) % textureGetHeight();

        // Fractional part of the coordinates used as blending weights
        float u_ratio = textureImgCoordinates.x - x0;
        float v_ratio = textureImgCoordinates.y - y0;

        // Colors of four corners of grid
        glm::vec3 tex00 = textureGetPixel(x0, y0); // Top-Left
        glm::vec3 tex10 = textureGetPixel(x1, y0); // Top-Right
        glm::vec3 tex01 = textureGetPixel(x0, y1); // Bottom-Left
        glm::vec3 tex11 = textureGetPixel(x1, y1); // Bottom-Right

        // Bilinear Interpolation
        glm::vec3 colorTop = glm::mix(tex00, tex10, u_ratio);
        glm::vec3 colorBottom = glm::mix(tex01, tex11, u_ratio);
        glm::vec3 finalTexColor = glm::mix(colorTop, colorBottom, v_ratio);

        return finalTexColor;

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

        // Calculate the integer coordinates of the top-left texel (x0, y0)
        int x0 = static_cast<int>(std::floor(textureImgCoordinates.x));
        int y0 = static_cast<int>(std::floor(textureImgCoordinates.y));

        // Calculate the coordinates of the bottom-right texel (x1, y1), modulo wraps around the texture edges safely
        int x1 = (x0 + 1) % textureGetWidth();
        int y1 = (y0 + 1) % textureGetHeight();

        // Fractional part of the coordinates used as blending weights
        float u_ratio = textureImgCoordinates.x - x0;
        float v_ratio = textureImgCoordinates.y - y0;

        // Colors of four corners of grid
        glm::vec3 tex00 = textureGetPixel(x0, y0); // Top-Left
        glm::vec3 tex10 = textureGetPixel(x1, y0); // Top-Right
        glm::vec3 tex01 = textureGetPixel(x0, y1); // Bottom-Left
        glm::vec3 tex11 = textureGetPixel(x1, y1); // Bottom-Right

        // Bilinear Interpolation
        glm::vec3 colorTop = glm::mix(tex00, tex10, u_ratio);
        glm::vec3 colorBottom = glm::mix(tex01, tex11, u_ratio);
        glm::vec3 finalTexColor = glm::mix(colorTop, colorBottom, v_ratio);
        
        return finalTexColor * pixelLighting;
    }

    return glm::vec3();
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

VertexData ObjectModel::interpolateVertex(const VertexData& v1, const VertexData& v2, float interpolationWeight) {

    VertexData interpolatedVertex = v1; // Copy base structure
    
    // Linear Interpolation for all required attributes
    interpolatedVertex.pointEyeCoordinates = glm::mix(v1.pointEyeCoordinates, v2.pointEyeCoordinates, interpolationWeight);
    interpolatedVertex.normalEyeCoordinates = glm::normalize(glm::mix(v1.normalEyeCoordinates, v2.normalEyeCoordinates, interpolationWeight));
    interpolatedVertex.textureCoordinates = glm::mix(v1.textureCoordinates, v2.textureCoordinates, interpolationWeight);
    interpolatedVertex.color = glm::mix(v1.color, v2.color, interpolationWeight);
    interpolatedVertex.lightingIntensity0to1 = glm::mix(v1.lightingIntensity0to1, v2.lightingIntensity0to1, interpolationWeight);
    
    return interpolatedVertex;
}

int ObjectModel::clipTriangleAgainstPlane(glm::vec3 planePoint, glm::vec3 planeNormal, 
                                          const std::array<VertexData, 3>& inputTriangle,
                                          std::array<std::array<VertexData, 3>, 2>& outTriangles) {
    
    auto calculateDistanceToPlane = [&](const glm::vec3& vertexPosition) {
        return glm::dot(planeNormal, vertexPosition - planePoint);
    };

    // Use a fixed-size stack array instead of a dynamic std::vector
    std::array<VertexData, 4> outputPolygon;
    int vertexCount = 0;

    for (int i = 0; i < 3; i++) {
        int prevIndex = (i == 0) ? 2 : i - 1;
        const VertexData& currentVertex = inputTriangle[i];
        const VertexData& prevVertex = inputTriangle[prevIndex];

        float distCurrent = calculateDistanceToPlane(currentVertex.pointEyeCoordinates);
        float distPrev = calculateDistanceToPlane(prevVertex.pointEyeCoordinates);

        if (distCurrent >= 0.0f) {
            if (distPrev < 0.0f) {
                float t = distPrev / (distPrev - distCurrent);
                outputPolygon[vertexCount++] = interpolateVertex(prevVertex, currentVertex, t);
            }
            outputPolygon[vertexCount++] = currentVertex;
        } 
        else {
            if (distPrev >= 0.0f) {
                float t = distPrev / (distPrev - distCurrent);
                outputPolygon[vertexCount++] = interpolateVertex(prevVertex, currentVertex, t);
            }
        }
    }

    // Break the resulting polygon back down into valid triangles
    if (vertexCount == 3) {
        outTriangles[0] = {outputPolygon[0], outputPolygon[1], outputPolygon[2]};
        return 1; // 1 triangle generated
    } 
    else if (vertexCount == 4) {
        outTriangles[0] = {outputPolygon[0], outputPolygon[1], outputPolygon[2]};
        outTriangles[1] = {outputPolygon[0], outputPolygon[2], outputPolygon[3]};
        return 2; // 2 triangles generated
    }

    return 0; // Completely clipped outside the plane
}
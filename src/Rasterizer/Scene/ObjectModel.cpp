#include "ObjectModel.h"
#include "LightingUtils.h"
#include "DefaultParams.h"
#include "./RasterUtils/BarycentricCoordinates.h"
#include "./RasterUtils/TransformUtils.h"
#include "./RasterUtils/DrawUtils.h"
#include "./RasterizerWorld.h"
#include "./Data/PipelineData.h"

#include <iostream>
#include <algorithm>
#include <glm/gtc/matrix_inverse.hpp>

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

bool ObjectModel::load(const std::string& fileName) {
    OBJLoader objLoader;
    try {
        objLoader.loadOBJ(fileName);
        verticesData = objLoader.getVertices();
        faces = objLoader.getFaces();
        boundingBoxDimensions = objLoader.getBoundingBoxDimensions();
        boundingBoxCenter = objLoader.getBoundingBoxCenter();
        
        size_t lastDot = fileName.find_last_of('.');
        if (lastDot != std::string::npos) {
            std::string texPath = fileName.substr(0, lastDot) + ".bmp";
            texture = std::make_unique<Texture2D>(texPath);
        }

        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to load object model '" << fileName << "': " << e.what() << std::endl;
        return false;
    }
}

bool ObjectModel::objectHasTexture() const { return texture != nullptr && texture->isValid(); }

void ObjectModel::render(const PlotPixelCallback& plotPixel) {

    std::vector<ClippingPlane> viewingPlanes = TransformUtils::getViewPlanes(
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
            for (const ClippingPlane& plane : viewingPlanes) {

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

            // Draw normals AFTER all faces are fully rasterized into the Z-buffer
        if (rasterizerWorld->displayNormals) {
            for (VertexData& vertexData : verticesData) {
                
                // 1. Manually project the base vertex to window coordinates
                glm::vec4 homoPoint(vertexData.pointEyeCoordinates, 1.0f);
                homoPoint = projectionM * homoPoint;
                if (homoPoint.w != 0.0f) homoPoint /= homoPoint.w;
                homoPoint = viewportM * homoPoint;
                glm::vec3 winCoords(homoPoint);

                int x = static_cast<int>(std::round(winCoords.x));
                int y = static_cast<int>(std::round(winCoords.y));

                // 2. Check if the vertex is inside the screen bounds
                if (x >= 0 && x < imageWidth && y >= 0 && y < imageHeight) {
                    int zIndex = y * imageWidth + x;
                    
                    // 3. ONLY draw the normal if the base vertex is visible in the Z-buffer
                    if (winCoords.z <= rasterizerWorld->zBuffer[zIndex] + 0.001f) {
                        
                        // Calculate the tip of the normal line
                        glm::vec4 t2(vertexData.pointEyeCoordinates + vertexData.normalEyeCoordinates * 0.1f, 1.0f);
                        t2 = projectionM * t2;
                        if (t2.w != 0.0f) t2 /= t2.w;
                        t2 = viewportM * t2;
                        
                        // Use YOUR original, untouched Bresenham algorithm
                        DrawUtils::drawLineBresenham(plotPixel, winCoords, glm::vec3(t2), glm::vec3(0.0f, 0.0f, 1.0f));
                    }
                }
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
        float vertexLighting = LightingUtils::lightingEquation(vertex.pointEyeCoordinates, vertex.normalEyeCoordinates, lightPositionEyeCoordinates, 
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
        DrawUtils::drawLineBresenham(plotPixel, vertex1.pointWindowCoordinates, vertex2.pointWindowCoordinates, glm::vec3(1.0f));
        DrawUtils::drawLineBresenham(plotPixel, vertex2.pointWindowCoordinates, vertex3.pointWindowCoordinates, glm::vec3(1.0f));
        DrawUtils::drawLineBresenham(plotPixel, vertex3.pointWindowCoordinates, vertex1.pointWindowCoordinates, glm::vec3(1.0f));
    
    // Polygon faces
    } else {
        glm::ivec4 boundingBox = calcBoundingBox(vertex1.pointWindowCoordinates, 
                                                 vertex2.pointWindowCoordinates, 
                                                 vertex3.pointWindowCoordinates, 
                                                 imageWidth, 
                                                 imageHeight);
    
        BarycentricCoordinates bc(vertex1.pointWindowCoordinates,
					              vertex2.pointWindowCoordinates, 
                                  vertex3.pointWindowCoordinates);

        // Calculate UVs at the start of the bounding box
        bc.calcCoordinatesForPoint(boundingBox.x, boundingBox.z);
        glm::vec2 tex00 = bc.interpolate(vertex1.textureCoordinates, vertex2.textureCoordinates, vertex3.textureCoordinates);

        // Calculate UVs 1 pixel to the right
        bc.calcCoordinatesForPoint(boundingBox.x + 1, boundingBox.z);
        glm::vec2 tex10 = bc.interpolate(vertex1.textureCoordinates, vertex2.textureCoordinates, vertex3.textureCoordinates);

        // Calculate UVs 1 pixel down
        bc.calcCoordinatesForPoint(boundingBox.x, boundingBox.z + 1);
        glm::vec2 tex01 = bc.interpolate(vertex1.textureCoordinates, vertex2.textureCoordinates, vertex3.textureCoordinates);

        // Calculate how much the UV coordinates change for a pixel step on the screen
        glm::vec2 uvChangePerPixelX = tex10 - tex00;
        glm::vec2 uvChangePerPixelY = tex01 - tex00;

        // Ratio of texture pixels crossed per screen pixel on X and Y axes
        float texelsPerPixelX = 0.0f;
        float texelsPerPixelY = 0.0f;

        if (objectHasTexture()) {
             texelsPerPixelX = glm::length(uvChangePerPixelX * glm::vec2(texture->getWidth(), texture->getHeight()));
             texelsPerPixelY = glm::length(uvChangePerPixelY * glm::vec2(texture->getWidth(), texture->getHeight()));
        }

        float faceLevelOfDetail = std::log2(std::max(std::max(texelsPerPixelX, texelsPerPixelY), 1.0f));

        // for flat shading
        float polygonLighting = LightingUtils::lightingEquation(vertex1.pointEyeCoordinates, faceNormal, lightPositionEyeCoordinates,
                                                            rasterizerWorld -> lighting_Diffuse, 
                                                            rasterizerWorld -> lighting_Specular,
                                                            rasterizerWorld -> lighting_Ambient, 
                                                            rasterizerWorld -> lighting_sHininess);

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
                        fragmentData.levelOfDetail = faceLevelOfDetail;

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
                            fragmentData.normalEyeCoordinates = glm::normalize(interpolatedEyeNormal);
                        
                        } else if (rasterizerWorld->displayType == DisplayTypeEnum::TEXTURE || rasterizerWorld->displayType == DisplayTypeEnum::TEXTURE_LIGHTING) {
                            if (!objectHasTexture()) {
                                plotPixel(x, y, glm::vec3(1.0f, 0.0f, 1.0f));
                                rasterizerWorld->zBuffer[zBufferIndex] = zDepth;
                                continue;
                            }
                            
                            fragmentData.textureCoordinates = bc.interpolate(vertex1.textureCoordinates, vertex2.textureCoordinates, vertex3.textureCoordinates);
                            
                            if (rasterizerWorld->displayType == DisplayTypeEnum::TEXTURE_LIGHTING) {
                                fragmentData.pointEyeCoordinates = bc.interpolate(vertex1.pointEyeCoordinates, vertex2.pointEyeCoordinates, vertex3.pointEyeCoordinates);
                                fragmentData.normalEyeCoordinates = bc.interpolate(vertex1.normalEyeCoordinates, vertex2.normalEyeCoordinates, vertex3.normalEyeCoordinates);
                            }
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
        float pixelLighting = LightingUtils::lightingEquation(fragmentData.pointEyeCoordinates, fragmentData.normalEyeCoordinates,
                                                        lightPositionEyeCoordinates,
                                                        rasterizerWorld -> lighting_Diffuse,
                                                        rasterizerWorld -> lighting_Specular,
                                                        rasterizerWorld -> lighting_Ambient,
                                                        rasterizerWorld -> lighting_sHininess);

        return glm::vec3(pixelLighting);

    } else if (rasterizerWorld->displayType == DisplayTypeEnum::TEXTURE || 
               rasterizerWorld->displayType == DisplayTypeEnum::TEXTURE_LIGHTING) {

        // Fallback to magenta color if no texture detected
        if (!objectHasTexture()) return glm::vec3(1.0f, 0.0f, 1.0f);

        glm::vec3 finalTexColor = texture->sample(fragmentData.levelOfDetail, fragmentData.textureCoordinates);

        if (rasterizerWorld->displayType == DisplayTypeEnum::TEXTURE_LIGHTING) {
            float pixelLighting = LightingUtils::lightingEquation(fragmentData.pointEyeCoordinates, fragmentData.normalEyeCoordinates,
                                                    lightPositionEyeCoordinates,
                                                    rasterizerWorld->lighting_Diffuse, rasterizerWorld->lighting_Specular,
                                                    rasterizerWorld->lighting_Ambient, rasterizerWorld->lighting_sHininess);
            return finalTexColor * pixelLighting;
        }

        return finalTexColor;
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

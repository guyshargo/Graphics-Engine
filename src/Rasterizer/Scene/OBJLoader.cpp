#define TINYOBJLOADER_IMPLEMENTATION 

#include "OBJLoader.h"
#include "../vendor/tiny_obj_loader.h"

#include <iostream>
#include <unordered_map>
#include <random>
#include <cmath>
#include <algorithm>
#include <glm/gtc/constants.hpp>

OBJLoader::OBJLoader(bool useSphericalMapping) 
    : useSphericalMapping(useSphericalMapping),
      minPositionX(std::numeric_limits<float>::max()), maxPositionX(std::numeric_limits<float>::lowest()),
      minPositionY(std::numeric_limits<float>::max()), maxPositionY(std::numeric_limits<float>::lowest()),
      minPositionZ(std::numeric_limits<float>::max()), maxPositionZ(std::numeric_limits<float>::lowest()) {
}

glm::vec3 OBJLoader::getBoundingBoxDimensions() const {
    return glm::vec3(maxPositionX - minPositionX, maxPositionY - minPositionY, maxPositionZ - minPositionZ);
}

glm::vec3 OBJLoader::getBoundingBoxCenter() const {
    return glm::vec3((maxPositionX + minPositionX) / 2.0f, 
                     (maxPositionY + minPositionY) / 2.0f,
                     (maxPositionZ + minPositionZ) / 2.0f);
}

void OBJLoader::loadOBJ(const std::string& filePath) {
    tinyobj::ObjReaderConfig reader_config;
    reader_config.triangulate = true;

    tinyobj::ObjReader reader;
    if (!reader.ParseFromFile(filePath, reader_config)) {
        if (!reader.Error().empty()) {
            std::cerr << "TinyObjReader Error: " << reader.Error() << std::endl;
        }
        return;
    }

    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();

    std::unordered_map<std::string, int> vertexMap;
    std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    int globalFaceIndex = 0;
    bool hasNormals = !attrib.normals.empty();
    bool hasTexCoords = !attrib.texcoords.empty();

    for (size_t s = 0; s < shapes.size(); s++) {
        size_t index_offset = 0;
        
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
            
            int indices[3];

            for (size_t v = 0; v < 3; v++) {
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

                glm::vec3 position(
                    attrib.vertices[3 * size_t(idx.vertex_index) + 0],
                    attrib.vertices[3 * size_t(idx.vertex_index) + 1],
                    attrib.vertices[3 * size_t(idx.vertex_index) + 2]
                );

                // Bounding box tracking
                if (position.x < minPositionX) minPositionX = position.x;
                if (position.x > maxPositionX) maxPositionX = position.x;
                if (position.y < minPositionY) minPositionY = position.y;
                if (position.y > maxPositionY) maxPositionY = position.y;
                if (position.z < minPositionZ) minPositionZ = position.z;
                if (position.z > maxPositionZ) maxPositionZ = position.z;

                glm::vec3 normal(0.0f);
                if (idx.normal_index >= 0) {
                    normal = glm::vec3(
                        attrib.normals[3 * size_t(idx.normal_index) + 0],
                        attrib.normals[3 * size_t(idx.normal_index) + 1],
                        attrib.normals[3 * size_t(idx.normal_index) + 2]
                    );
                }

                glm::vec2 texCoord(0.0f);
                if (idx.texcoord_index >= 0) {
                    texCoord = glm::vec2(
                        attrib.texcoords[2 * size_t(idx.texcoord_index) + 0],
                        attrib.texcoords[2 * size_t(idx.texcoord_index) + 1]
                    );
                }

                // Vertex deduplication key
                std::string key = std::to_string(position.x) + "_" + std::to_string(position.y) + "_" + std::to_string(position.z) + "_" +
                                  std::to_string(normal.x) + "_" + std::to_string(normal.y) + "_" + std::to_string(normal.z) + "_" +
                                  std::to_string(texCoord.x) + "_" + std::to_string(texCoord.y);

                if (vertexMap.find(key) != vertexMap.end()) {
                    indices[v] = vertexMap[key];
                } else {
                    glm::vec3 vertexColor;
                    if (globalFaceIndex == 0) {
                        switch (v) {
                            case 0: vertexColor = glm::vec3(1.0f, 0.0f, 0.0f); break;
                            case 1: vertexColor = glm::vec3(0.0f, 1.0f, 0.0f); break;
                            case 2: vertexColor = glm::vec3(0.0f, 0.0f, 1.0f); break;
                            default: vertexColor = glm::vec3(dist(rng), dist(rng), dist(rng));
                        }
                    } else {
                        vertexColor = glm::vec3(dist(rng), dist(rng), dist(rng));
                    }

                    // Directly using the matched constructor
                    VertexData vertex(position, normal, texCoord, vertexColor);

                    vertices.push_back(vertex);
                    indices[v] = static_cast<int>(vertices.size() - 1);
                    vertexMap[key] = indices[v];
                }
            }
            index_offset += 3;

            // Pack local indices into std::array TriangleFace
            std::array<int, 3> indexArray = {indices[0], indices[1], indices[2]};
            glm::vec3 faceColor(dist(rng), dist(rng), dist(rng));
            glm::vec3 faceNormal(0.0f); 

            faces.emplace_back(indexArray, faceColor, faceNormal);
            globalFaceIndex++;
        }
    }

    if (!hasNormals) {
        calculateNormalsSmoothShading();
    }

    if (!hasTexCoords) {
        if (useSphericalMapping) {
            calculateSphericalMapping();
        } else {
            calculatePlanarMapping();
        }
    }
}

void OBJLoader::calculateNormalsSmoothShading() {
    std::vector<glm::vec3> vertexNormals(vertices.size(), glm::vec3(0.0f));

    for (const auto& face : faces) {
        glm::vec3 v0 = vertices[face.indices[0]].pointObjectCoordinates;
        glm::vec3 v1 = vertices[face.indices[1]].pointObjectCoordinates;
        glm::vec3 v2 = vertices[face.indices[2]].pointObjectCoordinates;

        glm::vec3 edge1 = v1 - v0;
        glm::vec3 edge2 = v2 - v0;
        glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));

        vertexNormals[face.indices[0]] += normal;
        vertexNormals[face.indices[1]] += normal;
        vertexNormals[face.indices[2]] += normal;
    }

    for (size_t i = 0; i < vertices.size(); i++) {
        if (glm::length(vertexNormals[i]) > 0.0f) {
            vertices[i].normalObjectCoordinates = glm::normalize(vertexNormals[i]);
        }
    }
}

void OBJLoader::calculateSphericalMapping() {
    for (auto& vertex : vertices) {
        glm::vec3 pos = vertex.pointObjectCoordinates;
        float len = glm::length(pos);
        float u = 0.5f + (std::atan2(pos.z, pos.x) / (2.0f * glm::pi<float>()));
        float v = 0.5f - (std::asin(pos.y / (len == 0.0f ? 1.0f : len)) / glm::pi<float>());
        vertex.textureCoordinates = glm::vec2(u, v);
    }
}

void OBJLoader::calculatePlanarMapping() {
    for (auto& vertex : vertices) {
        glm::vec3 pos = vertex.pointObjectCoordinates;
        vertex.textureCoordinates = glm::vec2(pos.x, pos.z);
    }
}
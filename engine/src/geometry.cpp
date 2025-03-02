#include <ermy_geometry.h>

using namespace ermy;
using namespace ermy::geometry;


GeometryData ermy::geometry::CreateCubeGeometry(float size)
{
    float halfSize = size * 0.5f;
    GeometryData geo = {};
    geo.numVertices = 24;
    geo.numIndices = 36;

    // Allocate memory for vertices and indices
    geo.verticecs = new rendering::StaticVertexDedicated[24];
    geo.indices = new u16[36];

    // Vertex data for all 6 faces
    rendering::StaticVertexDedicated vertices[24] = {
        // Front face
        {-halfSize, -halfSize,  halfSize,  0, 0, 1,  1, 0, 0,  0, 1, 0,  0, 0,  0, 1,  1, 1, 1, 1},  // Bottom-left
        { halfSize, -halfSize,  halfSize,  0, 0, 1,  1, 0, 0,  0, 1, 0,  1, 0,  0, 1,  1, 1, 1, 1},  // Bottom-right
        { halfSize,  halfSize,  halfSize,  0, 0, 1,  1, 0, 0,  0, 1, 0,  1, 1,  0, 1,  1, 1, 1, 1},  // Top-right
        {-halfSize,  halfSize,  halfSize,  0, 0, 1,  1, 0, 0,  0, 1, 0,  0, 1,  0, 1,  1, 1, 1, 1},  // Top-left

        // Back face
        {-halfSize, -halfSize, -halfSize,  0, 0, -1, -1, 0, 0,  0, 1, 0,  1, 0,  1, 1,  1, 1, 1, 1},  // Bottom-right
        { halfSize, -halfSize, -halfSize,  0, 0, -1, -1, 0, 0,  0, 1, 0,  0, 0,  1, 1,  1, 1, 1, 1},  // Bottom-left
        { halfSize,  halfSize, -halfSize,  0, 0, -1, -1, 0, 0,  0, 1, 0,  0, 1,  1, 1,  1, 1, 1, 1},  // Top-left
        {-halfSize,  halfSize, -halfSize,  0, 0, -1, -1, 0, 0,  0, 1, 0,  1, 1,  1, 1,  1, 1, 1, 1},  // Top-right

        // Left face
        {-halfSize, -halfSize, -halfSize, -1, 0, 0,  0, 0, 1,  0, 1, 0,  0, 0,  0, 1,  1, 1, 1, 1},  // Bottom-left
        {-halfSize, -halfSize,  halfSize, -1, 0, 0,  0, 0, 1,  0, 1, 0,  1, 0,  0, 1,  1, 1, 1, 1},  // Bottom-right
        {-halfSize,  halfSize,  halfSize, -1, 0, 0,  0, 0, 1,  0, 1, 0,  1, 1,  0, 1,  1, 1, 1, 1},  // Top-right
        {-halfSize,  halfSize, -halfSize, -1, 0, 0,  0, 0, 1,  0, 1, 0,  0, 1,  0, 1,  1, 1, 1, 1},  // Top-left

        // Right face
        { halfSize, -halfSize,  halfSize,  1, 0, 0,  0, 0, -1,  0, 1, 0,  0, 0,  0, 1,  1, 1, 1, 1},  // Bottom-left
        { halfSize, -halfSize, -halfSize,  1, 0, 0,  0, 0, -1,  0, 1, 0,  1, 0,  0, 1,  1, 1, 1, 1},  // Bottom-right
        { halfSize,  halfSize, -halfSize,  1, 0, 0,  0, 0, -1,  0, 1, 0,  1, 1,  0, 1,  1, 1, 1, 1},  // Top-right
        { halfSize,  halfSize,  halfSize,  1, 0, 0,  0, 0, -1,  0, 1, 0,  0, 1,  0, 1,  1, 1, 1, 1},  // Top-left

        // Top face
        {-halfSize,  halfSize,  halfSize,  0, 1, 0,  1, 0, 0,  0, 0, 1,  0, 0,  0, 1,  1, 1, 1, 1},  // Front-left
        { halfSize,  halfSize,  halfSize,  0, 1, 0,  1, 0, 0,  0, 0, 1,  1, 0,  0, 1,  1, 1, 1, 1},  // Front-right
        { halfSize,  halfSize, -halfSize,  0, 1, 0,  1, 0, 0,  0, 0, 1,  1, 1,  0, 1,  1, 1, 1, 1},  // Back-right
        {-halfSize,  halfSize, -halfSize,  0, 1, 0,  1, 0, 0,  0, 0, 1,  0, 1,  0, 1,  1, 1, 1, 1},  // Back-left

        // Bottom face
        {-halfSize, -halfSize, -halfSize,  0, -1, 0,  1, 0, 0,  0, 0, -1,  0, 0,  0, 1,  1, 1, 1, 1},  // Back-left
        { halfSize, -halfSize, -halfSize,  0, -1, 0,  1, 0, 0,  0, 0, -1,  1, 0,  0, 1,  1, 1, 1, 1},  // Back-right
        { halfSize, -halfSize,  halfSize,  0, -1, 0,  1, 0, 0,  0, 0, -1,  1, 1,  0, 1,  1, 1, 1, 1},  // Front-right
        {-halfSize, -halfSize,  halfSize,  0, -1, 0,  1, 0, 0,  0, 0, -1,  0, 1,  0, 1,  1, 1, 1, 1}   // Front-left
    };

    // Index data (clockwise winding)
    u16 indices[36] = {
        0, 1, 2,  0, 2, 3,      // Front
        4, 5, 6,  4, 6, 7,      // Back
        8, 9, 10, 8, 10, 11,    // Left
        12, 13, 14, 12, 14, 15, // Right
        16, 17, 18, 16, 18, 19, // Top
        20, 21, 22, 20, 22, 23  // Bottom
    };

    // Copy data to geometry
    memcpy(geo.verticecs, vertices, sizeof(rendering::StaticVertexDedicated) * 24);
    memcpy(geo.indices, indices, sizeof(u16) * 36);

    return geo;
}

GeometryData ermy::geometry::CreateGeoSphereGeometry(int numSubdiv, float radius)
{
    GeometryData geo = {};

    // Base icosahedron vertices (12)
    const float t = (1.0f + sqrtf(5.0f)) * 0.5f; // Golden ratio
    const float len = sqrtf(1.0f + t * t);
    const float v = 1.0f / len;
    const float vt = t / len;

    // Initial icosahedron vertices
    rendering::StaticVertexDedicated baseVertices[12] = {
        {-v,  vt, 0, -v,  vt, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 1},
        { v,  vt, 0,  v,  vt, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 1},
        {-v, -vt, 0, -v, -vt, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 1},
        { v, -vt, 0,  v, -vt, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 1},
        {0, -v,  vt, 0, -v,  vt, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 1},
        {0,  v,  vt, 0,  v,  vt, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 1},
        {0, -v, -vt, 0, -v, -vt, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 1},
        {0,  v, -vt, 0,  v, -vt, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 1},
        { vt, 0, -v,  vt, 0, -v, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 1},
        { vt, 0,  v,  vt, 0,  v, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 1},
        {-vt, 0, -v, -vt, 0, -v, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 1},
        {-vt, 0,  v, -vt, 0,  v, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 1}
    };

    // Initial icosahedron indices (20 triangles)
    u16 baseIndices[60] = {
        0, 11, 5,  0, 5, 1,   0, 1, 7,   0, 7, 10,  0, 10, 11,
        1, 5, 9,   5, 11, 4,  11, 10, 2, 10, 7, 6,   7, 1, 8,
        3, 9, 4,   3, 4, 2,   3, 2, 6,   3, 6, 8,    3, 8, 9,
        4, 9, 5,   2, 4, 11,  6, 2, 10,  8, 6, 7,    9, 8, 1
    };

    // Calculate final vertex and index count
    int numTriangles = 20;
    int numVertices = 12;
    for (int i = 0; i < numSubdiv; i++) {
        numTriangles *= 4;
        numVertices += numTriangles / 2; // Each subdivision adds new vertices
    }
    geo.numVertices = numVertices;
    geo.numIndices = numTriangles * 3;

    // Allocate memory
    geo.verticecs = new rendering::StaticVertexDedicated[numVertices];
    geo.indices = new u16[geo.numIndices];

    // Copy initial vertices
    memcpy(geo.verticecs, baseVertices, sizeof(rendering::StaticVertexDedicated) * 12);
    memcpy(geo.indices, baseIndices, sizeof(u16) * 60);

    int currentVertCount = 12;
    int currentIndexCount = 60;

    // Subdivide
    for (int subdiv = 0; subdiv < numSubdiv; subdiv++) {
        int oldIndexCount = currentIndexCount;
        currentIndexCount = 0;

        u16* newIndices = new u16[numTriangles * 3 * 4];

        for (int i = 0; i < oldIndexCount; i += 3) {
            // Get triangle vertices
            u16 v1 = geo.indices[i];
            u16 v2 = geo.indices[i + 1];
            u16 v3 = geo.indices[i + 2];

            // Calculate midpoints
            u16 m1 = currentVertCount++;
            u16 m2 = currentVertCount++;
            u16 m3 = currentVertCount++;

            // Create new vertices
            rendering::StaticVertexDedicated& vert1 = geo.verticecs[v1];
            rendering::StaticVertexDedicated& vert2 = geo.verticecs[v2];
            rendering::StaticVertexDedicated& vert3 = geo.verticecs[v3];

            // Midpoint 1 (between v1 and v2)
            geo.verticecs[m1] = vert1;
            geo.verticecs[m1].x = (vert1.x + vert2.x) * 0.5f;
            geo.verticecs[m1].y = (vert1.y + vert2.y) * 0.5f;
            geo.verticecs[m1].z = (vert1.z + vert2.z) * 0.5f;

            // Midpoint 2 (between v2 and v3)
            geo.verticecs[m2] = vert2;
            geo.verticecs[m2].x = (vert2.x + vert3.x) * 0.5f;
            geo.verticecs[m2].y = (vert2.y + vert3.y) * 0.5f;
            geo.verticecs[m2].z = (vert2.z + vert3.z) * 0.5f;

            // Midpoint 3 (between v3 and v1)
            geo.verticecs[m3] = vert3;
            geo.verticecs[m3].x = (vert3.x + vert1.x) * 0.5f;
            geo.verticecs[m3].y = (vert3.y + vert1.y) * 0.5f;
            geo.verticecs[m3].z = (vert3.z + vert1.z) * 0.5f;

            // Add four new triangles
            newIndices[currentIndexCount++] = v1; newIndices[currentIndexCount++] = m1; newIndices[currentIndexCount++] = m3;
            newIndices[currentIndexCount++] = v2; newIndices[currentIndexCount++] = m2; newIndices[currentIndexCount++] = m1;
            newIndices[currentIndexCount++] = v3; newIndices[currentIndexCount++] = m3; newIndices[currentIndexCount++] = m2;
            newIndices[currentIndexCount++] = m1; newIndices[currentIndexCount++] = m2; newIndices[currentIndexCount++] = m3;
        }

        delete[] geo.indices;
        geo.indices = newIndices;
    }

    // Normalize and scale all vertices to radius, set normals
    for (int i = 0; i < geo.numVertices; i++) {
        rendering::StaticVertexDedicated& v = geo.verticecs[i];
        float len = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
        v.x = (v.x / len) * radius;
        v.y = (v.y / len) * radius;
        v.z = (v.z / len) * radius;

        // Set normal to point outward (same as position for a sphere)
        v.nx = v.x / radius;
        v.ny = v.y / radius;
        v.nz = v.z / radius;

        // Simple UV mapping (could be improved)
        v.u0 = atan2f(v.z, v.x) / (2.0f * 3.14159f) + 0.5f;
        v.v0 = asinf(v.y / radius) / 3.14159f + 0.5f;
    }

    // Calculate tangents/bitangents (simplified version)
    for (int i = 0; i < geo.numIndices; i += 3) {
        rendering::StaticVertexDedicated& v0 = geo.verticecs[geo.indices[i]];
        rendering::StaticVertexDedicated& v1 = geo.verticecs[geo.indices[i + 1]];
        rendering::StaticVertexDedicated& v2 = geo.verticecs[geo.indices[i + 2]];

        float dx1 = v1.x - v0.x;
        float dy1 = v1.y - v0.y;
        float dz1 = v1.z - v0.z;
        float dx2 = v2.x - v0.x;
        float dy2 = v2.y - v0.y;
        float dz2 = v2.z - v0.z;

        float du1 = v1.u0 - v0.u0;
        float dv1 = v1.v0 - v0.v0;
        float du2 = v2.u0 - v0.u0;
        float dv2 = v2.v0 - v0.v0;

        float f = 1.0f / (du1 * dv2 - du2 * dv1);

        v0.tx = v1.tx = v2.tx = f * (dv2 * dx1 - dv1 * dx2);
        v0.ty = v1.ty = v2.ty = f * (dv2 * dy1 - dv1 * dy2);
        v0.tz = v1.tz = v2.tz = f * (dv2 * dz1 - dv1 * dz2);

        v0.bx = v1.bx = v2.bx = f * (du1 * dx2 - du2 * dx1);
        v0.by = v1.by = v2.by = f * (du1 * dy2 - du2 * dy1);
        v0.bz = v1.bz = v2.bz = f * (du1 * dz2 - du2 * dz1);
    }

    return geo;
}
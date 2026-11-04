/* assimp: import models from memory through the C API and check the scene
 * against known answers -- an OBJ cube (8 vertices, 6 quads: 12 triangles
 * after triangulation, its bounding box), an ASCII STL tetrahedron (4
 * triangles; 12 vertices even when identical ones are joined, as each
 * corner carries its facet's normal; those normals) -- then export the cube
 * to ASCII STL and binary STL in memory: 12 facets, and 80-byte header +
 * count + 50 bytes per triangle. */
#include "common.h"
#include <math.h>
#include <string.h>
#include <assimp/cimport.h>
#include <assimp/cexport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

static const char cube_obj[] =
  "o cube\n"
  "v -1 -1 -1\nv 1 -1 -1\nv 1 1 -1\nv -1 1 -1\n"
  "v -1 -1 1\nv 1 -1 1\nv 1 1 1\nv -1 1 1\n"
  "f 1 4 3 2\nf 5 6 7 8\nf 1 2 6 5\nf 2 3 7 6\nf 3 4 8 7\nf 4 1 5 8\n";

static const char tetra_stl[] =
  "solid t\n"
  "facet normal 0 0 -1\nouter loop\nvertex 0 0 0\nvertex 0 1 0\nvertex 1 0 0\nendloop\nendfacet\n"
  "facet normal 0 -1 0\nouter loop\nvertex 0 0 0\nvertex 1 0 0\nvertex 0 0 1\nendloop\nendfacet\n"
  "facet normal -1 0 0\nouter loop\nvertex 0 0 0\nvertex 0 0 1\nvertex 0 1 0\nendloop\nendfacet\n"
  "facet normal 0.57735 0.57735 0.57735\nouter loop\nvertex 1 0 0\nvertex 0 1 0\nvertex 0 0 1\nendloop\nendfacet\n"
  "endsolid t\n";

int main(void) {
  QT_BEGIN("assimp");

  const struct aiScene *s = aiImportFileFromMemory(cube_obj, sizeof cube_obj - 1,
      aiProcess_Triangulate | aiProcess_JoinIdenticalVertices, "obj");
  QT_CHECK(s != NULL, "OBJ cube imports");
  if (s) {
    QT_CHECK(s->mNumMeshes == 1, "OBJ: one mesh");
    const struct aiMesh *m = s->mMeshes[0];
    QT_CHECK(m->mNumFaces == 12, "OBJ: 6 quads triangulated to 12 faces");
    QT_CHECK(m->mNumVertices == 8, "OBJ: 8 vertices after joining identical ones");
    float lo = 1e9f, hi = -1e9f;
    for (unsigned i = 0; i < m->mNumVertices; ++i) {
      if (m->mVertices[i].x < lo) lo = m->mVertices[i].x;
      if (m->mVertices[i].x > hi) hi = m->mVertices[i].x;
    }
    QT_CHECK(lo == -1.0f && hi == 1.0f, "OBJ: x spans [-1, 1]");
    aiReleaseImport(s);
  }

  s = aiImportFileFromMemory(tetra_stl, sizeof tetra_stl - 1, aiProcess_JoinIdenticalVertices, "stl");
  QT_CHECK(s != NULL, "STL tetrahedron imports");
  if (s) {
    const struct aiMesh *m = s->mMeshes[0];
    QT_CHECK(m->mNumFaces == 4, "STL: 4 triangles");
    QT_CHECK(m->mNumVertices == 12, "STL: 12 vertices (corners differ by normal)");
    QT_CHECK(m->mNormals != NULL && fabsf(m->mNormals[0].z + 1.0f) < 1e-5f, "STL: first facet normal is -z");

    aiReleaseImport(s);
  }

  s = aiImportFileFromMemory(cube_obj, sizeof cube_obj - 1, aiProcess_Triangulate, "obj");
  if (s) {
    const struct aiExportDataBlob *b = aiExportSceneToBlob(s, "stl", 0);
    QT_CHECK(b != NULL, "export the cube to ASCII STL");
    if (!b) printf("       %s\n", aiGetErrorString());
    if (b) {
      int facets = 0;
      const char *p = (const char *)b->data, *end = p + b->size;
      for (; p + 12 <= end; ++p) if (memcmp(p, "facet normal", 12) == 0) facets++;
      QT_CHECK(facets == 12, "ASCII STL has 12 facets");
      aiReleaseExportBlob(b);
    }
    b = aiExportSceneToBlob(s, "stlb", 0);
    QT_CHECK(b != NULL, "export the cube to binary STL");
    if (!b) printf("       %s\n", aiGetErrorString());
    if (b) {
      QT_CHECK(b->size == 80 + 4 + 12 * 50, "binary STL is 84 + 12 x 50 bytes");
      aiReleaseExportBlob(b);
    }
    aiReleaseImport(s);
  }
  QT_END();
}


#ifndef COMMON_HOST_DEVICE
#define COMMON_HOST_DEVICE

#ifdef __cplusplus
#include <glm/glm.hpp>
// GLSL Type
using vec2 = glm::vec2;
using vec3 = glm::vec3;
using vec4 = glm::vec4;
using mat4 = glm::mat4;
using uint = unsigned int;
#endif

#ifdef __cplusplus
#define BOOL(name) bool name; bool pad##name[3] // C++ bool is 1 byte, so use 3 pad bytes
#else
#define BOOL(name) bool name // GLSL bool is 4 bytes.
#endif


// clang-format off
#ifdef __cplusplus // Descriptor binding helper for C++ and GLSL
 #define START_ENUM(a) enum a {
 #define END_ENUM() }
#else
 #define START_ENUM(a)  const uint
 #define END_ENUM() 
#endif

START_ENUM(ScBindings)
  eMatrices  = 0,  // Global uniform containing camera matrices
  eObjDescs = 1,  // Access to the object descriptions
  eTextures = 2   // Access to textures
END_ENUM();

START_ENUM(RtBindings)
  eTlas     = 0,  // Top-level acceleration structure
  eOutImage = 1,   // Ray tracer output image
  eColorHistoryImage = 2
END_ENUM();
// clang-format on



// Information of a obj model when referenced in a shader
struct ObjDesc
{
  int      txtOffset;             // Texture index offset in the array of textures
  uint64_t vertexAddress;         // Address of the Vertex buffer
  uint64_t indexAddress;          // Address of the index buffer
  uint64_t materialAddress;       // Address of the material buffer
  uint64_t materialIndexAddress;  // Address of the triangle material index buffer
};

// Uniform buffer set at each frame
struct MatrixUniforms
{
  mat4 viewProj;     // Camera view * projection
  mat4 priorViewProj;     // Camera view * projection
  mat4 viewInverse;  // Camera inverse view matrix
  mat4 projInverse;  // Camera inverse projection matrix
};

// Push constant structure for the raster
struct PushConstantRaster
{
  mat4  modelMatrix;  // matrix of the instance
  vec3  lightPosition;
  uint  objIndex;
  float lightIntensity;
  int   lightType;
};


// Push constant structure for the ray tracer
struct PushConstantRay
{
  //int   randSeed;
  //int   randSeed2;
  //int   frame;
  //bool  historyView;
  //float jitter;
  //float numSteps;
  //bool  ExplicitLightRays;
  //float posTolerance;
  //float np_m;
  //float np_b;
  //bool  useHistory;

	float rr;
	int depth;
	uint frameSeed;
  BOOL(cameraMoved);
  BOOL(history);
  int alignmentTest;

	vec4 tempLightPos; // TEMPORARY � vec4(0.5f, 2.5f, 3.0f, 0.0);
	vec4 tempLightInt; // TEMPORARY -- vec4(2.5, 2.5, 2.5, 0.0);
	vec4 tempAmbient; // TEMPORARY � vec4(0.2);

};


struct Vertex  // Created by readModel; used in shaders
{
  vec3 pos;
  vec3 nrm;
  vec2 texCoord;
};

struct Material  // Created by readModel; used in shaders
{
  vec3  diffuse;
  vec3  specular;
  vec3  emission;
  float shininess;
  int   textureId;
};


// Push constant structure for the ray tracer
struct PushConstantDenoise
{
 /* float normFactor;
  float depthFactor;
  float varianceFactor;
  float lumenFactor;

  float n_threshold;
  float d_threshold;

  int  dist;
  bool varianceView;
  bool placeholder1;
  bool placeholder2;*/
	
	float normFactor, depthFactor, lumenFactor;
 int stepwidth; 
};

struct RayPayload
{
	bool hit; // Does the ray intersect anything or not?
	vec3 hitPos; // The world coordinates of the hit point.
  float hitDist;
  
	int instanceIndex; // Index of the object instance hit (we have only one, so =0)
	int primitiveIndex; // Index of the hit triangle primitive within object
	vec3 bc; // Barycentric coordinates of the hit point within triangle
  uint seed;


};

#endif

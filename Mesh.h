#include <unordered_map>

std::vector<Vertex> ModelVertices;
std::vector<Vertex> ModelUVs;
std::vector<Vertex> ModelNormals;
std::vector<uint32_t> ModelIndices;


void CreateMesh(const char *path)
{
	tinyobj::attrib_t VertexAttributes;
	std::vector<tinyobj::shape_t> Shapes;
	std::vector<tinyobj::material_t> Materials;
	
	std::string WarningString;
	std::string ErrorString;
	
	bool Success = tinyobj::LoadObj(&VertexAttributes, &Shapes, &Materials, &WarningString, &ErrorString, path);
	if(!Success)
	{
		printf("%s\n", ErrorString.c_str());
	}
	
	for(tinyobj::shape_t Shape : Shapes)
	{
		for(tinyobj::index_t Index : Shape.mesh.indices)
		{
			glm::vec3 Pos = 
			{
				VertexAttributes.vertices[3 * Index.vertex_index + 0],
				VertexAttributes.vertices[3 * Index.vertex_index + 2],
				VertexAttributes.vertices[3 * Index.vertex_index + 1]
			};
			
			glm::vec2 UVs = 
			{
				VertexAttributes.texcoords[2 * Index.texcoord_index + 0],
				VertexAttributes.texcoords[2 * Index.texcoord_index + 1]
			};
			
			glm::vec3 Normal = 
			{
				VertexAttributes.normals[3 * Index.normal_index + 0],
				VertexAttributes.normals[3 * Index.normal_index + 2],
				VertexAttributes.normals[3 * Index.normal_index + 1]
			};
			
			Vertex Vert(Pos, glm::vec3(1.0, 1.0, 1.0), UVs, Normal);
			
			ModelVertices.push_back(Vert);
			ModelIndices.push_back(ModelIndices.size());
			/*
			if(MapVertices.count(Vert) == 0)
			{
				MapVertices[Vert] = MapVertices.size();
				ModelVertices.push_back(Vert);
			}
			
			ModelIndices.push_back(MapVertices[Vert]);
			*/
		}
	}
}

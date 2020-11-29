UniformBufferObject UBO;
glm::mat4 Model;
void* Data;

//Fps
int Frame = 0;
int Final_Time;
int Init_Time = 0;
int Final_Fps = 0;
char CharFPS[32];

void CalcFps(const char *title)
{
	Frame ++;
	Final_Time = time(NULL);
	if(Final_Time - Init_Time > 0)	
	{
		Final_Fps = Frame/(Final_Time - Init_Time);
		Frame = 0;
		Init_Time = Final_Time;
	}
	
	sprintf(CharFPS,"%s %d", title, Final_Fps);
	glfwSetWindowTitle(window, CharFPS);
}

void LoadIdentity()
{
	Model = glm::mat4(1.0f);
}

void Translate(float x, float y, float z)
{
	Model = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, z));
	UBO.Model = Model;
}

void Rotate(float angle, float x, float y, float z)
{
	Model = glm::rotate(Model, angle, glm::vec3(x, y, z));
	UBO.Model = Model;
}

void Scale(float x, float y, float z)
{
	Model = glm::scale(Model, glm::vec3(x, y, z));
	UBO.Model = Model;
}

void PushMatrix()
{
	vkMapMemory(device, UniformBufferMemory, 0, sizeof(UBO.Model), 0, &Data);
	memcpy(Data, &UBO.Model, sizeof(UBO.Model));
	vkUnmapMemory(device, UniformBufferMemory);
}

glm::mat4 PerspectivMatrix( float aspect_ratio, float field_of_view, float near_plane, float far_plane ) 
{
    float f = 1.0f / tan(glm::radians(0.5f * field_of_view) );

    glm::mat4 perspective_projection_matrix = 
	{
      f / aspect_ratio,
      0.0f,
      0.0f,
      0.0f,

      0.0f,
      -f,
      0.0f,
      0.0f,

      0.0f,
      0.0f,
      far_plane / (near_plane - far_plane),
      -1.0f,
      
      0.0f,
      0.0f,
      (near_plane * far_plane) / (near_plane - far_plane),
      0.0f
    };
    return perspective_projection_matrix;
}


struct Vertex
{
	glm::vec3 Pos;
	glm::vec3 Color;
	glm::vec2 UVcoord;
	glm::vec3 Normal;
	
	Vertex(glm::vec3 Pos, glm::vec3 Color, glm::vec2 UVcoord, glm::vec3 Normal)
		: Pos(Pos), Color(Color), UVcoord(UVcoord), Normal(Normal)
	{}
	
	static VkVertexInputBindingDescription GetBindingDescription()
	{
		VkVertexInputBindingDescription VertexInputBindingDescription;
		VertexInputBindingDescription.binding = 0;
		VertexInputBindingDescription.stride = sizeof(Vertex);
		VertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		
		return VertexInputBindingDescription;
	}
	
	static std::vector<VkVertexInputAttributeDescription> GetAttributeDescription()
	{
		std::vector<VkVertexInputAttributeDescription> VertexInputAttributeDescription(4);
		VertexInputAttributeDescription[0].location = 0;
		VertexInputAttributeDescription[0].binding = 0;
		VertexInputAttributeDescription[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		VertexInputAttributeDescription[0].offset = offsetof(Vertex, Pos);
		
		VertexInputAttributeDescription[1].location = 1;
		VertexInputAttributeDescription[1].binding = 0;
		VertexInputAttributeDescription[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		VertexInputAttributeDescription[1].offset = offsetof(Vertex, Color);
		
		VertexInputAttributeDescription[2].location = 2;
		VertexInputAttributeDescription[2].binding = 0;
		VertexInputAttributeDescription[2].format = VK_FORMAT_R32G32_SFLOAT;
		VertexInputAttributeDescription[2].offset = offsetof(Vertex, UVcoord);
		
		VertexInputAttributeDescription[3].location = 3;
		VertexInputAttributeDescription[3].binding = 0;
		VertexInputAttributeDescription[3].format = VK_FORMAT_R32G32B32_SFLOAT;
		VertexInputAttributeDescription[3].offset = offsetof(Vertex, Normal);
		
		return VertexInputAttributeDescription;
	}
};


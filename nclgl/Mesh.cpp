#include "Mesh.h"
#include "Matrix2.h"
#include "ProfilingManager.h"

using std::string;

Mesh::Mesh(void)	{
	glGenVertexArrays(1, &arrayObject);
	
	for(int i = 0; i < MAX_BUFFER; ++i) {
		bufferObject[i] = 0;
	}

	numVertices  = 0;
	type		 = GL_TRIANGLES;

	numIndices		= 0;
	vertices		= nullptr;
	textureCoords	= nullptr;
	normals			= nullptr;
	tangents		= nullptr;
	indices			= nullptr;
	colours			= nullptr;
	weights			= nullptr;
	weightIndices	= nullptr;
}

Mesh::~Mesh(void)	{
	glDeleteVertexArrays(1, &arrayObject);			//Delete our VAO
	glDeleteBuffers(MAX_BUFFER, bufferObject);		//Delete our VBOs

	delete[]	vertices;
	delete[]	indices;
	delete[]	textureCoords;
	delete[]	tangents;
	delete[]	normals;
	delete[]	colours;
	delete[]	weights;
	delete[]	weightIndices;
}

void Mesh::Draw()	{
	glBindVertexArray(arrayObject);
	if(bufferObject[INDEX_BUFFER]) {
		ProfilingManager::DrawCalls++;
		ProfilingManager::VerticesCountCurrent += numVertices;
		ProfilingManager::TrianglesCountCurrent += numIndices / 3;
		glDrawElements(type, numIndices, GL_UNSIGNED_INT, 0);
	}
	else{
		ProfilingManager::DrawCalls++;
		ProfilingManager::VerticesCountCurrent += numVertices;
		ProfilingManager::TrianglesCountCurrent += numIndices / 3;
		glDrawArrays(type, 0, numVertices);
	}
	glBindVertexArray(0);	
}

void Mesh::Draw(int instanceAmount)
{
	glBindVertexArray(arrayObject);
	if (bufferObject[INDEX_BUFFER]) 
	{
		ProfilingManager::DrawCalls++;
		ProfilingManager::VerticesCountCurrent += numVertices;
		ProfilingManager::TrianglesCountCurrent += numIndices / 3;
		glDrawElementsInstanced(type, numIndices, GL_UNSIGNED_INT, 0, instanceAmount);
	}
	else 
	{
		ProfilingManager::DrawCalls++;
		ProfilingManager::VerticesCountCurrent += numVertices;
		ProfilingManager::TrianglesCountCurrent += numIndices / 3;
		glDrawArraysInstanced(type, 0, numVertices, instanceAmount);
	}
	glBindVertexArray(0);
}

void Mesh::DrawSubMesh(int i) {
	if (i < 0 || i >= (int)meshLayers.size()) {
		return;
	}
	SubMesh m = meshLayers[i];

	glBindVertexArray(arrayObject);
	if (bufferObject[INDEX_BUFFER]) {
		const GLvoid* offset = (const GLvoid * )(m.start * sizeof(unsigned int));
		ProfilingManager::DrawCalls++;
		ProfilingManager::VerticesCountCurrent += m.count;
		ProfilingManager::TrianglesCountCurrent += m.count / 3;
		glDrawElements(type, m.count, GL_UNSIGNED_INT, offset);
	}
	else {
		ProfilingManager::DrawCalls++;
		ProfilingManager::VerticesCountCurrent += m.count;
		ProfilingManager::TrianglesCountCurrent += m.count / 3;
		glDrawArrays(type, m.start, m.count);	//Draw the triangle!
	}
	glBindVertexArray(0);
}

void Mesh::DrawSubMesh(int i, int instanceAmount)
{
	if (i < 0 || i >= (int)meshLayers.size()) {
		return;
	}
	SubMesh m = meshLayers[i];

	glBindVertexArray(arrayObject);
	if (bufferObject[INDEX_BUFFER]) 
	{
		const GLvoid* offset = (const GLvoid*)(m.start * sizeof(unsigned int));
		//glDrawElements(type, m.count, GL_UNSIGNED_INT, offset);
		ProfilingManager::DrawCalls++;
		ProfilingManager::VerticesCountCurrent += m.count;
		ProfilingManager::TrianglesCountCurrent += m.count / 3;
		glDrawElementsInstanced(type, m.count, GL_UNSIGNED_INT, offset, instanceAmount);
	}
	else 
	{
		//glDrawArrays(type, m.start, m.count);	//Draw the triangle!
		ProfilingManager::DrawCalls++;
		ProfilingManager::VerticesCountCurrent += m.count;
		ProfilingManager::TrianglesCountCurrent += m.count / 3;
		glDrawArraysInstanced(type, m.start, m.count, instanceAmount);
	}
	glBindVertexArray(0);
}

Mesh* Mesh::GenerateTriangle()
{
	Mesh* m = new Mesh();
	m->numVertices = 3;

	ProfilingManager::VerticesCount += m->numVertices;
	ProfilingManager::TrianglesCount++;
	
	m->vertices		=	new Vector3[m->numVertices];
	m->vertices[0]	=	Vector3(-0.5f, -0.5f, 0.0f);
	m->vertices[1]	=	Vector3(0, 0.5f, 0.0f);
	m->vertices[2]	=	Vector3(0.5f, -0.5f, 0.0f);
	
	m->colours		=	new Vector4[m->numVertices];
	m->colours[0]	=	Vector4(1.0f, 0.0f, 0.0f, 1.0f);
	m->colours[1]	=	Vector4(0.0f, 1.0f, 0.0f, 1.0f);
	m->colours[2]	=	Vector4(0.0f, 0.0f, 1.0f, 1.0f);

	m->textureCoords	= new Vector2[m->numVertices];
	m->textureCoords[0] = Vector2(0.0f, 0.0f);
	m->textureCoords[1] = Vector2(0.5f, 1.0f);
	m->textureCoords[2] = Vector2(1.0f, 0.0f);
	
	m->GenerateBoundingBox();
	m->BufferData();

	return m;
}

Mesh* Mesh::GenerateQuad()
{
	Mesh* m = new Mesh();
	m->type = GL_TRIANGLE_STRIP;
	m->numVertices = 4;

	ProfilingManager::VerticesCount += m->numVertices;
	ProfilingManager::TrianglesCount += 2;

	m->vertices = new Vector3[m->numVertices];
	m->normals = new Vector3[m->numVertices];
	m->tangents = new Vector4[m->numVertices];
	/*m->vertices[0] = Vector3(0.5f, 0.5f, 0.0f);
	m->vertices[1] = Vector3(-0.5f, 0.5f, 0.0f);
	m->vertices[2] = Vector3(0.5f, -0.5f, 0.0f); 
	m->vertices[3] = Vector3(-0.5f, -0.5f, 0.0f);*/

	m->vertices[0] = Vector3(-1.0f, 1.0f, 0.0f);
	m->vertices[1] = Vector3(-1.0f, -1.0f, 0.0f);
	m->vertices[2] = Vector3(1.0f, 1.0f, 0.0f);
	m->vertices[3] = Vector3(1.0f, -1.0f, 0.0f);

	m->colours = new Vector4[m->numVertices];
	/*m->colours[0] = Vector4(1.0f, 0.0f, 0.0f, 1.0f);
	m->colours[1] = Vector4(0.0f, 1.0f, 0.0f, 1.0f);
	m->colours[2] = Vector4(0.0f, 0.0f, 1.0f, 1.0f);
	m->colours[3] = Vector4(0.0f, 0.0f, 0.0f, 1.0f);*/

	m->textureCoords = new Vector2[m->numVertices];
	m->textureCoords[0] = Vector2(0.0f, 1.0f);
	m->textureCoords[1] = Vector2(0.0f, 0.0f);
	m->textureCoords[2] = Vector2(1.0f, 1.0f);
	m->textureCoords[3] = Vector2(1.0f, 0.0f);

	for (int i = 0; i < 4; ++i)
	{
		m->colours[i] = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		m->normals[i] = Vector3(0.0f, 0.0f, -1.0f);
		m->tangents[i] = Vector4(1.0f, 0.0f, 0.0f, 1.0f);
	}

	m->GenerateBoundingBox();
	m->BufferData();

	return m;
}

Mesh* Mesh::GenerateCircle(float cx, float cy, float r, Vector4 color, GLuint type, int numSides)
{
	Mesh* m = new Mesh();
	m->type = type;
	m->numVertices = numSides;

	m->vertices = new Vector3[m->numVertices];
	m->colours = new Vector4[m->numVertices];
	for (int i = 0; i < (int)m->numVertices; i++)
	{
		float theta = 2.0f * 3.14f * float(i) / float(m->numVertices);
		float x = r * cos(theta);
		float y = r * sin(theta);

		m->vertices[i] = Vector3(cx + x, cy + y, 0.0);
		m->colours[i] = color;

		//GenerateCircle(x, y, 0.1f);
	}

	m->GenerateBoundingBox();
	m->BufferData();

	return m;
}

Mesh* Mesh::GenerateCube()
{
	Mesh* m = new Mesh();
	m->type = GL_TRIANGLES;
	m->numVertices = 36;

	ProfilingManager::VerticesCount += m->numVertices;
	ProfilingManager::TrianglesCount += 12;

	m->vertices = new Vector3[m->numVertices];
	m->colours = new Vector4[m->numVertices];

	/*m->vertices[0] = Vector3(-0.5f, -0.5f, -0.5f);
	m->vertices[1] = Vector3(0.5f, -0.5f, -0.5f);
	m->vertices[2] = Vector3(0.5f, 0.5f, -0.5f);
	m->vertices[3] = Vector3(0.5f, 0.5f, -0.5f);
	m->vertices[4] = Vector3(-0.5f, 0.5f, -0.5f);
	m->vertices[5] = Vector3(-0.5f, -0.5f, -0.5f);

	m->vertices[6] = Vector3(-0.5f, -0.5f, 0.5f);
	m->vertices[7] = Vector3(0.5f, -0.5f, 0.5f);
	m->vertices[8] = Vector3(0.5f, 0.5f, 0.5f);
	m->vertices[9] = Vector3(0.5f, 0.5f, 0.5f);
	m->vertices[10] = Vector3(-0.5f, 0.5f, 0.5f);
	m->vertices[11] = Vector3(-0.5f, -0.5f, 0.5f);

	m->vertices[12] = Vector3(-0.5f, 0.5f, 0.5f);
	m->vertices[13] = Vector3(-0.5f, 0.5f, -0.5f);
	m->vertices[14] = Vector3(-0.5f, -0.5f, -0.5f);
	m->vertices[15] = Vector3(-0.5f, -0.5f, -0.5f);
	m->vertices[16] = Vector3(-0.5f, -0.5f, 0.5f);
	m->vertices[17] = Vector3(-0.5f, 0.5f, 0.5f);

	m->vertices[18] = Vector3(0.5f, 0.5f, 0.5f);
	m->vertices[19] = Vector3(0.5f, 0.5f, -0.5f);
	m->vertices[20] = Vector3(0.5f, -0.5f, -0.5f);
	m->vertices[21] = Vector3(0.5f, -0.5f, -0.5f);
	m->vertices[22] = Vector3(0.5f, -0.5f, 0.5f);
	m->vertices[23] = Vector3(0.5f, 0.5f, 0.5f);

	m->vertices[24] = Vector3(-0.5f, -0.5f, -0.5f);
	m->vertices[25] = Vector3(0.5f, -0.5f, -0.5f);
	m->vertices[26] = Vector3(0.5f, -0.5f, 0.5f);
	m->vertices[27] = Vector3(0.5f, -0.5f, 0.5f);
	m->vertices[28] = Vector3(-0.5f, -0.5f, 0.5f);
	m->vertices[29] = Vector3(-0.5f, -0.5f, -0.5f);

	m->vertices[30] = Vector3(-0.5f, 0.5f, -0.5f);
	m->vertices[31] = Vector3(0.5f, 0.5f, -0.5f);
	m->vertices[32] = Vector3(0.5f, 0.5f, 0.5f);
	m->vertices[33] = Vector3(0.5f, 0.5f, 0.5f);
	m->vertices[34] = Vector3(-0.5f, 0.5f, 0.5f);
	m->vertices[35] = Vector3(-0.5f, 0.5f, -0.5f);*/

	m->vertices[0] = Vector3(-1.0f, 1.0f, -1.0f);
	m->vertices[1] = Vector3(-1.0f, -1.0f, -1.0f);
	m->vertices[2] = Vector3(1.0f, -1.0f, -1.0f);
	m->vertices[3] = Vector3(1.0f, -1.0f, -1.0f);
	m->vertices[4] = Vector3(1.0f, 1.0f, -1.0f);
	m->vertices[5] = Vector3(-1.0f, 1.0f, -1.0f);

	m->vertices[6] = Vector3(-1.0f, -1.0f, 1.0f);
	m->vertices[7] = Vector3(-1.0f, -1.0f, -1.0f);
	m->vertices[8] = Vector3(-1.0f, 1.0f, -1.0f);
	m->vertices[9] = Vector3(-1.0f, 1.0f, -1.0f);
	m->vertices[10] = Vector3(-1.0f, 1.0f, 1.0f);
	m->vertices[11] = Vector3(-1.0f, -1.0f, 1.0f);

	m->vertices[12] = Vector3(1.0f, -1.0f, -1.0f);
	m->vertices[13] = Vector3(1.0f, -1.0f, 1.0f);
	m->vertices[14] = Vector3(1.0f, 1.0f, 1.0f);
	m->vertices[15] = Vector3(1.0f, 1.0f, 1.0f);
	m->vertices[16] = Vector3(1.0f, 1.0f, -1.0f);
	m->vertices[17] = Vector3(1.0f, -1.0f, -1.0f);

	m->vertices[18] = Vector3(-1.0f, -1.0f, 1.0f);
	m->vertices[19] = Vector3(-1.0f, 1.0f, 1.0f);
	m->vertices[20] = Vector3(1.0f, 1.0f, 1.0f);
	m->vertices[21] = Vector3(1.0f, 1.0f, 1.0f);
	m->vertices[22] = Vector3(1.0f, -1.0f, 1.0f);
	m->vertices[23] = Vector3(-1.0f, -1.0f, 1.0f);

	m->vertices[24] = Vector3(-1.0f, 1.0f, -1.0f);
	m->vertices[25] = Vector3(1.0f, 1.0f, -1.0f);
	m->vertices[26] = Vector3(1.0f, 1.0f, 1.0f);
	m->vertices[27] = Vector3(1.0f, 1.0f, 1.0f);
	m->vertices[28] = Vector3(-1.0f, 1.0f, 1.0f);
	m->vertices[29] = Vector3(-1.0f, 1.0f, -1.0f);

	m->vertices[30] = Vector3(-1.0f, -1.0f, -1.0f);
	m->vertices[31] = Vector3(-1.0f, -1.0f, 1.0f);
	m->vertices[32] = Vector3(1.0f, -1.0f, -1.0f);
	m->vertices[33] = Vector3(1.0f, -1.0f, -1.0f);
	m->vertices[34] = Vector3(-1.0f, -1.0f, 1.0f);
	m->vertices[35] = Vector3(1.0f, -1.0f, 1.0f);
 

	m->textureCoords = new Vector2[m->numVertices];
	m->textureCoords[0] = Vector2(0.0f, 0.0f);
	m->textureCoords[1] = Vector2(1.0f, 0.0f);
	m->textureCoords[2] = Vector2(1.0f, 1.0f);
	m->textureCoords[3] = Vector2(1.0f, 1.0f);
	m->textureCoords[4] = Vector2(0.0f, 1.0f);
	m->textureCoords[5] = Vector2(0.0f, 0.0f);

	m->textureCoords[6] = Vector2(0.0f, 0.0f);
	m->textureCoords[7] = Vector2(1.0f, 0.0f);
	m->textureCoords[8] = Vector2(1.0f, 1.0f);
	m->textureCoords[9] = Vector2(1.0f, 1.0f);
	m->textureCoords[10] = Vector2(0.0f, 1.0f);
	m->textureCoords[11] = Vector2(0.0f, 0.0f);

	m->textureCoords[12] = Vector2(1.0f, 0.0f);
	m->textureCoords[13] = Vector2(1.0f, 1.0f);
	m->textureCoords[14] = Vector2(0.0f, 1.0f);
	m->textureCoords[15] = Vector2(0.0f, 1.0f);
	m->textureCoords[16] = Vector2(0.0f, 0.0f);
	m->textureCoords[17] = Vector2(1.0f, 0.0f);
	
	m->textureCoords[18] = Vector2(1.0f, 0.0f);
	m->textureCoords[19] = Vector2(1.0f, 1.0f);
	m->textureCoords[20] = Vector2(0.0f, 1.0f);
	m->textureCoords[21] = Vector2(0.0f, 1.0f);
	m->textureCoords[22] = Vector2(0.0f, 0.0f);
	m->textureCoords[23] = Vector2(1.0f, 0.0f);
	
	m->textureCoords[24] = Vector2(0.0f, 1.0f);
	m->textureCoords[25] = Vector2(1.0f, 1.0f);
	m->textureCoords[26] = Vector2(1.0f, 0.0f);
	m->textureCoords[27] = Vector2(1.0f, 0.0f);
	m->textureCoords[28] = Vector2(0.0f, 0.0f);
	m->textureCoords[29] = Vector2(0.0f, 1.0f);
	
	m->textureCoords[30] = Vector2(0.0f, 1.0f);
	m->textureCoords[31] = Vector2(1.0f, 1.0f);
	m->textureCoords[32] = Vector2(1.0f, 0.0f);
	m->textureCoords[33] = Vector2(1.0f, 0.0f);
	m->textureCoords[34] = Vector2(0.0f, 0.0f);
	m->textureCoords[35] = Vector2(0.0f, 1.0f);

	m->GenerateBoundingBox();
	m->BufferData();
	return m;
}

void Mesh::SetAllVertexColour(Mesh* M, const Vector4& colour)
{
	if (M->numVertices <= 0)
		return;

	for (size_t i = 0; i < M->numVertices; i++)
		M->colours[i] = colour;

	M->BufferData();
}
 
void UploadAttribute(GLuint* id, int numElements, int dataSize, int attribSize, int attribID, void* pointer, const string&debugName) {
	glGenBuffers(1, id);
	glBindBuffer(GL_ARRAY_BUFFER, *id);
	glBufferData(GL_ARRAY_BUFFER, numElements * dataSize, pointer, GL_STATIC_DRAW);

	glVertexAttribPointer(attribID, attribSize, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(attribID);

	glObjectLabel(GL_BUFFER, *id, -1, debugName.c_str());
}

void	Mesh::BufferData()	{
	glBindVertexArray(arrayObject);

	////Buffer vertex data
	UploadAttribute(&bufferObject[VERTEX_BUFFER], numVertices, sizeof(Vector3), 3, VERTEX_BUFFER, vertices, "Positions");

	if(textureCoords) {	//Buffer texture data
		UploadAttribute(&bufferObject[TEXTURE_BUFFER], numVertices, sizeof(Vector2), 2, TEXTURE_BUFFER, textureCoords, "TexCoords");
	}

	if (colours) {
		UploadAttribute(&bufferObject[COLOUR_BUFFER], numVertices, sizeof(Vector4), 4, COLOUR_BUFFER, colours, "Colours");
	}

	if (normals) {	//Buffer normal data
		UploadAttribute(&bufferObject[NORMAL_BUFFER], numVertices, sizeof(Vector3), 3, NORMAL_BUFFER, normals, "Normals");
	}

	if (tangents) {	//Buffer tangent data
		UploadAttribute(&bufferObject[TANGENT_BUFFER], numVertices, sizeof(Vector4), 4, TANGENT_BUFFER, tangents, "Tangents");
	}

	if (weights) {		//Buffer weights data
		UploadAttribute(&bufferObject[WEIGHTVALUE_BUFFER], numVertices, sizeof(Vector4), 4, WEIGHTVALUE_BUFFER, weights, "Weights");
	}

	//Buffer weight indices data...uses a different function since its integers...
	if (weightIndices) {
		glGenBuffers(1, &bufferObject[WEIGHTINDEX_BUFFER]);
		glBindBuffer(GL_ARRAY_BUFFER, bufferObject[WEIGHTINDEX_BUFFER]);
		glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(int) * 4, weightIndices, GL_STATIC_DRAW);
		glVertexAttribIPointer(WEIGHTINDEX_BUFFER, 4, GL_INT, 0, 0); //note the new function...
		glEnableVertexAttribArray(WEIGHTINDEX_BUFFER);

		glObjectLabel(GL_BUFFER, bufferObject[WEIGHTINDEX_BUFFER], -1, "Weight Indices");
	}

	//buffer index data
	if(indices) {
		glGenBuffers(1, &bufferObject[INDEX_BUFFER]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferObject[INDEX_BUFFER]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices*sizeof(GLuint), indices, GL_STATIC_DRAW);

		glObjectLabel(GL_BUFFER, bufferObject[INDEX_BUFFER], -1, "Indices");
	}
	glBindVertexArray(0);	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}


/*
* 
* Extra file loading stuff!
* 
* */

enum class GeometryChunkTypes {
	VPositions		= 1,
	VNormals		= 2,
	VTangents		= 4,
	VColors			= 8,
	VTex0			= 16,
	VTex1			= 32,
	VWeightValues	= 64,
	VWeightIndices	= 128,
	Indices			= 256,
	JointNames		= 512,
	JointParents	= 1024,
	BindPose		= 2048,
	BindPoseInv		= 4096,
	Material		= 65536,
	SubMeshes		= 1 << 14,
	SubMeshNames	= 1 << 15
};

void ReadTextFloats(std::ifstream& file, vector<Vector2>& element, int numVertices) {
	for (int i = 0; i < numVertices; ++i) {
		Vector2 temp;
		file >> temp.x;
		file >> temp.y;
		element.emplace_back(temp);
	}
}

void ReadTextFloats(std::ifstream& file, vector<Vector3>& element, int numVertices) {
	for (int i = 0; i < numVertices; ++i) {
		Vector3 temp;
		file >> temp.x;
		file >> temp.y;
		file >> temp.z;
		element.emplace_back(temp);
	}
}

void ReadTextFloats(std::ifstream& file, vector<Vector4>& element, int numVertices) {
	for (int i = 0; i < numVertices; ++i) {
		Vector4 temp;
		file >> temp.x;
		file >> temp.y;
		file >> temp.z;
		file >> temp.w;
		element.emplace_back(temp);
	}
}

void ReadTextVertexIndices(std::ifstream& file, vector<int>& element, int numVertices) {
	for (int i = 0; i < numVertices; ++i) {
		int indices[4];
		file >> indices[0];
		file >> indices[1];
		file >> indices[2];
		file >> indices[3];
		element.emplace_back(indices[0]);
		element.emplace_back(indices[1]);
		element.emplace_back(indices[2]);
		element.emplace_back(indices[3]);
	}
}

void ReadIndices(std::ifstream& file, vector<unsigned int>& elements, int numIndices) {
	for (int i = 0; i < numIndices; ++i) {
		unsigned int temp;
		file >> temp;
		elements.emplace_back(temp);
	}
}

void ReadJointParents(std::ifstream& file, vector<int>& dest) {
	int jointCount = 0;
	file >> jointCount;

	for (int i = 0; i < jointCount; ++i) {
		int id = -1;
		file >> id;
		dest.emplace_back(id);
	}
}

void ReadJointNames(std::ifstream& file, vector<string>& dest) {
	int jointCount = 0;
	file >> jointCount;
	for (int i = 0; i < jointCount; ++i) {
		std::string jointName;
		file >> jointName;
		dest.emplace_back(jointName);
	}
}

void ReadRigPose(std::ifstream& file, Matrix4** into) {
	int matCount = 0;
	file >> matCount;

	*into = new Matrix4[matCount];

	for (int i = 0; i < matCount; ++i) {
		Matrix4 mat;
		for (int i = 0; i < 16; ++i) {
			file >> mat.values[i];
		}
		(*into)[i] = mat;
	}
}

void ReadSubMeshes(std::ifstream& file, int count, vector<Mesh::SubMesh> & subMeshes) {
	for (int i = 0; i < count; ++i) {
		Mesh::SubMesh m;
		file >> m.start;
		file >> m.count;
		subMeshes.emplace_back(m);
	}
}

void ReadSubMeshNames(std::ifstream& file, int count, vector<string>& names) {
	std::string scrap;
	std::getline(file, scrap);

	for (int i = 0; i < count; ++i) {
		std::string meshName;
		std::getline(file, meshName);
		names.emplace_back(meshName);
	}
}

Mesh* Mesh::LoadFromMeshFile(const string& name) {
	Mesh* mesh = new Mesh();

	std::ifstream file(MESHDIR + name);

	std::string filetype;
	int fileVersion;

	file >> filetype;

	if (filetype != "MeshGeometry") {
		std::cout << "File is not a MeshGeometry file!" << std::endl;
		return nullptr;
	}

	file >> fileVersion;

	if (fileVersion != 1) {
		std::cout << "MeshGeometry file has incompatible version!" << std::endl;
		return nullptr;
	}

	int numMeshes	= 0; //read
	int numVertices = 0; //read
	int numIndices	= 0; //read
	int numChunks	= 0; //read

	file >> numMeshes;
	file >> numVertices;
	file >> numIndices;
	file >> numChunks;

	vector<Vector3> readPositions;
	vector<Vector4> readColours;
	vector<Vector3> readNormals;
	vector<Vector4> readTangents;
	vector<Vector2> readUVs;
	vector<Vector4> readWeights;
	vector<int> readWeightIndices;

	vector<unsigned int>		readIndices;

	for (int i = 0; i < numChunks; ++i) {
		int chunkType = (int)GeometryChunkTypes::VPositions;

		file >> chunkType;

		switch ((GeometryChunkTypes)chunkType) {
		case GeometryChunkTypes::VPositions:ReadTextFloats(file, readPositions, numVertices);  break;
		case GeometryChunkTypes::VColors:	ReadTextFloats(file, readColours, numVertices);  break;
		case GeometryChunkTypes::VNormals:	ReadTextFloats(file, readNormals, numVertices);  break;
		case GeometryChunkTypes::VTangents:	ReadTextFloats(file, readTangents, numVertices);  break;
		case GeometryChunkTypes::VTex0:		ReadTextFloats(file, readUVs, numVertices);  break;
		case GeometryChunkTypes::Indices:	ReadIndices(file, readIndices, numIndices); break;

		case GeometryChunkTypes::VWeightValues:		ReadTextFloats(file, readWeights, numVertices);  break;
		case GeometryChunkTypes::VWeightIndices:	ReadTextVertexIndices(file, readWeightIndices, numVertices);  break;
		case GeometryChunkTypes::JointNames:		ReadJointNames(file, mesh->jointNames);  break;
		case GeometryChunkTypes::JointParents:		ReadJointParents(file, mesh->jointParents);  break;
		case GeometryChunkTypes::BindPose:			ReadRigPose(file, &mesh->bindPose);  break;
		case GeometryChunkTypes::BindPoseInv:		ReadRigPose(file, &mesh->inverseBindPose);  break;
		case GeometryChunkTypes::SubMeshes: 		ReadSubMeshes(file, numMeshes, mesh->meshLayers); break;
		case GeometryChunkTypes::SubMeshNames: 		ReadSubMeshNames(file, numMeshes, mesh->layerNames); break;
		}
	}
	//Now that the data has been read, we can shove it into the actual Mesh object

	mesh->numVertices	= numVertices;
	mesh->numIndices	= numIndices;

	ProfilingManager::VerticesCount += mesh->numVertices;
	ProfilingManager::TrianglesCount += mesh->numIndices / 3;

	if (!readPositions.empty()) {
		mesh->vertices = new Vector3[numVertices];
		memcpy(mesh->vertices, readPositions.data(), numVertices * sizeof(Vector3));
	}

	if (!readColours.empty()) {
		mesh->colours = new Vector4[numVertices];
		memcpy(mesh->colours, readColours.data(), numVertices * sizeof(Vector4));
	}

	if (!readNormals.empty()) {
		mesh->normals = new Vector3[numVertices];
		memcpy(mesh->normals, readNormals.data(), numVertices * sizeof(Vector3));
	}

	if (!readTangents.empty()) {
		mesh->tangents = new Vector4[numVertices];
		memcpy(mesh->tangents, readTangents.data(), numVertices * sizeof(Vector4));
	}

	if (!readUVs.empty()) {
		mesh->textureCoords = new Vector2[numVertices];
		memcpy(mesh->textureCoords, readUVs.data(), numVertices * sizeof(Vector2));
	}
	if (!readIndices.empty()) {
		mesh->indices = new unsigned int[numIndices];
		memcpy(mesh->indices, readIndices.data(), numIndices * sizeof(unsigned int));
	}

	if (!readWeights.empty()) {
		mesh->weights = new Vector4[numVertices];
		memcpy(mesh->weights, readWeights.data(), numVertices * sizeof(Vector4));
	}

	if (!readWeightIndices.empty()) {
		mesh->weightIndices = new int[numVertices * 4];
		memcpy(mesh->weightIndices, readWeightIndices.data(), numVertices * sizeof(int) * 4);
	}

	mesh->GenerateBoundingBox();
	mesh->BufferData();

	return mesh;
}

int Mesh::GetIndexForJoint(const std::string& name) const {
	for (unsigned int i = 0; i < jointNames.size(); ++i) {
		if (jointNames[i] == name) {
			return i;
		}
	}
	return -1;
}

int Mesh::GetParentForJoint(const std::string& name) const {
	int i = GetIndexForJoint(name);
	if (i == -1) {
		return -1;
	}
	return jointParents[i];
}

int Mesh::GetParentForJoint(int i) const {
	if (i == -1 || i >= (int)jointParents.size()) {
		return -1;
	}
	return jointParents[i];
}

bool Mesh::GetSubMesh(int i, const SubMesh* s) const {
	if (i < 0 || i >= (int)meshLayers.size()) {
		return false;
	}
	s = &meshLayers[i];
	return true;
}

bool Mesh::GetSubMesh(const string& name, const SubMesh* s) const {
	for (unsigned int i = 0; i < layerNames.size(); ++i) {
		if (layerNames[i] == name) {
			return GetSubMesh(i, s);
		}
	}
	return false;
}

void Mesh::GenerateNormals()
{
	if (!normals)
		normals = new Vector3[numVertices];

	for (GLuint i = 0; i < numVertices; i++)
		normals[i] = Vector3();

	int triCount = GetTriCount();

	for (int i = 0; i < triCount; i++)
	{
		unsigned int a = 0, b = 0, c = 0;
		GetVertexIndicesForTri(i, a, b, c);

		Vector3 normal = Vector3::Cross((vertices[b] - vertices[a]), (vertices[c] - vertices[a]));

		normals[a] += normal;
		normals[b] += normal;
		normals[c] += normal;
	}

	for (size_t i = 0; i < numVertices; i++)
		normals[i].Normalise();
}

bool Mesh::GetVertexIndicesForTri(unsigned int i, unsigned int& a, unsigned int& b, unsigned int& c) const
{
	unsigned int triCount = GetTriCount();
	if (i >= triCount)
		return false;

	if (numIndices > 0)
	{
		a = indices[(i * 3)];
		b = indices[(i * 3) + 1];
		c = indices[(i * 3) + 2];
	}
	else
	{
		a = (i * 3);
		b = (i * 3) + 1;
		c = (i * 3) + 2;
	}

	return true;
}

void Mesh::GenerateTangents()
{
	if (!textureCoords)
		return;

	if (!tangents)
		tangents = new Vector4[numVertices];

	for (GLuint i = 0; i < numVertices; i++)
		tangents[i] = Vector4(0, 0, 0, 0);

	int triCount = GetTriCount();
	for (int i = 0; i < triCount; i++)
	{
		unsigned int a = 0, b = 0, c = 0;
		GetVertexIndicesForTri(i, a, b, c);

		Vector4 tangent = GenerateTangent(a, b, c);
		tangents[a] += tangent;
		tangents[b] += tangent;
		tangents[c] += tangent;
	}

	for (GLuint i = 0; i < numVertices; i++)
	{
		float handedness = tangents[i].w > 0.0f ? 1.0f : -1.0f;
		tangents[i].w = 0.0f;
		tangents[i].Normalise();
		tangents[i].w = handedness;
	}
}

Vector4 Mesh::GenerateTangent(int a, int b, int c)
{
	Vector3 ba = vertices[b] - vertices[a];
	Vector3 ca = vertices[c] - vertices[a];

	Vector2 tba = textureCoords[b] - textureCoords[a];
	Vector2 tca = textureCoords[c] - textureCoords[a];

	Matrix2 texMat = Matrix2(tba, tca);
	texMat.Invert();

	Vector3 tangent;
	Vector3 binormal;

	tangent = ba * texMat.values[0] + ca * texMat.values[1];
	binormal = ba * texMat.values[2] + ca * texMat.values[3];

	Vector3 normal = Vector3::Cross(ba, ca);
	Vector3 biCross = Vector3::Cross(tangent, normal);

	float handedness = 1.0f;
	if (Vector3::Dot(biCross, normal) < 0.0f)
		handedness = -1.0f;

	return Vector4(tangent.x, tangent.y, tangent.z, handedness);
}

void Mesh::GenerateBoundingBox()
{
	if (numVertices <= 0) 
		return;

	float minX, maxX;
	float minY, maxY;
	float minZ, maxZ;

	minX = maxX = vertices[0].x;
	minY = maxY = vertices[0].y;
	minZ = maxZ = vertices[0].z;

	for (int i = 0; i < (int)numVertices; i++)
	{
		if (vertices[i].x < minX) minX = vertices[i].x;
		if (vertices[i].x > maxX) maxX = vertices[i].x;

		if (vertices[i].y < minY) minY = vertices[i].y;
		if (vertices[i].y > maxY) maxY = vertices[i].y;

		if (vertices[i].z < minZ) minZ = vertices[i].z;
		if (vertices[i].z > maxZ) maxZ = vertices[i].z;
	}

	boundingBoxSize = Vector3(maxX - minX, maxY - minY, maxZ - minZ);
	boundingBoxCenter = Vector3((minX + maxX) * 0.5f, (minY + maxY) * 0.5f, (minZ + maxZ) * 0.5f);
	boundingBoxExtents = Vector3(maxX - boundingBoxCenter.x, maxY - boundingBoxCenter.y, maxZ - boundingBoxCenter.z);
}

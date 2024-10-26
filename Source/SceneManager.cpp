///////////////////////////////////////////////////////////////////////////////
// scenemanager.cpp
// ============
// manage the preparing and rendering of 3D scenes - textures, materials, lighting
//
//  AUTHOR: Alan Chumsawang
//	Created for CS-330-Computational Graphics and Visualization, Oct. 21st, 2024
///////////////////////////////////////////////////////////////////////////////

#include "SceneManager.h"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

#include <glm/gtx/transform.hpp>

// declaration of global variables
namespace
{
	const char* g_ModelName = "model";
	const char* g_ColorValueName = "objectColor";
	const char* g_TextureValueName = "objectTexture";
	const char* g_UseTextureName = "bUseTexture";
	const char* g_UseLightingName = "bUseLighting";
}

/***********************************************************
 *  SceneManager()
 *
 *  The constructor for the class
 ***********************************************************/
SceneManager::SceneManager(ShaderManager *pShaderManager)
{
	m_pShaderManager = pShaderManager;
	m_basicMeshes = new ShapeMeshes();

	// initialize the texture collection
	for (int i = 0; i < 16; i++)
	{
		m_textureIDs[i].tag = "/0";
		m_textureIDs[i].ID = -1;
	}
	m_loadedTextures = 0;
}

/***********************************************************
 *  ~SceneManager()
 *
 *  The destructor for the class
 ***********************************************************/
SceneManager::~SceneManager()
{
	m_pShaderManager = NULL;
	if (NULL != m_basicMeshes)
	{
		delete m_basicMeshes;
		m_basicMeshes = NULL;
	}

	// free the allocated OpenGL textures
	DestroyGLTextures();


}
void SceneManager::SetProjectionMode(bool isPerspective)
{
	m_isPerspective = isPerspective;
}

void SceneManager::UpdateProjectionMatrix(float aspectRatio)
{
	if (m_isPerspective) {
		m_projectionMatrix = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);
	}
	else {
		m_projectionMatrix = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.1f, 100.0f);
	}
	m_pShaderManager->use(); // Make sure to activate the shader first
	m_pShaderManager->setMat4Value("projection", m_projectionMatrix); // Update the projection matrix in the shader
}


/***********************************************************
 *  CreateGLTexture()
 *
 *  This method is used for loading textures from image files,
 *  configuring the texture mapping parameters in OpenGL,
 *  generating the mipmaps, and loading the read texture into
 *  the next available texture slot in memory.
 ***********************************************************/
bool SceneManager::CreateGLTexture(const char* filename, std::string tag)
{
	int width = 0;
	int height = 0;
	int colorChannels = 0;
	GLuint textureID = 0;

	// indicate to always flip images vertically when loaded
	stbi_set_flip_vertically_on_load(true);

	std::cout << "Attempting to load texture: " << filename << std::endl;

	// try to parse the image data from the specified image file
	unsigned char* image = stbi_load(
		filename,
		&width,
		&height,
		&colorChannels,
		0);

	// if the image was successfully read from the image file
	if (image)
	{
		std::cout << "Successfully loaded image: " << filename << ", width: " << width << ", height: " << height << ", channels: " << colorChannels << std::endl;

		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);

		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// if the loaded image is in RGB format
		if (colorChannels == 3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		// if the loaded image is in RGBA format - it supports transparency
		else if (colorChannels == 4)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		else
		{
			std::cout << "Not implemented to handle image with " << colorChannels << " channels" << std::endl;
			return false;
		}

		// generate the texture mipmaps for mapping textures to lower resolutions
		glGenerateMipmap(GL_TEXTURE_2D);

		// free the image data from local memory
		stbi_image_free(image);
		glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

		// register the loaded texture and associate it with the special tag string
		m_textureIDs[m_loadedTextures].ID = textureID;
		m_textureIDs[m_loadedTextures].tag = tag;
		m_loadedTextures++;

		return true;
	}

	std::cerr << "Failed to load texture: " << filename << std::endl;
	std::cerr << "STB Error: " << stbi_failure_reason() << std::endl;

	return false;
}
/***********************************************************
 *  BindGLTextures()
 *
 *  This method is used for binding the loaded textures to
 *  OpenGL texture memory slots.  There are up to 16 slots.
 ***********************************************************/
void SceneManager::BindGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  DestroyGLTextures()
 *
 *  This method is used for freeing the memory in all the
 *  used texture memory slots.
 ***********************************************************/
void SceneManager::DestroyGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		glGenTextures(1, &m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  FindTextureID()
 *
 *  This method is used for getting an ID for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureID(std::string tag)
{
	int textureID = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureID = m_textureIDs[index].ID;
			bFound = true;
		}
		else
			index++;
	}

	return(textureID);
}

/***********************************************************
 *  FindTextureSlot()
 *
 *  This method is used for getting a slot index for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureSlot(std::string tag)
{
	int textureSlot = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureSlot = index;
			bFound = true;
		}
		else
			index++;
	}

	return(textureSlot);
}

/***********************************************************
 *  FindMaterial()
 *
 *  This method is used for getting a material from the previously
 *  defined materials list that is associated with the passed in tag.
 ***********************************************************/
bool SceneManager::FindMaterial(std::string tag, OBJECT_MATERIAL& material)
{
	if (m_objectMaterials.size() == 0)
	{
		return(false);
	}

	int index = 0;
	bool bFound = false;
	while ((index < m_objectMaterials.size()) && (bFound == false))
	{
		if (m_objectMaterials[index].tag.compare(tag) == 0)
		{
			bFound = true;
			material.diffuseColor = m_objectMaterials[index].diffuseColor;
			material.specularColor = m_objectMaterials[index].specularColor;
			material.shininess = m_objectMaterials[index].shininess;
		}
		else
		{
			index++;
		}
	}

	return(true);
}

/***********************************************************
 *  SetTransformations()
 *
 *  This method is used for setting the transform buffer
 *  using the passed in transformation values.
 ***********************************************************/
void SceneManager::SetTransformations(
	glm::vec3 scaleXYZ,
	float XrotationDegrees,
	float YrotationDegrees,
	float ZrotationDegrees,
	glm::vec3 positionXYZ)
{
	// variables for this method
	glm::mat4 modelView;
	glm::mat4 scale;
	glm::mat4 rotationX;
	glm::mat4 rotationY;
	glm::mat4 rotationZ;
	glm::mat4 translation;

	// set the scale value in the transform buffer
	scale = glm::scale(scaleXYZ);
	// set the rotation values in the transform buffer
	rotationX = glm::rotate(glm::radians(XrotationDegrees), glm::vec3(1.0f, 0.0f, 0.0f));
	rotationY = glm::rotate(glm::radians(YrotationDegrees), glm::vec3(0.0f, 1.0f, 0.0f));
	rotationZ = glm::rotate(glm::radians(ZrotationDegrees), glm::vec3(0.0f, 0.0f, 1.0f));
	// set the translation value in the transform buffer
	translation = glm::translate(positionXYZ);

	modelView = translation * rotationZ * rotationY * rotationX * scale;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setMat4Value(g_ModelName, modelView);
	}
}

/***********************************************************
 *  SetShaderColor()
 *
 *  This method is used for setting the passed in color
 *  into the shader for the next draw command
 ***********************************************************/
void SceneManager::SetShaderColor(
	float redColorValue,
	float greenColorValue,
	float blueColorValue,
	float alphaValue)
{
	// variables for this method
	glm::vec4 currentColor;

	currentColor.r = redColorValue;
	currentColor.g = greenColorValue;
	currentColor.b = blueColorValue;
	currentColor.a = alphaValue;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, false);
		m_pShaderManager->setVec4Value(g_ColorValueName, currentColor);
	}
}

/***********************************************************
 *  SetShaderTexture()
 *
 *  This method is used for setting the texture data
 *  associated with the passed in ID into the shader.
 ***********************************************************/
void SceneManager::SetShaderTexture(
	std::string textureTag)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, true);

		int textureID = -1;
		textureID = FindTextureSlot(textureTag);
		m_pShaderManager->setSampler2DValue(g_TextureValueName, textureID);
	}
}

/***********************************************************
 *  SetTextureUVScale()
 *
 *  This method is used for setting the texture UV scale
 *  values into the shader.
 ***********************************************************/
void SceneManager::SetTextureUVScale(float u, float v)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setVec2Value("UVscale", glm::vec2(u, v));
	}
}

/***********************************************************
 *  SetShaderMaterial()
 *
 *  This method is used for passing the material values
 *  into the shader.
 ***********************************************************/
void SceneManager::SetShaderMaterial(
	std::string materialTag)
{
	if (m_objectMaterials.size() > 0)
	{
		OBJECT_MATERIAL material;
		bool bReturn = false;

		bReturn = FindMaterial(materialTag, material);
		if (bReturn == true)
		{
			m_pShaderManager->setVec3Value("material.diffuseColor", material.diffuseColor);
			m_pShaderManager->setVec3Value("material.specularColor", material.specularColor);
			m_pShaderManager->setFloatValue("material.shininess", material.shininess);
		}
	}
}

/**************************************************************/
/*** STUDENTS CAN MODIFY the code in the methods BELOW for  ***/
/*** preparing and rendering their own 3D replicated scenes.***/
/*** Please refer to the code in the OpenGL sample project  ***/
/*** for assistance.                                        ***/
/**************************************************************/
void SceneManager::LoadSceneTexture()
{
	std::cout << "Loading textures..." << std::endl;
	bool bReturn = false;

	bReturn = CreateGLTexture(
		"textures/rusticwood.jpg",
		"table");

	bReturn = CreateGLTexture(
		"textures/drywall.jpg",
		"wall");

	bReturn = CreateGLTexture(
		"textures/ball.jpg",
		"ball");

	bReturn = CreateGLTexture(
		"textures/window.jpg",
		"window");

	
	BindGLTextures();
}

/***********************************************************
 *  SetupSceneLights()
 *
 *  This method is called to add and configure the light
 *  sources for the 3D scene.  There are up to 4 light sources.
 ***********************************************************/
void SceneManager::SetupSceneLights()
{
	// this line of code is NEEDED for telling the shaders to render 
	// the 3D scene with custom lighting - to use the default rendered 
	// lighting then comment out the following line
	m_pShaderManager->setBoolValue(g_UseLightingName, true);

	// directional light to emulate sunlight coming into scene
	m_pShaderManager->setVec3Value("directionalLight.direction", 0.0f, -1.0f, -0.1f);
	m_pShaderManager->setVec3Value("directionalLight.ambient", 0.8f, 0.8f, 0.6f);
	m_pShaderManager->setVec3Value("directionalLight.diffuse", 0.07f, 0.06f, 0.04f);
	m_pShaderManager->setVec3Value("directionalLight.specular", 1.0f, 0.9f, 0.6f);
	m_pShaderManager->setBoolValue("directionalLight.bActive", true);


	// point light 1 (index 0)
	m_pShaderManager->setVec3Value("pointLights[0].position", -4.0f, 8.0f, 0.0f);
	m_pShaderManager->setVec3Value("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
	m_pShaderManager->setVec3Value("pointLights[0].diffuse", 0.3f, 0.3f, 0.1f);
	m_pShaderManager->setVec3Value("pointLights[0].specular", 0.2f, 0.2f, 0.0f);
	m_pShaderManager->setBoolValue("pointLights[0].bActive", true);
		// point light 2 (index 1)
	m_pShaderManager->setVec3Value("pointLights[1].position", 4.0f, 8.0f, 0.0f);
	m_pShaderManager->setVec3Value("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
	m_pShaderManager->setVec3Value("pointLights[1].diffuse", 0.3f, 0.3f, 0.1f);
	m_pShaderManager->setVec3Value("pointLights[1].specular", 0.2f, 0.2f, 0.0f);
	m_pShaderManager->setBoolValue("pointLights[1].bActive", true);
	// point light 3 (index 2)
	m_pShaderManager->setVec3Value("pointLights[2].position", 3.8f, 5.5f, 4.0f);
	m_pShaderManager->setVec3Value("pointLights[2].ambient", 0.05f, 0.05f, 0.05f);
	m_pShaderManager->setVec3Value("pointLights[2].diffuse", 0.2f, 0.2f, 0.0f);
	m_pShaderManager->setVec3Value("pointLights[2].specular", 0.8f, 0.8f, 0.6f);
	m_pShaderManager->setBoolValue("pointLights[2].bActive", true);
	// point light 4 (index 3)
	m_pShaderManager->setVec3Value("pointLights[3].position", 3.8f, 3.5f, 4.0f);
	m_pShaderManager->setVec3Value("pointLights[3].ambient", 0.05f, 0.05f, 0.05f);
	m_pShaderManager->setVec3Value("pointLights[3].diffuse", 0.2f, 0.2f, 0.0f);
	m_pShaderManager->setVec3Value("pointLights[3].specular", 0.8f, 0.8f, 0.6f);
	m_pShaderManager->setBoolValue("pointLights[3].bActive", true);
	// point light 5 (index 4)
	m_pShaderManager->setVec3Value("pointLights[4].position", -3.2f, 6.0f, -4.0f);
	m_pShaderManager->setVec3Value("pointLights[4].ambient", 0.05f, 0.05f, 0.05f);
	m_pShaderManager->setVec3Value("pointLights[4].diffuse", 0.9f, 0.9f, 0.7f);
	m_pShaderManager->setVec3Value("pointLights[4].specular", 0.2f, 0.2f, 0.0f);
	m_pShaderManager->setBoolValue("pointLights[4].bActive", true);



}
/***********************************************************
 *  DefineObjectMaterials()
 *
 *  This method is used for configuring the various material
 *  settings for all of the objects within the 3D scene.
 ***********************************************************/
void SceneManager::DefineObjectMaterials()
{
	OBJECT_MATERIAL basketballMaterial;
	basketballMaterial.diffuseColor = glm::vec3(0.4f, 0.4f, 0.4f);
	basketballMaterial.specularColor = glm::vec3(0.7f, 0.7f, 0.6f);
	basketballMaterial.shininess = 52.0;
	basketballMaterial.tag = "ball";

	m_objectMaterials.push_back(basketballMaterial);

	OBJECT_MATERIAL woodMaterial;
	woodMaterial.diffuseColor = glm::vec3(0.2f, 0.2f, 0.3f);
	woodMaterial.specularColor = glm::vec3(0.0f, 0.0f, 0.0f);
	woodMaterial.shininess = 0.1;
	woodMaterial.tag = "wood";

	m_objectMaterials.push_back(woodMaterial);

	OBJECT_MATERIAL mugMaterial;
	mugMaterial.diffuseColor = glm::vec3(0.8f, 0.5f, 0.3f); // Earthy color
	mugMaterial.specularColor = glm::vec3(0.2f, 0.2f, 0.2f); 
	mugMaterial.shininess = 10.0; // Less shiny for a ceramic look
	mugMaterial.tag = "mug";

	m_objectMaterials.push_back(mugMaterial);

	OBJECT_MATERIAL metalMaterial;
	metalMaterial.diffuseColor = glm::vec3(0.7f, 0.7f, 0.7f); // Silver/metallic color
	metalMaterial.specularColor = glm::vec3(1.0f, 1.0f, 1.0f); // High specular for a shiny effect
	metalMaterial.shininess = 100.0; // Very shiny for a metallic finish
	metalMaterial.tag = "metal";

	m_objectMaterials.push_back(metalMaterial);
}
/***********************************************************
 *  PrepareScene()
 *
 *  This method is used for preparing the 3D scene by loading
 *  the shapes, textures in memory to support the 3D scene 
 *  rendering
 ***********************************************************/

void SceneManager::PrepareScene()
{
	// only one instance of a particular mesh needs to be
	// loaded in memory no matter how many times it is drawn
	// in the rendered 3D scene
	LoadSceneTexture();

	m_basicMeshes->LoadPlaneMesh();
	m_basicMeshes->LoadSphereMesh();
	m_basicMeshes->LoadCylinderMesh();
	m_basicMeshes->LoadTorusMesh();
	m_basicMeshes->LoadBoxMesh();

	DefineObjectMaterials();

	SetupSceneLights();
}

/***********************************************************
 *  RenderScene()
 *
 *  This method is used for rendering the 3D scene by 
 *  transforming and drawing the basic 3D shapes
 ***********************************************************/
void SceneManager::RenderScene()
{
	RenderTable();
	RenderBall();
	RenderWall();
	RenderWindow();
	RenderLaptop();
	RenderCoffeeMug();
}

void SceneManager::RenderTable()
{
	// declare the variables for the transformations
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;

	scaleXYZ = glm::vec3(40.0f, 6.0f, 20.0f);
	positionXYZ = glm::vec3(4.0f, -3.0f, 0.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	//SetShaderColor(0.58, 0.224, 0.102, 1.0);
	
	SetShaderTexture("table");
	SetTextureUVScale(1.0, 1.0);
	SetShaderMaterial("wood");

	m_basicMeshes->DrawBoxMesh();
}

void SceneManager::RenderLaptop()
{
	// declare the variables for the transformations
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;

	scaleXYZ = glm::vec3(10.0f, 2.0f, 5.0f);
	positionXYZ = glm::vec3(5.0f, 0.0f, 5.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	
	SetShaderColor(0.2f, 0.2f, 0.2f, 1);
	SetShaderMaterial("metal");
	m_basicMeshes->DrawBoxMesh();


	scaleXYZ = glm::vec3(8.0f, 0.2f, 2.5f);
	positionXYZ = glm::vec3(5.0f, 1.0f, 6.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);

	SetShaderColor(1.0f, 0.9f, 0.9f, 1);
	SetShaderMaterial("wood");
	m_basicMeshes->DrawBoxMesh();


	scaleXYZ = glm::vec3(10.0f, 1.0f, 10.0f);
	positionXYZ = glm::vec3(5, 5.0f, 2.5f);
	SetTransformations(scaleXYZ, 90.0f, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderColor(0.2f, 0.2f, 0.2f, 1.0);
	SetShaderMaterial("metal");
	m_basicMeshes->DrawBoxMesh();

	scaleXYZ = glm::vec3(8.0f, 0.10f, 6.0f);
	positionXYZ = glm::vec3(5, 6.0f, 3.0f);
	SetTransformations(scaleXYZ, 90.0f, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderColor(0, 0, 0, 1.0);
	m_basicMeshes->DrawBoxMesh();
}
void SceneManager::RenderWall()
{
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;

	scaleXYZ = glm::vec3(40.0f, 1.0f, 40.0f);
	positionXYZ = glm::vec3(4.0f, 15.0f, -8.0f);
	SetTransformations(scaleXYZ, 90.0f, YrotationDegrees, ZrotationDegrees, positionXYZ);
	//SetShaderColor(0.043, 0.369, 0.149, 1.0);
	SetShaderTexture("wall");
	m_basicMeshes->DrawBoxMesh();
}

void SceneManager::RenderWindow()
{
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;

	scaleXYZ = glm::vec3(30.0f, 1.0f, 30.0f);
	positionXYZ = glm::vec3(4.0f, 15.0f, -7.0f);
	SetTransformations(scaleXYZ, 90.0f, YrotationDegrees, ZrotationDegrees, positionXYZ);
	//SetShaderColor(0.043, 0.369, 0.149, 1.0);
	SetShaderTexture("window");
	m_basicMeshes->DrawBoxMesh();
}


void SceneManager::RenderCoffeeMug()
{
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;

	scaleXYZ = glm::vec3(2.0f, 5.0f, 2.0f);
	positionXYZ = glm::vec3(15.0f, 0.0f, 3.0f);
	SetTransformations(scaleXYZ, 0.0f, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderColor(0.43, 0.4, 0.49, 1.0);
	
	m_basicMeshes->DrawCylinderMesh();

	scaleXYZ = glm::vec3(1.5f, 1.5f, 3.0f);
	positionXYZ = glm::vec3(18.0f, 2.0f, 3.0f);
	SetTransformations(scaleXYZ, 0.0f, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderColor(0.43, 0.4, 0.49, 1.0);

	m_basicMeshes->DrawTorusMesh();
}

void SceneManager::RenderBall()
{
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;
	// Basketball
	//Sphere mimicking a basketball 3D shape
	scaleXYZ = glm::vec3(4, 4, 4);
	positionXYZ = glm::vec3(-7.0f, 4, 5.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	//SetShaderColor(1.00, 0.34, 0.00, 1.0); // Orange color matching the basketball
	SetShaderTexture("ball");
	SetShaderMaterial("ball");
	SetTextureUVScale(1.0, 1.0);
	m_basicMeshes->DrawSphereMesh();
	//Used a torus shape to represent the lines on a basketball. By rotating the torus at the x axis I've mimicked  baskball lines
	scaleXYZ = glm::vec3(3.4f, 3.4f, 0.5f);
	positionXYZ = glm::vec3(-7.0f, 4, 5.0f);
	XrotationDegrees = 90.0f;
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderColor(0.0f, 0.0f, 0.0f, 1.0f);  // Black color for the lines
	m_basicMeshes->DrawTorusMesh();

	scaleXYZ = glm::vec3(3.4, 3.4, 0.1);
	positionXYZ = glm::vec3(-7.0f, 4.0f, 5.0f);
	XrotationDegrees = 135.0f;
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderColor(0.0f, 0.0f, 0.0f, 1.0f);  // Black color for the line
	m_basicMeshes->DrawTorusMesh();

	scaleXYZ = glm::vec3(3.4, 3.4, 0.1);
	positionXYZ = glm::vec3(-7.0f, 4.0f, 5.0f);
	XrotationDegrees = 45.0f;
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderColor(0.0f, 0.0f, 0.0f, 1.0f);  // Black color for the line
	m_basicMeshes->DrawTorusMesh();

}


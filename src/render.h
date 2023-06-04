#ifndef HEADERFILE_RENDER
#define HEADERFILE_RENDER

#include "glad/glad.h"
#include "Char.h"
#include "linmath.h"
#include <vector>

enum class Projection {
	Orthographic,Perspective2
};
class DrawListItem {
public:
	Animation* anim;
	int animNumber;
	int animIndex;
	int element;
	vec2 pos;
	vec2 scale;
	vec3 angle;
	Flip flip;
	Sprite* spr;
	float focalLength;
	bool visible;
	Projection projection;
	DrawListItem(Animation* anim, int element);
};

class DrawList {
public:
	std::vector<DrawListItem> items;
	int selectedIndex = -1;
	int getSize();
};

class Render {
private:
	int g_shader_programme;
	int t_shader_programme;
	GLuint gridVbo;
	GLuint gridVao;
	GLuint textureVao;
	GLuint textureVbo;

	int g_ProjMtx;
	int t_ProjMtx;
	int g_color;
	int t_uv;
	int t_alpha;

	vec2 viewPort;
	vec2 cameraOffset;
	float cameraScale;
	int compileShader(const char* vertexShader, const char* fragmentShader);
	GLuint loadTexture(const int width, const int height, const GLenum type, const unsigned char* buffer);
	void decodeImage(unsigned char* &out,Sprite* spr);
public:
	DrawList drawList;
	void drawScene(const int width, const int height);
	void moveCamera(float x, float y);
	void scaleCamera(float y);
	void resetCameraPosition();
	void resetCameraZoom();
	void toggleXYAxis();
	bool drawAxis;
	Render(unsigned int w,unsigned int h);
};
#endif

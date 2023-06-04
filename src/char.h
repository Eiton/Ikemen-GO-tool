#ifndef HEADERFILE_CHAR
#define HEADERFILE_CHAR
#include <string>
#include <map>
#include <vector>
#include "utility.h"
#include "linmath.h"

enum class Flip {
	None, V, H, VH
};
enum class Trans {
	None, Add, Sub
};

class AnimationElement {
public:
	int spriteGroup;
	int spriteIndex;
	float offsetX;
	float offsetY;
	Flip flip;
	int time;
	Trans trans;
	unsigned short transSource;
	unsigned short transDest;
	float scaleX;
	float scaleY;
	float angleZ;
	AnimationElement();
};

class Animation {
public:
	int animNumber;
	std::vector<AnimationElement*> elements;
	Animation(int number);
	~Animation();
private:
	std::string path;
};

class Palette {
public:
	short groupNumber;
	short groupIndex;
	short tmp;
	Palette* linkPalette;

	int size;
	int offset;
	short color;
	char buffer[256*4];
};

class Sprite {
public:
	unsigned int width;
	unsigned int height;
	vec2 offset;
	unsigned char colorDepth;
	//bool link;
	unsigned int bufferSize;
	unsigned char* buffer;
	Sprite* linkSprite;
	Palette* palette;
	int textureId;
	unsigned char compressType;
	Sprite(unsigned int width, unsigned int height, short offsetX, short offsetY,unsigned char colorDepth, unsigned char ct);
	~Sprite();
};

class Sff {
public:
	Utility::ExecutionResult<int> loadSff(std::string path);
	Utility::ExecutionResult<int> reload();
	char ver0,ver1,ver2,ver3;
	int numSprites;
	int numPalettes;
	int onDemandDataSize;
	int onDemandDataSizeTotal;
	int onLoadDataSize;
	std::map<std::pair<short, short>, Sprite*> spriteIndexMap;
	std::map<std::pair<short, short>, Palette*> paletteIndexMap;
private:
	std::string path;
	void clearSff();
};

class Air {
public:
	//std::map<int, Animation*> animMap;
	std::vector<Animation*> animList;
	std::vector<char*> animNameList;
	Utility::ExecutionResult<int> loadAir(std::string path);
	Utility::ExecutionResult<int> reload();
private:
	std::string path;
	void clearAir();
};

class Char {
public:
	Utility::ExecutionResult<int> loadDef(std::string path);
	Utility::ExecutionResult<int> reload();
	Sff sff;
	Air air;
private:
	std::string path;
};

#endif
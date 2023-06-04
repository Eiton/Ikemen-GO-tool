#include <fstream>
#include <algorithm>
#include <vector>
#include <sstream>
#include "char.h"
#include "utility.h"
#include "lodepng.h"
#include <GLFW/glfw3.h>
#include "linmath.h"

void trim(std::string& s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
		return !std::isspace(ch);
	}));
	s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
		return !std::isspace(ch);
	}).base(), s.end());
}
void cleanLine(std::string &line) {
	trim(line);
	//remove comments
	if (line.find(";") >= 0) {
		line = line.substr(0, line.find(";"));
	}
	return;
}
std::vector<std::string> splitAirParameters(std::string line) {
	std::vector<std::string> ret;
	std::string tmp = "";
	for (auto c : line)
	{
		if (c == ',')
		{
			trim(tmp);
			ret.push_back(tmp);
			tmp = "";
		}
		else {
			tmp = tmp + c;
		}
	}
	trim(tmp);
	ret.push_back(tmp);
	return ret;
}

Utility::ExecutionResult<int> Char::loadDef(std::string path) {
	std::ifstream defFile(path);
	if (!defFile) {
		return Utility::ExecutionResult<int>(false,"Failed to open the def file",NULL);
	}
	defer (defFile.close());
	std::string s;
	char sffPath[256] = { '\0' };
	char airPath[256] = { '\0' };
	while (std::getline(defFile,s)) {
		cleanLine(s);
		if (sscanf(s.c_str(), "sprite = %256[^\n]", sffPath)) {
			continue;
		}
		else if (sscanf(s.c_str(), "anim = %256[^\n]", airPath)) {
			continue;
		}
	}
	if (sffPath[0] == '\0') {
		return Utility::ExecutionResult<int>(false, "Cannot locate the path of sff", NULL);
	}
	if (airPath[0] == '\0') {
		return Utility::ExecutionResult<int>(false, "Cannot locate the path of air", NULL);
	}

	Utility::ExecutionResult<int> r = sff.loadSff(path.substr(0, path.rfind("\\") + 1) + sffPath);
	if (!r) {
		return r;
	}
	r = air.loadAir(path.substr(0, path.rfind("\\") + 1) + airPath);
	if (!r) {
		return r;
	}
	return Utility::ExecutionResult<int>(true, "Success", NULL);
}
Utility::ExecutionResult<int> Char::reload() {
	Utility::ExecutionResult<int> r = sff.reload();
	if (!r) {
		return r;
	}
	r = air.reload();
	if (!r) {
		return r;
	}
	return Utility::ExecutionResult<int>(true, "Success", NULL);
}
void Air::clearAir() {

	for (int i = 0; i < animList.size(); i++) {
		delete animNameList[i];
		delete animList[i];
	}
	animNameList.clear();
	animList.clear();
}
Utility::ExecutionResult<int> Air::reload() {
	return loadAir(path);
}

Utility::ExecutionResult<int> Air::loadAir(std::string path) {
	this->path = path;
	clearAir();
	std::ifstream airFile(path);
	if (!airFile) {
		return Utility::ExecutionResult<int>(false, "Failed to open the air file:\n"+path, NULL);
	}
	defer(airFile.close());
	std::string s;
	int group = -1;
	int lineNumber = 0;
	auto parseError = [&lineNumber](std::string message) {
		std::stringstream ss;
		ss << "Failed to parse air file at line " << lineNumber << ": "<< message;
		return Utility::ExecutionResult<int>(false, ss.str(), NULL);
	};
	char name[256] = "";
	while (std::getline(airFile, s)) {
		lineNumber++;
		if (s.empty()) {
			continue;
		}
		bool comment = sscanf(s.c_str(), ";%s", &name);
		std::transform(s.begin(), s.end(), s.begin(), ::tolower);
		int param;
		if (sscanf(s.c_str(), "[begin action %i]", &param)) {
			std::string str = std::to_string(param) +  " " + name;
			char* c = new char[str.size() + 1];
			memcpy(c, str.c_str(), str.size() + 1);
			animNameList.push_back(c);
			animList.push_back(new Animation(param));
			continue;
		}
		if (!comment) {
			name[0] = '\0';
		}

		cleanLine(s);
		if (s.empty() || s._Starts_with("clsn") || s._Starts_with("interpolate") || s == "loopstart") {
			continue;
		}
		std::vector<std::string> params = splitAirParameters(s);
		if (params.size() >= 5) {
			Animation* a = animList[animList.size()-1];
			AnimationElement* e = new AnimationElement();
			a->elements.push_back(e);
			e->spriteGroup = stoi(params[0]);
			e->spriteIndex = stoi(params[1]);
			e->offsetX = stoi(params[2]);
			e->offsetY = stoi(params[3]);
			e->time = stoi(params[4]);
			if (params.size() < 6) {
				continue;
			}
			if (params[5] == "v") {
				e->flip = Flip::V;
			}
			else if (params[5] == "h") {
				e->flip = Flip::H;
			}
			else if (params[5] == "vh" || params[5] == "hv") {
				e->flip = Flip::VH;
			}
			else {
				e->flip = Flip::None;
			}
			if (params.size() < 7) {
				continue;
			}
			if (params[6][0] == 'a') {
				e->trans = Trans::Add;
				if (params[6].size() > 1) {
					if (params[6] == "a1") {
						e->transSource = 256;
						e->transDest = 128;
					}
					else if (sscanf(params[6].c_str(), "as%hu d %hu",&e->transSource,&e->transDest)) {
						if (e->transSource > 256) {
							return parseError("Invalid trans parameter, source should be in 0-256");
						}
						else if (e->transDest > 256) {
							return parseError("Invalid trans parameter, dest should be in 0-256");
						}
					}
					else {
						return parseError("Invalid trans parameter");
					}
				}
				else {
					e->transDest = 256;
					e->transSource = 256;
				}
			}
			else if (params[6][0] == 's') {
				e->trans = Trans::Sub;
			}
			else if (!params[6].empty()){
				return parseError("Invalid trans parameter");
			}
			if (params.size() < 8) {
				continue;
			}
			if (!params[7].empty()) {
				e->scaleX = stof(params[7]);
			}
			if (params.size() < 9) {
				continue;
			}
			if (!params[8].empty()) {
				e->scaleY = stof(params[8]);
			}
			if (params.size() < 10) {
				continue;
			}
			if (!params[9].empty()) {
				e->angleZ = stof(params[9]);
			}
		}
		else {
			return parseError("too few parameters");
		}
	}
	
	return Utility::ExecutionResult<int>(true, "Success", NULL);
}

void Sff::clearSff() {
	for (auto anim : spriteIndexMap) {
		delete anim.second;
	}
	spriteIndexMap.clear();
	for (auto palette : paletteIndexMap) {
		if (palette.second->linkPalette == nullptr) {
			delete palette.second;
		}
		
	}
	paletteIndexMap.clear();
}
Utility::ExecutionResult<int> Sff::reload() {
	return loadSff(path);
}
Utility::ExecutionResult<int> Sff::loadSff(std::string path) {
	this->path = path;
	clearSff();

	std::ifstream sffFile(path,std::ios::binary);
	if (!sffFile) {
		return Utility::ExecutionResult<int>(false, "Failed to open the sff file:\n" + path, NULL);
	}
	auto readError = [](std::string message) {
		std::stringstream ss;
		ss << "Failed to read the sff file: " << message;
		return Utility::ExecutionResult<int>(false, ss.str(), NULL);
	};
	defer(sffFile.close());
	{
		char signature[12];
		sffFile.read(signature,12);
		if (strcmp(signature,"ElecbyteSpr") != 0) {
			return readError("unrecognized file signature");
		}
	}
	sffFile.read(&ver3, 1);
	sffFile.read(&ver2, 1);
	sffFile.read(&ver1, 1);
	sffFile.read(&ver0, 1);
	int dummy;
	unsigned int spriteHeaderOffset, paletteHeaderOffset, dataOffset1, dataOffset2;
	switch (ver0) {
	case 1:
		return readError("sff 1.0 is not supported");
	case 2:
		sffFile.seekg(0x14, std::ios_base::cur);
		sffFile.read((char*)(&spriteHeaderOffset), 4);
		sffFile.read((char*)(&numSprites), 4);
		sffFile.read((char*)(&paletteHeaderOffset), 4);
		sffFile.read((char*)(&numPalettes), 4);
		sffFile.read((char*)(&dataOffset1), 4);
		sffFile.seekg(4, std::ios_base::cur);
		sffFile.read((char*)(&dataOffset2), 4);
		break;
	default:
		return readError("unrecognized sff version");
	}
	std::vector<Palette*> palettes;
	palettes.reserve(numPalettes);
	for (int i = 0; i < numPalettes; i++) {
		sffFile.seekg(paletteHeaderOffset+i*16);
		unsigned short groupNumber, groupIndex,color, link;
		unsigned int offset, size;
		sffFile.read((char*)(&groupNumber), 2);
		sffFile.read((char*)(&groupIndex), 2);
		sffFile.read((char*)(&color), 2);
		sffFile.read((char*)(&link), 2);
		sffFile.read((char*)(&offset), 4);
		sffFile.read((char*)(&size), 4);

		if (paletteIndexMap.find(std::pair<short, short>{groupNumber, groupIndex}) != paletteIndexMap.end()) {
			return readError("duplicated palette");
		}
		if (size == 0) {
			Palette* p = new Palette();
			p->color = color;
			p->size = size;
			p->linkPalette = palettes[link];
			palettes.push_back(p);
			paletteIndexMap.insert(std::pair<std::pair<short, short>, Palette*>{std::pair<short, short>{groupNumber, groupIndex}, p});
		}
		else {
			Palette* p = new Palette();
			p->color = color;
			p->size = size;
			p->linkPalette = nullptr;
			sffFile.seekg(dataOffset1+offset);
			sffFile.read(p->buffer, 256*4);
			paletteIndexMap.insert(std::pair<std::pair<short, short>, Palette*>{std::pair<short, short>{groupNumber, groupIndex}, p});
			palettes.push_back(p);
		}
	}
	std::vector<Sprite*> sprites;
	sprites.reserve(numSprites);
	for (int i = 0; i < numSprites; i++) {
		sffFile.seekg(spriteHeaderOffset+i*28);
		short offsetX, offsetY;
		unsigned short groupNumber, groupIndex, width, height, link, palIndex, dataType;
		unsigned int dataOffset, dataSize;
		unsigned char compressionType, colorDepth;
		switch (ver0) {
		case 2:
			sffFile.read((char*)(&groupNumber), 2);
			sffFile.read((char*)(&groupIndex), 2);
			sffFile.read((char*)(&width), 2);
			sffFile.read((char*)(&height), 2);
			sffFile.read((char*)(&offsetX), 2);
			sffFile.read((char*)(&offsetY), 2);
			sffFile.read((char*)(&link), 2);
			sffFile.read((char*)(&compressionType), 1);
			sffFile.read((char*)(&colorDepth), 1);
			sffFile.read((char*)(&dataOffset), 4);
			sffFile.read((char*)(&dataSize), 4);
			sffFile.read((char*)(&palIndex), 2);
			sffFile.read((char*)(&dataType), 2);
			break;
		}
		Sprite* spr = new Sprite(width,height, offsetX, offsetY,colorDepth,compressionType);
		sprites.push_back(spr);
		spriteIndexMap.insert(std::pair<std::pair<short, short>, Sprite*>{std::pair<short, short>{groupNumber, groupIndex}, spr});
		if (dataSize == 0) {
			spr->linkSprite = sprites[link];
		}
		else {
			if (palIndex != 65535) {
				spr->palette = palettes[palIndex];

				if (spr->palette->linkPalette != nullptr) {
					spr->palette = spr->palette->linkPalette;
				}
			}
			
			switch (ver0) {
			case 2:
				if ((dataType & 1) == 0) {
					sffFile.seekg(dataOffset1+dataOffset);
				}
				else {
					sffFile.seekg(dataOffset2 + dataOffset);
				}
				if (compressionType == 0) {
					spr->buffer = new unsigned char[dataSize];
					spr->bufferSize = dataSize;
					sffFile.read((char*)spr->buffer, dataSize);
				}
				else if(compressionType >= 2 && compressionType <= 4) {
					sffFile.seekg(4, std::ios_base::cur);
					spr->buffer = new unsigned char[dataSize-4];
					spr->bufferSize = dataSize - 4;
					sffFile.read((char*)spr->buffer, dataSize - 4);
					
				} else if(compressionType == 10) {
					sffFile.seekg(4, std::ios_base::cur);
					spr->buffer = new unsigned char[dataSize - 4];
					spr->bufferSize = dataSize - 4;
					sffFile.read((char*)spr->buffer, dataSize-4);
					
				}
				else {
					sffFile.seekg(4, std::ios_base::cur);
					spr->buffer = new unsigned char[dataSize - 4];
					spr->bufferSize = dataSize - 4;
					sffFile.read((char*)spr->buffer, dataSize - 4);
					}
				break;
			}
		}

	}
	return Utility::ExecutionResult<int>(true, "Success", NULL);
}

AnimationElement::AnimationElement() {
	this->spriteGroup = -1;
	this->spriteIndex = 0;
	this->offsetX = 0;
	this->offsetY = 0;
	this->time = 1;
	this->scaleX = 1;
	this->scaleY = 1;
	this->angleZ = 0;
	this->flip = Flip::None;
	this->trans = Trans::None;
	this->transSource = 0;
	this->transDest = 0;
}
Animation::Animation(int number) : animNumber(number) {}
Animation::~Animation() {
	for (auto i = elements.begin(); i != elements.end(); i++) {
		delete *i;
	}
	elements.clear();
}
Sprite::Sprite(unsigned int w, unsigned int h, short oX, short oY, unsigned char c, unsigned char ct) : width(w), height(h), offset{(float)oX,(float)oY},colorDepth(c),compressType(ct), textureId(-1), linkSprite(nullptr), buffer(nullptr),  bufferSize(0), palette(nullptr) {}
Sprite::~Sprite() {
	if (linkSprite == nullptr) {
		delete[]buffer;
		if (textureId != -1) {
			glDeleteTextures(1, (GLuint*)&textureId);
		}
	}
}
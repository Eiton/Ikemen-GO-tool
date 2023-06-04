#include "render.h"
#include "glad/glad.h"
#include "linmath.h"
#include "lodepng.h"
#include "utility.h"
#include <GLFW/glfw3.h>

int Render::compileShader(const char* vertexShader, const char* fragmentShader) {
	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vertexShader, nullptr);
	glCompileShader(vs);
	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &fragmentShader, nullptr);
	glCompileShader(fs);
	int shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, fs);
	glAttachShader(shaderProgram, vs);
	glLinkProgram(shaderProgram);
	return shaderProgram;
}
GLuint Render::loadTexture(const int width, const int height, const GLenum type, const unsigned char* buffer) {
	GLuint textureId;
	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_2D, textureId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, type, width, height, 0, type, GL_UNSIGNED_BYTE, buffer);

	return textureId;
}

Render::Render(unsigned int w, unsigned int h) : viewPort{(float)w,(float)h},cameraScale(1), cameraOffset{ w * 0.5f,h * 0.5f }, drawAxis(true){
	const char* g_vertex_shader =
		"#version 400\n"
		"in vec2 Position;"
		"uniform mat4 ProjMtx;"
		"void main() {"
		"  gl_Position = ProjMtx * vec4(Position, 0.0, 1.0);"
		"}";
	const char* g_fragment_shader =
		"#version 400\n"
		"uniform vec4 InColor;"
		"out vec4 frag_colour;"
		"void main() {"
		"  frag_colour = InColor;"
		"}";

	const char* t_vertex_shader =
		"#version 400\n"
		"in vec2 Position;"
		"attribute vec2 UV;"
		"uniform mat4 ProjMtx;"
		"varying vec2 Frag_UV;"
		"void main() {"
		"  gl_Position = ProjMtx * vec4(Position, 0.0, 1.0);"
		"  Frag_UV = UV;"
		"}";
	const char* t_fragment_shader =
		"#version 400\n"
		"uniform sampler2D Texture;"
		"varying vec2 Frag_UV;"
		"uniform float Alpha;"
		"out vec4 frag_colour;"
		"void main() {"
		"  frag_colour = texture2D(Texture, Frag_UV);"
		"  frag_colour.a *= Alpha;"
		"}";
	g_shader_programme = compileShader(g_vertex_shader, g_fragment_shader);
	g_ProjMtx = glGetUniformLocation(g_shader_programme, "ProjMtx");
	g_color = glGetUniformLocation(g_shader_programme, "InColor");


	t_shader_programme = compileShader(t_vertex_shader, t_fragment_shader);
	t_ProjMtx = glGetUniformLocation(t_shader_programme, "ProjMtx");
	t_uv = glGetAttribLocation(t_shader_programme, "UV");
	t_alpha = glGetUniformLocation(t_shader_programme, "Alpha");

	
	float axisLines[]
	{
		-10000, 0, 10000,0,
		0, -10000, 0,10000
	};
	
	glGenBuffers(1, &gridVbo);
	glBindBuffer(GL_ARRAY_BUFFER, gridVbo);
	glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float), axisLines, GL_STATIC_DRAW);


	glGenVertexArrays(1, &gridVao);
	glBindVertexArray(gridVao);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, gridVbo);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

	const float rect[] = { 0.0f, 0.0f, 0.0f, 0.0f,
							 0.0f,  1.0f, 0.0f, 1.0f,
							  1.0f, 0.0f, 1.0f, 0.0f,
							  1.0f,  1.0f, 1.0f, 1.0f };
	glGenBuffers(1, &textureVbo);
	glBindBuffer(GL_ARRAY_BUFFER, textureVbo);
	glBufferData(GL_ARRAY_BUFFER, 16 * sizeof(float), rect, GL_STATIC_DRAW);

	glGenVertexArrays(1, &textureVao);
	glBindVertexArray(textureVao);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(t_uv);
	glBindBuffer(GL_ARRAY_BUFFER, textureVbo);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	glVertexAttribPointer(t_uv, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(sizeof(float) * 2));


}

	
void Render::drawScene(const int width,const int height) {
	glClearColor(setting.clearColor[0], setting.clearColor[1], setting.clearColor[2], 1.0f);
	if (viewPort[0] != width || viewPort[1] != height) {
		cameraOffset[0] += (width - viewPort[0]) * 0.5;
		cameraOffset[1] += (height - viewPort[1]) * 0.5;
		viewPort[0] = width;
		viewPort[1] = height;
	}
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(g_shader_programme);
	glDisable(GL_DEPTH_TEST);
	
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	mat4x4 mvp,tMat, tmp;

	auto mulMatrix = [&tmp](mat4x4 a, mat4x4 b) {
		mat4x4_mul(tmp,a,b);
		memcpy(a, tmp, sizeof(tmp));
	};
	auto scale = [&mvp, &tMat, &tmp](float x, float y, float z = 1) {
		mat4x4_scale_aniso(tmp, mvp, x, y, z);
		memcpy(mvp, tmp, sizeof(mvp));
	};
	auto rotate = [&mvp, &tMat, &tmp](float angle, float x, float y, float z) {
		mat4x4_rotate(tmp, mvp, x, y, z, angle*PI/180);
		memcpy(mvp, tmp, sizeof(mvp));
	};

	float fLength = 1024;

	if (drawAxis) {
		glBindVertexArray(gridVao);
		mat4x4_ortho(mvp, -width / 2, width / 2, height / 2, -height / 2, -65535, 65535);
		scale(cameraScale, cameraScale);
		mat4x4_translate_in_place(mvp, -width / 2, -height / 2, 0);
		mat4x4_translate_in_place(mvp, cameraOffset[0], cameraOffset[1], 0);
		glUniformMatrix4fv(g_ProjMtx, 1, GL_FALSE, (float*)&mvp);
		GLfloat c[4] = { setting.axisColor[0],setting.axisColor[1],setting.axisColor[2],1.0f };
		glUniform4fv(g_color, 1, &c[0]);
		glDrawArrays(GL_LINES, 0, 4);
	}
	
	if (drawList.getSize() > 0) {
		glUseProgram(t_shader_programme);
		glEnable(GL_BLEND);
		glActiveTexture(GL_TEXTURE0);
		glBindVertexArray(textureVao);
		for (auto drawItem : drawList.items) {
			if (drawItem.spr == nullptr || drawItem.visible == false) {
				continue;
			}
			Sprite* spr = drawItem.spr->linkSprite == nullptr ? drawItem.spr : drawItem.spr->linkSprite;
			AnimationElement* elm = drawItem.anim->elements[drawItem.element];
			if (spr->textureId == -1) {
				unsigned char* tex;
				decodeImage(tex, spr);
				spr->textureId = loadTexture(spr->width, spr->height, GL_RGBA, (unsigned char*)tex);
				delete[]tex;
			}
			glBindTexture(GL_TEXTURE_2D, spr->textureId);
			if (drawItem.projection == Projection::Orthographic) {
				mat4x4_ortho(mvp, -width / 2, width / 2, height / 2, -height / 2, -65535, 65535);
				scale(cameraScale, cameraScale);
				mat4x4_translate_in_place(mvp, drawItem.pos[0] + cameraOffset[0] - width / 2, drawItem.pos[1] + cameraOffset[1] - height / 2, 0);				
				scale(((drawItem.flip == Flip::H || drawItem.flip == Flip::VH) ? -1 : 1), ((drawItem.flip == Flip::V || drawItem.flip == Flip::VH) ? -1 : 1), 1);
				mat4x4_translate_in_place(mvp, elm->offsetX * drawItem.scale[0], elm->offsetY * drawItem.scale[1], 0);

				rotate(drawItem.angle[0], 1, 0, 0);
				rotate(drawItem.angle[1], 0, 1, 0);
				rotate(-(elm->angleZ + drawItem.angle[2]), 0, 0, 1);
				scale(elm->scaleX * drawItem.scale[0], elm->scaleY * drawItem.scale[1], 1);

				scale((elm->flip == Flip::H || elm->flip == Flip::VH) ? -1 : 1, (elm->flip == Flip::V || elm->flip == Flip::VH) ? -1 : 1);
				
				mat4x4_translate_in_place(mvp, -drawItem.spr->offset[0], -drawItem.spr->offset[1], 0);

				scale(spr->width, spr->height);
			}
			else {
				mat4x4_ortho(mvp, -width / 2, width / 2, height / 2, -height / 2, -65535, 65535);
				mat4x4_invert(tMat, mvp);
				scale(cameraScale, cameraScale);
				mat4x4_translate_in_place(mvp, drawItem.pos[0] + cameraOffset[0] - width / 2, drawItem.pos[1] + cameraOffset[1] - height / 2, 0);

				scale(((drawItem.flip == Flip::H || drawItem.flip == Flip::VH) ? -1 : 1), ((drawItem.flip == Flip::V || drawItem.flip == Flip::VH) ? -1 : 1), 1);

				mat4x4_translate_in_place(mvp, elm->offsetX * drawItem.scale[0], elm->offsetY * drawItem.scale[1] + height / 2, 0);

				mulMatrix(mvp, tMat);

				mat4x4_frustum(tMat, -width / 2 / drawItem.focalLength, width / 2 / drawItem.focalLength, height / drawItem.focalLength, 0, 1, 65535);
				mulMatrix(mvp, tMat);

				mat4x4_translate_in_place(mvp, 0, 0, -drawItem.focalLength);


				rotate(drawItem.angle[0], 1, 0, 0);
				rotate(drawItem.angle[1], 0, 1, 0);
				rotate(-(elm->angleZ + drawItem.angle[2]), 0, 0, 1);
				scale(elm->scaleX * drawItem.scale[0], elm->scaleY * drawItem.scale[1], 1);

				scale((elm->flip == Flip::H || elm->flip == Flip::VH) ? -1 : 1, (elm->flip == Flip::V || elm->flip == Flip::VH) ? -1 : 1);

				mat4x4_translate_in_place(mvp, -drawItem.spr->offset[0], -drawItem.spr->offset[1], 0);

				scale(spr->width, spr->height);
			}
			
			glUniformMatrix4fv(t_ProjMtx, 1, GL_FALSE, (float*)&mvp);
			
			float a = 1;

			if (elm->trans == Trans::None) {
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glUniform1fv(t_alpha, 1, &a);
				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			}
			else if (elm->trans == Trans::Add) {
				if (abs(elm->transSource + elm->transDest - 256) <= 1) {
					glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
					a = elm->transSource/256.0f;
					glUniform1fv(t_alpha, 1, &a);
					glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
				}else if (elm->transDest >= 255) {
					a = elm->transSource/256.0f;
					glBlendFunc(GL_SRC_ALPHA, GL_ONE);
					glUniform1fv(t_alpha, 1, &a);
					glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
				}
				else {
					a = (255-elm->transDest) / 256.0f;
					glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);
					glUniform1fv(t_alpha, 1, &a);
					glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

					a = (elm->transSource) / 256.0f;
					glBlendFunc(GL_SRC_ALPHA, GL_ONE);
					glUniform1fv(t_alpha, 1, &a);
					glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
				}
			}
			else if (elm->trans == Trans::Sub) {
				glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
				glUniform1fv(t_alpha, 1, &a);
				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			}



		}
	}
}
void Render::decodeImage(unsigned char* &out,Sprite* spr) {
	out = new unsigned char[spr->width * spr->height * 4];

	auto indexSpr2RGBA = [&out, spr](unsigned char* buffer) {
		for (int i = 0; i < spr->width * spr->height; i++) {
			unsigned char c = spr->palette->buffer[(buffer[i]) * 4];
			out[i * 4] = c;
			out[i * 4 + 1] = spr->palette->buffer[(buffer[i]) * 4 + 1];
			out[i * 4 + 2] = spr->palette->buffer[(buffer[i]) * 4 + 2];
			out[i * 4 + 3] = spr->palette->buffer[(buffer[i]) * 4 + 3];
		}
	};
	switch (spr->compressType) {
	case 0:
	{
		if (spr->palette == nullptr) {
			memcpy(out, spr->buffer, spr->width * spr->height * 4);
		}
		else {
			indexSpr2RGBA(spr->buffer);
		}
	}
	break;
	case 2:
	{
		//rle8 decode
		unsigned char* decompressedBuffer = new unsigned char[spr->width * spr->height];
		int i = 0, j = 0;
		while (j < spr->width * spr->height) {
			unsigned char d = spr->buffer[i++];
			if ((d & 0xc0) == 0x40) {
				for (int k = 0; k < (d & 0x3f); k++) {
					decompressedBuffer[j++] = spr->buffer[i];
				}
				i++;
			}
			else {
				decompressedBuffer[j++] = d;
			}
		}
		//spr->buffer = img;
		//delete[]buffer;
		indexSpr2RGBA(decompressedBuffer);
		delete[]decompressedBuffer;
	}
	break;
	case 3:
	{
		//rle5 decode
		unsigned char* decompressedBuffer = new unsigned char[spr->width * spr->height];
		int i = 0, j = 0;
		while (j < spr->width * spr->height) {
			char runLength = spr->buffer[i++];
			char dataLength = spr->buffer[i] & 0x7f;
			char color = 0;
			if ((spr->buffer[i++] & 0x128) > 0) {
				color = spr->buffer[i++];
			}
			for (int k = 0; k < runLength; k++) {
				decompressedBuffer[j++] = color;
			}
			for (int k = 0; k < dataLength; k++) {
				runLength = spr->buffer[i] >> 5;
				color = spr->buffer[i] & 0x1f;
				for (int l = 0; l < runLength; l++) {
					decompressedBuffer[j++] = color;
				}
			}
		}
		indexSpr2RGBA(decompressedBuffer);
		delete[]decompressedBuffer;
	}
	break;
	case 4:
	{
		//lz5 decode
		unsigned char* decompressedBuffer = new unsigned char[spr->width * spr->height];
		int i = 0, j = 0;
		unsigned char controlPacket = spr->buffer[i++], bit = 0, recycleBits = 0, recycleBitsCount = 0;
		unsigned char color = 0;
		unsigned int offset = 0;
		unsigned int copyLength = 0;
		while (j < spr->width * spr->height) {
			unsigned char d = (unsigned int)(spr->buffer[i++]);
			//LZ packet
			if ((controlPacket & (1 << bit)) != 0) {
				//long LZ packet
				if ((d & 0x3f) == 0) {
					//d = ((d << 2) | (unsigned int)(spr->buffer[i++])) + 1;
					offset = ((d << 2) | (spr->buffer[i++]));
					offset++;
					copyLength = spr->buffer[i++];
					copyLength += 3;

					//n = (unsigned int)(spr->buffer[i++]) + 2;
				}
				//short LZ packet
				else {
					copyLength = (unsigned int)(d & 0x3f);
					copyLength++;
					recycleBits = (recycleBits | ((d & 0xc0) >> recycleBitsCount));
					recycleBitsCount += 2;
					if (recycleBitsCount < 8) {
						offset = spr->buffer[i++];
						offset++;
					}
					else {
						offset = recycleBits;
						offset++;
						recycleBits = 0;
						recycleBitsCount = 0;
					}
				}
				while (copyLength > 0) {
					if (j < spr->width * spr->height) {
						decompressedBuffer[j] = decompressedBuffer[j - offset];
						j++;
					}
					copyLength--;
				}

			}
			//RLE packect
			else {
				//long RLE packet
				if ((d & 0xe0) == 0) {
					copyLength = spr->buffer[i++];
					copyLength += 8;
				}
				//short RLE packet
				else {
					copyLength = d >> 5;
				}
				color = d & 0x1f;
				while (copyLength > 0) {
					if (j < spr->width * spr->height) {
						decompressedBuffer[j++] = color;
					}
					copyLength--;
				}
			}
			bit++;
			if (bit >= 8) {
				controlPacket = spr->buffer[i++];
				bit = 0;
			}
		}
		indexSpr2RGBA(decompressedBuffer);
		delete[]decompressedBuffer;
	}
	break;
	case 10:
	{
		unsigned char* decompressedBuffer;
		unsigned int error = lodepng_decode_memory(&decompressedBuffer, &spr->width, &spr->height, spr->buffer, spr->bufferSize, LCT_PALETTE, spr->colorDepth);
		if (error) {
			std::string m = lodepng_error_text(error);
		}
		indexSpr2RGBA(decompressedBuffer);
		delete[]decompressedBuffer;
	}
	break;

	default:
	{
		delete[]out;
		//unsigned int error = lodepng_decode_memory(&out, &spr->width, &spr->height, spr->buffer, spr->bufferSize, LCT_RGBA, spr->colorDepth);
		unsigned int error = lodepng_decode_memory(&out, &spr->width, &spr->height, spr->buffer, spr->bufferSize, LCT_RGBA, 8);
		if (error) {
			std::string m = lodepng_error_text(error);
		}
	}
	break;
		
	}
}
void Render::moveCamera(float x, float y) {
	cameraOffset[0] += x / cameraScale;
	cameraOffset[1] += y / cameraScale;
}
void Render::scaleCamera(float y) {
	if (y > 0) {
		cameraScale += 0.2f;
	}
	else if(cameraScale-0.2f > 0.001){
		cameraScale -= 0.2f;
	}
}
void Render::resetCameraPosition() {
	cameraOffset[0] = viewPort[0] * 0.5f;
	cameraOffset[1] = viewPort[1] * 0.5f;
}
void Render::resetCameraZoom() {
	cameraScale = 1;
}
void Render::toggleXYAxis() {
	drawAxis = !drawAxis;
}
int DrawList::getSize() {
	return items.size();
}

DrawListItem::DrawListItem(Animation* anim, int element) : anim(anim), animNumber(anim != nullptr?anim->animNumber:-1), element(element), pos{ 0,0 }, scale{ 1,1 }, angle{ 0,0,0 }, flip(Flip::None), focalLength(1024), spr(nullptr), animIndex(0), visible(true), projection(Projection::Orthographic) {}
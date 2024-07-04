#include "../Libraries/texture.h"

texture::texture(const char* path, GLint wrapmode, GLint filter, bool srgb)
{
	glGenTextures(1, &ID);
	glBindTexture(GL_TEXTURE_2D, ID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapmode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapmode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);

	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load(path, &width, &height, &channels, 0);
	if (0 == channels || channels > 4) { throw std::runtime_error("textura com canais invalidos\n"); }

	if (data) {
		if(srgb){
            glTexImage2D(GL_TEXTURE_2D, 0, sch[channels-1], width, height, 0, ch[channels - 1], GL_UNSIGNED_BYTE, data);
        }
        else{
            glTexImage2D(GL_TEXTURE_2D, 0, ch[channels-1], width, height, 0, ch[channels - 1], GL_UNSIGNED_BYTE, data);
        }
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		std::cout << "Texture fail!" << std::endl;
	}

	stbi_image_free(data);
}

void texture::bind(int location)
{
	switch (location) {
	case 0:
		glActiveTexture(GL_TEXTURE0); break;
	case 1:
		glActiveTexture(GL_TEXTURE1); break;
	case 2:
		glActiveTexture(GL_TEXTURE2); break;
	case 3:
		glActiveTexture(GL_TEXTURE3); break;
	case 4:
		glActiveTexture(GL_TEXTURE4); break;
	case 5:
		glActiveTexture(GL_TEXTURE5); break;
	case 6:
		glActiveTexture(GL_TEXTURE6); break;
	case 7:
		glActiveTexture(GL_TEXTURE7); break;
	case 8:
		glActiveTexture(GL_TEXTURE8); break;
	case 9:
		glActiveTexture(GL_TEXTURE9); break;
	case 10:
		glActiveTexture(GL_TEXTURE10); break;
	case 11:
		glActiveTexture(GL_TEXTURE11); break;
	case 12:
		glActiveTexture(GL_TEXTURE12); break;
	case 13:
		glActiveTexture(GL_TEXTURE13); break;
	case 14:
		glActiveTexture(GL_TEXTURE14); break;
	case 15:
		glActiveTexture(GL_TEXTURE15);
	}
	glBindTexture(GL_TEXTURE_2D, ID);
}

void texture::erase()
{
	glDeleteTextures(1, &ID);
}

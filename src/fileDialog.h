#ifndef HEADERFILE_FILEDIALOG
#define HEADERFILE_FILEDIALOG
#include <string>
namespace FileDialog {
	bool showFileDialog(std::string fileType);
	extern std::string filePath;
	extern enum fileType;
};
#endif
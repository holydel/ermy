#include <editor_file_utils.h>

bool copyFile(const std::filesystem::path& sourcePath, std::ofstream& destinationStream) {
    std::ifstream sourceFile(sourcePath, std::ios::binary);
    if (!sourceFile) {
        return false;
    }

    if (!destinationStream) {
        sourceFile.close();
        return false;
    }

    char buffer[4096];
    while (sourceFile.read(buffer, sizeof(buffer))) {
        destinationStream.write(buffer, sourceFile.gcount());
    }

    if (sourceFile.eof() && sourceFile.gcount() > 0) {
        destinationStream.write(buffer, sourceFile.gcount());
    }

    sourceFile.close();

    return true;
}
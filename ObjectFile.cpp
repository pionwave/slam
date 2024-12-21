
#include "ObjectFile.hpp"
#include <fstream>
#include <stdexcept>

void WriteString(std::ofstream& ofs, const std::string& str) {
    size_t length = str.size();
    ofs.write(reinterpret_cast<const char*>(&length), sizeof(length));
    ofs.write(str.c_str(), length);
    if (!ofs) {
        throw std::runtime_error("Failed to write string to file.");
    }
}

std::string ReadString(std::ifstream& ifs) {
    size_t length;
    ifs.read(reinterpret_cast<char*>(&length), sizeof(length));
    if (!ifs) {
        throw std::runtime_error("Failed to read string length from file.");
    }
    std::string str(length, '\0');
    ifs.read(&str[0], length);
    if (!ifs) {
        throw std::runtime_error("Failed to read string data from file.");
    }
    return str;
}

void WriteObjectFile(const ObjectFile& obj, const std::string& path) {
    std::ofstream ofs(path, std::ios::binary);
    if (!ofs) {
        throw std::runtime_error("Failed to open file for writing: " + path);
    }

    ofs.write(reinterpret_cast<const char*>(&obj.codeSize), sizeof(obj.codeSize));
    if (!ofs) {
        throw std::runtime_error("Failed to write codeSize to file.");
    }

    size_t codeSegmentSize = obj.codeSegment.size();
    ofs.write(reinterpret_cast<const char*>(&codeSegmentSize), sizeof(codeSegmentSize));
    if (!ofs) {
        throw std::runtime_error("Failed to write codeSegment size to file.");
    }
    if (codeSegmentSize > 0) {
        ofs.write(reinterpret_cast<const char*>(obj.codeSegment.data()), codeSegmentSize);
        if (!ofs) {
            throw std::runtime_error("Failed to write codeSegment data to file.");
        }
    }

    size_t dataSegmentSize = obj.dataSegment.size();
    ofs.write(reinterpret_cast<const char*>(&dataSegmentSize), sizeof(dataSegmentSize));
    if (!ofs) {
        throw std::runtime_error("Failed to write dataSegment size to file.");
    }
    if (dataSegmentSize > 0) {
        ofs.write(reinterpret_cast<const char*>(obj.dataSegment.data()), dataSegmentSize);
        if (!ofs) {
            throw std::runtime_error("Failed to write dataSegment data to file.");
        }
    }

    size_t symbolTableSize = obj.symbolTable.size();
    ofs.write(reinterpret_cast<const char*>(&symbolTableSize), sizeof(symbolTableSize));
    if (!ofs) {
        throw std::runtime_error("Failed to write symbolTable size to file.");
    }
    for (const auto& symbol : obj.symbolTable) {
        WriteString(ofs, symbol.name);
        ofs.write(reinterpret_cast<const char*>(&symbol.address), sizeof(symbol.address));
        ofs.write(reinterpret_cast<const char*>(&symbol.isExternal), sizeof(symbol.isExternal));
        ofs.write(reinterpret_cast<const char*>(&symbol.isData), sizeof(symbol.isData));
        if (!ofs) {
            throw std::runtime_error("Failed to write a Symbol to file.");
        }
    }

    size_t fixupsSize = obj.fixups.size();
    ofs.write(reinterpret_cast<const char*>(&fixupsSize), sizeof(fixupsSize));
    if (!ofs) {
        throw std::runtime_error("Failed to write fixups size to file.");
    }
    for (const auto& fixup : obj.fixups) {
        ofs.write(reinterpret_cast<const char*>(&fixup.bytecodeOffset), sizeof(fixup.bytecodeOffset));
        WriteString(ofs, fixup.symbolName);
        ofs.write(reinterpret_cast<const char*>(&fixup.isDataLabel), sizeof(fixup.isDataLabel));
        ofs.write(reinterpret_cast<const char*>(&fixup.isMemoryReference), sizeof(fixup.isMemoryReference));
        if (!ofs) {
            throw std::runtime_error("Failed to write a Fixup to file.");
        }
    }

    ofs.close();
    if (!ofs) {
        throw std::runtime_error("Failed to finalize writing to file: " + path);
    }
}

ObjectFile ReadObjectFile(const std::string& path) {
    std::ifstream ifs(path, std::ios::binary);
    if (!ifs) {
        throw std::runtime_error("Failed to open file for reading: " + path);
    }

    ObjectFile obj;

    ifs.read(reinterpret_cast<char*>(&obj.codeSize), sizeof(obj.codeSize));
    if (!ifs) {
        throw std::runtime_error("Failed to read codeSize from file.");
    }

    size_t codeSegmentSize;
    ifs.read(reinterpret_cast<char*>(&codeSegmentSize), sizeof(codeSegmentSize));
    if (!ifs) {
        throw std::runtime_error("Failed to read codeSegment size from file.");
    }
    obj.codeSegment.resize(codeSegmentSize);
    if (codeSegmentSize > 0) {
        ifs.read(reinterpret_cast<char*>(obj.codeSegment.data()), codeSegmentSize);
        if (!ifs) {
            throw std::runtime_error("Failed to read codeSegment data from file.");
        }
    }

    size_t dataSegmentSize;
    ifs.read(reinterpret_cast<char*>(&dataSegmentSize), sizeof(dataSegmentSize));
    if (!ifs) {
        throw std::runtime_error("Failed to read dataSegment size from file.");
    }
    obj.dataSegment.resize(dataSegmentSize);
    if (dataSegmentSize > 0) {
        ifs.read(reinterpret_cast<char*>(obj.dataSegment.data()), dataSegmentSize);
        if (!ifs) {
            throw std::runtime_error("Failed to read dataSegment data from file.");
        }
    }

    size_t symbolTableSize;
    ifs.read(reinterpret_cast<char*>(&symbolTableSize), sizeof(symbolTableSize));
    if (!ifs) {
        throw std::runtime_error("Failed to read symbolTable size from file.");
    }
    obj.symbolTable.reserve(symbolTableSize);
    for (size_t i = 0; i < symbolTableSize; ++i) {
        Symbol symbol;
        symbol.name = ReadString(ifs);
        ifs.read(reinterpret_cast<char*>(&symbol.address), sizeof(symbol.address));
        ifs.read(reinterpret_cast<char*>(&symbol.isExternal), sizeof(symbol.isExternal));
        ifs.read(reinterpret_cast<char*>(&symbol.isData), sizeof(symbol.isData));
        if (!ifs) {
            throw std::runtime_error("Failed to read a Symbol from file.");
        }
        obj.symbolTable.push_back(std::move(symbol));
    }

    size_t fixupsSize;
    ifs.read(reinterpret_cast<char*>(&fixupsSize), sizeof(fixupsSize));
    if (!ifs) {
        throw std::runtime_error("Failed to read fixups size from file.");
    }
    obj.fixups.reserve(fixupsSize);
    for (size_t i = 0; i < fixupsSize; ++i) {
        Fixup fixup;
        ifs.read(reinterpret_cast<char*>(&fixup.bytecodeOffset), sizeof(fixup.bytecodeOffset));
        fixup.symbolName = ReadString(ifs);
        ifs.read(reinterpret_cast<char*>(&fixup.isDataLabel), sizeof(fixup.isDataLabel));
        ifs.read(reinterpret_cast<char*>(&fixup.isMemoryReference), sizeof(fixup.isMemoryReference));
        if (!ifs) {
            throw std::runtime_error("Failed to read a Fixup from file.");
        }
        obj.fixups.push_back(std::move(fixup));
    }

    ifs.close();
    if (!ifs) {
        throw std::runtime_error("Failed to finalize reading from file: " + path);
    }

    return obj;
}
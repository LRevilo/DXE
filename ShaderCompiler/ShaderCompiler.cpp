#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <array>
#include <cstdlib>

namespace fs = std::filesystem;

struct ShaderStage {
    std::string suffix;  // "vs", "ps", etc.
    std::string profile; // "vs_5_0", etc.
};

static const std::array<ShaderStage, 6> kStages{ {
    {"vs","vs_5_0"},
    {"ps","ps_5_0"},
    {"cs","cs_5_0"},
    {"hs","hs_5_0"},
    {"ds","ds_5_0"},
    {"gs","gs_5_0"},
} };

// Run fxc to compile shader stage
bool runFXC(const fs::path& hlsl, const fs::path& output, const ShaderStage& stage) {
    std::ostringstream cmd;
    cmd << "fxc /T " << stage.profile
        << " /E " << stage.suffix << "_main"
        << " /Fo \"" << output.string() << "\" \"" << hlsl.string() << "\"";
    std::cout << "  Running: " << cmd.str() << "\n";
    int ret = std::system(cmd.str().c_str());
    return ret == 0 && fs::exists(output);
}

// Write the byte array and length
void writeShaderBytes(std::ofstream& out, const std::string& symName, const fs::path& cso) {
    std::ifstream fin(cso, std::ios::binary);
    std::vector<unsigned char> data((std::istreambuf_iterator<char>(fin)),
        std::istreambuf_iterator<char>());

    out << "inline static unsigned char " << symName << "[] = {";
    for (size_t i = 0; i < data.size(); ++i) {
        if (i % 12 == 0) out << "\n    ";
        out << (int)data[i];
        if (i + 1 < data.size()) out << ",";
    }
    out << "\n};\n";
    out << "inline static unsigned int " << symName << "_len = " << data.size() << ";\n\n";
}

// Write the ShaderByteStruct wrapper
void writeShaderStruct(std::ofstream& out, const std::string& symName) {
    out << "inline static DXE::ShaderByteStruct " << symName << "_struct = { \""
        << symName << "\", " << symName << ", " << symName << "_len };\n\n";
}

// Write the ShaderStruct object for all stages
void writeShaderObject(std::ofstream& out, const std::string& shaderName,
    const std::vector<std::string>& stageStructs) {
    out << "inline static DXE::ShaderStruct " << shaderName << " = { ";
    for (const auto& s : stageStructs) {
        if (!s.empty())
            out << "&" << s << "_struct, ";
        else
            out << "nullptr, ";
    }
    out << "};\n\n";
}

int main(int argc, char** argv) {
    fs::path shaderDir = (argc > 1) ? argv[1] : fs::current_path();
    fs::path outDir = shaderDir / "Compiled";
    fs::create_directories(outDir);

    fs::path masterHeader = shaderDir / "AllShaders.h";
    std::ofstream master(masterHeader);

    // Header preamble
    master << "#pragma once\n\n";
    master << "#include <Renderer/ShaderByte.h>\n";
    master << "#include <unordered_map>\n";
    master << "#include <string>\n";


    master << "\nnamespace ShadersInternal {\n";


    std::vector<std::string> shaderObjects; // for g_Shaders array

    for (auto& file : fs::directory_iterator(shaderDir)) {
        if (file.path().extension() == ".hlsl") {
            std::string base = file.path().stem().string();
            std::cout << "Processing " << file.path() << "...\n";

            std::vector<std::string> stageStructs;
            for (auto& stage : kStages) {
                fs::path cso = outDir / (base + "_" + stage.suffix + ".cso");
                std::string sym = base + "_" + stage.suffix;

                if (runFXC(file.path(), cso, stage)) {
                    std::cout << "  Stage " << stage.suffix << " OK\n";
                    writeShaderBytes(master, sym, cso);
                    writeShaderStruct(master, sym);
                    stageStructs.push_back(sym);
                }
                else {
                    std::cout << "  Stage " << stage.suffix << " skipped.\n";
                    stageStructs.push_back("");
                }
            }

            writeShaderObject(master, base, stageStructs);
            shaderObjects.push_back(base);
        }
    }

    master << "} // namespace ShadersInternal\n\n";


    // // Global array of all shaders
    // master << "static ShaderStruct* g_Shaders[] = {\n";
    // for (auto& s : shaderObjects)
    //     master << "    &" << s << ",\n";
    // master << "    nullptr\n};\n";



    master << "inline static std::unordered_map<std::string, DXE::ShaderStruct> CompiledShaderMap = {\n";
    for (auto& s : shaderObjects)
        master << "    { \"" << s << "\", " << "ShadersInternal::" + s << " },\n";
    master << "};\n";

    std::cout << "AllShaders.h generated successfully.\n";
    return 0;
}
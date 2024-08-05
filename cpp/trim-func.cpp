#include <unistd.h>
#include <binaryen-c.h>
#include <iostream>
#include <memory>
#include <algorithm>
#include <iterator>
#include <fstream>
#include <vector>

static void usage(const char* program_name) { std::cerr << "Usage" << std::endl; }

struct Config {
    std::string output;
    std::string func_names;
    bool text;
};

class Module {
public:
    static std::unique_ptr<Module> parse_binary(const char* wasm_path)
    {
        std::ifstream file(wasm_path, std::ios::binary);
        if (!file) {
            return nullptr;
        }

        file.seekg(0, std::ios::end);
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);
        std::vector<char> buffer(size);

        std::unique_ptr<Module> module;

        if (file.read(buffer.data(), size)) {

            auto native = BinaryenModuleRead(buffer.data(), size);
            if (native) {
                module = std::make_unique<Module>(native);
            }
        }

        file.close();

        return module;
    }

    Module(BinaryenModuleRef r)
    : native_(r)
    {
    }

    ~Module()
    {
        if (native_) {
            BinaryenModuleDispose(native_);
        }
    }

    Module(const Module&) = delete;
    Module& operator=(const Module&) = delete;

    using Emitter = size_t (*)(BinaryenModuleRef, char*, size_t);

    std::vector<char> emit_binary() const { return emit(BinaryenModuleWrite); }

    std::vector<char> emit_text() const { return emit(BinaryenModuleWriteText); }

private:
    BinaryenModuleRef native_;

    std::vector<char> emit(Emitter emitter) const
    {
        const size_t init_buf_size = 1024;
        std::vector<char> buf(init_buf_size);

        size_t writen_size = init_buf_size;

        while (writen_size == buf.size()) {
            buf.resize(buf.size() << 1);
            writen_size = emitter(native_, (char*)buf.data(), buf.size());
        }
        buf.resize(writen_size);

        return buf;
    }
};

std::vector<std::string> split(const std::string& str, const std::string& delim)
{
    std::vector<std::string> result;
    size_t start = 0;

    for (size_t found = str.find(delim); found != std::string::npos;
         found = str.find(delim, start)) {
        result.emplace_back(str.begin() + start, str.begin() + found);
        start = found + delim.size();
    }
    if (start != str.size())
        result.emplace_back(str.begin() + start, str.end());
    return result;
}

int main(int argc, char* argv[])
{
    Config config = {
        .output = "",
        .func_names = "",
        .text = false,
    };

    int opt;
    while ((opt = getopt(argc, argv, "f:o:t")) != -1) {
        switch (opt) {
            case 'f':
                config.func_names = optarg;
                break;
            case 'o':
                config.output = optarg;
                break;

            case 't':
                config.text = true;
                break;

            default: /* '?' */
                usage(argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (optind >= argc) {
        usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    auto wasm_path = argv[optind];

    auto module = Module::parse_binary(wasm_path);
    if (!module) {
        std::cerr << "parse_binary fail" << std::endl;
    }

    if (config.text) {
        auto wasm_text = module->emit_text();

        if (config.output != "") {

        } else {
            std::copy(wasm_text.begin(), wasm_text.end(), std::ostream_iterator<char>(std::cout));
        }

    } else {

        if (config.output != "") {
            auto wasm_binary = module->emit_binary();
            std::ofstream file(config.output, std::ios::binary);
            if (file) {
                file.write(wasm_binary.data(), wasm_binary.size());
            }
            file.close();
        }
    }
    return 0;
}

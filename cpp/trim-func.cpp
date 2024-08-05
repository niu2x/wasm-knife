#include <unistd.h>
#include <binaryen-c.h>
#include <iostream>
#include <memory>
#include <fstream>
#include <vector>

static void usage(const char* program_name) { std::cerr << "Usage" << std::endl; }

struct Config {
    std::string output;
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

    std::vector<char> emit_binary()
    {

        const size_t init_buf_size = 1024;
        std::vector<char> buf(init_buf_size);

        size_t writen_size = init_buf_size;

        while (writen_size == buf.size()) {
            buf.resize(buf.size() << 1);
            writen_size = BinaryenModuleWrite(native_, (char*)buf.data(), buf.size());
        }
        buf.resize(writen_size);

        return buf;
    }

private:
    BinaryenModuleRef native_;
};

int main(int argc, char* argv[])
{
    Config config;

    int opt;
    while ((opt = getopt(argc, argv, "o:")) != -1) {
        switch (opt) {
            case 'o':
                config.output = optarg;
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

    auto wasm_binary = module->emit_binary();

    if (config.output != "") {
        std::ofstream file(config.output, std::ios::binary);
        if (file) {
            file.write(wasm_binary.data(), wasm_binary.size());
        }
        file.close();
    }

    return 0;
}

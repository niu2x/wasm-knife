#include <unistd.h>
#include <binaryen-c.h>
#include <iostream>
#include <memory>
#include <fstream>
#include <vector>

static void usage(const char* program_name) { std::cerr << "Usage" << std::endl; }

struct Config {
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


    std::vector<uint8_t> emit_binary() {
        
    }

private:
    BinaryenModuleRef native_;
};

int main(int argc, char* argv[])
{
    int opt;
    while ((opt = getopt(argc, argv, "nt:")) != -1) {
        switch (opt) {
            case 't':
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



    return 0;
}

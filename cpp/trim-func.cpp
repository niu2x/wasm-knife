#include <unistd.h>
#include <binaryen-c.h>
#include <iostream>
#include <memory>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <fstream>
#include <vector>
#include <stdexcept>

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

    std::vector<char> emit_binary(size_t guess_size = 1024) const
    {
        return emit(guess_size, BinaryenModuleWrite);
    }

    std::vector<char>
    emit_text(bool colors_enabled, size_t guess_size = 1024) const
    {
        BinaryenSetColorsEnabled(colors_enabled);
        return emit(guess_size, BinaryenModuleWriteText);
    }

    void trim_func(const char* name)
    {
        auto func = BinaryenGetFunction(native_, name);
        if (func) {
            auto params_type = BinaryenFunctionGetParams(func);
            auto results_type = BinaryenFunctionGetResults(func);
            BinaryenRemoveFunction(native_, name);

            auto placeholder_name
                = get_placeholder_func_name(params_type, results_type);
            add_empty_func(placeholder_name.c_str(), params_type, results_type);

            replace_elem(name, placeholder_name.c_str());
        }
    }

    bool validate() const { return BinaryenModuleValidate(native_); }

private:
    BinaryenModuleRef native_;

    std::string
    get_placeholder_func_name(BinaryenType params, BinaryenType results)
    {
        std::stringstream ss;
        ss << "_wasm_knife_placeholder_" << params << "_" << results;
        return ss.str();
    }

    void
    add_empty_func(const char* name, BinaryenType params, BinaryenType results)
    {

        if (BinaryenGetFunction(native_, name))
            return;

        BinaryenAddFunction(
            native_,
            name,
            params,
            results,
            nullptr,
            0,
            ir_default_return_expr(results));
    }

    std::vector<char> emit(size_t guess_size, Emitter emitter) const
    {
        const size_t init_buf_size = guess_size >> 1;
        std::vector<char> buf(init_buf_size);

        size_t writen_size = init_buf_size;

        while (writen_size == buf.size()) {
            buf.resize(buf.size() << 1);
            writen_size = emitter(native_, (char*)buf.data(), buf.size());
        }
        buf.resize(writen_size);

        return buf;
    }

    BinaryenExpressionRef ir_default_return_expr(BinaryenType results_type)
    {
        return ir_return_expr(ir_default_value_expr(results_type));
    }

    BinaryenExpressionRef ir_return_expr(BinaryenExpressionRef value)
    {
        return BinaryenReturn(native_, value);
    }

    BinaryenExpressionRef ir_default_value_expr(BinaryenType results_type)
    {
        if (results_type == BinaryenTypeNone()) {
            return nullptr;
        } else {
            return ir_const(results_type, 0);
        }
    }

    template <class T>
    BinaryenExpressionRef ir_const(BinaryenType type, const T& v)
    {
        if (type == BinaryenTypeInt32()) {

            return BinaryenConst(native_, BinaryenLiteralInt32(v));

        } else if (type == BinaryenTypeInt64()) {
            return BinaryenConst(native_, BinaryenLiteralInt64(v));

        } else if (type == BinaryenTypeFloat32()) {
            return BinaryenConst(native_, BinaryenLiteralFloat32(v));

        } else if (type == BinaryenTypeFloat64()) {
            return BinaryenConst(native_, BinaryenLiteralFloat64(v));

        } else {
            throw std::runtime_error("unsupport type " + std::to_string(type));
        }
    }

    void replace_elem(const char* old_name, const char* new_name)
    {

        struct Segment {
            std::vector<std::string> data;
            std::string name;
            std::string table;
        };

        using SegmentArray = std::vector<Segment>;

        SegmentArray segments;

        auto segment_num = BinaryenGetNumElementSegments(native_);
        for (BinaryenIndex i = 0; i < segment_num; i++) {
            Segment segment;
            auto seg_ref = BinaryenGetElementSegmentByIndex(native_, i);
            auto len = BinaryenElementSegmentGetLength(seg_ref);
            segment.name = BinaryenElementSegmentGetName(seg_ref);

            segment.table = BinaryenElementSegmentGetTable(seg_ref);
            auto is_passive = BinaryenElementSegmentIsPassive(seg_ref);
            if (is_passive)
                throw std::runtime_error("unsupport passive segment");

            for (BinaryenIndex k = 0; k < len; k++) {
                std::string n = BinaryenElementSegmentGetData(seg_ref, k);

                if (n == old_name) {
                    n = new_name;
                }

                segment.data.push_back(std::move(n));
            }

            segments.push_back(std::move(segment));
        }

        for (auto& seg : segments) {
            BinaryenRemoveElementSegment(native_, seg.name.c_str());
        }

        for (auto& seg : segments) {

            std::vector<const char*> func_names;
            func_names.resize(seg.data.size());

            std::transform(
                seg.data.begin(),
                seg.data.end(),
                func_names.begin(),
                [](auto& x) { return x.c_str(); });

            BinaryenAddActiveElementSegment(
                native_,
                seg.table.c_str(),
                seg.name.c_str(),
                func_names.data(),
                func_names.size(),
                ir_const(BinaryenTypeInt32(), 0));
        }
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

    std::clog << "parse binary" << std::endl;
    auto module = Module::parse_binary(wasm_path);
    if (!module) {
        std::cerr << "parse_binary fail" << std::endl;
        return EXIT_FAILURE;
    }

    std::clog << "trim func" << std::endl;
    if (config.func_names != "") {

        auto func_names = split(config.func_names, ",");

        for (auto& x : func_names) {
            module->trim_func(x.c_str());
        }
    }

    std::clog << "validate" << std::endl;
    if (!module->validate()) {
        std::cerr << "module validate fail" << std::endl;
        return EXIT_FAILURE;
    }

    std::clog << "emit" << std::endl;
    if (config.text) {
        if (config.output != "") {
            auto wasm_text = module->emit_text(false, 1 << 30);
            std::ofstream file(config.output, std::ios::binary);
            if (file) {
                file.write(wasm_text.data(), wasm_text.size());
            }
            file.close();

        } else {
            auto wasm_text = module->emit_text(true, 1 << 30);
            std::copy(wasm_text.begin(), wasm_text.end(), std::ostream_iterator<char>(std::cout));
        }

    } else {
        if (config.output != "") {
            auto wasm_binary = module->emit_binary(1 << 30);
            std::ofstream file(config.output, std::ios::binary);
            if (file) {
                file.write(wasm_binary.data(), wasm_binary.size());
            }
            file.close();
        }
    }
    return 0;
}

#include <unistd.h>
#include <binaryen-c.h>
#include <iostream>
#include <memory>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <map>
#include <fstream>
#include <vector>
#include <stdexcept>

#define MERGE_COMMON_SIGNATURE 1

static void usage(const char* program_name) { std::cerr << "Usage" << std::endl; }

struct Config {
    std::string output;
    std::string func_names;
    std::string config;
    bool text;
    bool debug;
};

// class Expression {
// public:
//   enum Id {
//     InvalidId = 0,
//     BlockId,
//     IfId,
//     LoopId,
//     BreakId,
//     SwitchId,
//     CallId,
//     CallIndirectId,
//     LocalGetId,
//     LocalSetId,
//     GlobalGetId,
//     GlobalSetId,
//     LoadId,
//     StoreId,
//     ConstId,
//     UnaryId,
//     BinaryId,
//     SelectId,
//     DropId,
//     ReturnId,
//     MemorySizeId,
//     MemoryGrowId,
//     NopId,
//     UnreachableId,
//     AtomicRMWId,
//     AtomicCmpxchgId,
//     AtomicWaitId,
//     AtomicNotifyId,
//     AtomicFenceId,
//     SIMDExtractId,
//     SIMDReplaceId,
//     SIMDShuffleId,
//     SIMDTernaryId,
//     SIMDShiftId,
//     SIMDLoadId,
//     SIMDLoadStoreLaneId,
//     MemoryInitId,
//     DataDropId,
//     MemoryCopyId,
//     MemoryFillId,
//     PopId,
//     RefNullId,
//     RefIsNullId,
//     RefFuncId,
//     RefEqId,
//     TableGetId,
//     TableSetId,
//     TableSizeId,
//     TableGrowId,
//     TableFillId,
//     TableCopyId,
//     TryId,
//     TryTableId,
//     ThrowId,
//     RethrowId,
//     ThrowRefId,
//     TupleMakeId,
//     TupleExtractId,
//     RefI31Id,
//     I31GetId,
//     CallRefId,
//     RefTestId,
//     RefCastId,
//     BrOnId,
//     StructNewId,
//     StructGetId,
//     StructSetId,
//     ArrayNewId,
//     ArrayNewDataId,
//     ArrayNewElemId,
//     ArrayNewFixedId,
//     ArrayGetId,
//     ArraySetId,
//     ArrayLenId,
//     ArrayCopyId,
//     ArrayFillId,
//     ArrayInitDataId,
//     ArrayInitElemId,
//     RefAsId,
//     StringNewId,
//     StringConstId,
//     StringMeasureId,
//     StringEncodeId,
//     StringConcatId,
//     StringEqId,
//     StringWTF16GetId,
//     StringSliceWTFId,
//     ContBindId,
//     ContNewId,
//     ResumeId,
//     SuspendId,
//     NumExpressionIds
//   };

class BinaryenExprWalker {
public:
    class Listener {
    public:
#define ENTER_EXIT(x)                                                          \
    virtual void enter_##x(BinaryenExpressionRef) { }                          \
    virtual void exit_##x(BinaryenExpressionRef) { }

        ENTER_EXIT(block);
        ENTER_EXIT(return );
        ENTER_EXIT(const);
        ENTER_EXIT(call_indirect);
        ENTER_EXIT(call);
        ENTER_EXIT(global_set);
        ENTER_EXIT(global_get);

        ENTER_EXIT(local_set);
        ENTER_EXIT(local_get);
        ENTER_EXIT(binary);
        ENTER_EXIT(break);
        ENTER_EXIT(load);
        ENTER_EXIT(store);
        ENTER_EXIT(unary);
        ENTER_EXIT(if);
        ENTER_EXIT(drop);
        ENTER_EXIT(loop);
        ENTER_EXIT(unreachable);
        ENTER_EXIT(select);
        ENTER_EXIT(memory_size);
        ENTER_EXIT(memory_grow);
        ENTER_EXIT(nop);
        ENTER_EXIT(switch);
    };

    BinaryenExprWalker() { }
    ~BinaryenExprWalker() { }

    void walk(BinaryenExpressionRef expr, Listener* listener)
    {
        do_walk(expr, listener);
    }

private:
    void do_walk(BinaryenExpressionRef expr, Listener* listener)
    {

#define DELEGATE(CLASS_TO_VISIT)                                               \
    static BinaryenExpressionId CLASS_TO_VISIT##_ID                            \
        = Binaryen##CLASS_TO_VISIT##Id();
#include "wasm-delegations.def"

        auto id = BinaryenExpressionGetId(expr);

        if (id == Nop_ID) {

            listener->enter_nop(expr);
            listener->exit_nop(expr);
        } else if (id == Block_ID) {
            listener->enter_block(expr);

            auto children_num = BinaryenBlockGetNumChildren(expr);
            for (BinaryenIndex i = 0; i < children_num; i++) {
                do_walk(BinaryenBlockGetChildAt(expr, i), listener);
            }
            listener->exit_block(expr);
        } else if (id == Call_ID) {
            listener->enter_call(expr);

            auto children_num = BinaryenCallGetNumOperands(expr);
            for (BinaryenIndex i = 0; i < children_num; i++) {
                do_walk(BinaryenCallGetOperandAt(expr, i), listener);
            }

            listener->exit_call(expr);
        }

        else if (id == Drop_ID) {
            listener->enter_drop(expr);

            do_walk(BinaryenDropGetValue(expr), listener);

            listener->exit_drop(expr);
        }

        else if (id == MemorySize_ID) {
            listener->enter_memory_size(expr);
            listener->exit_memory_size(expr);
        }

        else if (id == MemoryGrow_ID) {
            listener->enter_memory_grow(expr);

            do_walk(BinaryenMemoryGrowGetDelta(expr), listener);

            listener->exit_memory_grow(expr);
        }

        else if (id == Switch_ID) {
            listener->enter_switch(expr);

            do_walk(BinaryenSwitchGetCondition(expr), listener);

            if (auto v = BinaryenSwitchGetValue(expr)) {
                do_walk(v, listener);
            }

            listener->exit_switch(expr);
        }

        else if (id == Select_ID) {
            listener->enter_select(expr);

            do_walk(BinaryenSelectGetCondition(expr), listener);

            if (auto cond = BinaryenSelectGetIfTrue(expr)) {
                do_walk(cond, listener);
            }

            if (auto cond = BinaryenSelectGetIfFalse(expr)) {
                do_walk(cond, listener);
            }

            listener->exit_select(expr);
        }

        else if (id == Loop_ID) {
            listener->enter_loop(expr);

            do_walk(BinaryenLoopGetBody(expr), listener);

            listener->exit_loop(expr);
        }

        else if (id == Unreachable_ID) {
            listener->enter_unreachable(expr);
            listener->exit_unreachable(expr);
        }

        else if (id == If_ID) {
            listener->enter_if(expr);

            do_walk(BinaryenIfGetCondition(expr), listener);

            if (auto cond = BinaryenIfGetIfTrue(expr)) {
                do_walk(cond, listener);
            }

            if (auto cond = BinaryenIfGetIfFalse(expr)) {
                do_walk(cond, listener);
            }

            listener->exit_if(expr);
        }

        else if (id == Const_ID) {
            listener->enter_const(expr);
            listener->exit_const(expr);
        }

        else if (id == CallIndirect_ID) {
            listener->enter_call_indirect(expr);

            auto children_num = BinaryenCallIndirectGetNumOperands(expr);
            for (BinaryenIndex i = 0; i < children_num; i++) {
                do_walk(BinaryenCallIndirectGetOperandAt(expr, i), listener);
            }

            listener->exit_call_indirect(expr);
        }

        else if (id == Break_ID) {
            listener->enter_break(expr);
            if (auto cond = BinaryenBreakGetCondition(expr)) {
                do_walk(cond, listener);
            }

            if (auto cond = BinaryenBreakGetValue(expr)) {
                do_walk(cond, listener);
            }
            listener->exit_break(expr);
        }

        else if (id == Binary_ID) {
            listener->enter_binary(expr);

            // do_walk(BinaryenBinaryGetOp(expr), listener);
            do_walk(BinaryenBinaryGetLeft(expr), listener);
            do_walk(BinaryenBinaryGetRight(expr), listener);

            listener->exit_binary(expr);
        }

        else if (id == GlobalSet_ID) {
            listener->enter_global_set(expr);
            do_walk(BinaryenGlobalSetGetValue(expr), listener);
            listener->exit_global_set(expr);
        }

        else if (id == GlobalGet_ID) {
            listener->enter_global_get(expr);
            listener->exit_global_get(expr);
        }

        else if (id == Unary_ID) {
            listener->enter_unary(expr);
            // do_walk(BinaryenUnaryGetOp(expr), listener);
            do_walk(BinaryenUnaryGetValue(expr), listener);
            listener->exit_unary(expr);
        }

        else if (id == Load_ID) {
            listener->enter_load(expr);
            do_walk(BinaryenLoadGetPtr(expr), listener);
            listener->exit_load(expr);
        }

        else if (id == Store_ID) {
            listener->enter_store(expr);
            do_walk(BinaryenStoreGetPtr(expr), listener);
            do_walk(BinaryenStoreGetValue(expr), listener);
            listener->exit_store(expr);
        }

        else if (id == LocalSet_ID) {
            listener->enter_local_set(expr);
            do_walk(BinaryenLocalSetGetValue(expr), listener);
            listener->exit_local_set(expr);
        }

        else if (id == LocalGet_ID) {
            listener->enter_local_get(expr);
            listener->exit_local_get(expr);
        }

        else if (id == Return_ID) {
            listener->enter_return(expr);

            if (auto r = BinaryenReturnGetValue(expr)) {
                do_walk(r, listener);
            }

            listener->exit_return(expr);
        }

        else {
            throw std::runtime_error(
                "unsupport expr id: " + std::to_string(id));
        }
    }
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
        fetch_exports();
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
            std::clog << "trim " << name << std::endl;
            auto params_type = BinaryenFunctionGetParams(func);
            auto results_type = BinaryenFunctionGetResults(func);
            BinaryenRemoveFunction(native_, name);

#if MERGE_COMMON_SIGNATURE == 1
            if (is_export_func(name)) {
#endif
                add_empty_func(name, params_type, results_type);
#if MERGE_COMMON_SIGNATURE == 1
            } else {
                auto placeholder_name
                    = get_placeholder_func_name(params_type, results_type);
                add_empty_func(
                    placeholder_name.c_str(),
                    params_type,
                    results_type);

                names_transform_[name] = placeholder_name;
            }
#endif
        } else {
            std::clog << "no " << name << std::endl;
        }
    }

    void replace_call()
    {
        auto func_num = BinaryenGetNumFunctions(native_);
        for (BinaryenIndex i = 0; i < func_num; i++) {
            auto func = BinaryenGetFunctionByIndex(native_, i);
            auto body = BinaryenFunctionGetBody(func);
            std::clog << "replace_call: "
                      << "func_" << i << std::endl;
            if (body) {
                auto new_body = replace_body(body);
                if (new_body) {
                    BinaryenFunctionSetBody(func, new_body);
                }
            }
        }
    }

    void replace_elem()
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

                auto it = names_transform_.find(n);
                if (it != names_transform_.end()) {
                    n = it->second;
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

    bool is_export_func(const char* name) const
    {
        return exported_internal_names_.find(name)
            != exported_internal_names_.end();
    }

    void fetch_exports()
    {
        BinaryenIndex len = BinaryenGetNumExports(native_);
        for (BinaryenIndex i = 0; i < len; i++) {
            auto ref = BinaryenGetExportByIndex(native_, i);
            auto name = BinaryenExportGetValue(ref);
            exported_internal_names_[name] = true;
        }
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

    using NameTransform = std::map<std::string, std::string>;
    NameTransform names_transform_;
    std::map<std::string, bool> exported_internal_names_;

    class ReplaceCallListner : public BinaryenExprWalker::Listener {
    public:
        ReplaceCallListner(const NameTransform* t)
        : names_transform_(t)
        {
        }
        virtual ~ReplaceCallListner() { }

        void enter_call(BinaryenExpressionRef ref) override
        {
            auto target = BinaryenCallGetTarget(ref);
            auto it = names_transform_->find(target);
            if (it != names_transform_->end()) {
                BinaryenCallSetTarget(ref, it->second.c_str());
            }
        }

    private:
        const NameTransform* names_transform_;
    };

    BinaryenExpressionRef replace_body(BinaryenExpressionRef old_body)
    {
        BinaryenExprWalker walker;
        ReplaceCallListner listener(&names_transform_);
        walker.walk(old_body, &listener);
        // return ir_return_expr(nullptr);
        return nullptr;
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

std::vector<std::string> read_func_names(const char* config)
{

    std::vector<std::string> names;

    std::ifstream file(config);

    if (!file.is_open()) {
        return {};
    }

    std::string line;

    while (std::getline(file, line)) {
        if (line[0] == '-' && line[1] == ' ') {
            line = split(line, " ")[1];
            names.push_back(std::move(line));
        }
    }

    file.close();

    return names;
}

int main(int argc, char* argv[])
{
    Config config = {
        .output = "",
        .func_names = "",
        .config = "",
        .text = false,
        .debug = false,
    };

    int opt;
    while ((opt = getopt(argc, argv, "c:f:go:t")) != -1) {
        switch (opt) {
            case 'f':
                config.func_names = optarg;
                break;
            case 'o':
                config.output = optarg;
                break;

            case 'c':
                config.config = optarg;
                break;

            case 't':
                config.text = true;
                break;

            case 'g':
                config.debug = true;
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

    if (config.config != "") {
        auto func_names = read_func_names(config.config.c_str());
        for (auto& x : func_names) {
            module->trim_func(x.c_str());
        }
    }

#if MERGE_COMMON_SIGNATURE == 1
    std::clog << "replace_elem" << std::endl;
    module->replace_elem();

#endif

#if MERGE_COMMON_SIGNATURE == 1
    std::clog << "replace_call" << std::endl;
    module->replace_call();
#endif

    std::clog << "validate" << std::endl;
    if (!module->validate()) {
        std::cerr << "module validate fail" << std::endl;
        return EXIT_FAILURE;
    }

    BinaryenSetDebugInfo(config.debug);

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

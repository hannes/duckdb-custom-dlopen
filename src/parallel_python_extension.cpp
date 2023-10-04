#define DUCKDB_EXTENSION_MAIN

#include "parallel_python_extension.hpp"
#include "duckdb.hpp"
#include "duckdb/common/exception.hpp"
#include "duckdb/function/scalar_function.hpp"
#include "duckdb/main/extension_util.hpp"
#include <duckdb/parser/parsed_data/create_scalar_function_info.hpp>

#include "pmload.hpp"

namespace duckdb {

#define INIT_FUNCTION_PTR(name)                                                \
  {                                                                            \
    name = (decltype(name))pmload::my_dlsym(library_handle, #name);            \
    if (!name) {                                                               \
      throw InternalException("Failed to bind function pointer %s", #name);    \
    }                                                                          \
  }

// cough
#define Py_eval_input 258

typedef void PyObject;

struct PythonLocalState : public FunctionLocalState {
  explicit PythonLocalState(ClientContext &context) {
    Value library_name;
    context.TryGetCurrentSetting("parallel_python_library", library_name);
    if (library_name.IsNull()) {
      throw InvalidInputException("Need parallel_python_library setting");
    }
    auto library_name_str = library_name.ToString();
    library_handle =
        pmload::my_dlopen(library_name_str.c_str(), 0x2); // RTLD_NOW

    if (!library_handle) {
      throw InternalException("Failed to initialize Python runtime");
    }

    INIT_FUNCTION_PTR(Py_InitializeEx)
    INIT_FUNCTION_PTR(Py_FinalizeEx)
    INIT_FUNCTION_PTR(Py_DecRef)
    INIT_FUNCTION_PTR(PyImport_AddModule)
    INIT_FUNCTION_PTR(PyModule_AddObjectRef)
    INIT_FUNCTION_PTR(PyModule_GetDict)
    INIT_FUNCTION_PTR(PyRun_String)
    INIT_FUNCTION_PTR(PyLong_FromLong)
    INIT_FUNCTION_PTR(PyLong_AsLong)

    Py_InitializeEx(0);

    main_module = PyImport_AddModule("__main__");
    if (!main_module) {
      throw InternalException("eek");
    }
    globals = PyModule_GetDict(main_module);
    if (!globals) {
      throw InternalException("eek");
    }
  }

  ~PythonLocalState() { Py_FinalizeEx(); }
  void *library_handle = nullptr;

  PyObject *main_module, *globals;

  void (*Py_InitializeEx)(int);
  void (*Py_FinalizeEx)();
  void (*Py_DecRef)(PyObject *o);

  PyObject *(*PyImport_AddModule)(const char *name);
  int (*PyModule_AddObjectRef)(PyObject *module_, const char *name,
                               PyObject *value);
  PyObject *(*PyModule_GetDict)(PyObject *module_);
  PyObject *(*PyRun_String)(const char *str, int start, PyObject *globals,
                            PyObject *locals);
  PyObject *(*PyLong_FromLong)(long v);
  long (*PyLong_AsLong)(PyObject *obj);
};

static void PythonIntegerFunction(DataChunk &args, ExpressionState &state,
                                  Vector &result) {
  auto &lstate =
      ExecuteFunctionState::GetFunctionState(state)->Cast<PythonLocalState>();
  auto script = args.data[0].GetValue(0).ToString();

  UnaryExecutor::Execute<int64_t, int64_t>(
      args.data[1], result, args.size(), [&](int64_t in) {
        auto py_in = lstate.PyLong_FromLong(in);

        auto add_ref_res =
            lstate.PyModule_AddObjectRef(lstate.main_module, "x", py_in);
        D_ASSERT(!add_ref_res);

        auto py_result = lstate.PyRun_String(script.c_str(), Py_eval_input,
                                             lstate.globals, lstate.globals);
        auto result = lstate.PyLong_AsLong(py_result);

        lstate.Py_DecRef(py_result);
        return result;
      });
}

static unique_ptr<FunctionLocalState>
RandomInitLocalState(ExpressionState &state,
                     const BoundFunctionExpression &expr,
                     FunctionData *bind_data) {
  return make_uniq<PythonLocalState>(state.GetContext());
}

static void LoadInternal(DatabaseInstance &instance) {

  auto &config = DBConfig::GetConfig(instance);

  config.AddExtensionOption("parallel_python_library", "Python binary to use",
                            LogicalType::VARCHAR, Value());

  auto python_integer_function = ScalarFunction(
      "python_integer_function", {LogicalType::VARCHAR, LogicalType::BIGINT},
      LogicalType::BIGINT, PythonIntegerFunction, nullptr, nullptr, nullptr,
      RandomInitLocalState);
  // TODO does it?
  python_integer_function.side_effects = FunctionSideEffects::HAS_SIDE_EFFECTS;

  ExtensionUtil::RegisterFunction(instance, python_integer_function);
}

void ParallelPythonExtension::Load(DuckDB &db) { LoadInternal(*db.instance); }
std::string ParallelPythonExtension::Name() { return "parallel_python"; }

} // namespace duckdb

extern "C" {

DUCKDB_EXTENSION_API void parallel_python_init(duckdb::DatabaseInstance &db) {
  LoadInternal(db);
}

DUCKDB_EXTENSION_API const char *parallel_python_version() {
  return duckdb::DuckDB::LibraryVersion();
}
}

#ifndef DUCKDB_EXTENSION_MAIN
#error DUCKDB_EXTENSION_MAIN not defined
#endif

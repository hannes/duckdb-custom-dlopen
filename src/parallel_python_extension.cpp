#define DUCKDB_EXTENSION_MAIN

#include "parallel_python_extension.hpp"
#include "duckdb.hpp"
#include "duckdb/common/exception.hpp"
#include "duckdb/common/string_util.hpp"
#include "duckdb/function/scalar_function.hpp"
#include "duckdb/main/extension_util.hpp"
#include <duckdb/parser/parsed_data/create_scalar_function_info.hpp>


namespace duckdb {


static void LoadInternal(DatabaseInstance &instance) {

    auto &config = DBConfig::GetConfig(instance);

    config.AddExtensionOption("parallel_python_library", "Python binary to use",
                              LogicalType::VARCHAR, Value());
}

void ParallelPythonExtension::Load(DuckDB &db) {
	LoadInternal(*db.instance);
}
std::string ParallelPythonExtension::Name() {
	return "parallel_python";
}

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

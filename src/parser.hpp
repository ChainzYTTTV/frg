#pragma once
#include "ast.hpp"

// Parses one .frg file. Errors are reported as they're found; on an error, `ok` is false.
// `prelude` is for Forge's own built-in declarations: functions without bodies and `static` constants.
FileAST parseSource(const SourceFile& file, bool prelude = false);

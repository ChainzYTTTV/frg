#pragma once
#include "project.hpp"

// Pass 1 learns every function and global variable in every file, so things can be used
// before (or after) the place they're written. Pass 2 checks each function body.
bool checkProject(Project& proj);

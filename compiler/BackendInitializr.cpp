#include "IROperation.h"
#include "ARM64Backend.h"

// Définition du pointeur global en tant que const CodeGenBackend*
const CodeGenBackend* codegenBackend = nullptr;

static ARM64Backend backendInstance;

__attribute__((constructor))
static void initBackend() {
    codegenBackend = &backendInstance;
}

#include "ARM64Backend.h"
#include "X86Backend.h"

// Définition du pointeur global en tant que const CodeGenBackend*
const CodeGenBackend* codegenBackend = nullptr;

static const ARM64Backend backendInstance;

__attribute__((constructor))
static void initBackend() {
    codegenBackend = &backendInstance;
}

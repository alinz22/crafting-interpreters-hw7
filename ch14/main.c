#include "chunk.h"
#include "debug.h"
#include "memory.h"

int main(int argc, const char* argv[]) {
  Chunk chunk;
  initChunk(&chunk);

  for (int i = 0; i < 260; i++) {
    writeConstant(&chunk, i, 100 + i / 100);
  }
  writeChunk(&chunk, OP_RETURN, 200);

  disassembleChunk(&chunk, "test chunk");
  freeChunk(&chunk);
  freeHeap();
  return 0;
}

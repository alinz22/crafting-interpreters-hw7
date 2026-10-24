#include <stdio.h>
#include <time.h>

#include "chunk.h"
#include "debug.h"
#include "memory.h"
#include "vm.h"

static void emit(Chunk* chunk, const char* ops) {
  for (const char* c = ops; *c != '\0'; c++) {
    switch (*c) {
      case '+': writeChunk(chunk, OP_ADD, 1); break;
      case '-': writeChunk(chunk, OP_SUBTRACT, 1); break;
      case '*': writeChunk(chunk, OP_MULTIPLY, 1); break;
      case '/': writeChunk(chunk, OP_DIVIDE, 1); break;
      case '~': writeChunk(chunk, OP_NEGATE, 1); break;
      case ' ': break;
      default: writeConstant(chunk, *c - '0', 1); break;
    }
  }
  writeChunk(chunk, OP_RETURN, 1);
}

static void runExpression(const char* source, const char* postfix) {
  Chunk chunk;
  initChunk(&chunk);
  emit(&chunk, postfix);
  disassembleChunk(&chunk, source);
  printf("result: ");
  interpret(&chunk);
  printf("\n");
  freeChunk(&chunk);
}

static void deepStack(void) {
  Chunk chunk;
  initChunk(&chunk);
  for (int i = 0; i < 1000; i++) writeConstant(&chunk, 1, 1);
  for (int i = 0; i < 999; i++) writeChunk(&chunk, OP_ADD, 1);
  writeChunk(&chunk, OP_RETURN, 1);
  printf("sum of 1000 ones (stack grows past 256): ");
  interpret(&chunk);
  freeChunk(&chunk);
}

static void benchmarkNegate(void) {
  Chunk chunk;
  initChunk(&chunk);
  writeConstant(&chunk, 1, 1);
  for (int i = 0; i < 1000000; i++) writeChunk(&chunk, OP_NEGATE, 1);
  writeChunk(&chunk, OP_RETURN, 1);

  vmQuiet = true;
  clock_t start = clock();
  for (int i = 0; i < 200; i++) interpret(&chunk);
  double seconds = (double)(clock() - start) / CLOCKS_PER_SEC;
  vmQuiet = false;

  printf("\n200 runs of 1,000,000 OP_NEGATE: %.3f seconds\n", seconds);
  freeChunk(&chunk);
}

int main(int argc, const char* argv[]) {
  initVM();

  runExpression("1 * 2 + 3", "1 2 * 3 +");
  runExpression("1 + 2 * 3", "1 2 3 * +");
  runExpression("3 - 2 - 1", "3 2 - 1 -");
  runExpression("1 + 2 * 3 - 4 / -5", "1 2 3 * + 4 5 ~ / -");
  runExpression("4 - 3 * -2 (no OP_NEGATE)", "4 3 0 2 - * -");
  runExpression("4 - 3 * -2 (no OP_SUBTRACT)", "4 3 2 ~ * ~ +");

  deepStack();
  benchmarkNegate();

  freeVM();
  freeHeap();
  return 0;
}

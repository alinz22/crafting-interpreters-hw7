#include <stdio.h>

#include "common.h"
#include "debug.h"
#include "memory.h"
#include "vm.h"

VM vm;
bool vmQuiet = false;

static void resetStack(void) {
  vm.stackTop = vm.stack;
}

void initVM(void) {
  vm.stack = NULL;
  vm.stackCapacity = 0;
  vm.stack = GROW_ARRAY(Value, vm.stack, 0, STACK_INITIAL);
  vm.stackCapacity = STACK_INITIAL;
  resetStack();
}

void freeVM(void) {
  FREE_ARRAY(Value, vm.stack, vm.stackCapacity);
  vm.stack = NULL;
  vm.stackCapacity = 0;
}

void push(Value value) {
  int count = (int)(vm.stackTop - vm.stack);
  if (count == vm.stackCapacity) {
    int oldCapacity = vm.stackCapacity;
    vm.stackCapacity = GROW_CAPACITY(oldCapacity);
    vm.stack = GROW_ARRAY(Value, vm.stack, oldCapacity, vm.stackCapacity);
    vm.stackTop = vm.stack + count;
  }
  *vm.stackTop = value;
  vm.stackTop++;
}

Value pop(void) {
  vm.stackTop--;
  return *vm.stackTop;
}

static InterpretResult run(void) {
#define READ_BYTE() (*vm.ip++)
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])
#define BINARY_OP(op) \
    do { \
      double b = pop(); \
      vm.stackTop[-1] = vm.stackTop[-1] op b; \
    } while (false)

  for (;;) {
#ifdef DEBUG_TRACE_EXECUTION
    printf("          ");
    for (Value* slot = vm.stack; slot < vm.stackTop; slot++) {
      printf("[ ");
      printValue(*slot);
      printf(" ]");
    }
    printf("\n");
    disassembleInstruction(vm.chunk, (int)(vm.ip - vm.chunk->code));
#endif
    uint8_t instruction;
    switch (instruction = READ_BYTE()) {
      case OP_CONSTANT:
        push(READ_CONSTANT());
        break;
      case OP_CONSTANT_LONG: {
        int index = READ_BYTE();
        index |= READ_BYTE() << 8;
        index |= READ_BYTE() << 16;
        push(vm.chunk->constants.values[index]);
        break;
      }
      case OP_ADD:      BINARY_OP(+); break;
      case OP_SUBTRACT: BINARY_OP(-); break;
      case OP_MULTIPLY: BINARY_OP(*); break;
      case OP_DIVIDE:   BINARY_OP(/); break;
      case OP_NEGATE:
#ifdef NEGATE_POP_PUSH
        push(-pop());
#else
        vm.stackTop[-1] = -vm.stackTop[-1];
#endif
        break;
      case OP_RETURN: {
        Value result = pop();
        if (!vmQuiet) {
          printValue(result);
          printf("\n");
        }
        return INTERPRET_OK;
      }
    }
  }

#undef READ_BYTE
#undef READ_CONSTANT
#undef BINARY_OP
}

InterpretResult interpret(Chunk* chunk) {
  vm.chunk = chunk;
  vm.ip = vm.chunk->code;
  resetStack();
  return run();
}

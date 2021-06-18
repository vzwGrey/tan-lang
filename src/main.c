#include <history.h>
#include <readline.h>
#include <stdbool.h>

#include "interpreter.h"
#include "parser.h"

// ---------------
// REPL
// ---------------

bool ReadInput(char const *prompt, char **line)
{
  *line = readline(prompt);

  if (!*line)
    return false;

  if (**line)
    add_history(*line);

  return true;
}

int main(void)
{
  INTERPRETER_STATE interpreter = NewInterpreterState(256);

  char *line;
  while (ReadInput(">> ", &line))
  {
    if (!*line)
      continue;

    char const *source = line;
    AST_NODE *ast = ParseProgram(source);

    VALUE result = Evaluate(&interpreter, ast);
    putc('\t', stdout);
    PrintValue(&result);
    putc('\n', stdout);

    FreeAST(ast);

    free(line);
  }

  FreeInterpreterState(&interpreter);

  return 0;
}

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
  char *line;
  while (ReadInput(">> ", &line))
  {
    if (!*line)
      continue;

    char const *source = line;
    AST_NODE *ast = ParseProgram(source);

    double result = Evaluate(ast);
    printf("\t%f\n", result);

    FreeAST(ast);

    free(line);
  }

  return 0;
}

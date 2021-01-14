#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#define BUF_SIZE 100
void error_handing(char *message);

int main(void)
{
  FILE *fp1, *fp2;
  int n;
  char buf[BUF_SIZE];

  fp1 = fopen("data.txt", "r");
  if (fp1 == NULL)
    error_handing("fp1 fopen() error");

  fp2 = fopen("data_copy_anis.txt", "w");
  if (fp2 == NULL)
    error_handing("fp2 fopen() error");

  while (fgets(buf, BUF_SIZE, fp1) != NULL) {
    if (fputs(buf, fp2) == EOF)
      error_handing("write() error");
  }

  fclose(fp1);
  fclose(fp2);

  return EXIT_SUCCESS;
}

void error_handing(char *message)
{
  fputs(message, stderr);
  fputc('\n', stderr);
  exit(1);
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *foo(char *str, int *x) // multiple style - meanningless names
{
  char *str2; // style -
  int i;  // style - define a variable when used
  // correctness - no validation of *x
  x = strlen(str); // incorrect
  str2 = malloc(*x); // incorrect
  for (i = 0; i < *x; i++)  // style - missing parentheses
    str2[i] = str[*x - i]; // incorrect
  if (*x % 2 == 0)
  {
    printf("%s", str);  // incorrect
  }
  if (*x % 2 != 0) // style - extremely unreadable
  {
    printf("%s", str2);
  }
  return str2;
}

char *reverse_string(char *str, int *length)
{
  if (NULL == length) {
    return NULL;
  }

  *length = strlen(str);
  char *reverse = (char *)malloc((*length + 1) * sizeof(char));

  if (NULL == reverse) {
    return NULL;
  }

  for (int i = 0; i < *length; i++) {
    reverse[i] = str[*length - i - 1];
  }

  reverse[*length] = '\0';

  if (*length % 2 != 0) {
    printf("%s", str);
  } else {
    printf("%s", reverse);
  }

  return reverse;
}
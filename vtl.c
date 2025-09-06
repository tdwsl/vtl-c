#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

unsigned short vars[64];
char *s;
char *lines[1000];
unsigned short linen[1000];
int nlines = 0;
int pc = 0;
int prev = 0;
int start = 0;

#define MEMSZ 65536
int memsz = MEMSZ;
unsigned char mem[MEMSZ];

#ifdef TERMIOS
#include <termios.h>
int key() {
  struct termios term, old;
  tcgetattr(0, &old);
  memcpy(&term, &old, sizeof(struct termios));
  term.c_lflag &= ~ICANON&~ECHO;
  tcsetattr(0, TCSANOW, &term);
  int k = getchar();
  tcsetattr(0, TCSANOW, &old);
  return k;
}
#else
int key() {
  return fgetc(stdin);
}
#endif

int eval();

int value() {
  int n = 0;
  char *o = s;
  while(*s >= '0' && *s <= '9') n = n*10+*s++-'0';
  if(s != o) return n;
  switch(*s++) {
  case '$': return key();
  case '(': return eval();
  case '#': return pc;
  case '?':
    {
      char buf[100];
      fgets(buf, 99, stdin);
      char *l = s;
      s = buf;
      int n = eval();
      s = l;
      return n;
    }
  case '!': return prev;
  case '*': return memsz;
  case '&': return start;
  case ':': return mem[eval()];
  case '\'': return rand();
  }
  s--;
  if(*s >= '!' && *s <= '`') return vars[*s++-'!'];
}

int eval() {
  int n = value();
  int b;
  while(*s != '\n') {
    switch(*s++) {
    case ')': return n;
    case '+': n += value(); break;
    case '-': n -= value(); break;
    case '*': n *= value(); break;
    case '/': b = value(); vars['%'-'!'] = n%b; n /= b; break;
    case '=': n = n == value(); break;
    case '<': n = n < value(); break;
    case '>': n = n >= value(); break;
    default: s--; return n;
    }
  }
  return n;
}

void runLine(char *l) {
  s = l;
  char c = *s++;
  if(c == ':') {
    int n = eval();
    if(*s++ != '=') return;
    mem[n] = eval();
    return;
  }
  if(*s++ != '=') return;
  if(c == '?') {
    if(*s == '"') {
      while(*++s != '\n' && *s != '"') printf("%c", *s);
      if(*s) s++;
    } else printf("%d", (unsigned short)eval());
    if(*s != ';') printf("\n");
  }
  int n = eval();
  switch(c) {
  case '$':
    printf("%c", n);
    break;
  case '#':
    if(n) { prev = pc+1; pc = n-1; }
    break;
  case '&': start = n; break;
  case '*': memsz = n; break;
  default:
    if(c >= '!' && c <= '`') vars[c-'!'] = n;
    break;
  }
}

void addLine(char *s) {
  int n = 0;
  while(*s >= '0' && *s <= '9') n = n*10+*s++-'0';
  while(*s && *s <= ' ') s++;
  if(n) {
    start += sprintf(&mem[start], "%d ", n);
    lines[nlines] = &mem[start];
    start += sprintf(&mem[start], "%s", s);
    linen[nlines++] = n;
  } else runLine(s);
}

void run() {
  for(;;) {
    int i;
    for(i = 0; i < nlines && linen[i] < pc; i++);
    if(i >= nlines) break;
    pc = linen[i];
    runLine(lines[i]);
    pc++;
  }
}

int main(int argc, char **args) {
  char buf[100];
  srand(time(0));
  FILE *fp = stdin;
  if(argc > 1) fp = fopen(args[1], "r");
  if(!fp) return printf("failed to open %s\n", args[1]);
  while(!feof(fp)) {
    fgets(buf, 99, fp);
    addLine(buf);
  }
  if(fp != stdin) fclose(fp);
  run();
}

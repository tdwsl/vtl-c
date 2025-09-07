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
FILE *inp, *out;

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

unsigned short eval();

unsigned short value() {
  unsigned short n = 0;
  char *o = s;
  while(*s >= '0' && *s <= '9') n = n*10+*s++-'0';
  if(s != o) return n;
  switch(*s++) {
  case '$':
    if(inp == stdin) return key();
    if(!inp) return 65535;
    return fgetc(inp);
  case '(': return eval();
  case '#': return pc;
  case '?':
    {
      if(!inp) return 0;
      char buf[100];
      fgets(buf, 99, inp);
      char *l = s;
      s = buf;
      int n = eval();
      s = l;
      return n;
    }
  case '*': return memsz;
  case '&': return start;
  case ':': return mem[eval()];
  case '\'': return rand();
  }
  s--;
  if(*s >= '!' && *s <= '`') return vars[*s++-'!'];
}

unsigned short eval() {
  unsigned short n = value();
  unsigned short b;
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

FILE *openFile(FILE *fp, FILE *cmp, const char *mode) {
  if(fp && fp != cmp) fclose(fp);
  if(*s == '"') {
    char buf[100]; int i = 0;
    while(*++s != '\n' && *s != '"') buf[i++] = *s;
    if(!i) return cmp;
    buf[i] = 0;
    return fopen(buf, mode);
  } else return fopen(&mem[eval()], mode);
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
    if(!out) return;
    if(*s == '"') {
      while(*++s != '\n' && *s != '"') fprintf(out, "%c", *s);
      if(*s) s++;
    } else fprintf(out, "%d", eval());
    if(*s != ';') fputc('\n', out);
  }
  if(c == '<') { inp = openFile(inp, stdin, "r"); return; }
  if(c == '>') { inp = openFile(out, stdout, "w"); return; }
  int n = eval();
  switch(c) {
  case '$':
    if(out) fputc(n, out);
    break;
  case '#':
    if(n) { vars[0] = pc+1; pc = n-1; }
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
    *(short*)&mem[start] = nlines;
    start += 2; mem[start++] = strlen(s)+1;
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
  inp = stdin;
  out = stdout;
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

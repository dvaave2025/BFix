#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void main(int argc, char** argv[]){
	int a = argc;
	char *p = NULL;
	char **q = &p;
	p = malloc(10);
	strcpy(p, "hello");
	printf("%d, %s\n", a, p);
	if(a % 3 == 0){
		free(*q);
	}else if(a % 3 == 1){
		p[5] = 'a';
		p[6] = '\0';
		printf("%s\n", p);
	}else{
		p[5] = 'b';
		p[6] = '\0';
		printf("%s\n", p);
		free(p);
	}
}

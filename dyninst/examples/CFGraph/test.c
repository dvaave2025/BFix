#include <stdio.h>
#include <stdlib.h>

int main(int argc, char*argv[]) {
	char *p = (char*)malloc(argc);
	/*if(p == NULL)
		return -1;
	i = argc;
	if(i < 3){
		return -1;
	}*/
	p[0] = 'a';
	p[1] = argv[1];
	p[2] = '\0';
	printf("%s\n",p);
	free(p);
    return 0;
}

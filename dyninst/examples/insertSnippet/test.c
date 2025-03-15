#include <stdio.h>
#include <stdlib.h>

int main(int argc, char*argv[]) {
    int i, x, y;
	x = rand();
	y = 0;
	char *p = malloc(2);
	if(p == NULL)
		return -1;
	i = argc;
	if(x < 3){
		y = rand();
		//return 0;
	}else if(i == 3){
		p[0] = 'a';
		p[1] = '\0';
		printf("%s\n",p);
		free(p);
		y = 0;
	}else{
		y = rand();
	}
	if(y != 0){
		printf("%d", y);
	}
    return 0;
}

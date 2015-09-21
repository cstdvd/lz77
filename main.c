#include "lz77.h"

int main()
{
	// vars
	int ret, sb_size = 0;
	FILE *file;
	void *buffer, *la;
	struct token *t = NULL;
	
	// open the file
	file = fopen("./abc.txt", "r");
	if(file == NULL){
		printf("Error opening file.\n");
		exit(1);
	}
	
	while(feof(file) == 0){
		buffer = malloc((sb_size + LA_SIZE) * sizeof(unsigned char));
		ret = fread(buffer, 1, (sb_size + LA_SIZE), file);
		if(ret < (sb_size + LA_SIZE) && feof(file) == 0){
			printf("An error occurred.\n");
			exit(1);
		}
	
		la = (t == NULL)? buffer : (buffer + t->n + 1);	
		//printf("%c\n",*(char*)la);
		t = match(buffer, la);
		//printf("%d, %d, %c\n",t->off, t->n, t->next);
		sb_size = (sb_size < SB_SIZE)? (sb_size + t->n + 1) : SB_SIZE;
	
	}

	return 0;
}

struct token* match(void *sb, void *la)
{	
	struct token *t;
	int i;
	
	t = (struct token*)malloc(sizeof(struct token));
	t->off = 0;
	t->n = 0;
	t->next = *(char*)la;

	
	while(sb < la){
	   
		for(i = 0; *(char*)(sb+i) == *(char*)(la+i); i++){	
		}
		if(i > t->n){
			t->off = (int)(la - sb);
			t->n = i;
			t->next = *(char*)(la+i);
		}
		sb++;
	}
	
	return t;
}


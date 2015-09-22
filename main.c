#include "lz77.h"

unsigned char buffer[SB_SIZE];
unsigned char lookahead[LA_SIZE];

int main()
{
	// vars
	int i, ret;
	FILE *file;
    unsigned char c;
	struct token *t = NULL;
    int sb_index = 0, la_index = 0;
	
	// open the file
	file = fopen("./abc.txt", "r");
	if(file == NULL){
		printf("Error opening file.\n");
		exit(1);
	}
	
    for(i = 0; i < LA_SIZE; i++){
        ret = fread(&c, 1, 1, file);
        if(feof(file))
            lookahead[i] = c;
        else
            break;
    }
    
    t = match(sb_index, la_index);
    
	while(feof(file) == 0){
		
        for(i = 0; i < t->len; i++){
            ret = fread(&c, 1, 1, file);
            if(feof(file) == 0){
                buffer[sb_index] = lookahead[la_index];
                lookahead[la_index] = c;
                sb_index = (sb_index + 1) % SB_SIZE;
                la_index = (la_index + 1) % LA_SIZE;
            }
            else
                break;
        }
	
		t = match(sb_index, la_index);
		printf("<%d, %d, %c>\n",t->off, t->len, t->next);
	}

	return 0;
}

struct token* match(int sb, int la)
{	
	struct token *t;
    int i, j, c = 0;
	
	t = (struct token*)malloc(sizeof(struct token));
	t->off = 0;
	t->len = 0;
	t->next = lookahead[la];

	
	while(c < SB_SIZE){
	   
		for(i = 0; buffer[(sb+i)%SB_SIZE] == lookahead[(la+i)%LA_SIZE]; i++){
            if((i >= LA_SIZE) || (c+i >= SB_SIZE))
                break;
		}
        
        // nel caso in cui il match continui nel lookahead buffer
        if((c+i >= SB_SIZE) && (i < LA_SIZE)){
            for(j = la; lookahead[j%LA_SIZE] == lookahead[(la+i)%LA_SIZE]; i++){
                j++;
                if(i >= LA_SIZE)
                    break;
            }
        }
		if(i > t->len){
			t->off = SB_SIZE - c;
			t->len = i;
			t->next = lookahead[(la+i)%LA_SIZE];
		}
		sb = (sb + 1) % SB_SIZE;
        c++;
	}
	
	return t;
}


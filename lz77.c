#include "lz77.h"

unsigned char buffer[SB_SIZE];
unsigned char lookahead[LA_SIZE];
int la_size;    // actual lookahead size

int main(int argc, char *argv[])
{
	// vars
    int opt;
    FILE *file, *out;
	
    while ((opt = getopt(argc, argv, "c:d:h")) != -1)
    {
        switch(opt)
        {
            case 'c':       /* compression mode */
                if (file != NULL)
                {
                    fprintf(stderr, "Multiple input files not allowed.\n");
                    fclose(file);
                    exit(EXIT_FAILURE);
                }
                else if ((file = fopen(optarg, "rb")) == NULL)
                {
                    perror("Opening file");
                    exit(EXIT_FAILURE);
                }
                
                if((out = fopen("peter.lz77", "w")) == NULL)
                {
                    perror("Creating compressed file");
                    exit(EXIT_FAILURE);
                }
                
                encode(file, out);
                fclose(out);
                break;
                
            case 'd':       /* decompression mode */
                if (file != NULL)
                {
                    fprintf(stderr, "Multiple input files not allowed.\n");
                    fclose(file);
                    exit(EXIT_FAILURE);
                }
                else if ((file = fopen(optarg, "rb")) == NULL)
                {
                    perror("Opening file");
                    exit(EXIT_FAILURE);
                }
                
                decode(file);
                break;
                
            case 'h':
                printf("Usage: lz77 <options>\n");
                printf("  -c <filename> : Encode input file.\n");
                printf("  -d <filename> : Decode input file.\n");
                break;
        }
    }
                   
    return 0;
}


void encode(FILE *file, FILE *out)
{
    int i, ret, opt;
    unsigned char c;
    struct token *t = NULL;
    int sb_index = 0, la_index = 0;
    la_size = LA_SIZE;
    
    for(i = 0; i < LA_SIZE; i++){
        ret = fread(&c, 1, 1, file);
        if(feof(file) == 0 && ret != 0)
            lookahead[i] = c;
        else
            break;
    }
    
    t = match(sb_index, la_index);
    
    writecode(t, out);
    
	while(la_size > 0){
		
        for(i = 0; i < t->len + 1; i++){
            ret = fread(&c, 1, 1, file);
            if(feof(file) == 0){
                buffer[sb_index] = lookahead[la_index];
                lookahead[la_index] = c;
                sb_index = (sb_index + 1) % SB_SIZE;
                la_index = (la_index + 1) % LA_SIZE;
            }
            else{ // caso in cui EOF compaia prima del riempimento del lookahead
                buffer[sb_index] = lookahead[la_index];
                sb_index = (sb_index + 1) % SB_SIZE;
                la_index = (la_index + 1) % LA_SIZE;
                la_size--;
            }
        }
        
        if(la_size > 0){
            t = match(sb_index, la_index);
            writecode(t, out);
        }
	}
}

void decode(FILE *file)
{
    
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
            if((i >= la_size) || (c+i >= SB_SIZE))
                break;
		}
        
        // nel caso in cui il match continui nel lookahead buffer
        if((c+i >= SB_SIZE) && (i < la_size)){
            for(j = la; lookahead[j%LA_SIZE] == lookahead[(la+i)%LA_SIZE]; i++){
                j++;
                if(i >= la_size)
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

void writecode(struct token *t, FILE *out) //  off = 12 bits, len = 4 bits, next = 8 bits
{
    unsigned char code[3];
    int i;
    
    code[0] = (unsigned char)t->off;
    code[1] = (unsigned char)((t->off >> 8) & 0x0f);
    code[1] = code[1] | (unsigned char)((t->len << 4) & 0xf0);
    code[2] = t->next;
    
    for(i = 0; i < 3; i++)
        putc(code[i], out);
}


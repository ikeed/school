#include <stdlib.h>
#include <stdio.h>

typedef unsigned long dtype;

typedef struct tnode *tnd;
struct tnode {
	tnd children[10];
	char dig;
	long count;
};

int newNd(tnd *t, char dig)
{
	tnd nd;
	if ((nd = (tnd) calloc(1, sizeof(struct tnode))) == NULL) {
		return 0;
	}
	nd->dig = dig;
	nd->count = 0;
	*t = nd;
	return 1;
}

int getOrMakeChild(tnd t, char d, tnd *ch)
{
	if (!t->children[d] && !newNd(&t->children[d], d)) {
		return 0;
	}
	return (*ch = t->children[d]) != NULL;
}

int insert(tnd root, char *n){
	int c = n[0];
	tnd ch;

	while (1) {
		if (!getOrMakeChild(root, n[0], &ch)) {
			return 0;
		}
		if (n[1]) {
			n++;
			root = ch;
		}else {
			ch->count++;
			return 1;
		}
	}
	return 0;
}

		
		

dtype weak_rand()
{
	dtype ret = 0;
	int *p, i;
	for (i = 0; i < sizeof(ret)/sizeof(int); i++) {
		p = &((int *) &ret)[i];	
		*p = (int) rand();
	}
	return ret;
}

dtype strong_rand(int level)
{
	int bitwidth = 8 * sizeof(dtype);
	int bits_reqd = bitwidth;
	dtype r1, r2, mask, ret = 0;

	while (bits_reqd) {
		r1 = (level? strong_rand(level-1) : weak_rand());
		mask = r1 ^ (level? strong_rand(level-1) : weak_rand());
		while (mask) {
			if (mask & 0x1) {
				ret |= (r1 & 0x1);
				ret <<= 1;
				if (!(--bits_reqd)) break;
			}
			mask >>= 1;
			r1 >>= 1;
		}	
	
	}
	return mask;
}

int rip(char *p, dtype d) {
	int i = 0;
	while(d) {
		p[i++] = (d % 10) + '0';
		d/= 10;
	}
	

int main() {
	srand(time(NULL));
	char buf[40];
	int i;
	for (i = 0; i < 10000; i++) {
		rip(buf, strong_rand(1));
	}

	return 0;
}


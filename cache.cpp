#include "stdio.h"
#include "stdlib.h"
#include "time.h"
#include "string.h"

class CacheTester{
	#define MAXSIZE 0x8FFFFFF
	#define MINSIZE 1024
	#define RUNTIMES 50
	#define ITERATIONS 3000000
	#define COUNT_PAD 0
	#define SECONDS_ITERATIONS (ITERATIONS/(double)1000000000)
	private:
	    struct Node{
		   Node *next;
		   size_t pad[COUNT_PAD];
	    };
	    FILE * mFile;
	    bool cpuInfo();
	    void randomTest();
	    void sequentialTest();
	public:
	    CacheTester(FILE *f){
		mFile=f;
	    };
	    ~CacheTester(){}
	    void test();	    
};

bool CacheTester::cpuInfo() {
	FILE *cpuInfo;
	#ifdef __linux__
		cpuInfo = popen("lscpu | grep cache", "r");
	#else 
		return 0;
	#endif	
	fprintf(mFile, "System info:\n");
	char *line;
	char buf[1024];
	while ( (line = fgets(buf, sizeof buf, cpuInfo)) != NULL)  {			
		fprintf(mFile, "%s", line);
	}
	pclose(cpuInfo);
	return 1;
}

void CacheTester::randomTest() {
	FILE * rnd = fopen("rnd.csv", "w");
	printf("\nRandom access:\n");
	fprintf(mFile, "\nRandom access:\n");
	fprintf(mFile, "Bytes;NanoSeconds\n");	
	double runTime = 0;
	size_t cNodeSize = sizeof(Node);	
	size_t min = MINSIZE / cNodeSize;
	size_t max = MAXSIZE / cNodeSize;
	for(register size_t size = min; size < max; size*=2 ) {
		Node *buff = new Node[size];
		size_t *array = new size_t[size];
		register size_t i = 0;
		for(i = 0; i < size; i++) {
			array[i] = i;
		}
	        for (i = 0; i < size - 1; i++) {
        	  register size_t j = i + rand() / (RAND_MAX / (size - i) + 1);
        	  register size_t t = array[j];
        	  array[j] = array[i];
        	  array[i] = t;
        	}
		for(i = 0; i < size; i++)
			buff[i].next = &buff[array[i]];
		clock_t begin, end; 
		for (register int j = 0; j < RUNTIMES; j++) {
			begin = clock();
			Node *node = buff;
			for (i = 0; i < ITERATIONS; i++) {
				node = node->next;
			}
			end = clock();
			runTime += (double)(end - begin) / CLOCKS_PER_SEC / SECONDS_ITERATIONS; 
		}
		runTime /= RUNTIMES;		
		delete [] array;
		delete [] buff;
		fprintf(mFile, "%zdK\t %0.4f;\n", cNodeSize * size / 1024, runTime);
		fprintf(rnd, "%zdK; %0.4f;\n", cNodeSize * size / 1024, runTime);
		printf("%zdK\t ok\n", cNodeSize * size / 1024);
	}
	fclose(rnd);
}

void CacheTester::sequentialTest(){
	FILE * seq = fopen("seq.csv", "w");
	printf("\nSequential access:\n");
	fprintf(mFile, "\nSequential access:\n");
	fprintf(mFile, "Bytes; NanoSeconds\n");	
	register double runTime = 0;
	register size_t cNodeSize = sizeof(Node);
	register size_t min = MINSIZE / cNodeSize;
	register size_t max = MAXSIZE / cNodeSize;
	for(register size_t size = min; size < max; size *= 2 ) {
		Node *buff = new Node[size];
		for(register size_t i = 0; i < size - 1; i++) {
			buff[i].next = &buff[i+1];
		}
		buff[size - 1].next = &buff[0];
		register clock_t begin, end; 
		for (register int j = 0; j < RUNTIMES; j++) {
			begin = clock();
			Node *node = buff;
			for (int i = 0; i < ITERATIONS; i++)
				node = node->next;	
			end = clock();
			runTime += (double)(end - begin) / CLOCKS_PER_SEC / SECONDS_ITERATIONS;	
		}
		runTime /= RUNTIMES;
		delete [] buff;
		fprintf(mFile, "%zdK\t %0.4f;\n", cNodeSize * size / 1024, runTime);
		fprintf(seq, "%zdK; %0.4f;\n", cNodeSize * size / 1024, runTime);
		printf("%zdK\t ok\n", cNodeSize * size / 1024);
	}
	fclose(seq);
}

void CacheTester::test(){
	if (cpuInfo()){
		printf("testing...\nsize of Node=%ld\n", sizeof(Node));
		randomTest();
		sequentialTest();
		printf("test done! see res.txt\n");
		return;
	}
	printf("This test works with Linux kernel based OS only\n");
}

int main() {
	FILE *result = fopen("res.txt", "w");
	CacheTester tester(result);
	tester.test();
	fclose(result);
	return 0;
}


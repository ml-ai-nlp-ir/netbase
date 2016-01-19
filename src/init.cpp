#include <cstdlib> // malloc, exit:
#include <sys/sem.h> // sudo ipcrm -M 0x69190
//#include <errno.h>  // just use perror("blah")! Adds errorstr(errno) automagically
//#include <sys/types.h>
//#include <sys/ipc.h>
#include <sys/shm.h>

//Since using #include <stdlib.h> dumps all declared names in the global namespace,
//the preference should be to use #include <cstdlib>, unless you need compatibility with C
#include <cstdlib>
#include <string.h>

//#include <stdlib.h> // system(cmd)
#include <unistd.h> // getpid
#include <signal.h> // kill

#include "util.hpp"
#include "netbase.hpp"
#include "init.hpp"
#include "relations.hpp"

/* attach to the segment to get a pointer to it: */
//const void * shmat_root = (const void *) 0x101000000; // mac 64 bitpointer to it: */
#ifdef i386
const void * shmat_root = (const void *) 0x10000000; // just higher than system Recommendation
#else
//const void * shmat_root=(const void *) 0x300000000; // just higher than system Recommendation
//const void * shmat_root = (const void *) 0x0101000000; // just higher than system Recommendation
//const void * shmat_root=(const void *) 0x0120000000; // just higher than system Recommendation
//const void * shmat_root=(const void *) 0x021000000;// java !
const void * shmat_root=(const void *) 0x0220000000;// java !
//const void * shmat_root = (const void *) 0x0100137000;
#endif
//const void * shmat_root = (const void *)0x105800000;//test

bool virgin_memory=false;
#if defined(__FreeBSD__) || defined(__APPLE__)
/* union semun is defined by including <sys/sem.h> */
#else

/* according to X/OPEN we have to define it ourselves */
union semun {
	int val; /* Value for SETVAL */
	struct semid_ds *buf; /* Buffer for IPC_STAT, IPC_SET */
	unsigned short *array; /* Array for GETALL, SETALL */
	struct seminfo *__buf; /* Buffer fTheor IPC_INFO
	 (Linux-specific) */
};
#endif

int semrm(key_t key, int id=0) {
	union semun arg;
	id=semget(key, 0, 0);
	if (id == -1) return -1;
	return semctl(id, 0, IPC_RMID, arg);
}

// silent: in messages:
//Jun  3 15:53:30 507 abrt[13188]: abrtd is not running. If it crashed, /proc/sys/kernel/core_pattern contains a stale value, consider resetting it to 'core'
void signal_handler(int signum) {
	printf("Process %d got signal %d\n", getpid(), signum);
	signal(signum, SIG_DFL);
	kill(getpid(), signum);
}

void detach_shared_memory(){
        // TODO (?) programmatically
    p("If you cannot start netbase try:\n ./increase-shared-memory.sh && ./clear-shared-memory.sh");
//    sudo: no tty present and no askpass program specified
//    system("ipcrm -M '0x69190'");
//    system("ipcrm -M '0x69191'");
//    system("ipcrm -M '0x69192'");
//    system("ipcrm -M '0x69193'");
//    system("ipcrm -M '0x69194'");
}

void increaseShmMax(){
    // TODO (?)  programmatically
    p("If you still cannot start netbase decrease maxNodes = 30*million in netbase.hpp or adjust shmmax, see clear-shared-memory.sh");
}

void* share_memory(key_t key, long sizeOfSharedMemory, void* root, const void * desired) {
	if (root) {
//		ps("root_memory already attached!");
		return root;
	}
//	if (sizeOfSharedMemory > 2147483648) {
//		p("WARNING sizeOfSharedMemory>2147483648 2GB is limit on most systems\n");
//	}
	/* make the key: */
	//	int key = 0x69190; //0x57020303;// #netbase ftok("netbase", 'RW');
	int shmid;
	//                virgin_memory=0;
	int READ_WRITE=0666; //
	if ((shmid=shmget(key, sizeOfSharedMemory, READ_WRITE)) == -1) {
		ps("share_memory used for the first time");
		pf("requesting 0x%lx bytes\n",sizeOfSharedMemory);
		virgin_memory=1;
		if ((shmid=shmget(key, sizeOfSharedMemory, READ_WRITE | IPC_CREAT)) == -1) {
			semrm(key); // clean and try again
			if ((shmid=shmget(key, sizeOfSharedMemory, READ_WRITE | IPC_CREAT)) == -1) {
				perror("share_memory failed!\nSize changed or NOT ENOUGH MEMORY??\n shmget");
				//			printf("try calling ./clear-shared-memory.sh\n");
				//			perror(strerror(errno)); <= ^^ redundant !!!
				printf("try ipcclean && sudo ipcrm -M 0x69190 ... \n./clear-shared-memory.sh\n");
                detach_shared_memory();
                increaseShmMax();
			}
			if ((shmid=shmget(key, sizeOfSharedMemory, READ_WRITE | IPC_CREAT)) == -1) {
				perror("share_memory failed: shmget! Not enough memory?");
				exit(1);
			}
		}
	}
	root=(char *) shmat(shmid, (const void *) desired, 0);
	if (root == 0 || root == (void *) (-1)) { //virgin_memory=1;
		ps("receiving other share_memory address");
		root=(char *) shmat(shmid, (const void *) 0, 0); //void
	}
	if (root == 0 || root == (void *) (-1)) {
		perror("share_memory failed: shmat! Not enough memory?");
		exit(1);
	}
//	Context* c=currentContext(); // getContext(node->context);
	if ((char*) root != (char*) desired) { // 64 BIT : %x -> %016llX
		printf("FYI: root_memory != desired shmat_root %p!=%p \n", root, desired);
//		fixPointers();
	}
	printf("share_memory at %p	size = %zX	max = %p\n", root, sizeOfSharedMemory, (char*) root + sizeOfSharedMemory);
	return root;
}

long getMemory() {
	//long phypz = sysconf(_SC_PHYS_PAGES);
	//long psize = sysconf(_SC_PAGE_SIZE);
	//return phypz*psize;
	return 0;
}

long GetAvailableMemory(void) {
	void *p, *q;
	long siz=10000000;
	q=p=calloc(1, siz);
	while (q) {
		siz=siz * 1.2; // Can be more to speed up things
		q=realloc(p, siz);
		if (q) // infinite virtual mem :{ 31107287834197
			p=q;
	}
	free(p);
	return siz;
}

// modify char* in vivo / inline!

void initRootContext() {
	Context* rootContext=(Context*) context_root;
	memset(contexts, 0, contextOffset); // ? why only?
	initContext(rootContext);
	//	if(rootContext)
	strcpy(rootContext->name, "ROOT CONTEXT");
//	rootContext->id=1; // not virgin_memory any more
	//	rootContext->nodes=(Node*)&context_root[contextOffset];
	//	rootContext->statements=(Statement*)&context_root[contextOffset+nodeSize * maxNodes];
	rootContext->nodes=(Node*) node_root;
	rootContext->statements=(Statement*) statement_root;
	rootContext->nodeNames=name_root;
}

void checkRootContext() {
	Context* rootContext=(Context*) context_root;
	if (rootContext->nodeCount==0) {
		p("STARTING WITH CLEAN MEMORY");
		initRootContext();
//        clearMemory();
		return;
	}

	p("USING SHARED MEMORY");
////	if (rootContext->nodes != (Node*) node_root) {	//  &context_root[contextOffset]) {
//	if (currentContext()->nodes != (Node*) node_root) {	//  &context_root[contextOffset]) {
//		p("rootContext->nodes != (Node*) node_root");
//		pf("%X != %X\n",rootContext->nodes,node_root);
//		showContext(rootContext);
//		
////		currentContext()->nodes=rootContext->nodes;	// hack
//	}
    if (currentContext()->nodeNames!=name_root)
        fixPointers();
    rootContext->nodes=(Node*) node_root;
    rootContext->statements=statement_root;
//	else if (currentContext()->nodes != rootContext->nodes) {
//		fixPointers();
//		currentContext()->nodes=rootContext->nodes;
//	}
}

extern "C" void init(bool relations) {
	signal(SIGSEGV, signal_handler);

    if(!relations)testing=true;
	//    if ((i = setjmp(try_context)) == 0) {// try once
	int key=0x69190;
	char* root=(char*) shmat_root;
	long context_size=contextOffset;
	long node_size=maxNodes * nodeSize;
	long abstract_size=maxNodes * ahashSize * 2;
	long name_size=maxNodes * averageNameLength;
	long statement_size=maxStatements * statementSize;
//	node_root=&context_root[contextOffset];
//	p("abstract_root:");
	abstract_root=(Node*) share_memory(key , abstract_size * 2, abstract_root,root);//  ((char*) context_root) + context_size
//	p("name_root:");
	name_root=(char*) share_memory(key + 1, name_size, name_root, ((char*) abstract_root) + abstract_size * 2);
//	p("node_root:");
	node_root=(Node*) share_memory(key + 2, node_size, node_root, name_root + name_size);
//	p("statement_root:");
	statement_root=(Statement*) share_memory(key + 3, statement_size, statement_root, ((char*) node_root) + node_size);
//	p("keyhash_root:");
//	short ns = sizeof(Node*); // ;
//	keyhash_root = (Node**) share_memory(key + 5, 1 * billion * ns, keyhash_root, ((char*) statement_root) + statement_size);
//   	p("context_root:");
	context_root=(Context*) share_memory(key+4, context_size, context_root,  ((char*) statement_root) + statement_size);

//	freebaseKey_root=(int*) share_memory(key + 5, freebaseHashSize* sizeof(int), freebaseKey_root, ((char*) statement_root) + statement_size);

	abstracts=(Ahash*) (abstract_root); // reuse or reinit
	extrahash=(Ahash*) &abstracts[maxNodes]; // (((char*)abstract_root + abstractHashSize);
	contexts=(Context*) context_root;
	checkRootContext();
	getContext(current_context);
    if(relations){
	initRelations();
	if (currentContext()->nodeCount < 1000)
        currentContext()->nodeCount=1000;
    }
}

void fixNodes(Context* context, Node* oldNodes) {
#ifndef explicitNodes
	return;
#else
	int max=context->statementCount; // maxStatements;
	for (int i=0; i < max; i++) {
		Statement* n=&context->statements[i];

		if (!checkStatement(n)) {
			showStatement(n);
			continue;
		}
		n->Subject=&context->nodes[n->subject];
		n->Predicate=&context->nodes[n->predicate];
		n->Object=&context->nodes[n->object];
	}
#endif
}
FILE* open_binary(cchar* c){
	printf("Opening File %s\n", (data_path + c).data());
     FILE* fp= fopen((data_path + c).data(), "rb");
    if(fp==NULL)perror("Error opening file");
    return fp;
}

void load(bool force) {

//	clock_t start=clock();
//	double diff;
	//  diff = ( std::clock() - start ) / (double)CLOCKS_PER_SEC;

	Context* c=currentContext();
	//    showContext(c->id);
	Node* oldNodes=c->nodes;
	char* oldnodeNames=c->nodeNames;
	oldnodeNames=initContext(c);

	//  #include <sys/stat.h>
	//  struct stat stFileInfo;
	//  int intStat = stat((path+ "contexts.bin").data(),&stFileInfo);
	//  if(intStat != 0) { p("file not found");}
	//
	if (!force && context_root) {	//&&root_memory[1000]!=0){// first id==0
		//        pi(root_memory[0]);
		ps("loaded from shared memory");
//		if (virgin_memory || !hasWord("instance"))
//			collectAbstracts(); //or load abstracts->bin
		showContext(wordnet);
		return;
	}

	ps("Loading graph from files!");

	FILE *fp=open_binary("contexts.bin");
	if (fp== NULL) {
		p("starting with fresh context!");
		clearMemory();
		return;
		//        exit(1);
	} else {
		fread(contexts, sizeof(Context), maxContexts, fp);
		fclose(fp);
	}
	//    fp=fopen("wordnet.bin", "rb");
	//    fread(c, sizeof(Context), 1, fp);
	//    fclose (fp);

    
	fp=open_binary("names.bin");
	fread(c->nodeNames, sizeof(char), c->currentNameSlot + 100, fp);
	fclose(fp);

	fp=open_binary( "statements.bin");
	fread(c->statements, sizeof(Statement), c->statementCount, fp); //c->statementCount maxStatements
	fclose(fp);

    fp=open_binary( "nodes.bin");
	fread(c->nodes, sizeof(Node), c->nodeCount, fp);
   	fclose(fp);

	//
	//    fp = fopen((path+"abstracts->bin").data(), "rb");
	//    fread(abstracts, sizeof (Node), c->nodeCount, fp);

	//    showContext(c);

#ifndef inlineName
	if (oldnodeNames != c->nodeNames) {
		p("Fixing nodeNames");
		fixNodeNames(c, oldnodeNames);
		//        init();// to get back relations CAREFUL!
	}
#endif

	if (oldNodes != c->nodes) {
		p("Fixing nodes");
		fixNodes(c, oldNodes);
	}

	collectAbstracts(); //or load abstracts->bin
	showContext(currentContext());

	//    initContext(c);//
	//
	//
	//    fp=fopen("test.bin", "rb");
	//    if(fp)fread(test, sizeof(char), 100, fp);
	//   fclose (fp);

	//  cout<<"nanoseconds "<< diff <<'\n';
}

void fixPointers() {
    if(currentContext()->nodeNames==name_root)
        return;// all INT now, no more pointers!!! except chars
	p("ADJUSTING SHARED MEMORY");
	Context* context=currentContext();
//	if(!checkC)
	//	showContext(context);
	fixPointers(context);
	//	showContext(context);
	//	context = currentContext();// &contexts[wordnet]; // todo: all
	//	showContext(context);
	//	fixPointers(context);
	showContext(context);
	collectAbstracts();
}

void fixPointers(Context* context) {
	p("ADJUSTING Context");
	Node* oldNodes=context->nodes;
	char* oldNames=context->nodeNames;
	initContext(context);
//	check((char*) context->nodes == context_root + contextOffset);
	//	context->nodes=root_memory
	fixNodes(context, oldNodes);
	fixNodeNames(context, oldNames);
}

int collectAbstracts() {
	ps("collecting abstracts");// int now
	initRelations();
	memset(abstracts, 0, maxNodes*ahashSize * 2);
	Context* c=currentContext();
	int max=c->nodeCount; // maxNodes;
	int count=0;
	// collect Abstracts
	for (int i=0; i < max; i++) {
		Node* n=&c->nodes[i];
		if (i > 1000 && !checkNode(n)) break;
		if (n == null || n->name == null || n->id == 0) continue;
		if (n->kind == Abstract->id) {
			insertAbstractHash(n);
			count++;
		}
	}
	return count;
}

int collectInstances() {
	ps("collecting instances");// int now
	initRelations();
	memset(abstracts, 0, maxNodes*ahashSize * 2);
	Context* c=currentContext();
	int max=c->nodeCount; // maxNodes;
	int count=0;
	// collect Abstracts
	for (int i=0; i < max; i++) {
		Node* n=&c->nodes[i];
		if (i > 1000 && !checkNode(n)) break;
		if (n == null || n->name == null || n->id == 0) continue;
		if (n->kind == Abstract->id) {
			insertAbstractHash(n);
			count++;
		}else{
			addStatement(getAbstract(n->name),Instance,n,false);
		}
	}
	return count;
}

void fixNodeNames(Context* context, char* oldnodeNames) {
#ifdef inlineName
	printf("inlineNames!");
	return;
#else
	long newOffset=context->nodeNames - oldnodeNames;
	if(newOffset==0)return;
	int max=context->nodeCount; // maxNodes;
	for (int i=0; i < max; i++) {
		Node* n=&context->nodes[i];
		//		show(n,true);
		if (!checkNode(n,i,0,0)) continue;
//		n->name=newOffset;
        n->name=n->name+newOffset;
	}
#endif
}

bool clearMemory() {
    if (!virgin_memory) {
		ps("Cleansing Memory!");
        detach_shared_memory();
        init();
        //		if (!node_root) memset(context_root, 0, sizeOfSharedMemory);
        // EXPENSIVE on big machines!! like: 10 minutes!!
//		else {
//			memset(context_root, 0, contextOffset);
//			memset(node_root, 0, nodeSize * maxNodes); //calloc!
//			memset(statement_root, 0, statementSize * maxStatements);
//			memset(name_root, 0, maxNodes * averageNameLength);
//			memset(abstracts, 0, abstractHashSize * 2);
//			//		memset(extrahash, 0, abstractHashSize / 2);
//		}
        virgin_memory=false;
	}
    initRootContext();
    if(testing){
    memset(node_root+1000, 0, 100000); // for testing
    memset(statement_root, 0, 100000); // for testing
    memset(name_root, 0, 100000); // for testing
	memset(abstract_root, 0, maxNodes*ahashSize * 2);
//	memset(abstracts, 0, maxNodes*ahashSize * 2);
    currentContext()->nodeCount=1000;// 0 = ANY
    currentContext()->nodeNames=name_root;
    currentContext()->statementCount=1;// 0 = ERROR
    }
//    if(!testing)
	initRelations();
	return true;
	//		Context* context=currentContext();
}

char* initContext(Context* context) {
	printf("Initiating context %d\n", context->id);
	Node* nodes=0;
	Statement* statements=0;
	char* nodeNames=0;
//	int nodeSize=sizeof(Node); // 40
//	int statementSize=sizeof(Statement); //
//	int ahashSize=sizeof(Ahash); //
	//    int contextOffset=sizeof (Context) *maxContexts;
	long nameSegmentSize=sizeof(char) * averageNameLength * maxNodes;
	long nodeSegmentSize=nodeSize * maxNodes;
	long statementSegmentSize=statementSize * maxStatements;
//	long abstractOffset=contextOffset + nodeSegmentSize + nameSegmentSize + statementSegmentSize; //just put them at the end!!
	if (node_root) {
		p("Multiple shared memory segments");
		nodes=(Node*) node_root;
		nodeNames=(char*) name_root;
		statements=(Statement*) statement_root;
	} else if (context_root) {
		p("ONE shared memory segment");
		if (contextOffset + nodeSegmentSize + nameSegmentSize + statementSegmentSize > sizeOfSharedMemory + ahashSize * 2) { //
			ps("ERROR sizeOfSharedMemory TOO SMALL!");
			ps("contextOffset+nodeSegmentSize+nameSegmentSize+statementSegmentSizeabstractSegment > sizeOfSharedMemory !");
			p(contextOffset + nodeSegmentSize + nameSegmentSize + statementSegmentSize);
			p(sizeOfSharedMemory);
			exit(1);
		}
		nodes=(Node*) (context_root + contextOffset);
		nodeNames=(char*) &context_root[contextOffset + nodeSegmentSize];
		statements=(Statement*) &context_root[contextOffset + nodeSegmentSize + nameSegmentSize];
	} else do {
		p("malloc memory segments");
		statements=(Statement*) malloc(statementSegmentSize + 1);
		nodes=(Node*) malloc(nodeSegmentSize + 1);
		nodeNames=(char*) malloc(nameSegmentSize + 1);
		//        nodeNames=(char*)malloc(sizeof(char)*nameBatch);// incremental
		if (nodes == 0 || statements == 0 || nodeNames == 0) {
			ps("System has not enough memory to support ");
			printf("%ld Nodes and %ld Statements\nDividing by 2 ...\n", maxNodes, maxStatements);
			maxStatements=maxStatements / 2;
			maxNodes=maxNodes / 2;
			// negotiate memory cleverly!!
		}
	} while (nodes == 0 || statements == 0);
//	if (!context_root || virgin_memory) clearMemory();
//	Statement* oldstatements=context->statements;
	char* oldnodeNames=context->nodeNames;
//	Node* oldnodes=context->nodes;
	context->nodes=nodes;
	context->statements=statements;
	context->nodeNames=nodeNames;
	if(context->statementCount==0)// ??
		context->statementCount=1;// 0 = error
//	px(context);
//	px(nodes);
//	px(nodeNames);
//	px(statements);
//	px(abstracts);
//	px(extrahash);
	//		check((Node*)nodeNames==&context->nodes[maxNodes]);// statements == bounds of nodes
	return oldnodeNames;
}

#ifdef sharedLib
/* The shared library's read-only segment (in particular, its .text section) can be shared among all processes;
 * while its write-able sections (such as .data and .bss) can be allocated uniquely for each executing process.
 * This write-able segment is also referred to as the object's Static Data Segment.
 * It is this static data segment that creates most of the complexity for the implementation of shared libraries.
 http://www.cadenux.com/xflat/NoMMUSharedLibs.html#shlibs  */
//#define sharedLib
void __attribute__((constructor)) my_init(void) {
	if (debug)
	printf("loading shared library \n");
	share_memory();
	if (context_root[0] != 1)
	buildDictionary(); //4sec.
	else if (debug)
	printf("attached to shared memory of library libDictionary.so \n");
	// saveDictionary();
	// loadDictionary();
	return 0;
}
#endif


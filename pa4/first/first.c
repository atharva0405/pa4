#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

typedef struct Node {
  unsigned long int index;
  unsigned long int tag;
  struct Node *next;
} cache_t;


unsigned long int cacheSize;
char* asso;
char* replace;
unsigned long int blockSize;
unsigned long int numSets;
unsigned long int assocVal;
unsigned long int indexBit;
unsigned long int offsetBit;
unsigned long int tagBit;
unsigned long int cacheHits;
unsigned long int cacheMisses;
unsigned long int memReads;
unsigned long int memWrites;
unsigned long int cacheHitspre;
unsigned long int cacheMissespre;
unsigned long int memReadspre;
unsigned long int memWritespre;
unsigned long int n;//
cache_t** A;
cache_t** B;


cache_t* insertNode(cache_t* front, unsigned long int tagThis, unsigned long int ind);
int checkNode(cache_t* front, unsigned long int tagThis);
cache_t* updateLRU(cache_t* front, unsigned long int tagThis);
cache_t* replaceNodeDirect(cache_t* front, unsigned long tagThis, unsigned long int ind);
cache_t* replaceNodeLRU(cache_t* front, unsigned long tagThis, unsigned long int ind, unsigned long int count);
void noprefetch(cache_t* front, unsigned long int tagThis, unsigned long int ind,char* rw, char* asso, char* replace,unsigned long int n);
void prefetch(cache_t* front, unsigned long int tagThis, unsigned long int ind, char* rw, char* asso, char* replace, unsigned long int n,unsigned long int tagThispre, unsigned long int indpre);
void print(unsigned long int cacheHits, unsigned long int cacheMisses, unsigned long int memReads, unsigned long int memWrites, unsigned long int cacheHitspre, unsigned long int cacheMissespre, unsigned long int memReadspre, unsigned long int memWritespre);


cache_t* insertNode(cache_t* front, unsigned long int tagThis, unsigned long int ind) {
  cache_t* ptr = front;
  cache_t* current = (cache_t*)malloc(sizeof(cache_t));
  current->index = ind;
  current->tag = tagThis;
  current->next = NULL;

  if(front == NULL) {
    front = current;
    return front;
  }
  while(ptr->next != NULL) {
    ptr = ptr->next;
  }
  ptr->next = current;
  return front;
}


int checkNode(cache_t* front, unsigned long int tagThis) {
  cache_t* ptr = front;
  while(ptr != NULL) {
    if(ptr->tag == tagThis) {
      return 1;
    }
    ptr = ptr->next;
  }
  return 0;
}


cache_t* updateLRU(cache_t* front, unsigned long int tagThis) {
  cache_t* ptr = front;
  cache_t* prev = NULL;
  while(ptr != NULL) {
    if(ptr->tag == tagThis) {
      if(prev == NULL)  {
        break;
      }
      else if(ptr->next == NULL) {
        cache_t* temp = ptr;
        prev->next = NULL;
        temp->next = front;
        front = temp;
        break;
      }
      else {
        cache_t* temp = ptr;
        prev->next = ptr->next;
        temp->next = front;
        front = temp;
        break;
      }
    }
    else {
      prev = ptr;
      ptr = ptr->next;
    }
  }
  return front;
}

cache_t* replaceNodeDirect(cache_t* front, unsigned long tagThis, unsigned long int ind) {
  cache_t* ptr = front;
  cache_t* current = (cache_t*)malloc(sizeof(cache_t));
  current->index = ind;
  current->tag = tagThis;
  current->next = NULL;
  front = current;
  free(ptr);
  return front;
}


cache_t* replaceNodeLRU(cache_t* front, unsigned long tagThis, unsigned long int ind, unsigned long int count) {
  unsigned long int len = 0;
  cache_t* ptr = front;
  cache_t* prev = NULL;
  cache_t* current = (cache_t*)malloc(sizeof(cache_t));
  current->index = ind;
  current->tag = tagThis;
  current->next = NULL;

  while(ptr != NULL) {
    len++;
    ptr = ptr->next;
  }
  ptr = front;
  if(len +1 <= count) {
    current->next = front;
    front = current;
  } else {
    while(ptr->next != NULL) {
      prev = ptr;
      ptr = ptr->next;
    }
    cache_t* temp = ptr;
    prev->next = NULL;
    free(temp);
    current->next = front;
    front = current;
  }
  return front;
}


void noprefetch(cache_t* front, unsigned long int tagThis, unsigned long int ind,char* rw, char* asso, char* replace, unsigned long int n) {
  int ret = 0;
  int count = n;
  if(strcmp(asso,"direct")==0) {
    if(strcmp(rw,"R")==0) {
      if(front == NULL) {
	       cacheMisses++;
         memReads++;
	       front = insertNode(front,tagThis,ind);
	    }
      else {
	       ret = checkNode(front,tagThis);
         if(ret == 1) {
	          cacheHits++;
	       }
         else {
	          cacheMisses++;
	          memReads++;
	          front = replaceNodeDirect(front,tagThis,ind);
	       }
	    }
    }
    else {
	     ret = checkNode(front,tagThis);
	      if(ret == 1) {
	         cacheHits++;
	          memWrites++;
	      }
        else {
	         cacheMisses++;
	         memReads++;
	         if(front == NULL) {
	            front = insertNode(front,tagThis,ind);
	         }
           else {
	            front = replaceNodeDirect(front,tagThis,ind);
	         }
	         memWrites++;
	      }
    }
  }
  else {
      if(strcmp(rw,"R")==0) {
        ret = checkNode(front,tagThis);
        if(ret ==1) {
          cacheHits++;
	        front= updateLRU(front,tagThis);
	      }
        else {
	        cacheMisses++;
	        memReads++;
	        if(front == NULL) {
	          front = insertNode(front,tagThis,ind);
	        }
          else {
	          front = replaceNodeLRU(front,tagThis,ind,count);
          }
	      }
      }
      else {
	      ret = checkNode(front,tagThis);
	      if(ret == 1) {
	        front = updateLRU(front,tagThis);
	        cacheHits++;
	        memWrites++;
	      }
        else {
	        cacheMisses++;
	        memReads++;
	        if(front == NULL) {
	          front = insertNode(front,tagThis,ind);
	        }
          else {
	          front= replaceNodeLRU(front,tagThis,ind,count);
	        }
	        memWrites++;
	      }
      }
  }
  A[ind] = front;
}


void prefetch(cache_t* front, unsigned long int tagThis, unsigned long int ind, char* rw, char* asso, char* replace, unsigned long int n, unsigned long int tagThispre, unsigned long int indpre) {
  int ret = 0;
  if(strcmp(asso,"direct")==0) {
    if(strcmp(rw,"R")==0) {
	    if(front == NULL) {
	      cacheMissespre++;
	      memReadspre++;
	      front = insertNode(front,tagThis,ind);
	    }
      else {
	      ret = checkNode(front,tagThis);
	      if(ret == 1) {
	        cacheHitspre++;
	        return;
	      }
        else {
	        cacheMissespre++;
	        memReadspre++;
	        front = replaceNodeDirect(front,tagThis,ind);
	      }
	    }
	    B[ind] = front;
	    ret = checkNode(B[indpre],tagThispre);
	    if(ret == 0) {
	      memReadspre++;
	      if(B[indpre] == NULL) {
	        B[indpre] = insertNode(B[indpre],tagThispre,indpre);
	      }
        else {
	        B[indpre] = replaceNodeDirect(B[indpre],tagThispre,indpre);
	      }
	    }
      }
      else {
        ret = checkNode(front,tagThis);
	      if(ret == 1) {
	        cacheHitspre++;
	        memWritespre++;
	        return;
	      }
        else {
	        cacheMissespre++;
	        memReadspre++;
	        if(front == NULL) {
	          front = insertNode(front,tagThis,ind);
	        }
          else {
	          front = replaceNodeDirect(front,tagThis,ind);
	        }
	        memWritespre++;
	      }
	      B[ind]=front;
	      ret = checkNode(B[indpre],tagThispre);
	      if(ret == 0) {
	        memReadspre++;
	        if(B[indpre] == NULL) {
	          B[indpre] = insertNode(B[indpre],tagThispre, indpre);
	        }
          else {
	          B[indpre] = replaceNodeDirect(B[indpre],tagThispre,indpre);
	        }
	      }
      }
    }
    else {
      if(strcmp(rw,"R")==0) {
	      ret = checkNode(front,tagThis);
	      if(ret ==1) {
	        cacheHitspre++;
	        front = updateLRU(front,tagThis);
	        B[ind]=front;
	        return;
	      }
        else {
	        cacheMissespre++;
	        memReadspre++;
	        if(front == NULL) {
	          front = insertNode(front,tagThis,ind);
	        }
          else {
	          front = replaceNodeLRU(front,tagThis,ind,n);
	        }
	      }
	      B[ind]=front;
	      ret = checkNode(B[indpre],tagThispre);
	      if(ret == 0) {
	        memReadspre++;
	        if(B[indpre] == NULL) {
	          B[indpre] = insertNode(B[indpre],tagThispre,indpre);
	        }
          else {
	          B[indpre] = replaceNodeLRU(B[indpre],tagThispre,indpre,n);
          }
	      }
      }
      else {
	      ret = checkNode(front,tagThis);
	      if(ret == 1) {
	        cacheHitspre++;
	        memWritespre++;
	        front = updateLRU(front,tagThis);
	        B[ind] = front;
	        return;
	      }
        else {
	        cacheMissespre++;
	        memReadspre++;
	        if(front == NULL) {
	          front = insertNode(front,tagThis,ind);
	        }
          else {
	          front = replaceNodeLRU(front,tagThis,ind,n);
	        }
	        memWritespre++;
	      }
	      B[ind] = front;
	      ret = checkNode(B[indpre],tagThispre);
	      if(ret == 0) {
	        memReadspre++;
	        if(B[indpre] == NULL) {
	          B[indpre] = insertNode(B[indpre],tagThispre,indpre);
	        }
          else {
	          B[indpre] = replaceNodeLRU(B[indpre],tagThispre, indpre,n);
	        }
	      }
      }
    }
}


void print(unsigned long int cacheHits, unsigned long int cacheMisses, unsigned long int memReads, unsigned long int memWrites, unsigned long int cacheHitspre, unsigned long int cacheMissespre, unsigned long int memReadspre, unsigned long int memWritespre) {

  printf("no-prefetch\n");
  printf("Memory reads: %lu\n",memReads);
  printf("Memory writes: %lu\n",memWrites);
  printf("Cache hits: %lu\n", cacheHits);
  printf("Cache misses: %lu\n",cacheMisses);
  printf("with-prefetch\n");
  printf("Memory reads: %lu\n", memReadspre);
  printf("Memory writes: %lu\n",memWritespre);
  printf("Cache hits: %lu\n",cacheHitspre);
  printf("Cache misses: %lu\n",cacheMissespre);

}


int main (int argc, char* argv[]) {
  cacheSize = 0;
  blockSize = 0;
  FILE *traceFile = NULL;
  numSets = 0;
  assocVal=0;
  cacheHits=0;
  cacheMisses=0;
  memReads=0;
  memWrites=0;
  cacheHitspre = 0;
  cacheMissespre = 0;
  memReadspre = 0;
  memWritespre = 0;
  n=0;

  sscanf(argv[1],"%lu",&cacheSize);
  asso = argv[2];
  replace = argv[3];
  sscanf(argv[4],"%lu",&blockSize);

  if(strcmp(asso,"direct")==0) {
    numSets = cacheSize/blockSize;
  }
  else if(strcmp(asso,"assoc")==0) {
    numSets = 1;
    n = cacheSize/blockSize;
  }
  else {
    int j;
    int len = strlen(asso);
    char* try;
    try = (char*)malloc((len-6+1)*sizeof(char));
    for(j=6;j<len;j++) {
      try[j-6] = asso[j];
    }
    n = strtol(try,NULL,10);
    numSets = cacheSize/(blockSize*n);
  }
  assocVal = cacheSize/(numSets*blockSize);
  indexBit = log(numSets)/log(2);
  offsetBit = log(blockSize)/log(2);
  tagBit = 48 - indexBit - offsetBit;
  int maskIndex  = (1<<indexBit)-1;
  traceFile = fopen(argv[5],"r");
  A = (cache_t**)malloc(numSets*sizeof(cache_t*));
  int i;
  for(i = 0; i < numSets; i++) {
    A[i] = NULL;
  }
  B = (cache_t**)malloc(numSets*sizeof(cache_t*));
  for(i=0;i<numSets;i++) {
    B[i]=NULL;
  }
  char* empty;
  empty = (char*)malloc(20*sizeof(char));
  char* rw;
  rw = (char*)malloc(2*sizeof(char));
  unsigned long int address;

  while(!feof(traceFile)) {
    fscanf(traceFile,"%s %s %lx",empty,rw,&address);
    if(strcmp(empty,"#eof")==0) {
      break;
    }
    unsigned long int tag = (address >> (offsetBit + indexBit));
    unsigned long int ind = (address >> offsetBit) & maskIndex;
    noprefetch(A[ind],tag,ind,rw,asso,replace,n);
    unsigned long int tagpre = (address + blockSize) >> (offsetBit + indexBit);
    unsigned long int indpre = ((address + blockSize) >> offsetBit) & maskIndex;
    prefetch(B[ind],tag,ind,rw,asso,replace,n,tagpre,indpre);
  }
  print(cacheHits, cacheMisses, memReads, memWrites, cacheHitspre, cacheMissespre, memReadspre, memWritespre);
  fclose(traceFile);
  return 0;
}

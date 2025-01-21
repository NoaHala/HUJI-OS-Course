#include "VirtualMemory.h"
#include "PhysicalMemory.h"

#define EMPTY_CELL_VALUE 0
#define ROOT_INDEX 0
#define SUCCESS 1
#define FAILURE 0

void clearFrame(uint64_t frameID){
  // page size == frame size, so it a for loop for each row in the frame
  for (int i = 0; i < PAGE_SIZE; i++)
  {
    PMwrite (frameID * PAGE_SIZE + i, EMPTY_CELL_VALUE);
  }
}

/* Initialize the virtual memory */
void VMinitialize(){
  //clear frame 0
  clearFrame(0);
}

word_t getCurAddrPart(uint64_t virtualAddress, int i){
  // gets the relevant part of the Virtual Address.
  // shifts the virtual address' bits to the right until only OFFSET_WIDTH
  // (AKA the number of bits that represent a single level) remained.
  int numOfBitsToShift = (VIRTUAL_ADDRESS_WIDTH - ((i+1) * OFFSET_WIDTH));
  word_t maskForOnlyRelevantDigits = (1 << OFFSET_WIDTH) - 1;
  word_t curAddressPart =
      (virtualAddress >> numOfBitsToShift) & maskForOnlyRelevantDigits;

  return curAddressPart;
}

bool isAddressValid(uint64_t virtualAddress){
  return VIRTUAL_MEMORY_SIZE > virtualAddress;
}

void dfsOnPageTablesTree(word_t curNodeLevel,
                         word_t curNodeFrameIndex,
                         word_t parentFrameIndex,
                         word_t* emptyTableIndex,
                         word_t* maxFrameIndexInTree,
                         uint64_t pageToSwapIn,
                         uint64_t pageToSwapOut,
                         word_t* maxDist,
                         word_t* maxDistPage,
                         word_t* maxDistFrame){

  //check for maximal index
  if (curNodeFrameIndex > *maxFrameIndexInTree){
    *maxFrameIndexInTree = curNodeFrameIndex;
  }

  //if it's a max-level-leaf AKA containing a page
  if(curNodeLevel == TABLES_DEPTH){
    word_t distBeforeCyclic = (pageToSwapIn > pageToSwapOut) ?
        (pageToSwapIn - pageToSwapOut) : (pageToSwapOut - pageToSwapIn);

    word_t pageDistance = (distBeforeCyclic > (NUM_PAGES - distBeforeCyclic)) ?
                          (NUM_PAGES - distBeforeCyclic) : distBeforeCyclic;
    if (pageDistance > *maxDist)
    {
      *maxDist = pageDistance;
      *maxDistPage = (word_t) pageToSwapOut;
      *maxDistFrame = curNodeFrameIndex;
    }
    return;
  }

  // loop on inside frame
  bool isThisFrameEmpty = true;
  for (int entry = 0; entry < PAGE_SIZE; entry++)
  {
    word_t value = 0;
    PMread (curNodeFrameIndex * PAGE_SIZE + entry, &value);
    if (value != 0){
      isThisFrameEmpty = false;
      dfsOnPageTablesTree (curNodeLevel + 1,
                           value,
                           parentFrameIndex,
                           emptyTableIndex,
                           maxFrameIndexInTree,
                           pageToSwapIn,
                           (pageToSwapOut << OFFSET_WIDTH) + entry,
                           maxDist,
                           maxDistPage,
                           maxDistFrame);
    }
  }

  // if found empty & in not a parent
  if (isThisFrameEmpty &
  (curNodeFrameIndex != parentFrameIndex) &
  (curNodeFrameIndex < *emptyTableIndex))
  {
    *emptyTableIndex = curNodeFrameIndex;
  }
}

void cleanTreeFromFrame (int depth,word_t frame, word_t father)
{
  for (int entry = 0; entry < PAGE_SIZE; ++entry)
  {
    word_t nextAddr;
    PMread (father * PAGE_SIZE + entry, &nextAddr);

    if (nextAddr != 0){
      if (nextAddr == frame)
      {
        PMwrite (father * PAGE_SIZE + entry, EMPTY_CELL_VALUE);
        return;
      }

      if (depth+1 < TABLES_DEPTH){
        cleanTreeFromFrame (depth + 1,frame, nextAddr );
      }
    }
  }
}

word_t getAvailableFrame(uint64_t parentFrameIndex, uint64_t pageToSwapIn, uint64_t pageToSwapOut){

  // set variables for the result of the DFS
  word_t emptyTableIndex = NUM_FRAMES;
  word_t maxFrameIndexInTree = 0;
  word_t maxDist = 0;
  word_t maxDistPage= 0;
  word_t maxDistFrame = 0;

  //DFS on the tree
  dfsOnPageTablesTree(0, 0, parentFrameIndex,
                      &emptyTableIndex, &maxFrameIndexInTree, pageToSwapIn,
                      pageToSwapOut,
                      &maxDist, &maxDistPage, &maxDistFrame);

  // option 1: if there is a frame containing an empty table
  if (emptyTableIndex != NUM_FRAMES)
  {
    return emptyTableIndex;
  }

  // option 2: if there is an unused frame
  if (maxFrameIndexInTree < NUM_FRAMES - 1)
  {
    return maxFrameIndexInTree + 1;
  }

  // option 3: if all frames are already used - evict
  PMevict (maxDistFrame, maxDistPage);
  return maxDistFrame;
}

word_t getPhysicalAddress(uint64_t virtualAddress){
  word_t curAddressPart;
  word_t prevLoopAddress = 0;
  word_t curLoopAddress = 0;

  // loops through all the hierarchical tables
  for (int i = 0; i < TABLES_DEPTH; i++)
  {
    curAddressPart = getCurAddrPart (virtualAddress , i);
    PMread (prevLoopAddress * PAGE_SIZE + curAddressPart, &curLoopAddress);

    if (curLoopAddress == 0){
      // if not exist find frame to put it in
      curLoopAddress = getAvailableFrame(prevLoopAddress,
                                         virtualAddress >> OFFSET_WIDTH,
                                         0);
      cleanTreeFromFrame (0, curLoopAddress, ROOT_INDEX );
    if (i < TABLES_DEPTH - 1){
      clearFrame (curLoopAddress);
    }

      // put the relevant information in the new frame
      PMwrite (prevLoopAddress * PAGE_SIZE + curAddressPart, curLoopAddress);
    }
    prevLoopAddress = curLoopAddress;
  }

  PMrestore(curLoopAddress, virtualAddress >> OFFSET_WIDTH);

  // return final address
  curAddressPart = getCurAddrPart (virtualAddress , TABLES_DEPTH);
  return prevLoopAddress * PAGE_SIZE + curAddressPart;
}


/* writes a word to the given virtual address */
int VMwrite(uint64_t virtualAddress, word_t value){

  //check address valid
  if (!isAddressValid(virtualAddress)){
    return FAILURE;
  }

  word_t finalAddress = getPhysicalAddress(virtualAddress);
  PMwrite(finalAddress, value);
  return SUCCESS;
}

/* reads a word from the given virtual address
 * and puts its content in *value. */
int VMread(uint64_t virtualAddress, word_t* value){

  //check address valid
  if (!isAddressValid(virtualAddress)){
    return FAILURE;
  }

  word_t finalAddress = getPhysicalAddress(virtualAddress);
  PMread(finalAddress, value);
  return SUCCESS;
}

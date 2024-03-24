#include "Heap.hpp"
#include "Memory.hpp"
#include <stdio.h>
#include <stdlib.h>

#define COHERENCY_CHECK_DEBUG
//whenever doing stuff, clean up and zero out the old memory TODO
#define CLEANUP_DEBUG


STGHeap::STGHeap()
{
	//TODO: do this properly ?
	
	//size_t pc = 1;
	//PageHeader* ph = ((PageHeader*)p_alloc(pc));
	//ph->sizeBytes = p_size() * pc;
	//pd_head = pd_tail = ph;

	//this should handle everything above
	create_and_init_page(p_size(), true);
}

STGHeap::~STGHeap()
{
	printf("deconstructor");
	//TODO: walk through the page directory dll and free all the pages.
	//anything else????
	PageHeader* c = pd_head;
	while (c != nullptr)
	{
		PageHeader* cc = c;
		c = c->next;
		p_free(cc, cc->sizeBytes / p_size());
	}
}

PageHeader* STGHeap::create_and_init_page(size_t minSizeBytes, bool overheadAlreadyIncluded = false)
{
	const size_t overheadSize = (sizeof(PageHeader) + sizeof(HeapNodeHeader) - sizeof(HeapNodeHeader::HeapNodeMandatoryBody) + sizeof(HeapNodeFooter));
	minSizeBytes += overheadAlreadyIncluded ? 0 : overheadSize;
	//1. round minSizeBytes up to the nearest p_size
	minSizeBytes = (minSizeBytes % p_size() != 0) ? (minSizeBytes + (p_size() - (minSizeBytes % p_size()))) : minSizeBytes;
	size_t pageCount = (minSizeBytes / p_size());
	//2. p_alloc it, assign size
	PageHeader* ph = ((PageHeader*)p_alloc(pageCount));
	ph->sizeBytes = minSizeBytes;
	//3. declare the body of the page to be one giant free block
	HeapNodeHeader* n = ph->immediateFirstNode();
	n->setCurrentFree(true);
	size_t largestPossibleSize = minSizeBytes - overheadSize;
	largestPossibleSize = largestPossibleSize - (largestPossibleSize % 16); //even though this footer will always be last, make sure it's aligned as though it hypothetically might not be.
	n->setSize(largestPossibleSize); //the whole thing
	n->myFooter()->header = n;
	//4. assign head and tail of EFL
	ph->efl_head = ph->efl_tail = n;
	//5. sink/swim in page directory.
	//note: needs to handle cases where head/tails are null in pd dll.
#ifdef COHERENCY_CHECK_DEBUG
	if ((pd_head == nullptr && pd_tail != nullptr) || (pd_head != nullptr && pd_tail == nullptr))
	{
		fprintf(stderr, "Failed coherency, page directory had a head but no tail or vice versa.");
		exit(-1);
	}
#endif
	if (pd_head == nullptr)
		pd_head = pd_tail = ph;
	else
	{
		//linearly guess if the new ph belongs closer to the big end of the pd dll or the small end.
		size_t hs = pd_head->sizeBytes, ts = pd_tail->sizeBytes;
		if (minSizeBytes >= hs)
		{
			//make it the new head because it's the biggest
			ph->next = pd_head;
			pd_head->prev = ph;
			pd_head = ph;
		}
		else if (ts >= minSizeBytes)
		{
			//make it the new tail because it's the smallest.
			ph->prev = pd_tail;
			pd_tail->next = ph;
			pd_tail = ph;
		}
		else
		{
			fprintf(stderr, "Non edge-case page size (sink/swim in pd dll); Not yet implemented.");
			exit(-1);
			//TODO:
			size_t td = hs - minSizeBytes, bd = minSizeBytes - ts;
			if (td > bd)
			{
				//it's closer to the bottom, start small and swim
			}
			else
			{
				//it's closer to the top, start big and sink
			}
		}
	}
}

inline static void exciseEFL(HeapNodeHeader* toRemove)
{
	if (toRemove->content.efl.prev != nullptr)
		toRemove->content.efl.prev->content.efl.next = toRemove->content.efl.next;
	if (toRemove->content.efl.next != nullptr)
		toRemove->content.efl.next->content.efl.prev = toRemove->content.efl.prev;
	toRemove->content.efl.prev = nullptr;
	toRemove->content.efl.next = nullptr;
}

inline static void insertEFL(HeapNodeHeader* target, HeapNodeHeader* toInsert, bool leftOfTarget, PageHeader* ph)
{
	//assumptions, "toInsert" is not null, "target" is not null.
	if (ph->efl_head == toInsert)
		ph->efl_head = toInsert->content.efl.next;
	if (ph->efl_tail == toInsert)
		ph->efl_tail = toInsert->content.efl.prev;
	exciseEFL(toInsert);
	if (leftOfTarget)
	{
		HeapNodeHeader* oldPrev = target->content.efl.prev;
		if (oldPrev != nullptr)
			oldPrev->content.efl.next = toInsert;
		toInsert->content.efl.prev = oldPrev;
		toInsert->content.efl.next = target;
		target->content.efl.prev = toInsert;
	}
	else
	{
		HeapNodeHeader* oldNext = target->content.efl.next;
		if (oldNext != nullptr)
			oldNext->content.efl.prev = toInsert;
		toInsert->content.efl.next = oldNext;
		toInsert->content.efl.prev = target;
		target->content.efl.next = toInsert;
	}
}

//start small, go big
void swim(HeapNodeHeader* node, PageHeader* ph, bool fromCurrentPosition = false)
{ //inversion of sink
	HeapNodeHeader* current = fromCurrentPosition ? node : ph->efl_tail;
#ifdef COHERENCY_CHECK_DEBUG
	if (!node->isCurrentFree())
	{
		fprintf(stderr, "Swim was called with non-freed node");
		exit(-1);
	}
#endif
	while (current != nullptr)
	{
		if (node->size() < current->size()) //not <= because if current was init'd to node, it would pass on the first iteration.
			break; //we can insert to the right of current
		current = current->content.efl.prev; //move biggerward
	}
	if (current == nullptr)
	{ //either we walked off the end of the EFL, or "node" was null, or the EFL was empty.
#ifdef COHERENCY_CHECK_DEBUG
		if (node == nullptr) //|| ph->efl_head != nullptr I think the last half of this condition cannot occur
		{
			fprintf(stderr, "Swim was called with null node");
			exit(-1);
		}
#endif
		if (ph->efl_tail == nullptr)
		{ //efl empty?
#ifdef COHERENCY_CHECK_DEBUG
			if (ph->efl_head != nullptr)
			{
				fprintf(stderr, "Failed coherency, page had an efl head but no efl tail.");
				exit(-1);
			}
#endif
			ph->efl_head = ph->efl_tail = node;
			node->content.efl.next = node->content.efl.prev = nullptr;
		}
		else
		{
			//insert at the very end.
			if (ph->efl_head != node)
				insertEFL(ph->efl_head, node, true, ph);
		}
	}
	else
	{
		insertEFL(current, node, false, ph);
	}
}

//start big, go small
void sink(HeapNodeHeader* node, PageHeader* ph, bool fromCurrentPosition = false)
{ //original authoring
	HeapNodeHeader* current = fromCurrentPosition ? node : ph->efl_head;
	//assumption checking:
	//"node" has a next, if not, do nothing (COHERENCY_CHECK_DEBUG make sure "node" is the efl tail)
	//start sinking, if we reach a node with no next, then swap. (COHERENCY_CHECK_DEBUG make sure that that node was the efl tail)
	
	//	if (current == nullptr) //this should only happen if !fromCurrentPosition and this page has an empty EFL.
	//	{
	//#ifdef COHERENCY_CHECK_DEBUG
	//		if (fromCurrentPosition) //|| ph->efl_head != nullptr I think the last half of this condition cannot occur
	//		{
	//			fprintf(stderr, "Sink was called with null node");
	//			exit(-1);
	//		}
	//#endif
	//		//not from current position, and also empty EFL
	//		ph->efl_head = ph->efl_tail = node;
	//		return;
	//	}

	//while (current->content.efl.next != nullptr)
	//{
	//	if (node->size() >= current->content.efl.next->size())
	//	{
	//		//the node we're examining is smaller than the node we're sinking, we can insert the node to the left of ex (larger side)
	//		break;
	//	}
	//	current = current->content.efl.next;
	//}

#ifdef COHERENCY_CHECK_DEBUG
	if (!node->isCurrentFree())
	{
		fprintf(stderr, "Sink was called with non-freed node");
		exit(-1);
	}
#endif
	while (current != nullptr)
	{
		if (node->size() > current->size()) //not >= because if current was init'd to node, it would pass on the first iteration.
			break; //we can insert to the left of current
		current = current->content.efl.next; //move smallerward
	}
	if (current == nullptr)
	{ //either we walked off the end of the EFL, or "node" was null, or the EFL was empty.
#ifdef COHERENCY_CHECK_DEBUG
		if (node == nullptr) //|| ph->efl_head != nullptr I think the last half of this condition cannot occur
		{
			fprintf(stderr, "Sink was called with null node");
			exit(-1);
		}
#endif
		if (ph->efl_head == nullptr)
		{ //efl empty?
#ifdef COHERENCY_CHECK_DEBUG
			if (ph->efl_tail != nullptr)
			{
				fprintf(stderr, "Failed coherency, page had an efl tail but no efl head.");
				exit(-1);
			}
#endif
			ph->efl_head = ph->efl_tail = node;
			node->content.efl.next = node->content.efl.prev = nullptr;
		}
		else
		{
			//insert at the very end.
			if (ph->efl_tail != node)
				insertEFL(ph->efl_tail, node, false, ph);
		}
	}
	else
	{
		insertEFL(current, node, true, ph);
	}
}

void sink_or_swim(HeapNodeHeader* node, PageHeader* ph, bool fromCurrentPosition = false)
{
	if (fromCurrentPosition)
	{
		//check prev & next, go in correct direction
		//don't want prev & next, because then there's a gap. Let the starting point represent prev.
		HeapNodeHeader* prev = node->content.efl.prev; //prev is the larger direction
		HeapNodeHeader* next = node->content.efl.next; //next is the smaller direction
		size_t cs = node->size(), ns = next ? next->size() : 0, ps = prev ? prev->size() : ~0;
		//if we sit in-between prev and next, insert ourselves here.
		if (cs <= ps && ns <= cs)
		{
			//no action needed
			//hypothetically we would need to insert node between prev and next, but it already is
		}
		//if both prev and next are smaller than us, then swim.
		else if (ps <= cs && ns <= cs)
		{
			swim(node, ph, true);
		}
		//if both prev and next are bigger than us, then sink.
		else
		{
			sink(node, ph, true);
		}
	}
	else
	{
		//linear system currently, investigate logarithmic?
		//note, if the thing we're working on is smaller than the currently sorted efl, then "smalld" will be negative
		//in that case, bigd would be much larger (+), and again, "smalld" is negative, so it get's classified as "kinda small"
		//similar logic applies for vice versa
		int bigd = ph->efl_head->size() - node->size(), smalld = node->size() - ph->efl_tail->size();
		if (bigd > smalld)
		{
			//this is further away in size from big things, so it's "kinda small", therefore, start at tail and swim
			swim(node, ph);
		}
		else
		{
			//this is further away in size from small things, so it's "kinda big", therefore, start at head and sink
			sink(node, ph);
		}
	}
}

void* STGHeap::stg_malloc(size_t size)
{
	//0. calculate the minimum size (ensure that header size + body + padding (if necessary) + footer size) is good so that the body of subsequent bodies can be 16-aligned
	size = (size < sizeof(HeapNodeHeader::HeapNodeMandatoryBody)) ? sizeof(HeapNodeHeader::HeapNodeMandatoryBody) : size; //make sure it's larger than the mandatory size
	size = (size % 16 != 0) ? size + (16 - (size % 16)) : size; //make sure it's a multiple of 16

	//start at the smallest pagesize then walk bigwards until a page is reached whose largest EFL could fit this malloc
	HeapNodeHeader* candidate = nullptr;
	PageHeader* pp = pd_tail; 
	while (pp != nullptr)
	{
		if (pp->efl_head != nullptr && pp->efl_head->size() >= size)
		{
			//this page is guaranteed to contain a free node that could fit our thing
			candidate = pp->efl_head;
			break;
		}
		else
			pp = pp->prev;
	}
	if (pp == nullptr)
	{
		pp = create_and_init_page(size);
		candidate = pp->efl_head;
	}
	while (candidate != nullptr) //we know that at least one efl element could fit this malloc, but we should look for the smallest valid one.
	{
		//should this be <= ?
		if (candidate->size() < size) //when this condition is true, we went one step too far
			break;
		else
			candidate = candidate->content.efl.next;
	}
	if (candidate == nullptr) //in case we walk off the efl (all efl elements are large enough to house this malloc).
		candidate = pp->efl_tail;
	else
		candidate = candidate->content.efl.prev; //so back up one

	// is this still applicable? 1. check if the largest free spot in the large-elements page
	// ...
	//sizeof(PageHeader) 40b
	// | 40 bytes (PH) | 8 bytes (HNH) + [16 byte aligned] >=16 bytes (BODY) | 8 bytes (HNF) | 8 bytes (HNH) + [16 byte aligned] >=16 bytes (BODY) ...
	//if candidate can be split into two pieces, then do so.
	if (candidate->size() - (sizeof(HeapNodeHeader) + sizeof(HeapNodeFooter)) >= size)
	{
		//split it up, remove this one from efl
		//add the smaller chunk to the efl and sink (since it's guaranteed to be smaller).
		HeapNodeHeader *prev = candidate->content.efl.prev, *next = candidate->content.efl.next;
		HeapNodeFooter *ff = candidate->myFooter();
		size_t oldBigSize = candidate->size();
		candidate->setSize(size);
		candidate->setCurrentFree(false);
		candidate->myFooter()->header = candidate;
		ff->header = candidate->immediateNextNeighbor();
		HeapNodeHeader* nf = ff->header;
		nf->setCurrentFree(true);
		nf->setSize(oldBigSize - (sizeof(HeapNodeHeader) + sizeof(HeapNodeFooter) + size - sizeof(HeapNodeHeader::HeapNodeMandatoryBody)));
		//nf->setSize(oldBigSize - (sizeof(HeapNodeHeader) + sizeof(HeapNodeFooter) + size));
		nf->content.efl.prev = prev;
		nf->content.efl.next = next;
		if (prev != nullptr)
			prev->content.efl.next = nf;
		else //no "prev" = that SHOULD mean that nf (split up formerly candidate) is the head.
			pp->efl_head = nf;
		if (next != nullptr)
			next->content.efl.prev = nf;
		else //no "next" = that SHOULD mean that nf (split up formerly candidate) is the tail.
			pp->efl_tail = nf;
		sink(nf, pp, true);
	}
	else
	{
		//just keep this as one piece, remove from efl

		//if "candidate" was head or tail of efl, need to fix that

		if (candidate->content.efl.prev != nullptr)
			candidate->content.efl.prev->content.efl.next = candidate->content.efl.next;
		else //no "prev" = that SHOULD mean that candidate is the head.
		{
#ifdef COHERENCY_CHECK_DEBUG
			if (pp->efl_head != candidate)
			{
				fprintf(stderr, "Free HeapNodeHeader has no prev in EFL, implies that it's the head of the EFL, and yet was not.");
				exit(-1);
			}
#endif
			pp->efl_head = candidate->content.efl.next;
			if (candidate->content.efl.next != nullptr)
				candidate->content.efl.next->content.efl.prev = nullptr;
		}
		if (candidate->content.efl.next != nullptr)
			candidate->content.efl.next->content.efl.prev = candidate->content.efl.prev;
		else //no "next" = that SHOULD mean that candidate is the tail.
		{
#ifdef COHERENCY_CHECK_DEBUG
			if (pp->efl_tail != candidate)
			{
				fprintf(stderr, "Free HeapNodeHeader has no next in EFL, implies that it's the tail of the EFL, and yet was not.");
				exit(-1);
			}
#endif
			pp->efl_tail = candidate->content.efl.prev;
			if (candidate->content.efl.prev != nullptr)
				candidate->content.efl.prev->content.efl.next = nullptr;
		}
		candidate->setCurrentFree(false);
	}
	
	//X. if choosing a particular chunk, check to see if it can be split into the new node + a free node, or if the free node isn't large enough to fit a header+efl+footer info

	return &candidate->content.mem;
}

void STGHeap::stg_free(void* p)
{
	//find the page we belong to:
	//start from a page, then check if "p" lies within the address range of that page, if not, iterate and check the next page etc.
	PageHeader* page = nullptr;
	{
		PageHeader* currentPage = pd_head;
		do
		{
			//check if it's in the current page.
			//pointer lies past the page header, but before the end of the page
			//mins sizeof heapnodefooter *ONLY* because p points to the body, not the header.
			if ((currentPage + 1) <= p && p < (((char*)currentPage) + currentPage->sizeBytes - sizeof(HeapNodeFooter)))
			{
				page = currentPage;
				break;
			}
			//last step; step forward
			currentPage = currentPage->next; //this is a dll, take advantage of that somehow?
		} while (currentPage != nullptr);
#ifdef COHERENCY_CHECK_DEBUG
		if (page == nullptr)
		{
			//panic, couldn't find the page. User gave bad ptr?
			fprintf(stderr, "Failed coherency, couldn't find the page that %p belongs to, stg_free passed bad ptr?", p);
			exit(-1);
		}
#endif
	}
	//minus sizeof because the pointer that we gave via stg_malloc (and ultimately given back to us) points to the BODY, not the header.
	HeapNodeHeader *currentNode = (HeapNodeHeader*)(((char*)p) - (sizeof(HeapNodeHeader) - sizeof(HeapNodeHeader::HeapNodeMandatoryBody))), *prevNode, *nextNode;
	HeapNodeFooter *currentFooter = currentNode->myFooter(), *prevFooter, *nextFooter;
	//these are so that we know if we even *have* an immediate prev/next neighbor
	bool prevNeighborExists = !(page->immediateFirstNode() == currentNode), nextNeighborExists = !(page->immediateLastNode() == currentNode);
	bool prevFree = prevNeighborExists ? currentNode->isImmediatePreviousFree() : false, nextFree = nextNeighborExists ? currentNode->isImmediateNextFree() : false;
	//encode in state because I don't want a giant nested if statement
	char state = (prevFree ? 1 : 0) | (nextFree ? 2 : 0) | (prevNeighborExists ? 4 : 0) | (nextNeighborExists ? 8 : 0);
	switch (state)
	{
	case 0: //I am the one and only in the entire page(s)
	case 1:
	case 2:
	case 3: //prevFree/nextFree options N/A
		{
			//Free myself, make me the only entry in the EFL. I am the only node in the page(s)
			currentNode->setCurrentFree(true);
			currentNode->content.efl.prev = nullptr;
			currentNode->content.efl.next = nullptr;
			page->efl_head = currentNode;
			page->efl_tail = currentNode;
			//change this code so that it still uses sink swim for uniformity?
		}
		break;

	case 5: //prev exists, next does not.
	case 7: //prev exists, next does not.
		{
			prevNode = currentNode->immediatePrevNeighbor();
			prevFooter = prevNode->myFooter();
			//next DNE, prev is free
			//ph: update size
			//pf: ignore
			//ch: ignore
			//cf: update ptr
			//swim

			//update size
			prevNode->setSize(prevNode->size() +
				sizeof(HeapNodeFooter) +
				(sizeof(HeapNodeHeader) - sizeof(HeapNodeHeader::HeapNodeMandatoryBody)) +
				currentNode->size());
#ifdef COHERENCY_CHECK_DEBUG
			size_t diff = (((char*)currentFooter) - ((char*)prevNode)) - (sizeof(HeapNodeHeader) - sizeof(HeapNodeHeader::HeapNodeMandatoryBody));
			if (prevNode->size() != diff)
			{
				fprintf(stderr, "Failed coherency (cases 5/7), coalesce size calculation is wrong?");
				exit(-1);
			}
#endif
			//update ptr
			currentFooter->header = prevNode;
			//swim ourselves in EFL
			swim(prevNode, page); //swim does the actual adding to the EFL
		}
		break; //2. prev is free, next is not (coalesce this one in with the previous one, swim it up in efl).
		       //1. both are free (coalesce next and this one into previous, swim it up in efl).

	case 4: //prev exists, next does not.
	case 6: //prev exists, next does not.
		{
			//next DNE, prev is non-free
			//ph: ignore
			//pf: ignore
			//ch: mark free
			//cf: ignore
			//sink or swim
			currentNode->setCurrentFree(true);
			currentNode->content.efl.prev = nullptr;
			currentNode->content.efl.next = nullptr;
			//insert efl sink/swim
			sink_or_swim(currentNode, page); //sink/swim does the actual adding to the EFL
		}
		break; //4. prev and next are both non-free (just mark this one as free, insert & sink or swim it in efl).
		       //3. prev is non-free, next is (coalesce next into this one, swim it up in efl).

	case 8: //prev does not exist, next does.
	case 9: //prev does not exist, next does.
		{
			//prev DNE, next is non-free
			//ch: mark free
			//cf: ignore
			//nh: ignore
			//nf: ignore
			//sink or swim
			currentNode->setCurrentFree(true);
			currentNode->content.efl.prev = nullptr;
			currentNode->content.efl.next = nullptr;
			//insert efl sink/swim
			sink_or_swim(currentNode, page); //sink/swim does the actual adding to the EFL
		}
		break; //4. prev and next are both non-free (just mark this one as free, insert & sink or swim it in efl).
		       //2. prev is free, next is not (coalesce this one in with the previous one, swim it up in efl).

	case 10: //prev does not exist, next does.
	case 11: //prev does not exist, next does.
		{
			nextNode = currentNode->immediateNextNeighbor();
			nextFooter = nextNode->myFooter();
			//prev DNE, next is free
			//ch: update size, mark free
			//cf: ignore
			//nh: remove efl
			//nf: update ptr
			//swim from position of next's old efl position
			//update size
			currentNode->setSize(currentNode->size() +
				sizeof(HeapNodeFooter) +
				(sizeof(HeapNodeHeader) - sizeof(HeapNodeHeader::HeapNodeMandatoryBody)) +
				nextNode->size());
#ifdef COHERENCY_CHECK_DEBUG
			size_t diff = (((char*)nextFooter) - ((char*)currentNode)) - (sizeof(HeapNodeHeader) - sizeof(HeapNodeHeader::HeapNodeMandatoryBody));
			if (currentNode->size() != diff)
			{
				fprintf(stderr, "Failed coherency (cases 10/11), coalesce size calculation is wrong?");
				exit(-1);
			}
#endif
			HeapNodeHeader *swimStart;
			//TODO: IS THE BELOW TODO TRUE??? since we're crafting the new efl out of this element + the portion we just freed (slightly larger than it was a second ago)???
			{   //TODO: THIS IS A RANDOM PORTION OF THE EFL, WE HAVE NO IDEA WHAT THE SIZES OF THESE THINGS ARE IN RELATION TO US
				//WE SHOULD NOT USE THIS AS A SWIMSTART, ALSO FIX THIS ELSEWHERE
				//remove efl
				HeapNodeHeader *nprev = nextNode->content.efl.prev, *nnext = nextNode->content.efl.next;
				swimStart = nnext; //nnext is the smaller of the two, will swim in the bigger direction.
				nnext->content.efl.prev = nprev; //nnext might be null sometimes?
				nprev->content.efl.next = nnext;
			}
			//update ptr
			nextFooter->header = currentNode;
			//insert & swim ourselves in EFL
			swim(currentNode, page, swimStart); //swim does the actual adding to the EFL TODO FIX SWIMSTART
		}
		break; //3. prev is non-free, next is (coalesce next into this one, swim it up in efl).
		       //1. both are free (coalesce next and this one into previous, swim it up in efl).

	case 12: //prev/next exist
		{
			//4. prev and next are both non-free (just mark this one as free, insert & sink or swim it in efl).
			//ph: ignore
			//pf: ignore
			//ch: mark free, swim efl
			//cf: ignore
			//nh: ignore
			//nf: ignore
			currentNode->setCurrentFree(true);
			currentNode->content.efl.prev = nullptr;
			currentNode->content.efl.next = nullptr;
			sink_or_swim(currentNode, page);
		}
		break;
	case 13: //prev/next exist
		{
			prevNode = currentNode->immediatePrevNeighbor();
			prevFooter = prevNode->myFooter();
			nextNode = currentNode->immediateNextNeighbor();
			nextFooter = nextNode->myFooter();
			//2. prev is free, next is not (coalesce this one in with the previous one, swim it up in efl).
			//ph: update size, swim efl
			//pf: ignore
			//ch: ignore
			//cf: udpate ptr
			//nh: ignore
			//nf: ignore

			//update size
			prevNode->setSize(prevNode->size() +
				sizeof(HeapNodeFooter) +
				(sizeof(HeapNodeHeader) - sizeof(HeapNodeHeader::HeapNodeMandatoryBody)) +
				currentNode->size());
#ifdef COHERENCY_CHECK_DEBUG
			size_t diff = (((char*)currentFooter) - ((char*)prevNode)) - (sizeof(HeapNodeHeader) - sizeof(HeapNodeHeader::HeapNodeMandatoryBody));
			if (prevNode->size() != diff)
			{
				fprintf(stderr, "Failed coherency (case 13), coalesce size calculation is wrong?");
				exit(-1);
			}
#endif
			//update ptr
			currentFooter->header = prevNode;
			//insert & swim ourselves in EFL
			swim(prevNode, page); //swim does the actual adding to the EFL
		}
		break;
	case 14: //prev/next exist
		{
			prevNode = currentNode->immediatePrevNeighbor();
			prevFooter = prevNode->myFooter();
			nextNode = currentNode->immediateNextNeighbor();
			nextFooter = nextNode->myFooter();
			//3. prev is non-free, next is (coalesce next into this one, swim it up in efl).
			//ph: ignore
			//pf: ignore
			//ch: update size, mark free, swim efl
			//cf: ignore
			//nh: remove efl
			//nf: update ptr

			//update size
			currentNode->setSize(currentNode->size() +
				sizeof(HeapNodeFooter) +
				(sizeof(HeapNodeHeader) - sizeof(HeapNodeHeader::HeapNodeMandatoryBody)) + //TODO (for all 16 cases) this includes the size of mandatory body, fix me.
				nextNode->size());
			//mark free
			currentNode->setCurrentFree(true);
			currentNode->content.efl.prev = nullptr;
			currentNode->content.efl.next = nullptr;
#ifdef COHERENCY_CHECK_DEBUG
			size_t diff = (((char*)nextFooter) - ((char*)currentNode)) - (sizeof(HeapNodeHeader) - sizeof(HeapNodeHeader::HeapNodeMandatoryBody)); //TODO (for all 16 cases) this includes the size of mandatory body, fix me.
			if (currentNode->size() != diff)
			{
				fprintf(stderr, "Failed coherency (case 14), coalesce size calculation is wrong?");
				exit(-1);
			}
#endif
			//HeapNodeHeader* swimStart;
			//{
			//	//remove efl
			//	HeapNodeHeader *nprev = nextNode->content.efl.prev, *nnext = nextNode->content.efl.next;
			//	swimStart = nnext; //nnext is the smaller of the two, will swim in the bigger direction.
			//	nnext->content.efl.prev = nprev; //TODO: nnext can be nullptr sometimes.
			//	nprev->content.efl.next = nnext;
			//}
			//update ptr
			nextFooter->header = currentNode;
			//insert & swim ourselves in EFL
			swim(currentNode, page, true); //swim does the actual adding to the EFL
		}
		break;
	case 15: //prev/next exist
		{
			prevNode = currentNode->immediatePrevNeighbor();
			prevFooter = prevNode->myFooter();
			nextNode = currentNode->immediateNextNeighbor();
			nextFooter = nextNode->myFooter();
			//1. both are free (coalesce next and this one into previous, swim it up in efl).
			//ph: update size, swim efl
			//pf: ignore
			//ch: ignore
			//cf: ignore
			//nh: remove efl
			//nf: update ptr

			//update size
			prevNode->setSize(prevNode->size() +
				sizeof(HeapNodeFooter) +
				(sizeof(HeapNodeHeader) - sizeof(HeapNodeHeader::HeapNodeMandatoryBody)) +
				currentNode->size() +
				sizeof(HeapNodeFooter) +
				(sizeof(HeapNodeHeader) - sizeof(HeapNodeHeader::HeapNodeMandatoryBody)) +
				nextNode->size());
#ifdef COHERENCY_CHECK_DEBUG
			size_t diff = (((char*)nextFooter) - ((char*)prevNode)) - (sizeof(HeapNodeHeader) - sizeof(HeapNodeHeader::HeapNodeMandatoryBody));
			if (prevNode->size() != diff)
			{
				fprintf(stderr, "Failed coherency (case 15), coalesce size calculation is wrong?");
				exit(-1);
			}
#endif
			//HeapNodeHeader* swimStart;
			//{
			//	//remove efl
			//	HeapNodeHeader *nprev = nextNode->content.efl.prev, *nnext = nextNode->content.efl.next;
			//	swimStart = nnext; //nnext is the smaller of the two, will swim in the bigger direction.
			//	nnext->content.efl.prev = nprev; //nnext might be null sometimes.
			//	nprev->content.efl.next = nnext; 
			//}
			//update ptr
			nextFooter->header = prevNode;
			//insert & swim ourselves in EFL
			swim(prevNode, page, true); //swim does the actual adding to the EFL
		}
		break;
	}
}

void STGHeap::heap_check()
{
	#ifdef COHERENCY_CHECK_DEBUG
	bool goodHeap = true;
	//walk through all pages
	PageHeader* currentPage = pd_head;
	while (currentPage != nullptr)
	{
		if (currentPage->efl_head->content.efl.prev != nullptr)
		{
			//bad
			fprintf(stderr, "Heap Check: efl head has a non-null prev efl member");
			goodHeap = false;
		}
		if (currentPage->efl_tail->content.efl.next != nullptr)
		{
			//bad
			fprintf(stderr, "Heap Check: efl tail has a non-null next efl member");
			goodHeap = false;
		}
		//TODO: walk through the EFL to make sure there aren't any breaks between head and tail.
		//last step; step forward
		currentPage = currentPage->next; //this is a dll, take advantage of that somehow?
	}
	//last thing
	if (!goodHeap)
	{
		fprintf(stderr, "Failed heap check!");
		exit(-1);
	}
    #else
	printf("Compiled without the COHERENCY_CHECK_DEBUG define, no checks were made.");
	#endif // COHERENCY_CHECK_DEBUG
}

#pragma region Old
//void swapEFL(HeapNodeHeader* a, HeapNodeHeader* b, PageHeader* ph = nullptr) //unused?
//{
//	//assumptions, a and b are not null
//	//we do *not* assume that a & b's efl neighbors are non-null.
//	HeapNodeHeader *c, *d, *e, *f;
//	c = a->content.efl.prev;
//	d = a->content.efl.next;
//	e = b->content.efl.prev;
//	f = b->content.efl.next;
//	b->content.efl.prev = c;
//	b->content.efl.next = d;
//	a->content.efl.prev = e;
//	a->content.efl.next = f;
//	if (c)
//		c->content.efl.next = b;
//	if (d)
//		d->content.efl.prev = b;
//	if (e)
//		e->content.efl.next = a;
//	if (f)
//		f->content.efl.prev = a;
//	if (ph != nullptr)
//	{
//		if (ph->efl_head == a)
//			ph->efl_head = b;
//		else if (ph->efl_head == b) //if the else wasn't there, then this second if would just undo the first one.
//			ph->efl_head = a;
//		if (ph->efl_tail == a)
//			ph->efl_tail = b;
//		else if (ph->efl_tail == b) // "" ""
//			ph->efl_tail = a;
//	}
//}

////start big, go small
//void sink(HeapNodeHeader* node, PageHeader* ph, bool fromCurrentPosition = false)
//{
//	//prev is the larger direction
//	//next is the smaller direction
//	if (fromCurrentPosition)
//	{
//		//we know that node is in the efl, chances are, it shrunk (malloc bifurcated us), and we should start examining starting
//		//from the smaller neighbor. If we're still bigger than our smaller neighbor, then we're already in a good spot.
//		//if not, then we need to keep sinking.
//		HeapNodeHeader* ex = node->content.efl.next; //node is not null, but node's efl neighbors might be
//		while (ex != nullptr)
//		{
//			if (node->size() >= ex->size())
//			{
//				//the node we're examining is smaller than the node we're sinking, we can insert the node to the left of ex (larger side)
//				break;
//			}
//			//last step, move smallerward
//			ex = ex->content.efl.next;
//		}
//		//this needs to happen regardless
//		//reconnect the broken ends where node is currently sitting (extract node from old position of efl)
//		if (node->content.efl.prev != nullptr)
//			node->content.efl.prev->content.efl.next = node->content.efl.next; //if node's next is null, then it's just assigning null to node's previous' next (fine)
//		if (node->content.efl.next != nullptr)
//			node->content.efl.next->content.efl.prev = node->content.efl.prev; //similar "" ""
//		if (ex == nullptr)
//		{
//			//either node->content.efl.next was null, or we walked off the bottom of the efl (should be made tail)
//			//if node->content.efl.next was null, then node *is* the bottom of the efl
//			//either way, (re)-making it the bottom of the efl is fine
//
//			//consider the circumstance in which "node" is the only node on the pages (caller circumstance makes this unlikely?):
//			if (ph->efl_tail != node)
//			{
//				ph->efl_tail->content.efl.next = node;//this line could assign node's .next to itself (bad), hence the if statement
//				node->content.efl.prev = ph->efl_tail;//this line could assign node's .prev to itself (bad), hence the if statement
//			}
//			//this line is innocuous
//			ph->efl_tail = node;
//		}
//		else
//		{
//			//either: the condition was met instantly (i.e., "we're already in a good spot"), or we moved. either way, we can do the following:
//			//from:
//			//             v-- node could have been here, or anywhere else to the left of here.
//			//... a <-> b <-> ex <-> c <-> d ...
//			//to:
//			//... a <-> b <-> node <-> ex <-> c <-> d ...
//			HeapNodeHeader* b = ex->content.efl.prev;
//#ifdef COHERENCY_CHECK_DEBUG
//			if (b == nullptr)
//			{
//				//if no walking occurred, then at the very least, b should == node.
//				//if walking occurred, then prev should always be non null since ex isn't null
//				//this is bad
//				fprintf(stderr, "Failed coherency, a node's next->prev was null");
//			}
//#endif
//			if (b != node) //consider if b == node (bad), in that case node is already where it should be, hence the if statement
//			{
//				//I think there is some missing steps here? maybe a prev neighbor doesn't get their next assigned?
//				b->content.efl.next = node;
//				node->content.efl.prev = b;
//				node->content.efl.next = ex;
//				ex->content.efl.prev = node;
//			}
//		}
//	}
//	else
//	{
//		//head is large things, tail is small
//		HeapNodeHeader* ex = ph->efl_head;
//		while (ex != nullptr)
//		{
//			if (node->size() >= ex->size())
//			{
//				//the node we're examining is smaller than the node we're sinking, we can insert the node to the left of ex (larger side)
//				break;
//			}
//			//last step, move smallerward
//			ex = ex->content.efl.next;
//		}
//		if (ex == nullptr)
//		{
//			//we walked through the entire EFL, which means we're the smallest. Add us as the tail
//			if (ph->efl_tail != nullptr)
//			{
//				ph->efl_tail->content.efl.next = node;
//				node->content.efl.prev = ph->efl_tail;
//				ph->efl_tail = node;
//			}
//			else
//			{
//				//this should only ever be the case if the head & tail are both null because there is no efl
//#ifdef COHERENCY_CHECK_DEBUG
//				if (ph->efl_head != nullptr)
//				{
//					//panic, couldn't find the page. User gave bad ptr?
//					fprintf(stderr, "Failed coherency, page efl tail was null, but head was not");
//					exit(-1);
//				}
//#endif
//				ph->efl_head = ph->efl_tail = node;
//			}
//		}
//		else
//		{
//			//assumptions, ex is not null
//			//ex.content.efl->prev could be null, in that case, ex is the head of the efl, and node should be made the new head of the efl
//			//if ex walked all the way down, it would walk off into null, so we don't have to worry about the tail case.
//			//we picked something, insert to the left
//			//from:
//			//... a <-> b <-> ex <-> c <-> d ...
//			//to:
//			//... a <-> b <-> node <-> ex <-> c <-> d ...
//			HeapNodeHeader* b = ex->content.efl.prev;
//			if (b == nullptr)
//			{ //ex is the head of the efl, and node should be made the new head
//#ifdef COHERENCY_CHECK_DEBUG
//				if (ph->efl_head != ex)
//				{
//					//panic, couldn't find the page. User gave bad ptr?
//					fprintf(stderr, "Failed coherency, EFL structure implied that %p was the head of the EFL, but it was not", ex);
//					exit(-1);
//				}
//#endif
//				ex->content.efl.prev = node;
//				node->content.efl.next = ex;
//				ph->efl_head = node;
//			}
//			else
//			{
//				b->content.efl.next = node;
//				node->content.efl.prev = b;
//				node->content.efl.next = ex;
//				ex->content.efl.prev = node;
//			}
//		}
//	}
//}
//
////start small, go big
//void swim(HeapNodeHeader* node, PageHeader* ph, bool fromCurrentPosition = false)
//{
//
//}
#pragma endregion
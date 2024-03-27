#include "HeapStructures.hpp"

HeapNodeHeader::HeapNodeHeader()
{
	header_data = 0;
	content.efl.prev = nullptr;
	content.efl.next = nullptr;
}
HeapNodeHeader::HeapNodeHeader(unsigned long long size, bool isCurrentFree)
{
	header_data = (size << 1ULL) | (((unsigned long long)(!!isCurrentFree)));
	content.efl.prev = nullptr;
	content.efl.next = nullptr;
}

bool HeapNodeHeader::isImmediatePreviousFree()
{
	return immediatePrevNeighbor()->isCurrentFree();
}
bool HeapNodeHeader::isCurrentFree()
{
	return (header_data & 1);
}
void HeapNodeHeader::setCurrentFree(bool isFree)
{
	if (isFree)
		header_data |= 1ULL;
	else
		header_data &= ~1ULL;
}
bool HeapNodeHeader::isImmediateNextFree()
{
	return immediateNextNeighbor()->isCurrentFree();
}
unsigned long long HeapNodeHeader::size()
{
	return (header_data >> 1);
}
void HeapNodeHeader::setSize(unsigned long long size)
{
	header_data &= 1ULL; //zero the previous size
	header_data |= (size << 1ULL); //or in the new size
}
HeapNodeHeader* HeapNodeHeader::immediatePrevNeighbor()
{
	return ((HeapNodeFooter*)(((char*)this) - sizeof(HeapNodeFooter)))->header;
}
HeapNodeHeader* HeapNodeHeader::immediateNextNeighbor()
{
	return ((HeapNodeHeader*)(myFooter() + 1));
}
HeapNodeFooter* HeapNodeHeader::myFooter()
{
	return ((HeapNodeFooter*)(((char*)this) + sizeof(HeapNodeHeader) + size() - sizeof(HeapNodeHeader::HeapNodeMandatoryBody)));
}
HeapNodeHeader* PageHeader::immediateFirstNode()
{
	return ((HeapNodeHeader*)(this + 1));
}
HeapNodeHeader* PageHeader::immediateLastNode()
{
	//TODO: fix this (perhaps?)
	char* addr = ((((char*)this) + this->sizeBytes) - sizeof(HeapNodeFooter));
	addr -= (((size_t)addr) % 16 == 8) ? 0 : 8;
	return ((HeapNodeFooter*)addr)->header;
}
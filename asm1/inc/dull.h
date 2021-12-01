#ifndef HEADER_DULL
#define HEADER_DULL

struct Dull_header
{
	char magic[4]; // dull
	unsigned id;
};

struct Dull_section
{
};

struct Dull
{
	struct Dull_header header;
	unsigned long start_offset;
};

#endif
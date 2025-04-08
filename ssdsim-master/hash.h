#pragma once

#ifndef CPP_HASH_H
#define CPP_HASH_H

#include <string.h>
#include "initialize.h"


#ifdef __cplusplus
extern "C" {
#endif


	int insert_page(struct ssd_info* ssd, char * data, unsigned int ppn, unsigned int lpn, int tag);
	int delete_page(unsigned int ppn);
	int update_page(unsigned int ppn, const char* data);
	struct page_info* find_page(unsigned int ppn);

	void delete_fingerprintStore();
	void delete_pageDatabase();



#ifdef __cplusplus
}
#endif


#endif			// end of CPP_HASH_H

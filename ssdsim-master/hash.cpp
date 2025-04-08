#include "hash.h"

#include <iostream>
#include <string>
#include <unordered_map>
#include <sstream>
#include <iomanip>

#define PAGE_SIZE 256


/***********
* 哈希表定义
************/
static std::unordered_map<unsigned int, page_info> pageDatabase;             // { ppn ， page_info }
static std::unordered_map<std::string, unsigned int> fingerprintStore;       // { SHA-1 ，ppn }

struct page_info deep_copy_page_info(const struct page_info& src);
void free_page_info(struct page_info& p);
std::string sha1Cal(char* data);


/**********************************
*哈希表基础操作：增删改查 + 深拷贝
***********************************/
int insert_page(struct ssd_info* ssd, char* data, unsigned int ppn, unsigned int lpn, int tag)
{
    struct page_info* new_page = new page_info();
    std::string new_string;

    //创建新数据页结构
	new_page->valid_state = 1;
	new_page->free_state = 0;
	new_page->lpn = lpn;
    new_page->written_count = 0;

    new_page->ref_count = 1;
	new_page->dedup_tag = tag;
    new_page->data = nullptr;
    new_page->hash_value = nullptr;

    new_page->data = new char[PAGE_SIZE + 1];
    strcpy_s(new_page->data, PAGE_SIZE + 1, data);

    //如果请求块大于选择性重删设定的块大小阈值，则进行重删
	if (new_page->dedup_tag == 1)
	{
        //计算SHA-1哈希值
        new_string = sha1Cal(new_page->data);
    //    std::cout << new_string << std::endl;

        //在指纹表中查询页是否存在，存在则更新其引用计数，否则更新指纹表和页数据库
        auto it = fingerprintStore.find(new_string);
        if (it != fingerprintStore.end()) {
            auto iter = pageDatabase.find(it->second);
            if (iter != pageDatabase.end()) {
                iter->second.ref_count++;
                ssd->duplicate_chunks++;

                delete[] new_page->data;
                delete new_page;
                return TRUE;
            }
            else {
                printf("In fingerprintStore, but not found in pageDatabase.\n");

                delete[] new_page->data;
                delete new_page;
                return FALSE;
            }
        }
        else {
            // 存储哈希值（深拷贝）
            new_page->hash_value = new char[new_string.size() + 1];
            strcpy_s(new_page->hash_value, new_string.size() + 1, new_string.c_str());

            // 更新页信息
            fingerprintStore[new_string] = ppn;
            pageDatabase[ppn] = deep_copy_page_info(*new_page);
            ssd->unique_chunks++;

            delete[] new_page->data;
            if (new_page->hash_value) 
                delete[] new_page->hash_value;
            delete new_page;
            return TRUE;
        }
    }

    //否则直接写入数据库
    pageDatabase[ppn] = deep_copy_page_info(*new_page);
    ssd->unique_chunks++;

    delete[] new_page->data;
    delete new_page;

    return TRUE;
}

int delete_page(unsigned int ppn) {
    auto it = pageDatabase.find(ppn);
    if (it != pageDatabase.end()) {
        it->second.valid_state = 0;

        delete[] it->second.data;
        if (it->second.hash_value)
            delete[] it->second.hash_value;
        return TRUE;
    }
    else {
        printf("delete_page: not found page %u\n", ppn);
        return FALSE;
    }
}

int update_page(unsigned int ppn, const char* data) {
    auto it = pageDatabase.find(ppn);
    if (it != pageDatabase.end()) {
        strcpy_s(it->second.data, strlen(it->second.data) + 1, data);
        return TRUE;
    }

    printf("update_page: not found page %u\n", ppn);
    return FALSE;
}

struct page_info* find_page(unsigned int ppn) {
    auto it = pageDatabase.find(ppn);
    if (it != pageDatabase.end()) {
        return &(it->second);
    }

    printf("find_page: not found page %u\n", ppn);
    return nullptr;
}

void delete_fingerprintStore() {
    fingerprintStore.clear();
}

void delete_pageDatabase() {
    for (auto& kv : pageDatabase) {
        free_page_info(kv.second);
    }
    pageDatabase.clear();
}

//page_info深拷贝
struct page_info deep_copy_page_info(const struct page_info& src) {
    page_info copy = src;

    // 深拷贝 data
    if (src.data) {
        copy.data = new char[strlen(src.data) + 1];
        strcpy_s(copy.data, strlen(src.data) + 1, src.data);
    }
    else {
        copy.data = nullptr;
    }

    // 深拷贝 hash_value
    if (src.hash_value) {
        copy.hash_value = new char[strlen(src.hash_value) + 1];
        strcpy_s(copy.hash_value, strlen(src.hash_value) + 1, src.hash_value);
    }
    else {
        copy.hash_value = nullptr;
    }

    return copy;
}

//释放page_info空间
void free_page_info(struct page_info& p) {
    if (p.data) 
        delete[] p.data;
    if (p.hash_value) 
        delete[] p.hash_value;
}


/********************************************
*模拟计算块的哈希值SHA-1，通过 shaCal() 调用
*********************************************/
unsigned circleShift(unsigned word, int bits) {
    return (word << bits) | (word >> (32 - bits));
}

unsigned sha1Fun(unsigned B, unsigned C, unsigned D, unsigned t) {
    switch (t / 20) {
    case 0:     return (B & C) | ((~B) & D);
    case 2:     return (B & C) | (B & D) | (C & D);
    case 1:
    case 3:     return B ^ C ^ D;
    }
    return t;
}

std::string sha1Cal(char* data) {
    unsigned int length = strlen(data);  // Ensure data is null-terminated
    std::string str(data, length);

    str += (unsigned char)(0x80);

    // Ensure bit length is padded correctly
    while ((str.size() << 3) % 512 != 448) {
        str += (char)0;
    }

    // Append length as 64-bit integer
    for (int i = 56; i >= 0; i -= 8) {
        str += (unsigned char)((((unsigned __int64)length) << 3) >> i);
    }

    const unsigned K[4] = { 0x5a827999, 0x6ed9eba1, 0x8f1bbcdc, 0xca62c1d6 };
    unsigned A = 0x67452301, B = 0xefcdab89, C = 0x98badcfe, D = 0x10325476, E = 0xc3d2e1f0, T;
    unsigned W[80] = { 0 };

    for (unsigned i = 0; i < str.size(); i += 64) {
        // Populate W array with message blocks
        for (unsigned t = 0; t < 16; ++t) {
            W[t] = ((unsigned)str[i + 4 * t] & 0xff) << 24 |
                ((unsigned)str[i + 4 * t + 1] & 0xff) << 16 |
                ((unsigned)str[i + 4 * t + 2] & 0xff) << 8 |
                ((unsigned)str[i + 4 * t + 3] & 0xff);
        }

        // Extend W array to 80 words
        for (unsigned t = 16; t < 80; ++t) {
            W[t] = circleShift(W[t - 3] ^ W[t - 8] ^ W[t - 14] ^ W[t - 16], 1);
        }

        // Main SHA1 processing loop
        for (unsigned t = 0; t < 80; ++t) {
            T = circleShift(A, 5) + sha1Fun(B, C, D, t) + E + W[t] + K[t / 20];
            E = D;
            D = C;
            C = circleShift(B, 30);
            B = A;
            A = T;
        }

        A += 0x67452301;
        B += 0xefcdab89;
        C += 0x98badcfe;
        D += 0x10325476;
        E += 0xc3d2e1f0;
    }

    std::stringstream ss;
    ss << std::setw(8) << std::setfill('0') << std::hex << A << B << C << D << E;
    return ss.str();
}
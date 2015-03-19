#ifndef UNICODE
#define UNICODE
#endif

#ifndef _UNICODE
#define _UNICODE
#endif

#include <windows.h>
#include <stdio.h>
#include "auxiliary.h"
#include "debug.h"
#pragma comment(lib, "advapi32.lib")
#define DASH TEXT('-')
#define COMMA TEXT(',')
#define SDASH TEXT("-")
#define SCOMMA TEXT(",")

// Retrieve the system error message for the last-error code
LPCWSTR ErrorMessage(DWORD error){ 
    LPVOID lpMsgBuf;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
    			  NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), (LPWSTR) &lpMsgBuf, 0, NULL );
    return((LPCWSTR)lpMsgBuf);}


BOOL signalled(HANDLE manual_reset_event){
	return WaitForSingleObjectEx(manual_reset_event, 0, TRUE) != WAIT_TIMEOUT;}


ILIST range2ilist_destructive(RANGE range);
VOID range_extend(RANGE *r, int *cur_len, LPWSTR str);
VOID ilist_sort(ILIST *il);

BOOL in_ilist(ILIST il, int n){
	int i;
	for(i=0;i<il.len;i++)
		if (n == il.arr[i]) return TRUE;
	return FALSE;}

ILIST range2ilist(RANGE range){
	int len = wcslen(range);
	ILIST il;
	RANGE range_copy = (RANGE) malloc((len+1)*sizeof(WCHAR));
	wcscpy_s(range_copy, len+1, range);
	il = range2ilist_destructive(range_copy);
	free(range_copy);
	return il;}

RANGE ilist2range(ILIST il){
	RANGE range;
	WCHAR num_str[10];  // storage for _itow_s
	int range_len = 0;
	int cur, previous, start;
	int i;
	ilist_sort(&il);
	if (0 == il.len){
		range = (RANGE) malloc(100*sizeof(WCHAR));
		range[0] = 0;}  // ""
	else {
		start = il.arr[0];
		previous = il.arr[0];                         // previous = start
		_itow_s(start, num_str, 10, 10);
		range_extend(&range, &range_len, num_str);    // range = "start"
		for(i=1;i<il.len;i++){
			cur = il.arr[i];
			if (cur <= previous + 1) previous = cur;  // update previous
			else {
				if (start == previous){  // range = range + ",cur"
					range_extend(&range, &range_len, SCOMMA);
					_itow_s(cur, num_str, 10, 10);
					range_extend(&range, &range_len, num_str);}
				else {                   // range = range + "-previous,cur"
					range_extend(&range, &range_len, SDASH);
					_itow_s(previous, num_str, 10, 10);
					range_extend(&range, &range_len, num_str);
					range_extend(&range, &range_len, SCOMMA);
					_itow_s(cur, num_str, 10, 10);
					range_extend(&range, &range_len, num_str);}
				start = cur;
				previous = cur;}}
		if (previous > start){           // range = range + "-previous"
			range_extend(&range, &range_len, SDASH);
			_itow_s(previous, num_str, 10, 10);
			range_extend(&range, &range_len, num_str);}}
	return range;}

RANGE sort_range(RANGE r){
	ILIST il = range2ilist(r);
	RANGE result = ilist2range(il); // ilist is sorted before conversion
	free(il.arr);
	return result;}


ILIST range2ilist_destructive(RANGE range){
	ILIST il, il1, il2;
	int i, start;
	WCHAR *comma;
	WCHAR *dash;
	WCHAR *end;
	if (0 == wcslen(range)){                // empty range
		il.len = 0;
		il.arr = NULL;}
	else {                                  // non-empty range
		comma = wcschr(range, COMMA);
		if (NULL != comma){                 // it is "range1,range2"
			*comma = 0;                     // now range is separated "range1\0range2"
			end = comma + 1;                // pointer to "range2"
			il1 = range2ilist_destructive(range);
			il2 = range2ilist_destructive(end);
			il.len = il1.len + il2.len;
			il.arr = (int*) malloc(il.len*sizeof(int));
			for(i=0;i<il1.len;i++) il.arr[i] = il1.arr[i];
			for(i=0;i<il2.len;i++) il.arr[il1.len+i] = il2.arr[i];
			free(il1.arr);
			free(il2.arr);}
		else {                              // no comma in range, it can be either 123 or 123-126
			dash = wcschr(range, DASH);
			if (NULL == dash){              // no dash in range, it is simple number
				il.len = 1;
				il.arr = (int*) malloc(il.len*sizeof(int));
				il.arr[0] = _wtoi(range);}
			else {                          // it is "123-126"
				*dash = 0;                  // now range is separated "123\0126"
				end = dash + 1;             // pointer to "126"
				start = _wtoi(range);       // start = 123
				il.len = _wtoi(end) - start + 1;
				il.arr = (int*) malloc(il.len*sizeof(int));
				for(i=0;i<il.len;i++) il.arr[i] = start + i;}}}
	il.allocated = il.len;
	return il;}


VOID range_extend(RANGE *r, int *cur_len, LPWSTR str){
	int block_size = 100;  // we extend allocated memory by 100;
	int blocks = (*cur_len / block_size) + 1; // current number of allocated blocks
	int new_len;
	if (0 == *cur_len) new_len = wcslen(str) + 1;
	else new_len = *cur_len + wcslen(str);
	int new_blocks = (new_len / block_size) + 1; // needed number of allocated blocks
	RANGE tmp;
	if ((new_blocks > block_size) || (0 == *cur_len)){
		tmp = (RANGE) malloc(new_blocks*block_size*sizeof(WCHAR));
		if (0 == *cur_len){
			wcscpy_s(tmp, new_len, str);
			*r = tmp;
			*cur_len = new_len;}
		else {
			wcscpy_s(tmp, new_len, *r);
			wcscat_s(tmp, new_len, str);
			free(*r);
			*r = tmp;
			*cur_len = new_len;}}
	else {
		wcscat_s(*r, new_len, str);
		*cur_len = new_len;}}

VOID ilist_extend(ILIST *il, int n){
	int block_size = 100;  // we extend allocated memory by 100 ints;
	int *tmp;
	int i;
	if (NULL == il->arr) { // empty
		il->len = 1;
		il->arr = (int*) malloc(block_size*sizeof(int));
		il->arr[0] = n;
		il->allocated = block_size;}
	else {
		if (il->allocated < (il->len + 1)) {
			tmp = (int*) malloc( (il->allocated + block_size)*sizeof(int) );
			for(i=0;i<il->len;i++) tmp[i] = il->arr[i];
			il->allocated += block_size;}
		il->arr[il->len] = n;
		il->len++;}}

int intcmp(const void *aa, const void *bb){
    const int *a = aa, *b = bb;
    return (*a < *b) ? -1 : (*a > *b);}

VOID ilist_sort(ILIST *il){
	if (0 == il->len) return;
	qsort(il->arr, il->len, sizeof(int), intcmp);
	// also removes duplicates
	int i, j, cur, next, curlen;
	i = 0;
	cur = il->arr[i];
	curlen = il->len;
	for(j=1;j<curlen;j++){
		next = il->arr[j];
		if (next == cur) il->len = il->len - 1;
		else {
			cur = next;
			i++;
			if (i != j) il->arr[i] = next;}}}


int range_len_destructive(RANGE range);

int range_len(RANGE range){
	int len = wcslen(range);
	RANGE range_copy = (RANGE) malloc((len+1)*sizeof(WCHAR));
	wcscpy_s(range_copy, len+1, range);
	len = range_len_destructive(range_copy);
	free(range_copy);
	return len;}

int range_len_destructive(RANGE range){
	WCHAR *comma;
	WCHAR *dash;
	WCHAR *end;
	if (0 == wcslen(range)) return 0;   // empty range
	comma = wcschr(range, COMMA);
	if (NULL == comma){                 // no comma in range, it can be either 123 or 123-126
		dash = wcschr(range, DASH);
		if (NULL == dash)               // no dash in range, it is simple number
			return 1;
		else {                          // it is "123-126"
			*dash = 0;                  // now range is separated "123\0126"
			end = dash + 1;             // pointer to "126"
			return _wtoi(end) - _wtoi(range) + 1;}}
	else {                              // it is "range1,range2"
		*comma = 0;                     // now range is separated "range1\0range2"
		end = comma + 1;                // pointer to "range2"
		return range_len_destructive(range) + range_len_destructive(end);}}

#pragma once

// Retrieve the system error message for the last-error code
LPCWSTR ErrorMessage(DWORD error);

// check if manual-reset event was signalled
BOOL signalled(HANDLE manual_reset_event);

////////////////////// array<->range operations

// range is a null-terminated string
// example: 1,2,10-15,31
typedef LPWSTR RANGE;

// ilist is array of ints with known length
typedef struct ilist {
	int *arr;
	int len;
} ILIST;

RANGE sort_range(RANGE r);
ILIST range2ilist(RANGE range);
RANGE ilist2range(ILIST ilist);
int range_len(RANGE range);
VOID ilist_sort(ILIST il);
BOOL in_ilist(ILIST il, int n);
VOID list_extend(ILIST il, int n);
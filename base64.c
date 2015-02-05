#ifndef UNICODE
#define UNICODE
#endif

#ifndef _UNICODE
#define _UNICODE
#endif

#include <windows.h>

// ANSI https://msdn.microsoft.com/en-us/library/aa245259%28v=vs.60%29.aspx
// '+' : 43, pos 62
// '/' : 47, pos 63
// '0'-'9' : 48-57, pos 52-61
// 'A'-'Z' : 65-90, pos 0-25
// 'a'-'z' : 97-122, pos 26-51
const WCHAR cb64[]=TEXT("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/");
const WCHAR padding = TEXT('=');

BYTE wchar2byte(WCHAR wc){
	BYTE c = (BYTE)wc;
	if (43 == c) return 62; // '+'
	if (47 == c) return 63; // '/'
	if (61 == c) return 64; // '='
	if (57 >= c) return c-48+52; // '0'-'9'
	if (90 >= c) return c-65+0; // 'A'-'Z'
	if (122 >= c) return c-97+26; // 'a'-'z'
	return 65;} // ERROR DATA PASSED!!


// encode block of 1-3 bytes into block of 4 WCHARs
VOID encodeblock(BYTE *in, WCHAR *out, int len){
	if (0 == len) return;
    out[0] = cb64[ (int)(in[0] >> 2) ];
    out[1] = cb64[ (int)(((in[0] & 0x03) << 4) | ((in[1] & 0xf0) >> 4)) ];
    out[2] = len == 1 ? padding : cb64[ (int)(((in[1] & 0x0f) << 2) | ((in[2] & 0xc0) >> 6)) ];
    out[3] = len != 3 ? padding : cb64[ (int)(in[2] & 0x3f) ];}

// decode block of 4 WCHARs into block of 1-3 bytes. 
size_t decodeblock(WCHAR *in, BYTE *out){
	int i;
	BYTE bin[4];
	for(i=0;i<4;i++) bin[i] = wchar2byte(in[i]);
    out[0] = (bin[0] << 2 | bin[1] >> 4);
	if (64 == bin[2]) return 1;
	else {
		out[1] = (bin[1] << 4 | bin[2] >> 2);
		if (64 == bin[3]) return 2;
		else out[2] = (((bin[2] << 6) & 0xc0) | bin[3]);}
	return 3;}

size_t base64encode(WCHAR* in, WCHAR* out, size_t outsize){
	BYTE *bin = (BYTE*)in;
	size_t i;
	int in_len = wcslen(in);
	if (0 == in_len){
		out[0] = 0;
		return 0;}
	int bin_len = in_len * sizeof(WCHAR);
	size_t need_out_len = (bin_len/3 + ((bin_len%3)>0 ? 1 : 0))*4;
	int processed, out_len;
	if (need_out_len < outsize){
		processed = in_len;
		out_len = need_out_len;}
	else {
		out_len = (outsize/4)*4 - ((outsize%4 == 0) ? 4 : 0);
		processed = ((out_len/4)*3)/sizeof(WCHAR);}
	out[out_len] = 0;
	for(i=0;i<(processed*sizeof(WCHAR))/3;i++){
		encodeblock(bin, out, 3);
		bin += 3;
		out += 4;}
	encodeblock(bin, out, (processed*sizeof(WCHAR))%3);
	return processed;}

size_t base64decode(WCHAR* in, WCHAR* out, size_t outsize){
	BYTE temp[3];
	BYTE *bout = (BYTE*)out;
	int last_len;
	int in_len = wcslen(in);
	size_t b_need_out_len = in_len/4*3;
	size_t need_out_len;
	int out_len;
	int processed;
	int i;
	if (0 == in_len){
		out[0] = 0;
		return 0;}
	WCHAR *lastblock = in + in_len - 4;
	last_len = decodeblock(lastblock, temp);
	b_need_out_len -= 3 - last_len;
	need_out_len = b_need_out_len/sizeof(WCHAR);
	if (need_out_len < outsize){
		processed = in_len;
		out_len = need_out_len;}
	else {
		processed = (((outsize - 1)*sizeof(WCHAR))/3)*4;
		out_len = processed/4*3/sizeof(WCHAR);}
	out[out_len] = 0;
	for(i=0;i<processed/4;i++){
		decodeblock(in, bout);
		in += 4;
		bout += 3;}
	return processed;}

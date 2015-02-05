#pragma once

// both return number of processed characters in in
// in must be a 0-terminated string
size_t base64encode(WCHAR* in, WCHAR* out, size_t outsize);
size_t base64decode(WCHAR* in, WCHAR* out, size_t outsize);

#pragma once

// Retrieve the system error message for the last-error code
LPCWSTR ErrorMessage(DWORD error);

// check if manual-reset event was signalled
BOOL signalled(HANDLE manual_reset_event);

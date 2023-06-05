#pragma once

#include <stdbool.h>

#include "struct.h"

// memo: グローバル変数は server.c の main で初期化する

message_state message;
bool should_exit = false;
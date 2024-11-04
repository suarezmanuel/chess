#pragma once

#ifndef SEARCH_H
#define SEARCH_H

#include "bbc.h"
#include "board.h"
#include "time_controls.h"
#include "evaluation.h"
#include "t_table.h"
#include "openings.h"

void search_position(int depth);

int negamax(int alpha, int beta, int depth);


#endif // SEARCH_H

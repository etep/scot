#ifndef __SJ_D2P_HPP__
#define __SJ_D2P_HPP__

#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

#define PRIMAL_AND_DUAL_FEASIBLE 1
#define NOT_PRIMAL_AND_DUAL_FEASIBLE -1
#define MEMORY_ALLOCATION_ERROR -2
#define GP_SOLVER_ERROR -3
#define DGOPT_ERROR -4

#define EXPRESSION_LEN 16

int dual2primal( const std::string & file_name, int numvar, int numcon, double * primal_solution, double * at_act, double * u_lower, double * p_obj, double * d_obj );

#endif

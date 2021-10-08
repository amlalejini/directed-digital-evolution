#pragma once

#include <cstdint>

#include "emp/math/math.hpp"

namespace dirdevo {

// a collection of one-input math functions that can be used as objectives
namespace MATH_1IN {
/*
 * 1AA: 2*A
 * 1AB: A**2
 * 1AC: A**3
 */

// 1AA: 2*A
double AA(double a) { return 2*a; }

// 1AB: A**2
double AB(double a) { return a*a; }

// 1AC: A**3
double AC(double a) { return a*a*a; }

}


// a collection of two-input math functions that can be used as objectives
namespace MATH_2IN {
/*
 * 2AA: A+B
 * 2AB: A*B
 * 2AC: A-B
 * 2AD: (A**2)+(B**2)
 * 2AE: (A**3)+(B**3)
 * 2AF: (A**2)-(B**2)
 * 2AG: (A**3)-(B**3)
 * 2AH: (A+B)/2
 */

// AA: A+B
double AA(double a, double b) { return a+b; }
// AB: A*B
double AB(double a, double b) { return a*b; }
// AC: A-B
double AC(double a, double b) { return a-b; }
// AD: (A**2)+(B**2)
double AD(double a, double b) { return (a*a)+(b*b); }
// AE: (A**3)+(B**3)
double AE(double a, double b) { return emp::Pow(a,3)+emp::Pow(b,3); }
// AF: (A**2)-(B**2)
double AF(double a, double b) { return emp::Pow(a,2)-emp::Pow(b,2); }
// AG: (A**3)-(B**3)
double AG(double a, double b) { return emp::Pow(a,3)-emp::Pow(b,3); }
// AH: (A+B)/2
double AH(double a, double b) { return (a+b)/2.0; }


}

} // end dirdevo namespace
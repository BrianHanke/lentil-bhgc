case zeiss__biotar__1927__58mm:
{
out[0] =  + 35.1489 *dx + 0.432266 *x + 7.45465 *dx*lambda + 0.173489 *x*lambda + -5.14858 *dx*lens_ipow(lambda, 2) + -22.9437 *dx*lens_ipow(dy, 2) + -22.2915 *lens_ipow(dx, 3) + -0.211488 *y*dx*dy + -0.000830058 *lens_ipow(y, 2)*dx + -0.121485 *x*lens_ipow(lambda, 2) + -0.211797 *x*lens_ipow(dy, 2) + -0.397613 *x*lens_ipow(dx, 2) + -0.00545491 *x*y*dy + -0.000172719 *x*lens_ipow(y, 2) + -0.00322696 *lens_ipow(x, 2)*dx + -0.000118282 *lens_ipow(x, 3) + -0.535554 *y*dx*lens_ipow(dy, 3) + -1.75879e-05 *lens_ipow(x, 2)*lens_ipow(y, 2)*dx + -0.000571988 *lens_ipow(x, 3)*lens_ipow(dx, 2) + -2.92895e-07 *lens_ipow(x, 3)*lens_ipow(y, 2) + -2.23348e-05 *lens_ipow(x, 4)*dx + -2.91234e-07 *lens_ipow(x, 5) + 0.0667704 *x*y*lens_ipow(dx, 2)*dy*lambda + -1.43404e-05 *lens_ipow(x, 3)*y*dy*lens_ipow(lambda, 2) + -4.16252e-08 *lens_ipow(x, 3)*lens_ipow(y, 3)*dy + -1.12545e-09 *lens_ipow(x, 3)*lens_ipow(y, 4) + -3.82538e-05 *lens_ipow(y, 4)*lens_ipow(dx, 3)*lambda + 23.5194 *x*lens_ipow(dx, 2)*lens_ipow(dy, 6);
out[1] =  + 36.7124 *dy + 0.466997 *y + 1.81549 *dy*lambda + 0.040883 *y*lambda + -22.9376 *lens_ipow(dy, 3) + -23.3952 *lens_ipow(dx, 2)*dy + -0.416494 *y*lens_ipow(dy, 2) + -0.204649 *y*lens_ipow(dx, 2) + -0.00469986 *lens_ipow(y, 2)*dy + -0.000128385 *lens_ipow(y, 3) + -0.00114048 *lens_ipow(x, 2)*dy + -0.000127148 *lens_ipow(x, 2)*y + -0.602487 *x*dx*dy*lambda + -0.014519 *x*y*dx*lambda + 0.0165266 *lens_ipow(y, 2)*lens_ipow(dy, 3) + -1.32635e-05 *lens_ipow(y, 4)*dy + -2.40237e-07 *lens_ipow(y, 5) + 0.036497 *x*y*dx*lens_ipow(dy, 2) + 0.0233551 *lens_ipow(x, 2)*lens_ipow(dx, 2)*dy + -1.62499e-05 *lens_ipow(x, 2)*lens_ipow(y, 2)*dy + -5.18445e-07 *lens_ipow(x, 2)*lens_ipow(y, 3) + 0.000354216 *lens_ipow(x, 3)*dx*dy + -2.16555e-07 *lens_ipow(x, 4)*y + 0.0211471 *x*y*dx*lens_ipow(lambda, 3) + -1.97264e-05 *x*lens_ipow(y, 3)*dx*lambda + -1.38489e-05 *lens_ipow(x, 3)*y*dx*lambda + 0.859791 *x*dx*dy*lens_ipow(lambda, 4) + 0.00557656 *lens_ipow(x, 2)*y*lens_ipow(dy, 6);
out[2] =  + 0.479392 *dx + -0.0227245 *x + -0.0297226 *dx*lambda + 0.00687204 *x*lambda + -0.223243 *dx*lens_ipow(dy, 2) + -0.291729 *lens_ipow(dx, 3) + -0.0168216 *y*dx*dy + -0.000121514 *lens_ipow(y, 2)*dx + -0.00487729 *x*lens_ipow(lambda, 2) + -0.00922672 *x*lens_ipow(dy, 2) + -0.0263168 *x*lens_ipow(dx, 2) + -0.000163467 *x*y*dy + -9.97029e-06 *x*lens_ipow(y, 2) + -9.21941e-06 *lens_ipow(x, 3) + -0.00104114 *lens_ipow(x, 2)*dx*lambda + 0.000916374 *lens_ipow(x, 2)*dx*lens_ipow(lambda, 2) + -1.9967e-08 *lens_ipow(x, 3)*lens_ipow(y, 2) + -9.63389e-09 *lens_ipow(x, 5) + 11.4182 *lens_ipow(dx, 5)*lens_ipow(dy, 2) + 3.5299 *lens_ipow(dx, 7) + -2.96752e-11 *x*lens_ipow(y, 6) + -1.82171e-07 *lens_ipow(x, 3)*lens_ipow(y, 2)*lens_ipow(dx, 2) + -3.34341e-08 *lens_ipow(x, 5)*lens_ipow(dy, 2) + -6.09981e-09 *lens_ipow(x, 4)*lens_ipow(y, 2)*dx*lambda + -3.55193e-09 *lens_ipow(x, 3)*lens_ipow(y, 3)*dy*lens_ipow(lambda, 2) + 1.91217e-10 *lens_ipow(x, 6)*y*dx*dy + -25.1971 *y*lens_ipow(dx, 3)*lens_ipow(dy, 7) + -6.41651e-16 *lens_ipow(x, 7)*lens_ipow(y, 4);
out[3] =  + 0.463472 *dy + -0.023279 *y + 0.00864086 *y*lambda + -0.357988 *lens_ipow(dy, 3) + -0.00661105 *y*lens_ipow(lambda, 2) + -0.0233253 *y*lens_ipow(dy, 2) + -0.00796186 *y*lens_ipow(dx, 2) + -7.77032e-06 *lens_ipow(y, 3) + -0.0148795 *x*dx*dy + -0.000148897 *x*y*dx + -9.76553e-06 *lens_ipow(x, 2)*y + -0.478026 *lens_ipow(dx, 2)*dy*lambda + -0.000390996 *lens_ipow(y, 2)*dy*lambda + -0.000215594 *lens_ipow(x, 2)*dy*lambda + 1.05347 *lens_ipow(dy, 5) + -1.37849e-05 *lens_ipow(y, 3)*lens_ipow(dx, 2) + -4.77688e-07 *lens_ipow(y, 4)*dy + -1.40346e-08 *lens_ipow(y, 5) + 0.0011623 *x*y*dx*lens_ipow(dy, 2) + 3.32433e-05 *x*lens_ipow(y, 2)*dx*dy + -1.92989e-08 *lens_ipow(x, 2)*lens_ipow(y, 3) + 2.92165 *lens_ipow(dx, 2)*lens_ipow(dy, 3)*lambda + -3.56736e-05 *lens_ipow(y, 3)*lens_ipow(dy, 2)*lambda + -0.112777 *x*lens_ipow(dx, 3)*dy*lens_ipow(lambda, 2) + -2.61308e-11 *lens_ipow(x, 6)*y + -4.49244e-09 *lens_ipow(x, 3)*lens_ipow(y, 3)*dx*lens_ipow(lambda, 2) + -3.38712e-07 *lens_ipow(x, 4)*y*lens_ipow(dy, 4) + -1.73323e-13 *lens_ipow(x, 4)*lens_ipow(y, 5);
out_transmittance =  + 0.656719  + 0.50033 *lambda + -0.691443 *lens_ipow(lambda, 2) + -0.000239983 *x*dx + 0.335931 *lens_ipow(lambda, 3) + -0.491401 *lens_ipow(dy, 4) + -0.990926 *lens_ipow(dx, 2)*lens_ipow(dy, 2) + -0.520168 *lens_ipow(dx, 4) + -0.0117579 *y*lens_ipow(dy, 3) + -0.0102295 *y*lens_ipow(dx, 2)*dy + -3.99765e-08 *lens_ipow(y, 4) + -0.00879357 *x*dx*lens_ipow(dy, 2) + -0.00906212 *x*lens_ipow(dx, 3) + -8.06172e-08 *lens_ipow(x, 2)*lens_ipow(y, 2) + -3.80865e-08 *lens_ipow(x, 4) + -0.000160962 *lens_ipow(y, 2)*lens_ipow(dy, 2)*lambda + -3.15666e-06 *lens_ipow(y, 3)*dy*lambda + 5.40434e-08 *lens_ipow(x, 2)*lens_ipow(y, 2)*lambda + -1.05632 *lens_ipow(dy, 6) + -3.36452 *lens_ipow(dx, 2)*lens_ipow(dy, 4) + -3.05685 *lens_ipow(dx, 4)*lens_ipow(dy, 2) + -0.903067 *lens_ipow(dx, 6) + -1.17527e-06 *lens_ipow(y, 4)*lens_ipow(dx, 4) + -0.00165167 *lens_ipow(x, 2)*lens_ipow(dy, 6) + 2.95808e-09 *lens_ipow(x, 3)*lens_ipow(y, 3)*dx*dy + -6.58566e-13 *lens_ipow(x, 4)*lens_ipow(y, 4) + 0.125714 *x*y*lens_ipow(dx, 3)*lens_ipow(dy, 5) + 1.79559e-06 *lens_ipow(x, 2)*lens_ipow(y, 3)*lens_ipow(dx, 2)*lens_ipow(dy, 3);
} break;

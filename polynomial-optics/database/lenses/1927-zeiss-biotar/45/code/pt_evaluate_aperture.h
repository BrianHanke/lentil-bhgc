case zeiss__biotar__1927__45mm:
{
out[0] =  + 28.6047 *dx + 0.467239 *x + 1.14365 *dx*lambda + 0.0371139 *x*lambda + -17.5745 *dx*lens_ipow(dy, 2) + -17.5355 *lens_ipow(dx, 3) + -0.194251 *x*lens_ipow(dy, 2) + -0.406714 *x*lens_ipow(dx, 2) + -0.000173671 *x*lens_ipow(y, 2) + -0.00715064 *lens_ipow(x, 2)*dx + -0.000179487 *lens_ipow(x, 3) + -0.335571 *y*dx*dy*lambda + -0.00775566 *x*y*dy*lambda + 0.0210202 *lens_ipow(y, 2)*dx*lens_ipow(dy, 2) + 0.0518338 *x*y*lens_ipow(dx, 2)*dy + -2.86047e-05 *x*lens_ipow(y, 3)*dy + -8.8463e-07 *x*lens_ipow(y, 4) + -4.24333e-05 *lens_ipow(x, 2)*lens_ipow(y, 2)*dx + -2.6356e-05 *lens_ipow(x, 3)*y*dy + -1.50714e-06 *lens_ipow(x, 3)*lens_ipow(y, 2) + -8.0149e-07 *lens_ipow(x, 5) + -1.63866e-05 *lens_ipow(y, 4)*dx*lambda + 0.0369267 *lens_ipow(x, 2)*lens_ipow(dx, 3)*lambda + -0.0348545 *lens_ipow(y, 2)*lens_ipow(dx, 5) + -2.76212e-07 *lens_ipow(x, 6)*dx*lambda + 0.105319 *x*y*lens_ipow(dy, 3)*lens_ipow(lambda, 6) + -2.34493e-14 *lens_ipow(x, 7)*lens_ipow(y, 4) + 6.95643e-10 *lens_ipow(x, 8)*dx*lens_ipow(lambda, 2);
out[1] =  + 28.4706 *dy + 0.467672 *y + 1.23103 *dy*lambda + 0.0416275 *y*lambda + -16.8339 *lens_ipow(dy, 3) + -15.35 *lens_ipow(dx, 2)*dy + -0.419807 *y*lens_ipow(dy, 2) + -0.00642829 *lens_ipow(y, 2)*dy + -0.000269039 *lens_ipow(y, 3) + -0.000196919 *lens_ipow(x, 2)*y + -0.341036 *y*lens_ipow(dx, 2)*lambda + -0.302272 *x*dx*dy*lambda + -0.00744506 *x*y*dx*lambda + 0.0401756 *x*y*dx*lens_ipow(dy, 2) + -2.54482e-05 *x*lens_ipow(y, 3)*dx + -4.26211e-05 *lens_ipow(x, 2)*lens_ipow(y, 2)*dy + -1.40259e-06 *lens_ipow(x, 2)*lens_ipow(y, 3) + -6.36972e-07 *lens_ipow(x, 4)*y + -3.36196e-05 *lens_ipow(x, 3)*y*dx*lambda + -0.00111466 *lens_ipow(y, 3)*lens_ipow(dy, 2)*lens_ipow(lambda, 2) + -2.14855e-09 *lens_ipow(y, 7) + -14.415 *x*lens_ipow(dx, 3)*lens_ipow(dy, 3) + -0.000106831 *lens_ipow(x, 4)*lens_ipow(dy, 3) + -2.86041e-07 *lens_ipow(y, 6)*dy*lambda + -1131.33 *lens_ipow(dx, 4)*lens_ipow(dy, 3)*lens_ipow(lambda, 2) + -2.59299e-08 *lens_ipow(y, 7)*lens_ipow(dy, 2)*lens_ipow(lambda, 2) + -3.16998e-14 *lens_ipow(x, 4)*lens_ipow(y, 7) + -6.77921e-08 *lens_ipow(x, 6)*y*lens_ipow(dy, 4);
out[2] =  + 0.459939 *dx + -0.0300371 *x + 0.0124931 *x*lambda + -0.160754 *dx*lens_ipow(dy, 2) + -0.202157 *lens_ipow(dx, 3) + -0.0208008 *y*dx*dy + -0.0100829 *x*lens_ipow(lambda, 2) + -0.0112328 *x*lens_ipow(dy, 2) + -0.0286454 *x*lens_ipow(dx, 2) + -2.12163e-05 *x*lens_ipow(y, 2) + -2.44465e-05 *lens_ipow(x, 3) + -0.000374271 *lens_ipow(y, 2)*dx*lambda + -0.000338572 *x*y*dy*lambda + -0.000918477 *lens_ipow(x, 2)*dx*lambda + -5.11606e-08 *x*lens_ipow(y, 4) + -0.0792312 *x*lens_ipow(dx, 4)*lambda + -3.64441e-06 *x*lens_ipow(y, 3)*dy*lambda + -0.119164 *x*lens_ipow(dx, 2)*lens_ipow(dy, 2)*lens_ipow(lambda, 2) + -0.000100698 *x*lens_ipow(y, 2)*lens_ipow(dy, 2)*lens_ipow(lambda, 2) + -3.53991e-10 *lens_ipow(x, 5)*lens_ipow(y, 2) + -8.2615e-11 *lens_ipow(x, 7) + -1.1138e-06 *x*lens_ipow(y, 4)*lens_ipow(dx, 4) + 5.05298e-09 *lens_ipow(x, 6)*y*dx*dy*lambda + -0.00972194 *lens_ipow(y, 3)*lens_ipow(dx, 3)*lens_ipow(dy, 3)*lens_ipow(lambda, 2) + -3.18087e-15 *lens_ipow(x, 3)*lens_ipow(y, 8) + 2.16463e-07 *lens_ipow(x, 5)*y*lens_ipow(dx, 2)*dy*lens_ipow(lambda, 2) + -3.79504e-15 *lens_ipow(x, 7)*lens_ipow(y, 4) + -1.70414e-12 *lens_ipow(x, 9)*lens_ipow(dy, 2);
out[3] =  + 0.458036 *dy + -0.0302526 *y + 0.0126122 *y*lambda + -0.185902 *lens_ipow(dy, 3) + -0.0100008 *y*lens_ipow(lambda, 2) + -0.0305904 *y*lens_ipow(dy, 2) + -0.0083834 *y*lens_ipow(dx, 2) + -2.21994e-05 *lens_ipow(y, 3) + -0.0170505 *x*dx*dy + -1.963e-05 *lens_ipow(x, 2)*y + -0.388933 *lens_ipow(dx, 2)*dy*lambda + -0.000669968 *lens_ipow(y, 2)*dy*lambda + -0.0003175 *lens_ipow(x, 2)*dy*lambda + -6.91563e-07 *lens_ipow(y, 4)*dy + -3.73981e-08 *lens_ipow(x, 4)*y + -8.77736e-05 *lens_ipow(y, 3)*lens_ipow(dx, 2)*lambda + -4.57703e-06 *x*lens_ipow(y, 3)*dx*lambda + -1.84463e-06 *lens_ipow(x, 3)*y*dx*lambda + 10.2163 *lens_ipow(dx, 2)*lens_ipow(dy, 5) + -1.13247e-10 *lens_ipow(y, 7) + -0.155801 *x*lens_ipow(dx, 3)*dy*lens_ipow(lambda, 2) + -6.25276e-10 *lens_ipow(x, 2)*lens_ipow(y, 5) + 0.00946412 *lens_ipow(y, 2)*lens_ipow(dy, 5)*lambda + -1.04817e-06 *lens_ipow(x, 2)*lens_ipow(y, 3)*lens_ipow(dy, 2)*lambda + -3.52057e-08 *lens_ipow(x, 2)*lens_ipow(y, 4)*dy*lambda + -1.31661e-12 *lens_ipow(x, 6)*lens_ipow(y, 3) + 0.0131491 *x*y*dx*lens_ipow(dy, 2)*lens_ipow(lambda, 5) + 3.77028e-11 *lens_ipow(x, 8)*lens_ipow(dy, 3);
out_transmittance =  + 0.653477  + 0.516111 *lambda + -0.718579 *lens_ipow(lambda, 2) + 0.351138 *lens_ipow(lambda, 3) + -0.000631136 *x*dx*lambda + -1.35242 *lens_ipow(dx, 2)*lens_ipow(dy, 2) + -0.333503 *lens_ipow(dx, 4) + -0.0135462 *y*lens_ipow(dy, 3) + -0.0075004 *y*lens_ipow(dx, 2)*dy + -5.49755e-08 *lens_ipow(y, 4) + -0.0090516 *x*dx*lens_ipow(dy, 2) + 0.000312883 *x*y*dx*dy + -8.24737e-05 *lens_ipow(x, 2)*lens_ipow(dy, 2) + -1.96175e-07 *lens_ipow(x, 2)*lens_ipow(y, 2) + -0.00024264 *lens_ipow(y, 2)*lens_ipow(dy, 2)*lambda + -7.81186e-06 *lens_ipow(y, 3)*dy*lambda + -9.28099 *lens_ipow(dy, 6) + -2.24756 *lens_ipow(dx, 6) + -2.79841e-10 *lens_ipow(y, 6) + -0.0886047 *x*lens_ipow(dx, 5) + -1.8891e-08 *lens_ipow(x, 5)*dx + -5.46797e-10 *lens_ipow(x, 6) + 47.2481 *lens_ipow(dy, 8) + 0.000181056 *lens_ipow(x, 3)*lens_ipow(dx, 5) + -4.22345e-12 *lens_ipow(x, 4)*lens_ipow(y, 4) + -39.7622 *lens_ipow(dx, 4)*lens_ipow(dy, 4)*lambda + -90.3476 *lens_ipow(dy, 10) + -1.80384e-05 *lens_ipow(y, 4)*lens_ipow(dx, 6);
} break;
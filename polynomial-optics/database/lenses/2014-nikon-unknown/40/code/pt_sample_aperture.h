case nikon__unknown__2014__40mm:
{
double pred_x;
double pred_y;
double pred_dx;
double pred_dy;
double sqr_err = FLT_MAX;
for(int k=0;k<5&&sqr_err > 1e-4f;k++)
{
  const double begin_x = x + dist * dx;
  const double begin_y = y + dist * dy;
  const double begin_dx = dx;
  const double begin_dy = dy;
  const double begin_lambda = lambda;
  pred_x =  + 25.7302 *begin_dx + 0.293882 *begin_x + 6.58463 *begin_dx*begin_lambda + 0.235593 *begin_x*begin_lambda + -4.54122 *begin_dx*lens_ipow(begin_lambda, 2) + -12.8642 *begin_dx*lens_ipow(begin_dy, 2) + -12.7559 *lens_ipow(begin_dx, 3) + 0.00207335 *lens_ipow(begin_y, 2)*begin_dx + -0.172285 *begin_x*lens_ipow(begin_lambda, 2) + -0.0859977 *begin_x*lens_ipow(begin_dy, 2) + -0.142121 *begin_x*lens_ipow(begin_dx, 2) + 0.00187845 *begin_x*begin_y*begin_dy + -0.000250022 *begin_x*lens_ipow(begin_y, 2) + 0.00353244 *lens_ipow(begin_x, 2)*begin_dx + -0.000206067 *lens_ipow(begin_x, 3) + 0.000110974 *begin_x*lens_ipow(begin_y, 2)*begin_lambda + -0.345645 *begin_y*begin_dx*lens_ipow(begin_dy, 3) + -0.438949 *begin_y*lens_ipow(begin_dx, 3)*begin_dy + -0.012105 *lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 3) + -4.19305e-07 *begin_x*lens_ipow(begin_y, 4) + -0.0204557 *lens_ipow(begin_x, 2)*begin_dx*lens_ipow(begin_dy, 2) + -0.010444 *lens_ipow(begin_x, 2)*lens_ipow(begin_dx, 3) + -7.81489e-07 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 2) + -5.94993e-07 *lens_ipow(begin_x, 5) + 4.89429e-07 *lens_ipow(begin_x, 5)*begin_lambda + -0.0688526 *begin_x*begin_y*lens_ipow(begin_dy, 5) + -2.12791e-10 *lens_ipow(begin_x, 6)*lens_ipow(begin_y, 3)*begin_dx*begin_dy + 1.57789e-10 *lens_ipow(begin_x, 8)*begin_dx*lens_ipow(begin_lambda, 2);
  pred_y =  + 25.8075 *begin_dy + 0.29468 *begin_y + 6.44578 *begin_dy*begin_lambda + 0.235435 *begin_y*begin_lambda + -4.47832 *begin_dy*lens_ipow(begin_lambda, 2) + -12.9744 *lens_ipow(begin_dy, 3) + -12.8068 *lens_ipow(begin_dx, 2)*begin_dy + -0.171039 *begin_y*lens_ipow(begin_lambda, 2) + -0.160455 *begin_y*lens_ipow(begin_dy, 2) + -0.000317019 *lens_ipow(begin_y, 3) + 0.00186033 *begin_x*begin_y*begin_dx + 0.00201847 *lens_ipow(begin_x, 2)*begin_dy + -0.000243015 *lens_ipow(begin_x, 2)*begin_y + -0.153224 *begin_y*lens_ipow(begin_dx, 2)*begin_lambda + 0.00369536 *lens_ipow(begin_y, 2)*begin_dy*begin_lambda + 0.000126257 *lens_ipow(begin_y, 3)*begin_lambda + 9.78775e-05 *lens_ipow(begin_x, 2)*begin_y*begin_lambda + 0.000521245 *lens_ipow(begin_y, 3)*lens_ipow(begin_dy, 2) + 1.41458e-05 *lens_ipow(begin_y, 4)*begin_dy + -0.459062 *begin_x*begin_dx*lens_ipow(begin_dy, 3) + -0.295035 *begin_x*lens_ipow(begin_dx, 3)*begin_dy + -0.0117382 *lens_ipow(begin_x, 2)*lens_ipow(begin_dy, 3) + -7.95867e-07 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 3) + -4.10607e-07 *lens_ipow(begin_x, 4)*begin_y + -0.0368216 *lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 2)*begin_dy*begin_lambda + -6.48573e-10 *lens_ipow(begin_y, 7) + -0.202996 *begin_x*begin_y*lens_ipow(begin_dx, 5)*lens_ipow(begin_lambda, 2) + -3.54037e-08 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 4)*begin_dx*begin_dy;
  pred_dx =  + 0.334302 *begin_dx + -0.0355676 *begin_x + -0.0239044 *begin_dx*begin_lambda + 0.0136166 *begin_x*begin_lambda + -0.364943 *begin_dx*lens_ipow(begin_dy, 2) + -0.409715 *lens_ipow(begin_dx, 3) + -0.0130144 *begin_y*begin_dx*begin_dy + -0.010679 *begin_x*lens_ipow(begin_lambda, 2) + -0.00698354 *begin_x*lens_ipow(begin_dy, 2) + -0.0191151 *begin_x*lens_ipow(begin_dx, 2) + -2.97416e-05 *begin_x*lens_ipow(begin_y, 2) + -2.01204e-05 *lens_ipow(begin_x, 3) + 1.36788e-05 *begin_x*lens_ipow(begin_y, 2)*begin_lambda + 1.97719e-06 *lens_ipow(begin_x, 3)*begin_y*begin_dy + -2.02291e-08 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 2) + 1.83045e-06 *lens_ipow(begin_x, 4)*begin_dx + -4.90805e-08 *lens_ipow(begin_x, 5) + 8.19724e-08 *lens_ipow(begin_x, 5)*begin_lambda + 2.13105 *lens_ipow(begin_dx, 7) + 9.93445e-10 *lens_ipow(begin_y, 6)*begin_dx + 8.59367e-09 *begin_x*lens_ipow(begin_y, 5)*begin_dy + 1.13938e-08 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 4)*begin_dx + -2.38204e-07 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 2) + 94.8767 *lens_ipow(begin_dx, 5)*lens_ipow(begin_dy, 4) + -7.58249e-13 *lens_ipow(begin_x, 5)*lens_ipow(begin_y, 4) + -1.35249e-10 *lens_ipow(begin_x, 7)*lens_ipow(begin_lambda, 2) + 4.94568e-05 *lens_ipow(begin_x, 4)*begin_y*lens_ipow(begin_dx, 3)*lens_ipow(begin_dy, 3) + 9.6551e-14 *lens_ipow(begin_x, 8)*lens_ipow(begin_y, 2)*begin_dx;
  pred_dy =  + 0.319863 *begin_dy + -0.0366104 *begin_y + 0.0160443 *begin_y*begin_lambda + -0.424475 *lens_ipow(begin_dy, 3) + -0.0115659 *begin_y*lens_ipow(begin_lambda, 2) + -0.0196326 *begin_y*lens_ipow(begin_dy, 2) + -2.14172e-05 *lens_ipow(begin_y, 3) + -0.0118753 *begin_x*begin_dx*begin_dy + 0.000205025 *begin_x*begin_y*begin_dx + -1.94561e-05 *lens_ipow(begin_x, 2)*begin_y + -0.66061 *lens_ipow(begin_dx, 2)*begin_dy*begin_lambda + -0.0121327 *begin_y*lens_ipow(begin_dx, 2)*begin_lambda + 0.540037 *lens_ipow(begin_dy, 5) + 2.00932e-06 *lens_ipow(begin_y, 4)*begin_dy + 2.02832e-06 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 2)*begin_dy + -2.14706e-08 *lens_ipow(begin_x, 4)*begin_y + -0.00149217 *lens_ipow(begin_y, 2)*lens_ipow(begin_dy, 3)*lens_ipow(begin_lambda, 2) + -5.45409e-10 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 5) + 1.5124e-08 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 3)*begin_dx + 1.28962e-09 *lens_ipow(begin_x, 6)*begin_dy + -0.00355184 *begin_x*begin_y*lens_ipow(begin_dx, 3)*lens_ipow(begin_lambda, 3) + 6.57105e-10 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 5)*begin_lambda + -5.10586e-10 *lens_ipow(begin_y, 7)*lens_ipow(begin_dx, 2) + -1.17136e-13 *lens_ipow(begin_y, 9) + 9.09353e-12 *lens_ipow(begin_x, 7)*begin_y*begin_dx + 91.9537 *lens_ipow(begin_dx, 4)*lens_ipow(begin_dy, 3)*lens_ipow(begin_lambda, 4) + -1.07302e-07 *lens_ipow(begin_y, 6)*lens_ipow(begin_dx, 2)*begin_dy*lens_ipow(begin_lambda, 2) + -1.3679e-11 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 5)*lens_ipow(begin_dy, 2);
  Eigen::Matrix2d dx1_domega0;
  dx1_domega0(0, 0) =  + 25.7302  + 6.58463 *begin_lambda + -4.54122 *lens_ipow(begin_lambda, 2) + -12.8642 *lens_ipow(begin_dy, 2) + -38.2676 *lens_ipow(begin_dx, 2) + 0.00207335 *lens_ipow(begin_y, 2) + -0.284241 *begin_x*begin_dx + 0.00353244 *lens_ipow(begin_x, 2) + -0.345645 *begin_y*lens_ipow(begin_dy, 3) + -1.31685 *begin_y*lens_ipow(begin_dx, 2)*begin_dy + -0.0363151 *lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 2) + -0.0204557 *lens_ipow(begin_x, 2)*lens_ipow(begin_dy, 2) + -0.0313321 *lens_ipow(begin_x, 2)*lens_ipow(begin_dx, 2) + -2.12791e-10 *lens_ipow(begin_x, 6)*lens_ipow(begin_y, 3)*begin_dy + 1.57789e-10 *lens_ipow(begin_x, 8)*lens_ipow(begin_lambda, 2)+0.0f;
  dx1_domega0(0, 1) =  + -25.7285 *begin_dx*begin_dy + -0.171995 *begin_x*begin_dy + 0.00187845 *begin_x*begin_y + -1.03693 *begin_y*begin_dx*lens_ipow(begin_dy, 2) + -0.438949 *begin_y*lens_ipow(begin_dx, 3) + -0.0409114 *lens_ipow(begin_x, 2)*begin_dx*begin_dy + -0.344263 *begin_x*begin_y*lens_ipow(begin_dy, 4) + -2.12791e-10 *lens_ipow(begin_x, 6)*lens_ipow(begin_y, 3)*begin_dx+0.0f;
  dx1_domega0(1, 0) =  + -25.6136 *begin_dx*begin_dy + 0.00186033 *begin_x*begin_y + -0.306448 *begin_y*begin_dx*begin_lambda + -0.459062 *begin_x*lens_ipow(begin_dy, 3) + -0.885106 *begin_x*lens_ipow(begin_dx, 2)*begin_dy + -0.0736432 *lens_ipow(begin_y, 2)*begin_dx*begin_dy*begin_lambda + -1.01498 *begin_x*begin_y*lens_ipow(begin_dx, 4)*lens_ipow(begin_lambda, 2) + -3.54037e-08 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 4)*begin_dy+0.0f;
  dx1_domega0(1, 1) =  + 25.8075  + 6.44578 *begin_lambda + -4.47832 *lens_ipow(begin_lambda, 2) + -38.9232 *lens_ipow(begin_dy, 2) + -12.8068 *lens_ipow(begin_dx, 2) + -0.320909 *begin_y*begin_dy + 0.00201847 *lens_ipow(begin_x, 2) + 0.00369536 *lens_ipow(begin_y, 2)*begin_lambda + 0.00104249 *lens_ipow(begin_y, 3)*begin_dy + 1.41458e-05 *lens_ipow(begin_y, 4) + -1.37719 *begin_x*begin_dx*lens_ipow(begin_dy, 2) + -0.295035 *begin_x*lens_ipow(begin_dx, 3) + -0.0352147 *lens_ipow(begin_x, 2)*lens_ipow(begin_dy, 2) + -0.0368216 *lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 2)*begin_lambda + -3.54037e-08 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 4)*begin_dx+0.0f;
  Eigen::Matrix2d invJ;
  const double invdet = 1.0f/(dx1_domega0(0, 0)*dx1_domega0(1, 1) - dx1_domega0(0, 1)*dx1_domega0(1, 0));
  invJ(0, 0) =  dx1_domega0(1, 1)*invdet;
  invJ(1, 1) =  dx1_domega0(0, 0)*invdet;
  invJ(0, 1) = -dx1_domega0(0, 1)*invdet;
  invJ(1, 0) = -dx1_domega0(1, 0)*invdet;
  const Eigen::Vector2d dx1(out_x - pred_x, out_y - pred_y);
  for(int i=0;i<2;i++)
  {
    dx += invJ(0, i)*dx1(i);
    dy += invJ(1, i)*dx1(i);
  }
  sqr_err = dx1(0)*dx1(0) + dx1(1)*dx1(1);
}
out_dx = pred_dx;
out_dy = pred_dy;
} break;
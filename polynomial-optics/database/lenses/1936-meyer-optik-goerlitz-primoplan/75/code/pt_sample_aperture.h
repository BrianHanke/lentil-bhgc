case meyer_optik_goerlitz__primoplan__1936__75mm:
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
  pred_x =  + 47.0034 *begin_dx + 0.587174 *begin_x + 1.33686 *begin_y*begin_dx*begin_dy + 0.0322488 *begin_x*begin_y*begin_dy + 0.0360982 *lens_ipow(begin_x, 2)*begin_dx + 15.6255 *lens_ipow(begin_dx, 3) + 21.4697 *begin_dx*lens_ipow(begin_dy, 2) + 0.000262752 *begin_x*lens_ipow(begin_y, 2) + 0.778028 *begin_x*lens_ipow(begin_dy, 2) + 1.69536 *begin_x*lens_ipow(begin_dx, 2) + 0.0148954 *lens_ipow(begin_y, 2)*begin_dx + 0.000428019 *lens_ipow(begin_x, 3)*begin_lambda + -5.02979e-07 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 2) + 5.68624 *begin_x*lens_ipow(begin_dx, 4)*begin_lambda + -2.33808e-07 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 3)*begin_dy + -4.53937e-09 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 4) + 0.000119501 *lens_ipow(begin_x, 4)*begin_dx*lens_ipow(begin_lambda, 2) + 0.000414493 *begin_x*lens_ipow(begin_y, 3)*lens_ipow(begin_dx, 2)*begin_dy + -1.68624e-07 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 4)*begin_dx + 0.0203951 *lens_ipow(begin_x, 3)*lens_ipow(begin_dx, 2)*lens_ipow(begin_lambda, 2) + 1.03832 *lens_ipow(begin_x, 2)*lens_ipow(begin_dx, 3)*lens_ipow(begin_lambda, 2) + -8.32952e-07 *lens_ipow(begin_x, 5)*lens_ipow(begin_lambda, 2) + -2.74603e-07 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 2)*begin_dx*begin_lambda + 81.2628 *begin_x*lens_ipow(begin_dx, 6)*lens_ipow(begin_lambda, 3) + 7.6954e-05 *begin_x*lens_ipow(begin_y, 4)*lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 2)*begin_lambda + -8.79204e-05 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 3)*begin_dx*lens_ipow(begin_dy, 3)*begin_lambda + 3313.51 *lens_ipow(begin_dx, 9)*lens_ipow(begin_lambda, 2) + -3.83583e-08 *lens_ipow(begin_x, 5)*lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 2)*lens_ipow(begin_lambda, 2);
  pred_y =  + 0.591943 *begin_y + 46.4005 *begin_dy + 1.5057 *begin_dy*begin_lambda + 0.824423 *begin_y*lens_ipow(begin_dx, 2) + 0.027811 *lens_ipow(begin_y, 2)*begin_dy + 1.30781 *begin_y*lens_ipow(begin_dy, 2) + 1.29332 *begin_x*begin_dx*begin_dy + 0.0322068 *begin_x*begin_y*begin_dx + 21.1912 *lens_ipow(begin_dx, 2)*begin_dy + 0.000266444 *lens_ipow(begin_x, 2)*begin_y + 0.0144468 *lens_ipow(begin_x, 2)*begin_dy + 10.8321 *lens_ipow(begin_dy, 3) + 0.000272145 *lens_ipow(begin_y, 3)*begin_lambda + -6.46083e-07 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 3) + 0.00013376 *lens_ipow(begin_y, 4)*begin_dy*begin_lambda + 0.0169126 *lens_ipow(begin_y, 3)*lens_ipow(begin_dy, 2)*begin_lambda + 13.3259 *begin_y*lens_ipow(begin_dy, 4)*begin_lambda + 0.843646 *lens_ipow(begin_y, 2)*lens_ipow(begin_dy, 3)*begin_lambda + 0.000247138 *lens_ipow(begin_x, 3)*begin_y*begin_dx*lens_ipow(begin_dy, 2) + -2.26643e-07 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 3)*begin_dx + -1.84505 *begin_y*lens_ipow(begin_dx, 6) + -3.96203e-09 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 3) + -1.23991e-07 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 2)*begin_dy + 21.6211 *begin_y*lens_ipow(begin_dy, 6)*begin_lambda + 648.27 *lens_ipow(begin_dy, 7)*begin_lambda + -3.88112e-07 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 4)*begin_dy*begin_lambda + 1.54248e-06 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 4)*lens_ipow(begin_dx, 2)*begin_dy*begin_lambda + -4.97352e-08 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 5)*lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 2);
  pred_dx =  + 1.40451 *begin_dx + -0.00484464 *begin_x + -0.160233 *begin_dx*begin_lambda + 0.00193042 *begin_x*begin_y*begin_dy + 0.00265099 *lens_ipow(begin_x, 2)*begin_dx + 1.60028 *lens_ipow(begin_dx, 3) + 1.52955 *begin_dx*lens_ipow(begin_dy, 2) + 2.20572e-05 *begin_x*lens_ipow(begin_y, 2) + 1.47213e-05 *lens_ipow(begin_x, 3) + 0.0343307 *begin_x*lens_ipow(begin_dy, 2) + 0.0833062 *begin_x*lens_ipow(begin_dx, 2) + 0.00105418 *lens_ipow(begin_y, 2)*begin_dx + 0.241326 *begin_y*begin_dx*begin_dy*begin_lambda + -5.51372e-08 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 2) + 0.00358005 *begin_x*begin_y*lens_ipow(begin_dx, 2)*begin_dy + -0.213603 *begin_y*begin_dx*begin_dy*lens_ipow(begin_lambda, 2) + 7.47824e-05 *lens_ipow(begin_x, 3)*lens_ipow(begin_dy, 2) + -0.0646663 *begin_y*begin_dx*lens_ipow(begin_dy, 3) + 0.000234475 *lens_ipow(begin_x, 2)*begin_y*begin_dx*begin_dy + 0.0073239 *lens_ipow(begin_x, 2)*begin_dx*lens_ipow(begin_dy, 2) + 0.000558386 *lens_ipow(begin_x, 3)*lens_ipow(begin_dx, 2) + 3.03686e-05 *lens_ipow(begin_x, 3)*begin_y*lens_ipow(begin_dx, 2)*begin_dy + 0.000216273 *lens_ipow(begin_x, 4)*lens_ipow(begin_dx, 3) + -6.51557e-09 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 2)*begin_dx + 3.05763e-05 *lens_ipow(begin_x, 5)*lens_ipow(begin_dx, 4) + 1.05328e-05 *lens_ipow(begin_x, 5)*lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 2)*begin_lambda + 1.47366e-07 *lens_ipow(begin_x, 6)*begin_dx*lens_ipow(begin_dy, 2)*begin_lambda + 1.51331e-06 *lens_ipow(begin_x, 6)*lens_ipow(begin_dx, 5);
  pred_dy =  + -0.00455927 *begin_y + 1.3564 *begin_dy + 0.0346639 *begin_y*lens_ipow(begin_dx, 2) + 0.00261008 *lens_ipow(begin_y, 2)*begin_dy + 0.00196059 *begin_x*begin_y*begin_dx + 2.02926e-05 *lens_ipow(begin_x, 2)*begin_y + -0.122139 *begin_dy*lens_ipow(begin_lambda, 2) + 1.31784e-05 *lens_ipow(begin_y, 3) + 0.0010512 *lens_ipow(begin_x, 2)*begin_dy + 1.55828 *lens_ipow(begin_dy, 3) + 0.275257 *begin_y*lens_ipow(begin_dy, 2)*begin_lambda + 3.2858 *lens_ipow(begin_dx, 2)*begin_dy*begin_lambda + 0.1406 *begin_x*begin_dx*begin_dy*begin_lambda + 0.00418467 *lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 2)*begin_dy + 4.82866e-05 *lens_ipow(begin_y, 3)*lens_ipow(begin_dx, 2) + 0.00051215 *lens_ipow(begin_y, 3)*lens_ipow(begin_dy, 2) + -0.239769 *begin_y*lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 2) + 0.000192137 *lens_ipow(begin_y, 4)*lens_ipow(begin_dy, 3) + 0.0062842 *lens_ipow(begin_x, 2)*lens_ipow(begin_dx, 2)*begin_dy*lens_ipow(begin_lambda, 2) + -2.70332e-10 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 3) + -9.69342e-09 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 2)*begin_dy + -3.77088e-11 *lens_ipow(begin_x, 5)*lens_ipow(begin_y, 3)*begin_dx + 2.76946e-05 *lens_ipow(begin_y, 5)*lens_ipow(begin_dy, 4) + -1.86032 *begin_x*begin_dx*lens_ipow(begin_dy, 3)*lens_ipow(begin_lambda, 5) + -7.87241e-09 *lens_ipow(begin_x, 5)*lens_ipow(begin_y, 2)*begin_dx*lens_ipow(begin_dy, 3) + 7.5683e-11 *lens_ipow(begin_x, 5)*lens_ipow(begin_y, 3)*lens_ipow(begin_dx, 3) + 1.40134e-06 *lens_ipow(begin_y, 6)*lens_ipow(begin_dy, 5) + -92.0108 *lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 3)*lens_ipow(begin_lambda, 6);
  Eigen::Matrix2d dx1_domega0;
  dx1_domega0(0, 0) =  + 47.0034  + 1.33686 *begin_y*begin_dy + 0.0360982 *lens_ipow(begin_x, 2) + 46.8766 *lens_ipow(begin_dx, 2) + 21.4697 *lens_ipow(begin_dy, 2) + 3.39072 *begin_x*begin_dx + 0.0148954 *lens_ipow(begin_y, 2) + 22.7449 *begin_x*lens_ipow(begin_dx, 3)*begin_lambda + 0.000119501 *lens_ipow(begin_x, 4)*lens_ipow(begin_lambda, 2) + 0.000828986 *begin_x*lens_ipow(begin_y, 3)*begin_dx*begin_dy + -1.68624e-07 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 4) + 0.0407903 *lens_ipow(begin_x, 3)*begin_dx*lens_ipow(begin_lambda, 2) + 3.11496 *lens_ipow(begin_x, 2)*lens_ipow(begin_dx, 2)*lens_ipow(begin_lambda, 2) + -2.74603e-07 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 2)*begin_lambda + 487.577 *begin_x*lens_ipow(begin_dx, 5)*lens_ipow(begin_lambda, 3) + 0.000153908 *begin_x*lens_ipow(begin_y, 4)*begin_dx*lens_ipow(begin_dy, 2)*begin_lambda + -8.79204e-05 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 3)*lens_ipow(begin_dy, 3)*begin_lambda + 29821.5 *lens_ipow(begin_dx, 8)*lens_ipow(begin_lambda, 2) + -7.67166e-08 *lens_ipow(begin_x, 5)*lens_ipow(begin_y, 2)*begin_dx*lens_ipow(begin_lambda, 2)+0.0f;
  dx1_domega0(0, 1) =  + 1.33686 *begin_y*begin_dx + 0.0322488 *begin_x*begin_y + 42.9393 *begin_dx*begin_dy + 1.55606 *begin_x*begin_dy + -2.33808e-07 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 3) + 0.000414493 *begin_x*lens_ipow(begin_y, 3)*lens_ipow(begin_dx, 2) + 0.000153908 *begin_x*lens_ipow(begin_y, 4)*lens_ipow(begin_dx, 2)*begin_dy*begin_lambda + -0.000263761 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 3)*begin_dx*lens_ipow(begin_dy, 2)*begin_lambda+0.0f;
  dx1_domega0(1, 0) =  + 1.64885 *begin_y*begin_dx + 1.29332 *begin_x*begin_dy + 0.0322068 *begin_x*begin_y + 42.3824 *begin_dx*begin_dy + 0.000247138 *lens_ipow(begin_x, 3)*begin_y*lens_ipow(begin_dy, 2) + -2.26643e-07 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 3) + -11.0703 *begin_y*lens_ipow(begin_dx, 5) + 3.08497e-06 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 4)*begin_dx*begin_dy*begin_lambda+0.0f;
  dx1_domega0(1, 1) =  + 46.4005  + 1.5057 *begin_lambda + 0.027811 *lens_ipow(begin_y, 2) + 2.61562 *begin_y*begin_dy + 1.29332 *begin_x*begin_dx + 21.1912 *lens_ipow(begin_dx, 2) + 0.0144468 *lens_ipow(begin_x, 2) + 32.4963 *lens_ipow(begin_dy, 2) + 0.00013376 *lens_ipow(begin_y, 4)*begin_lambda + 0.0338253 *lens_ipow(begin_y, 3)*begin_dy*begin_lambda + 53.3036 *begin_y*lens_ipow(begin_dy, 3)*begin_lambda + 2.53094 *lens_ipow(begin_y, 2)*lens_ipow(begin_dy, 2)*begin_lambda + 0.000494275 *lens_ipow(begin_x, 3)*begin_y*begin_dx*begin_dy + -1.23991e-07 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 2) + 129.726 *begin_y*lens_ipow(begin_dy, 5)*begin_lambda + 4537.89 *lens_ipow(begin_dy, 6)*begin_lambda + -3.88112e-07 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 4)*begin_lambda + 1.54248e-06 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 4)*lens_ipow(begin_dx, 2)*begin_lambda + -9.94703e-08 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 5)*begin_dy*lens_ipow(begin_lambda, 2)+0.0f;
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